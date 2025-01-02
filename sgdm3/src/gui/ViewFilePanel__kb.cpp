// ViewFilePanel__kb.cpp
// main keyboard/keybinding-related stuff.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fs.h>
#include <fl.h>
#include <de.h>
#include <fd.h>
#include <gui.h>

//////////////////////////////////////////////////////////////////

bool ViewFilePanel::_kb_alert(void) const
{
	::wxBell();
	return true;
}

bool ViewFilePanel::_kb_insert_text(const wxString & str)
{
//#ifdef DEBUG
//	//wxLogTrace(wxTRACE_Messages, _T("_kb_insert_text: [%s] before insert"), util_printable_s(str).wc_str());
//#endif

	_insertText(str,false);

//#ifdef DEBUG
//	//wxLogTrace(wxTRACE_Messages, _T("_kb_insert_text: [%s] after insert"), util_printable_s(str).wc_str());
//#endif

	_kb_ensureCaretVisible(true);
	return true;
}

//////////////////////////////////////////////////////////////////

bool ViewFilePanel::_kb_getEOLCol(int row, int * pColResult) const
{
	// compute the column number of the end of the ROW'th line.
	//
	// all column numbers assume tabs have been expanded.
	// (so we should return 24 for a line with 3 tabs when
	// the tab stops are 8).
	//
	// we return the value of the right-edge of the last
	// non-eol character on the line.  that is, the value
	// of the left-edge of the first eol char.
	//
	// return true if the line has an eol character.

	const fl_line * pFlLine = _mapRowToFlLine(row);
	if (!pFlLine)			// a void row
	{
		*pColResult = 0;
		return false;
	}

	int cColTabWidth = m_pViewFile->getTabStop();

	int col = 0;

	for (const fl_run * pFlRun=pFlLine->getFirstRunOnLine(); (pFlRun && (pFlRun->getLine()==pFlLine)); pFlRun=pFlRun->getNext())
	{
		if (pFlRun->isLF() || pFlRun->isCR())
		{
			*pColResult = col;
			return true;
		}

		col += ((pFlRun->isTAB()) ? (cColTabWidth - (col % cColTabWidth)) : (int)pFlRun->getLength());
	}

	*pColResult = col;
	return false;
}

int ViewFilePanel::_kb_getEOFRow(void) const
{
	// compute the row number of the EOF.

	int kSync = getSync();

	const TVector_Display * pDis = m_pDeDe->getDisplayList(kSync);
	int nrRows = (int)pDis->size() - 1;

	wxASSERT_MSG( ((*pDis)[nrRows].isEOF()), _T("Coding Error") );	// last row in display list is the EOF marker.

	return nrRows;
}

//////////////////////////////////////////////////////////////////

bool ViewFilePanel::_kb_getNextCol(int row, int colArg, int * pColResult)
{
	// compute the column number of the next character on the line
	// taking into account the current tab expansion.
	//
	// return true if there is a next character.
	// special case for EOF row, always return true and set col to zero.

	if (row == _kb_getEOFRow())
	{
		*pColResult = 0;
		return true;
	}
	
	const fl_line * pFlLine = _mapRowToFlLine(row);
	if (!pFlLine)
		return false;		// a void row

	int cColTabWidth = m_pViewFile->getTabStop();

	int col = 0;

	for (const fl_run * pFlRun=pFlLine->getFirstRunOnLine(); (pFlRun && (pFlRun->getLine()==pFlLine)); pFlRun=pFlRun->getNext())
	{
		if (pFlRun->isLF() || pFlRun->isCR())
			return false;

		if (pFlRun->isTAB())
		{
			int pad = cColTabWidth - (col % cColTabWidth);
			if (col+pad > colArg)
			{
				*pColResult = col+pad;
				return true;
			}

			col += pad;
		}
		else
		{
			int w = (int)pFlRun->getLength();
			if (col+w > colArg)
			{
				*pColResult = colArg+1;
				return true;
			}

			col += w;
		}
	}

	return false;
}

bool ViewFilePanel::_kb_getPrevCol(int row, int colArg, int * pColResult)
{
	// compute the column number of the previous character on the line
	// taking into account the current tab expansion.
	//
	// return true if there is a previous character.

	if (colArg == 0)
		return false;

	// special case for EOF row, always return true and set col to zero.

	if (row == _kb_getEOFRow())
	{
		*pColResult = 0;
		return true;
	}

	const fl_line * pFlLine = _mapRowToFlLine(row);
	if (!pFlLine)
		return false;		// a void row

	int cColTabWidth = m_pViewFile->getTabStop();

	int col = 0;

	for (const fl_run * pFlRun=pFlLine->getFirstRunOnLine(); (pFlRun && (pFlRun->getLine()==pFlLine)); pFlRun=pFlRun->getNext())
	{
		if (pFlRun->isLF() || pFlRun->isCR())
		{
			bool bHavePrev = (col > 0);
			if (bHavePrev)
				*pColResult = col;
			return bHavePrev;
		}

		if (pFlRun->isTAB())
		{
			int pad = cColTabWidth - (col % cColTabWidth);
			if (col+pad >= colArg)
			{
				*pColResult = col;
				return true;
			}

			col += pad;
		}
		else
		{
			int w = (int)pFlRun->getLength();
			if (col+w >= colArg)
			{
				*pColResult = colArg-1;
				return true;
			}

			col += w;
		}
	}

	return false;	// should not happen
}

bool ViewFilePanel::_kb_getThisCol(int row, int colArg, int * pColResult)
{
	// fetch the given column number on the line.  if the requested column
	// is inside a multi-column character (think tab-expansion or CRLF),
	// return the column number of the actual start of the character.
	// [we use this with the "goal" column when {next,prev}-{line,page,etc}
	// is pressed.]
	//
	// return true if there is a previous character (this fails for void
	// lines, for example).
	//
	// special case for EOF row, always return true and set col to zero.

	if (row == _kb_getEOFRow())
	{
		*pColResult = 0;
		return true;
	}

	const fl_line * pFlLine = _mapRowToFlLine(row);
	if (!pFlLine)
		return false;		// a void row

	int cColTabWidth = m_pViewFile->getTabStop();

	int col = 0;

	for (const fl_run * pFlRun=pFlLine->getFirstRunOnLine(); (pFlRun && (pFlRun->getLine()==pFlLine)); pFlRun=pFlRun->getNext())
	{
		if (pFlRun->isLF() || pFlRun->isCR())
		{
			*pColResult = col;	// if we haven't found the goal column by the time we
			return true;		// hit the CRLF, we return the left-edge of the CRLF.
		}

		if (pFlRun->isTAB())
		{
			int pad = cColTabWidth - (col % cColTabWidth);
			if (col+pad > colArg)		// if tab spans the goal column, we
			{							// return the left-edge of the TAB.
				*pColResult = col;
				return true;
			}

			col += pad;
		}
		else
		{
			int w = (int)pFlRun->getLength();
			if (col+w >= colArg)		// if a normal (1char/1column) run crosses
			{							// the goal column, we return the requested
				*pColResult = colArg;	// column.
				return true;
			}

			col += w;
		}
	}

	*pColResult = col;		// special case: we ran out of line data without seeing
	return true;			// a CR/LF, so we return the right-edge of the line.
}

//////////////////////////////////////////////////////////////////

static bool s_classifyChar(const wxChar chTest)
{
	if (chTest <  _T('0')) return false;
	if (chTest <= _T('9')) return true;			// is in [0-9]
	if (chTest <  _T('A')) return false;
	if (chTest <= _T('Z')) return true;			// is in [A-Z]
	if (chTest == _T('_')) return true;			// is _
	if (chTest <  _T('a')) return false;
	if (chTest <= _T('z')) return true;			// is in [a-z]
	if (chTest <   0x00c0) return false;
	return true;								// is in [A-Grave ...] unicode characters
}


bool ViewFilePanel::_kb_getNextWordBreakCol(int row, int colArg, int * pColResult)
{
	// compute the column number of the next word break on the line
	// taking into account the current tab expansion.
	//
	// return true if there is a next word break.
	//
	// if looking at a non-word-character (such as whitespace), skip forward to
	// first word-character before looking for the word break.
	//
	// TODO one could argue that we should define a regex (or something)
	// TODO for this to let each ruleset have its own word boundaries...

	// special case for EOF row, always return true and set col to zero.

	if (row == _kb_getEOFRow())
	{
		*pColResult = 0;
		return true;
	}

	const fl_line * pFlLine = _mapRowToFlLine(row);
	if (!pFlLine)
		return false;		// a void row

	int cColTabWidth = m_pViewFile->getTabStop();

	wxString strLine = pFlLine->buildTabExpandedStringFromRuns(false,cColTabWidth);
	int lenLine = (int)strLine.Length();

	if (colArg >= lenLine)
		return false;

	const wxChar * szLine = strLine.wc_str();

	bool bClass = s_classifyChar(szLine[colArg]);		// look at char to the right
	int  kChar  = colArg + 1;

	if (!bClass)
	{
		// if looking at non-word-character (such as whitespace)
		// scan forwards until we find a word-character before
		// looking for the end of the word.

		for (/*kChar*/; (kChar<lenLine); kChar++)
			if (s_classifyChar(szLine[kChar]))
				goto found_start_of_word;
		return false;

	found_start_of_word:
		;
	}

	// scan forwards to the end of the word or the end of the line.

	for (/*kChar*/; (kChar<lenLine); kChar++)
		if (!s_classifyChar(szLine[kChar]))
			break;

	*pColResult = kChar;
	return (kChar != colArg);
}

bool ViewFilePanel::_kb_getPrevWordBreakCol(int row, int colArg, int * pColResult)
{
	// compute the column number of the prev word break on the current line
	// taking into account the current tab expansion.  we do not wrap to
	// the previous line.
	//
	// return true if there is a prev word break.
	//
	// if looking at a non-word-character (such as whitespace), skip backward
	// to first word-character before looking for the word break.  if we don't
	// find one, just return false.
	// 
	// TODO one could argue that we should define a regex (or something)
	// TODO for this to let each ruleset have its own word boundaries...

	if (colArg == 0)
		return false;
	
	// special case for EOF row, always return true and set col to zero.

	if (row == _kb_getEOFRow())
	{
		*pColResult = 0;
		return true;
	}

	const fl_line * pFlLine = _mapRowToFlLine(row);
	if (!pFlLine)
		return false;		// a void row

	int cColTabWidth = m_pViewFile->getTabStop();

	wxString strLine = pFlLine->buildTabExpandedStringFromRuns(false,cColTabWidth);
	int lenLine = (int)strLine.Length();

	if (lenLine == 0)
		return false;

	const wxChar * szLine = strLine.wc_str();

	int  kLimit = MyMin(colArg,lenLine);
	int  kChar  = kLimit - 1;
	bool bClass = s_classifyChar(szLine[kChar]);	// look at char to the left of the column given
	
	if (!bClass)
	{
		// if looking at non-word-character (such as whitespace)
		// scan backwards until we find a word-character before
		// looking for the beginning of the word.
		
		for (/*kChar*/; (kChar>0); kChar--)
			if (s_classifyChar(szLine[kChar-1]))
				goto found_end_of_word;
		return false;

	found_end_of_word:
		;
	}

	// scan backwards to the beginning of the word or until we
	// reach the beginning of the line.

	for (/*kChar*/; (kChar>0); kChar--)
		if (!s_classifyChar(szLine[kChar-1]))
			break;
	
	*pColResult = kChar;
	return (kChar != colArg);
}

bool ViewFilePanel::_kb_getWordBoundsAtCol(int row, int colArg,
										   int * pRowResultStart, int * pColResultStart,
										   int * pRowResultEnd, int * pColResultEnd)
{
	// compute the word bounds of the "word" around the given (row,col)
	// taking into account the current tab expansion.  we do not wrap
	// to other lines (unless we have a blank line and need to represent
	// the EOL).
	//
	// if we are looking at characters that are part of word, return
	// the bounds of the word.  if we are looking at non-word chars
	// (such as whitespace), return the bounds of the non-word (glob
	// all the whitespace).

	*pRowResultStart = row;
	*pRowResultEnd = row;
	*pColResultStart = colArg;
	*pColResultEnd = colArg;

	// special case for EOF row, we have no word (and we can't just
	// say select the entire line, because we don't have one).

	if (row == _kb_getEOFRow())
		return false;

	const fl_line * pFlLine = _mapRowToFlLine(row);
	if (!pFlLine)
		return false;		// a void row

	int cColTabWidth = m_pViewFile->getTabStop();

	wxString strLine = pFlLine->buildTabExpandedStringFromRuns(false,cColTabWidth);
	int lenLine = (int)strLine.Length();

	if (lenLine == 0)		// we have an empty line, return the EOL (ie the entire line).
	{
		*pRowResultStart = row;
		*pRowResultEnd = row+1;
		*pColResultStart = 0;
		*pColResultEnd = 0;
		return true;
	}

	if (colArg >= lenLine)	// past the last non-EOL char on the line, return the EOL.
	{
		*pRowResultStart = row;
		*pRowResultEnd = row+1;
		*pColResultStart = lenLine;
		*pColResultEnd = 0;
		return true;
	}
	
	const wxChar * szLine = strLine.wc_str();
	bool bClass = s_classifyChar(szLine[colArg]);	// look at char to the right of caret

	int colStart = colArg;		// first char in result
	int colEnd = colArg+1;		// just past end of result

	for (int c=colArg-1; c>=0; c--)
		if (s_classifyChar(szLine[c]) == bClass)
			colStart = c;
		else
			break;
	
	for (int c=colArg+1; c<lenLine; c++)
		if (s_classifyChar(szLine[c]) == bClass)
			colEnd = c+1;
		else
			break;
	
	*pRowResultStart = row;
	*pRowResultEnd = row;
	*pColResultStart = colStart;
	*pColResultEnd = colEnd;
	return true;
}

//////////////////////////////////////////////////////////////////

bool ViewFilePanel::_kb_getNextRow(int row, int * pRowResult)
{
	// compute the row number of the next valid row (skipping voids)
	//
	// return true if there is a next row.

	int kSync = getSync();

	const TVector_Display * pDis = m_pDeDe->getDisplayList(kSync);
	int nrRows = (int)pDis->size() - 1;

	wxASSERT_MSG( ((*pDis)[nrRows].isEOF()), _T("Coding Error") );	// last row in display list is the EOF marker.

	if (row == nrRows)		// already looking at EOF row, so no next row
		return false;
	
	for (int kRow=row+1; (kRow<nrRows); kRow++)
	{
		const de_row & rDeRow = (*pDis)[kRow];
		const de_line * pDeLine = rDeRow.getPanelLine(m_kPanel);
		if (pDeLine && pDeLine->getFlLine())
		{
			*pRowResult = kRow;
			return true;
		}
	}

	// we've hit the special EOF row -- but should we return it?
	
	fim_ptable * pPTable = m_pViewFile->getPTable(m_kSync,m_kPanel);
	if (!pPTable->hasFinalEOL())	// if no final EOL, EOF ends at the right edge of last valid line.
		return false;
	
	// if there is an EOL, we want the EOF pseudo-line

	*pRowResult = nrRows;
	return true;
}

bool ViewFilePanel::_kb_getPrevRow(int row, int * pRowResult)
{
	// compute the row number of the prev valid row (skipping voids)
	//
	// return true if there is a prev row.

	int kSync = getSync();

	const TVector_Display * pDis = m_pDeDe->getDisplayList(kSync);
	int nrRows = (int)pDis->size() - 1;

	wxASSERT_MSG( ((*pDis)[nrRows].isEOF()), _T("Coding Error") );	// last row in display list is the EOF marker.

	if (row > nrRows)
		row = nrRows;

	for (int kRow=row-1; (kRow>=0); kRow--)
	{
		const de_row & rDeRow = (*pDis)[kRow];
		const de_line * pDeLine = rDeRow.getPanelLine(m_kPanel);
		if (pDeLine && pDeLine->getFlLine())
		{
			*pRowResult = kRow;
			return true;
		}
	}

	return false;
}

//////////////////////////////////////////////////////////////////

bool ViewFilePanel::_kb_getPrev(bool bByWord, const ViewFileCoord * pVFC, ViewFileCoord * pVFCprev, fim_offset * pDocPos)
{
	// compute (row,col) of previous {word,char} from given coords.
	// return false if no previous position exists.

	int col = pVFC->getCol();
	int row = pVFC->getRow();

	if (bByWord)
	{
		while (!_kb_getPrevWordBreakCol(row,col, &col))
		{
			if (!_kb_getPrevRow(row, &row))			// no previous row (at row 0 or only voids)
			{
				if (col > 0)
				{
					col = 0;			// special case when going backwards by word and we reach the
					break;				// beginning of the document, count the leading whitespace as a word.
				}
				
				return false;
			}
			
			col = INT_MAX;
		}
	}
	else
	{
		while (!_kb_getPrevCol(row,col, &col))
		{
			if (!_kb_getPrevRow(row, &row))
				return false;	// no previous row (at row 0 or only voids)

			if (_kb_getEOLCol(row, &col))	// if there is an EOL char on the line, we're done.
				break;						// otherwise, loop again, so that we back up one actual character.
		}
	}

	pVFCprev->set(row,col);

	if (pDocPos)
		return _mapCoordToDocPosition(pVFCprev,pDocPos);

	return true;
}

bool ViewFilePanel::_kb_getNext(bool bByWord, const ViewFileCoord * pVFC, ViewFileCoord * pVFCnext, fim_offset * pDocPos)
{
	// compute (row,col) of next {word,char} from given coords.
	// return false if no next position exists.

	int col = pVFC->getCol();
	int row = pVFC->getRow();

	if (bByWord)				// foreward-word
	{
		while (!_kb_getNextWordBreakCol(row,col, &col))
		{
			if (!_kb_getNextRow(row, &row))
				return false;	// no next row (at row eof or only voids)

			col = 0;
		}
	}
	else						// foreward-character
	{
		if (!_kb_getNextCol(row,col, &col))
		{
			if (!_kb_getNextRow(row, &row))
				return false;	// no next row (at row eof or only voids)

			col = 0;
		}
	}

	pVFCnext->set(row,col);

	if (pDocPos)
		return _mapCoordToDocPosition(pVFCnext,pDocPos);

	return true;
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::_kb_ensureCaretVisible(bool bForceRefresh)
{
	if (!m_vfcCaret.isSet())
		return;					// sanity check incase caret is bogus

	_kb_ensureVisible(m_vfcCaret.getRow(),m_vfcCaret.getCol(),bForceRefresh);
}

void ViewFilePanel::_kb_ensureVisible(int row, int col, bool bForceRefresh)
{
	// warp scroll to ensure that the given (row,col) is visible.
	// try to make the smallest movement possible.

	bool bScrolled = false;

	// vertical is easy.

	int yRowFirstVisible = m_pViewFile->getScrollThumbCharPosition(m_kSync,wxVERTICAL);
	int yRowsVisible     = getRowsDisplayable() - 1;
	int yRowLastVisible  = yRowFirstVisible + yRowsVisible;

	if (row < yRowFirstVisible)
	{
		m_pViewFile->adjustVerticalScrollbar(m_kSync,row);
		bScrolled = true;
	}
	else if (row >= yRowLastVisible)
	{
		m_pViewFile->adjustVerticalScrollbar(m_kSync,(row - yRowsVisible));
		bScrolled = true;
	}

	// horizontally is a little harder.

	int hThumbBest = _kb_ensureVisible_computeBestHThumb(row,col);
	int hThumbCurrent = m_pViewFile->getScrollThumbCharPosition(m_kSync,wxHORIZONTAL);
	if (hThumbBest != hThumbCurrent)
	{
		m_pViewFile->adjustHorizontalScrollbar(m_kSync,hThumbBest);
		bScrolled = true;
	}

	if (bForceRefresh && !bScrolled)
	{
		Refresh(true);
		Update();
	}
}

//////////////////////////////////////////////////////////////////

int ViewFilePanel::_kb_ensureVisible_computeBestHThumb(int row, int col)
{
	// compute the best value for the horizontal thumb to ensure
	// that the absolute column given on the given row is minimally
	// visible.  that is, try to compute the minimium amount of
	// hscroll needed to get the column on screen.
	//
	// NOTE: thumb positions are calibrated in fixed-pitch font units.
	// NOTE: so we need to map the given column into pixels (by
	// NOTE: measuring the data on the line) and then map that into
	// NOTE: a fixed-pitch unit.

	int kSync = getSync();

	const TVector_Display * pDis = m_pDeDe->getDisplayList(kSync);

	// if we are at the EOF, horizontal scroll is a little arbitrary, so
	// warp to column 0 (as we do for blank or void lines).

	int yRowEOF = _kb_getEOFRow();
	if (row >= yRowEOF)
		return 0;

	// NOTE horizontal thumb is calibrated assuming fixed-pitch font.  that is, each
	// NOTE single-step moves us one "average character" width (i.e. an 'x').  but
	// NOTE we cannot rely on that when we want to ensure that a specific actual
	// NOTE column is visible.
	//
	// so we need to compute the actual pixel coordinates (of left and right edges
	// of the requrested column (and taking tabs into account)) and then convert that
	// back into "average characters".  we also need to take into account the scroll
	// position.

	const de_row & rDeRow = (*pDis)[row];
	const de_line * pDeLine = rDeRow.getPanelLine(m_kPanel);

	if (!pDeLine)		// a void row
		return 0;

	int cColTabWidth = m_pViewFile->getTabStop();
	bool bDisplayInvisibles = m_pViewFile->getPilcrow();

	// create a temporary/scratch bitmap/dc so that we measure strings.

	wxBitmap bm(1,1);
	wxMemoryDC dcMem;
	dcMem.SelectObject(bm);
			
	wxCoord x0 = 0;
	wxCoord x1 = x0;
	int col0 = 0;
	int col1 = col0;
	bool bStop = false;

	const fl_line * pFlLine = pDeLine->getFlLine();
	for (const fl_run * pFlRun=pFlLine->getFirstRunOnLine(); (!bStop && pFlRun && (pFlRun->getLine()==pFlLine)); pFlRun=pFlRun->getNext())
	{
		const wxChar * sz = pFlRun->getContent();
		long len = (long)pFlRun->getLength();
		if (len == 0)		// we could probably just assert len > 0
			continue;

		fr_prop frProp = pFlRun->getFragProp();
		if (FR_PROP_TEST(frProp,FR_PROP__INSERTED))
			dcMem.SetFont(*gpViewFileFont->getBoldFont());
		else
			dcMem.SetFont(*gpViewFileFont->getNormalFont());
			
		if (pFlRun->isLF() || pFlRun->isCR())
		{
			// since they probably don't want to see just the CR in a CRLF sequence, we
			// play some tricks here so that we compute the bounds of both chars.

			if (bDisplayInvisibles)
				_fakeDrawString(dcMem, sz,len, bDisplayInvisibles,cColTabWidth, x1,col1);
		}
		else if (pFlRun->isTAB())
		{
			_fakeDrawString(dcMem, sz,len, bDisplayInvisibles,cColTabWidth, x1,col1);
			if (col1 >= col)
				bStop = true;
			else
			{
				x0 = x1;
				col0 = col1;
			}
		}
		else // a normal run
		{
			if (col0+len <= col)		// use whole run and go on
			{
				// compute the left-edge of the desired actual column

				_fakeDrawString(dcMem, sz,len, bDisplayInvisibles,cColTabWidth, x1,col1);
				x0 = x1;
				col0 = col1;
			}
			else						// use partial run and stop
			{
				wxASSERT_MSG( (len > col-col0), _T("Coding Error") );

				long w = col - col0;
				if (w > 0)
				{
					// compute the left-edge of the desired actual column within this run

					_fakeDrawString(dcMem, sz,w, bDisplayInvisibles,cColTabWidth, x1,col1);
					x0 = x1;
					col0 = col1;
					sz += w;
				}
						
				// compute the right-edge of the desired actual column

				_fakeDrawString(dcMem, sz,1, bDisplayInvisibles,cColTabWidth, x1,col1);

				bStop = true;
			}
		}
	}

	// at this point:
	// [] x0 is the absolute pixel coordinate of the left-edge of the col'th character.
	// [] x1 is the absolute pixel coordinate of the right-edge.
	// not counting scroll position and not counting line-number area.

	// now see where we are currently scrolled and what is currently visible.
			
	int xPixelsClientSize, yPixelsClientSize;
	GetClientSize(&xPixelsClientSize,&yPixelsClientSize);

	xPixelsClientSize -= getXPixelText();

	int xColFirstVisible = m_pViewFile->getScrollThumbCharPosition(m_kSync,wxHORIZONTAL);
	int xPixelFirstVisible = xColFirstVisible * getPixelsPerCol();

	if (x0 <= xPixelFirstVisible)
	{
		// map the pixel value of the left-edge of this actual column into
		// an "average column" column number.  round down to get the left
		// edge of it.  then scroll as best we can to get this on the left-
		// edge of the window.

		int pseudoCol = x0 / getPixelsPerCol();
		return pseudoCol;
	}

	if (x1 >= (xPixelFirstVisible+xPixelsClientSize))
	{
		// map the pixel value of the right-edge of this actual column
		// into an "average column" column number.  round up so that we
		// are sure to see it.  then scroll as best we can to get this
		// on the right-edge of the window.

		int pseudoCol = (x1 + getPixelsPerCol() - 1) / getPixelsPerCol();
		return (pseudoCol - getColsDisplayable());
	}

	// no horizontal scroll needed

	return xColFirstVisible;
}

