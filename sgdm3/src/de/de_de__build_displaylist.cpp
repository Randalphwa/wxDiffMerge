// de_de__build_displaylist.cpp -- diff engine -- routines related
// to building the display list after the diff has been computed.
// this is dependent upon the display mode (all, diffs, ctx, etc)
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <poi.h>
#include <fim.h>
#include <fs.h>
#include <fl.h>
#include <rs.h>
#include <de.h>

//////////////////////////////////////////////////////////////////

#define CLASSIFY_TYPE__MARK		(-3)
#define CLASSIFY_TYPE__EOF		(-2)
#define CLASSIFY_TYPE__OMIT		(-1)
#define CLASSIFY_TYPE__DIFF		(0)
#define CLASSIFY_TYPE__EQ		(1)		// EQ or Pseudo-EQ

#define CLASSIFY_DISPLAY__EOF	(-2)
#define CLASSIFY_DISPLAY__HIDE	(0)
#define CLASSIFY_DISPLAY__SHOW	(1)
#define CLASSIFY_DISPLAY__MAYBE	(2)


//////////////////////////////////////////////////////////////////

TVector_Display * de_de::getDisplayList(long kSync)
{
	if (m_bNeedRun)
		run();

	wxASSERT_MSG( (m_bDisplayValid[kSync]), _T("Coding Error") );

	return &m_vecDisplay[kSync];
}

//////////////////////////////////////////////////////////////////

void de_de::_build_display(long kSync)
{
	//UTIL_PERF_START_CLOCK(_T("de_de::_build_display"));

	// use all of the data strutures that we have built and the
	// display ops to compute the vertical positioning and some
	// of the attributes of all lines.

	de_display_ops dops = m_dops[kSync];
	
	m_bDisplayValid[kSync] = false;

	size_t nrSize = m_vecDisplay[kSync].size();
	m_vecDisplay[kSync].clear();				// erase all rows in this display list.
	if (nrSize > 0)								// but if we had a previous size, use it to
		m_vecDisplay[kSync].reserve(nrSize);	// pre-allocate and avoid lots of realloc's.

	// start at the beginning of each document (the first sync node) and start
	// packing rows.  we have to do this across the documents rather than
	// processing the documents sequentially, since we have voids to deal with.

	long ctxGoal = gpGlobalProps->getLong(GlobalProps::GPL_FILE_CONTEXT_GOAL);	// nr lines of context desired
	long ctx = 0;
	bool bGap = false;		// used to mark de_rows that follow a break [like when we're only showing diffs or diff-with-context]
	
	for (const de_sync * pDeSync = m_sync_list[kSync].getHead(); (pDeSync); pDeSync=pDeSync->getNext())
	{
		switch (_classify_for_display(pDeSync,dops))
		{
		default:
		case CLASSIFY_DISPLAY__EOF:				// EOF -- add special EOF row
			_emit_eof(kSync,pDeSync,bGap);
			break;
		case CLASSIFY_DISPLAY__HIDE:			// hidden -- do nothing
			bGap = true;
			break;
		case CLASSIFY_DISPLAY__SHOW:			// show all of this node
			_add_node_to_display(kSync,pDeSync,bGap);
			ctx = ctxGoal;
			break;
		case CLASSIFY_DISPLAY__MAYBE:			// scan for context
			_maybe_add_context_to_display(kSync,pDeSync,dops,ctxGoal,ctx,bGap);
			break;
		}
	}

	m_bDisplayValid[kSync] = true;
#if 0
#ifdef DEBUG
	dump_row_vector(10,kSync,dops);
#endif
#endif

	_fixup_next_prev_change_links(kSync);

	//UTIL_PERF_STOP_CLOCK(_T("de_de::_build_display"));
}

void de_de::_emit_omitted(long kSync, const de_sync * pDeSync, long kRowBegin, long kRowEnd, bool & bGap)
{
	// "omitted" sync nodes contain the pointers to the first de_line
	// and a length for each panel.  we have to walk the fl_line list
	// to populate our rows.
	//
	// only emit the requested range of lines.

	const de_sync_line_omitted * pDeSyncLineOmitted = static_cast<const de_sync_line_omitted *>(pDeSync);
	long kPanel;
	long lenMaxAvailable = pDeSync->getMaxLen();
	long lenMax = MyMin(kRowEnd, lenMaxAvailable);
	
	de_line * pDeLines[__NR_TOP_PANELS__];
	for (kPanel=0; (kPanel < __NR_TOP_PANELS__); kPanel++)
		pDeLines[kPanel] = pDeSyncLineOmitted->getFirstLine((PanelIndex)kPanel);

	if (kRowBegin > 0)
	{
		// we are asked to write the last n rows of our sync node
		// (probably) as part of the pre-context of a following change.
		// TODO 2013/09/04 When the left/right sides of this omitted
		// TODO            node has different lengths, should we
		// TODO            bottom-align this omitted chunk so that all
		// TODO            sides are populated in the pre-context?
		// TODO            Or should we just assume top-alignment and
		// TODO            only populate the columns that have the
		// TODO            final rows?
		// TODO
		// TODO            For now, assume top-alignment.
		
		for (long kRow=0; kRow<kRowBegin; kRow++)
			for (kPanel=0; (kPanel < __NR_TOP_PANELS__); kPanel++)
				if (kRow < pDeSync->getLen((PanelIndex)kPanel))
				{
					fl_line * pFlLineNext = pDeLines[kPanel]->getFlLine()->getNext();
					pDeLines[kPanel] = ((pFlLineNext) ? pFlLineNext->getSlotValue( *_lookupSlot(kSync,(PanelIndex)kPanel) ) : NULL);
				}
	}

	for (long kRow=kRowBegin; kRow<lenMax; kRow++)
	{
		m_vecDisplay[kSync].push_back( de_row(pDeSync,kRow,bGap) );
		de_row & rDeRow = m_vecDisplay[kSync].back();
		size_t    rowNr = m_vecDisplay[kSync].size() - 1;

		bGap = false;

		for (kPanel=0; (kPanel < __NR_TOP_PANELS__); kPanel++)
			if (kRow < pDeSync->getLen((PanelIndex)kPanel))
			{
				rDeRow.setPanel((PanelIndex)kPanel,pDeLines[kPanel]);
				pDeLines[kPanel]->setRowNr(kSync,rowNr);

				fl_line * pFlLineNext = pDeLines[kPanel]->getFlLine()->getNext();

				pDeLines[kPanel] = ((pFlLineNext) ? pFlLineNext->getSlotValue( *_lookupSlot(kSync,(PanelIndex)kPanel) ) : NULL);
			}
	}
}
	
void de_de::_add_node_to_display(long kSync, const de_sync * pDeSync, bool & bGap)
{
	// create display rows for all lines in this sync node.
	
	if (pDeSync->isOmitted())		// we have an "omitted" sync node.
	{
		long rowBegin = 0;
		long rowEnd   = pDeSync->getMaxLen();
		_emit_omitted(kSync, pDeSync, rowBegin, rowEnd, bGap);
	}
	else if (pDeSync->isMARK())		// we have a MARK sync node
	{
		// add a "mark" row

		_emit_mark(kSync,pDeSync,bGap);
	}
	else							// a regular sync node containing normal content
	{
		long rowBegin = 0;
		long rowEnd   = pDeSync->getMaxLen();
		_emit_normal(kSync, pDeSync, rowBegin, rowEnd, bGap);
	}
}

void de_de::_emit_normal(long kSync, const de_sync * pDeSync, long kRowBegin, long kRowEnd, bool & bGap)
{
	// normal sync nodes contain the ndx [into m_vecLineCmp] of the first line and a
	// length for each panel.  we get the lines for each row using "vec[k][ndx+row]".

	for (long kRow=kRowBegin; kRow<kRowEnd; kRow++)
	{
		m_vecDisplay[kSync].push_back( de_row(pDeSync,kRow,bGap) );
		de_row & rDeRow = m_vecDisplay[kSync].back();
		size_t    rowNr = m_vecDisplay[kSync].size() - 1;

		bGap = false;
			
		for (long kPanel=0; (kPanel < __NR_TOP_PANELS__); kPanel++)
			if (kRow < pDeSync->getLen((PanelIndex)kPanel))
			{
				de_line * pDeLine = (*_lookupVecLineCmp(kSync,(PanelIndex)kPanel))[pDeSync->getNdx((PanelIndex)kPanel)+kRow];
				rDeRow.setPanel((PanelIndex)kPanel,pDeLine);
				pDeLine->setRowNr(kSync,rowNr);
			}
	}
}

void de_de::_emit_eof(long kSync, const de_sync * pDeSync, bool & bGap)
{
	// add "eof" row

	m_vecDisplay[kSync].push_back( de_row(pDeSync,bGap) );

	bGap = false;
}

void de_de::_emit_mark(long kSync, const de_sync * pDeSync, bool & bGap)
{
	// add "mark" row

	m_vecDisplay[kSync].push_back( de_row(pDeSync,pDeSync->getMark(),bGap) );

	// since we're not actually representing a line of content,
	// we probably shouldn't turn off the gap.
	//bGap = false;
}
	
void de_de::_maybe_add_context_to_display(long kSync, const de_sync * pDeSync, de_display_ops dops, long lenGoal, long & lenHead, bool & bGap)
{
	wxASSERT_MSG( (DE_DOP__IS_MODE_CTX(dops)), _T("Coding Error") );

	// conditionally create display rows for part or all of this sync node.
	// the user wants n lines of context around each change.  we are called
	// with an EQUAL (or an unimportant change that we treat as EQUAL) sync
	// node.
	//
	// update 2013/09/04 we can also now be called with an OMITTED item. W8612.
	//
	// if the previous sync node was treated as a change, then we need to
	// emit the first lenHead lines -- for the after-context.
	//
	// if the following sync node will be treated as a change, then we need
	// to emit the last lenGoal lines -- for the before-context.
	//
	// in either case, if we are less than n lines, we need to request that
	// the next node continue our work.

	long lenEmit, lenTail, lenUntilChange;
	long lenMax   = pDeSync->getMaxLen();		// our total height
	long rowEnd   = lenMax;

	if (lenHead)				// we need to emit some after-context
	{
		lenEmit = MyMin(lenHead,lenMax);
		if (pDeSync->isOmitted())
			_emit_omitted(kSync,pDeSync,0,lenEmit,bGap);
		else
			_emit_normal(kSync,pDeSync,0,lenEmit,bGap);
		lenHead -= lenEmit;

		if (lenHead > 0)		// we're too short, let next node contribute (if possible)
			return;

		lenMax -= lenEmit;
		if (lenMax == 0)		// we have nothing left
			return;
	}

	// look forward in sync list and see if we need to start some before-context.

	lenUntilChange = 0;
	const de_sync * pDeSyncNext = pDeSync->getNext();
	long syncType               = _classify_type(pDeSyncNext,dops);
	while ((syncType == CLASSIFY_TYPE__EQ) || (syncType == CLASSIFY_TYPE__MARK) || (syncType == CLASSIFY_TYPE__OMIT))
	{
		lenUntilChange += pDeSyncNext->getMaxLen();

		pDeSyncNext    = pDeSyncNext->getNext();
		syncType       = _classify_type(pDeSyncNext,dops);
	}

	if (syncType == CLASSIFY_TYPE__EOF)	// ran into EOF, so there's no change following us.
	{								// and we don't need to output any before-context.
		bGap = true;
		return;
	}

//	if (syncType == CLASSIFY_TYPE__OMIT)	// ran into an omitted line, so there's a gap following us
//	{								// and we're not contiguous with the next change/equal region.
//		bGap = true;				// -- therefore, we don't need to output any before-context.
//		return;
//	}
		
	if (lenUntilChange >= lenGoal)	// there's enough in the next nodes that we don't need to contribute
	{
		bGap = true;
		return;
	}
	
	// otherwise, we need part of our tail

	lenTail = lenGoal - lenUntilChange;
	lenEmit = MyMin(lenTail,lenMax);
	bGap |= (lenMax > lenTail);
	if (pDeSync->isOmitted())
		_emit_omitted(kSync,pDeSync,rowEnd-lenEmit,rowEnd,bGap);
	else
		_emit_normal(kSync,pDeSync,rowEnd-lenEmit,rowEnd,bGap);

	lenHead = lenGoal - lenEmit;	// force next node to continue emitting context
	return;
}

//////////////////////////////////////////////////////////////////

long de_de::_classify_type(const de_sync * pDeSync, de_display_ops dops)
{
	if (pDeSync->isMARK())
		return CLASSIFY_TYPE__MARK;

	if (pDeSync->isEND())
		return CLASSIFY_TYPE__EOF;

	if (pDeSync->isOmitted())
		return CLASSIFY_TYPE__OMIT;

	if (   pDeSync->isSameType(DE_ATTR_DIF_2EQ)					// 2-way-equal on diff window
		|| pDeSync->isSameType(DE_ATTR_MRG_3EQ))				// 3-way-equal on merge window
		return CLASSIFY_TYPE__EQ;

	if (!pDeSync->isUnimportant())								// if an important change
		return CLASSIFY_TYPE__DIFF;

	if (!DE_DOP__IS_SET(dops,DE_DOP_IGN_UNIMPORTANT))			// if we are not hiding unimportant changes
		return CLASSIFY_TYPE__DIFF;
	
	// we have an unimportant change and we are trying to
	// hide (not highlight) them.
	//
	// when in _ALL_ display mode, we want them to be treated
	// as EQUAL lines (and not highlighted).  we also do not
	// want them to be displayed in _DIFF_ONLY_ mode.  and we
	// do not want them to trigger a context in _DIFF_CTX_ mode.
	// in all 3 cases, we want to treat them as EQUAL.
	//
	// BUT, when in _EQUAL_ONLY_ mode, we do not want to see
	// them.

	if (DE_DOP__IS_MODE_EQL(dops))
		return CLASSIFY_TYPE__DIFF;
	return CLASSIFY_TYPE__EQ;
}

long de_de::_classify_for_display(const de_sync * pDeSync, de_display_ops dops)
{
	long sync_type = _classify_type(pDeSync,dops);

	if (sync_type == CLASSIFY_TYPE__MARK)
	{
		// only show marks in _ALL mode.
		if (DE_DOP__IS_MODE(dops,DE_DOP_ALL))
			return CLASSIFY_DISPLAY__SHOW;
		else
			return CLASSIFY_DISPLAY__HIDE;
	}

	if (sync_type == CLASSIFY_TYPE__EOF)
		return CLASSIFY_DISPLAY__EOF;

	if (sync_type == CLASSIFY_TYPE__OMIT)
	{
		switch (dops & DE_DOP__MODE_MASK)
		{
		case DE_DOP_ALL:
			// we only show "omitted" lines when in _ALL mode when requested.
			if (DE_DOP__IS_SET(dops,DE_DOP_HIDE_OMITTED))
				return CLASSIFY_DISPLAY__HIDE;
			else
				return CLASSIFY_DISPLAY__SHOW;

		case DE_DOP_DIF:
			return CLASSIFY_DISPLAY__HIDE;

		case DE_DOP_CTX:
			// hiding omitted lines does not mesh well with
			// a unified context diff because it throws off
			// the @@ headers when the omitted line is adjacent
			// to an actual change.  it can also make the gui
			// display a little weird.  mark these as maybe
			// so that we can include them when adjacent to
			// a change. W8612.
			return CLASSIFY_DISPLAY__MAYBE;

		case DE_DOP_EQL:
			return CLASSIFY_DISPLAY__HIDE;

		default:
			wxASSERT_MSG( (0), _T("Coding Error"));
			return CLASSIFY_DISPLAY__HIDE;
		}
	}

	wxASSERT_MSG( (sync_type==CLASSIFY_TYPE__DIFF || sync_type==CLASSIFY_TYPE__EQ), _T("Coding Error"));

	switch (dops & DE_DOP__MODE_MASK)
	{
	case DE_DOP_ALL:	return CLASSIFY_DISPLAY__SHOW;
	case DE_DOP_DIF:	return ((sync_type==CLASSIFY_TYPE__DIFF) ? CLASSIFY_DISPLAY__SHOW : CLASSIFY_DISPLAY__HIDE);
	case DE_DOP_CTX:	return ((sync_type==CLASSIFY_TYPE__EQ)   ? CLASSIFY_DISPLAY__MAYBE : CLASSIFY_DISPLAY__SHOW);
	case DE_DOP_EQL:	return ((sync_type==CLASSIFY_TYPE__EQ)   ? CLASSIFY_DISPLAY__SHOW : CLASSIFY_DISPLAY__HIDE);
	default:
		wxASSERT_MSG( (0), _T("Coding Error"));
		return CLASSIFY_DISPLAY__HIDE;
	}
}

//////////////////////////////////////////////////////////////////

const fl_line * de_de::getFlLineFromDisplayListRow(long kSync, PanelIndex kPanel, int row)
{
	// map display-list-row-number into layout line.
	// this accounts for voids which may be in the display list.
	// this returns the line in the document.

	const TVector_Display * pDis = getDisplayList(kSync);
	int nrRows = (int)pDis->size();

	if (row >= nrRows)	// bogus row number
		return NULL;
	
	const de_row & rDeRow = (*pDis)[row];
	const de_line * pDeLine = rDeRow.getPanelLine(kPanel);

	if (!pDeLine)		// a void row or eof row
		return NULL;

	const fl_line * pFlLine = pDeLine->getFlLine();
	return pFlLine;
}

bool de_de::getDisplayListRowFromFlLine(long kSync, PanelIndex kPanel, const fl_line * pFlLineGiven, int * pRow)
{
	// find the given line in the display list and
	// return its row number.  this can fail for
	// several reasons, such as when the line (while
	// present in the document) is not in the display
	// list -- such as when the display mode is _CTX
	// and the given line is not near a delta.
	//
	// this row number should not be confused with the
	// document line number.
	//
	// WARNING: this is a little expensive.

	wxASSERT_MSG( (pFlLineGiven && pRow), _T("Coding Error") );
	
	const TVector_Display * pDis = getDisplayList(kSync);
	int nrRows = (int)pDis->size();

	for (int kRow=0; kRow<nrRows; kRow++)
	{
		const de_row & rDeRow = (*pDis)[kRow];
		const de_line * pDeLine = rDeRow.getPanelLine(kPanel);

		if (!pDeLine)		// a void row
			continue;

		const fl_line * pFlLineK = pDeLine->getFlLine();
		if (pFlLineK != pFlLineGiven)
			continue;
		
		*pRow = kRow;
		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////

bool de_de::mapCoordToDocPosition2(long kSync, PanelIndex kPanel, int row, int col, int colTabWidth, fim_offset * pDocPos, bool bSkipVoids)
{
	// map (row,col) from display list into piecetable coordinates.
	// note that (row,col) are relative to the display list.  (so
	// row numbers are including VOID lines and column numbers are
	// after tabs have been expanded.
	//
	// if (bSkipVoids) we advance to the first non-void row when we
	// are called for a void row.

	const TVector_Display * pDis = getDisplayList(kSync);
	int nrRows = (int)pDis->size();
	int eofRow = nrRows - 1;			// row number of EOF marker

	fim_ptable * pPTable = getPTable(kSync,kPanel);

	if (row == eofRow)
	{
		wxASSERT_MSG( (col==0), _T("Coding Error") );

		*pDocPos = pPTable->getAbsoluteLength();
		return true;
	}

	for (int kRow=row; (kRow<=eofRow); kRow++)
	{
		const de_row & rDeRow = (*pDis)[kRow];
		const de_line * pDeLine = rDeRow.getPanelLine(kPanel);

		if (pDeLine)		// we found a non-void row
		{
			const fl_line * pFlLine = pDeLine->getFlLine();
			
			const fim_frag * pFrag;
			fr_offset offsetFrag;
			pFlLine->getFragAndOffsetOfColumn(col,colTabWidth,&pFrag,&offsetFrag);

			*pDocPos = pPTable->getAbsoluteOffset(pFrag,offsetFrag);
			return true;
		}

		// we found a void row

		if (!bSkipVoids)	// the first time thru, if first row is void,
			return false;	// we just fail.
	}

	return false;
}

bool de_de::mapCoordToDocPosition(long kSync, PanelIndex kPanel, int row, int col, int colTabWidth, fim_offset * pDocPos)
{
	// return the docPos of the given row, DO NOT search forward over voids.

	return mapCoordToDocPosition2(kSync,kPanel,row,col,colTabWidth,pDocPos,false);
}

bool de_de::mapDocPositionToRowCol(long kSync, PanelIndex kPanel, int colTabWidth, fim_offset docPos, int * pRow, int * pCol)
{
	// map an absolute document-position into a display-list (row,col)

	// WARNING: this is a little expensive.

	fim_ptable * pPTable = getPTable(kSync,kPanel);
	
	fim_offset docPosEOF = pPTable->getAbsoluteLength();
	wxASSERT_MSG( (docPos <= docPosEOF), _T("Coding Error") );

	if (docPos >= docPosEOF)		// special case when looking at the EOF marker
	{								// because it doesn't have FlLine's.

		// when we are sitting on the EOF, we have an opportunity
		// for ambiguity -- if the last line in the document does
		// not have a EOL character, then the absolute-doc-offset
		// of the end of the line is the same as the offset of the
		// EOF pseudo-line.

		if (pPTable->hasFinalEOL())		// if there is an EOL, we want the EOF pseudo-line
		{
			const TVector_Display * pDis = getDisplayList(kSync);
			int nrRows = (int)pDis->size();
			int eofRow = nrRows - 1;			// row number of EOF marker

			*pRow = eofRow;
			*pCol = 0;
			return true;
		}

		// otherwise, we want the (row,col) of actual end of the
		// last line (so we can let the caret dangle out there).
	}

	fim_frag * pFragResult;
	fr_offset offsetResult;

	pPTable->getFragAndOffset(docPos,&pFragResult,&offsetResult);

	fl_fl * pFlFl = getLayout(kSync,kPanel);

	int rowResult;
	int colResult;
	const fl_line * pFlLineResult;
	if (!pFlFl->getLineAndColumnOfFragAndOffset(pFragResult,offsetResult,colTabWidth,&pFlLineResult,&colResult))
		return false;

	if (!getDisplayListRowFromFlLine(kSync,kPanel,pFlLineResult,&rowResult))
		return false;

	*pRow = rowResult;
	*pCol = colResult;

	return true;
}
