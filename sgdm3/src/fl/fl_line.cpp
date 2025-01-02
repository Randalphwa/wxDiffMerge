// fl_line.cpp -- a "line" -- describes a displayable line of text, a sequence of runs.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <poi.h>
#include <fim.h>
#include <fl.h>

//////////////////////////////////////////////////////////////////

fl_line::fl_line(fl_fl * pFlFl, fl_run * pRunFirst)
	: m_next(NULL), m_prev(NULL),
	  m_pFlFl(pFlFl),
	  m_pRunFirst(pRunFirst),
	  m_invalidate(0),
	  m_lineNr(0),
	  m_editOpCounter(0)
{
	DEBUG_CTOR(T_FL_LINE,L"fl_line");
}

fl_line::~fl_line(void)
{
#if 0
#ifdef DEBUG
	// make sure all slots have been released
	for (TVec_SlotValueIterator it=m_vecSlotValue.begin(); (it < m_vecSlotValue.end()); it++)
	{
		de_line * pDeLine = (*it);
		wxASSERT_MSG( (pDeLine==NULL), _T("Coding Error: slot not released before fl_line deleted.") );
	}
#endif
#endif

	DEBUG_DTOR(T_FL_LINE);
}

//////////////////////////////////////////////////////////////////

de_line * fl_line::getSlotValue(fl_slot slot) const
{
	if (slot >= (int)m_vecSlotValue.size())
		return NULL;
	else
		return m_vecSlotValue[slot];
}

void fl_line::setSlotValue(fl_slot slot, de_line * pDeLine)
{
	while (slot >= (int)m_vecSlotValue.size())
		m_vecSlotValue.push_back(NULL);

	m_vecSlotValue[slot] = pDeLine;
}

void fl_line::buildStringsFromRuns(bool bIncludeEOL, wxString * pstrLine, wxString * pstrEOL) const
{
	// return a string containing the content on this line.
	// if (bIncludeEOL), the EOL chars on this line will be included in the returned string.
	// if (!bIncludeEOL), the EOL chars will be put into the argument string.

	pstrLine->Empty();
	if (pstrEOL)
		pstrEOL->Empty();

	for (const fl_run * pRun=m_pRunFirst; pRun && (pRun->getLine()==this); pRun=pRun->getNext())
	{
		wxString strTemp(pRun->getContent(),pRun->getLength());
		
		if ( (pRun->isLF() || pRun->isCR())  &&  (!bIncludeEOL) )
		{
			if (pstrEOL)
				*pstrEOL += strTemp;
		}
		else
		{
			*pstrLine += strTemp;
		}
	}
}

wxString fl_line::buildTabExpandedStringFromRuns(bool bIncludeEOL, int cColTabWidth) const
{
	// return a string containing the content on this line (including EOL chars if directed)
	// with tabs expanded using the given tabstop.

	wxString s;
	int col = 0;
	
	for (const fl_run * pRun=m_pRunFirst; pRun && (pRun->getLine()==this); pRun=pRun->getNext())
	{
		wxString strTemp = wxString(pRun->getContent(),pRun->getLength());
		
		if (pRun->isLF() || pRun->isCR())
		{
			if (bIncludeEOL)
				s += strTemp;
			col++;
		}
		else if (pRun->isTAB())
		{
			int pad = cColTabWidth - (col % cColTabWidth);
			s.Append(_T(' '),pad);
			col += pad;
		}
		else
		{
			s += strTemp;
			col += (int)pRun->getLength();
		}
	}

	return s;
}

//////////////////////////////////////////////////////////////////

void fl_line::getFragAndOffsetOfColumn(int colWanted, int cColTabWidth,
									   const fim_frag ** ppFrag, fr_offset * pOffset) const
{
	// compute the (frag,offset) from the given column
	// on this line -- assuming the given tab-stop.

	int col = 0;

	for (const fl_run * pRun=m_pRunFirst; pRun && (pRun->getLine()==this); pRun=pRun->getNext())
	{
		if (col == colWanted)					// return (frag,offset) of the "left-edge" of this run
		{
			*ppFrag = pRun->getFrag();
			*pOffset = pRun->getFragOffset();
			return;
		}

		if (pRun->isTAB())
		{
			int pad = cColTabWidth - (col % cColTabWidth);
			if (colWanted < col+pad)
			{
				// they want a position within the tab. this should not happen.
				// but it is OK if it does.

				wxASSERT_MSG( (0), _T("Coding Error") );
				*ppFrag = pRun->getFrag();
				*pOffset = pRun->getFragOffset();
				return;
			}
			if ((colWanted == col+pad) && !pRun->getNext())
			{
				// SCREW-CASE: the document ends with a TAB and no final EOL char.
				// return the (frag,offset) of the "right-edge" of this run.  note
				// this does not include the pad, rather we add 1, because the offset
				// is a document offset not a display column.

				*ppFrag = pRun->getFrag();
				*pOffset = pRun->getFragOffset() + 1;
				return;
			}
			
			col += pad;
		}
		else
		{
			int len = (int)pRun->getLength();
			if (colWanted <= col+len)
			{
				*ppFrag = pRun->getFrag();
				*pOffset = pRun->getFragOffset() + (colWanted - col);
				return;
			}

			col += len;
		}
	}

	// we ran off the end of the line before we got to the desired column.
	// this should not happen.

	*ppFrag = NULL;
	*pOffset = 0;
	wxASSERT_MSG( (0), _T("Coding Error") );
}

int fl_line::getColumnOfFragAndOffset(int cColTabWidth, const fim_frag * pFrag, fr_offset offset) const
{
	// compute the column on this line from the (frag,offset)
	// and assuming the given tab-stop.

	int col = 0;

	for (const fl_run * pRun=m_pRunFirst; pRun && (pRun->getLine()==this); pRun=pRun->getNext())
	{
		int pad = ((pRun->isTAB()) ? (cColTabWidth - (col % cColTabWidth)) : (int)pRun->getLength());
		
		if ((pRun->getFrag() == pFrag) && (offset <= pRun->getFragEndOffset()))
		{
			if (pRun->getFragOffset() == offset)	// exactly on the left edge of the run
				return col;
			if (offset == pRun->getFragEndOffset())	// exactly on the right edge of the run
				return col+pad;
			return (int)(col + (offset - pRun->getFragOffset()));	// a non-tab, with length>1
		}

		col += pad;
	}

	wxASSERT_MSG( (0), _T("Coding Error") );
	return 0;
}

//////////////////////////////////////////////////////////////////

#ifdef DEBUG
void fl_line::dump(int indent) const
{
	wxLogTrace(TRACE_FLLINE_DUMP, _T("%*cFL_LINE: [%p][inv %x][lineNr %d][editOpCounter %ld]"),
			   indent,' ',this,
			   m_invalidate,
			   m_lineNr,
			   m_editOpCounter);

	for (fl_run * pRun=m_pRunFirst; (pRun && pRun->getLine()==this); pRun=pRun->getNext())
		pRun->dump(indent+5);
}
#endif
