// de_de.cpp -- diff engine -- stuff related to patches.
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

void de_de::unsetPatchHighlight(long kSync)
{
	if (!m_bPatchHighlight[kSync])				// already off
		return;

	m_bPatchHighlight[kSync] = false;
	m_chgs |= ((kSync == SYNC_EDIT) ? DE_CHG_EDIT_DISP : DE_CHG_VIEW_DISP);

	// broadcast message to tell views that the display lists have changed.

	m_cblChange.callAll( util_cbl_arg(this, m_chgs) );
}

bool de_de::isPatch(long kSync, int yRowClick)
{
	const TVector_Display * pDis = getDisplayList(kSync);
	int nrRows = (int)pDis->size() - 1;

	if (yRowClick >= nrRows)		// on or past the special EOF row
		return false;

	// since the display-list may be hiding things (mode _CTX or
	// hide-omitted, for example) we need to map the row number
	// to the sync-list and use it to quickly find the bounds of
	// the change.  normally, this will be exactly one sync-node;
	// but in case of a complex conflict, there may be multiple
	// adjacent sync-nodes in the list.
	//
	// get the sync-node of the line they clicked on

	const de_row & rDeRow = (*pDis)[yRowClick];
	const de_sync * pSync = rDeRow.getSync();

	bool bMerge = (m_pFsFs->getNrTops() == 3);
	de_attr attrEQ = ((bMerge) ? DE_ATTR_MRG_3EQ : DE_ATTR_DIF_2EQ);

	// if they clicked on an EQ line or a mark, turn off
	// the highlight and ignore the click.

	if (pSync->isSameType(attrEQ) || pSync->isMARK())
		return false;

	// [1] ML_MEMBER: if they clicked on an omitted line, see if we
	//     want to treat it as part of the patch.
	//     (see _insert_omitted_lines_into_sync_list()).

	if (pSync->isOmitted())
		return pSync->isMLMember();
	
	return true;
}

void de_de::setPatchHighlight(long kSync, long yRowClick, bool bDontExtend, bool bQuietly)
{
	wxASSERT_MSG( (isPatch(kSync,yRowClick)), _T("Coding Error") );

	bool bMerge = (m_pFsFs->getNrTops() == 3);
	de_attr attrEQ = ((bMerge) ? DE_ATTR_MRG_3EQ : DE_ATTR_DIF_2EQ);

	m_bPatchHighlight[kSync] = true;
	m_yRowPatchClick[kSync] = yRowClick;

	const TVector_Display * pDis = getDisplayList(kSync);

#if 1
	// find the bounds of the patch under the click.  the previous
	// version (in the #else) of this walked the sync-list and grabbed
	// as much as possible from the current sync-node and adjacent ones
	// when appropriate.  but this has problems when we are hiding
	// content (either _CTX or _DIFFS_ONLY or hide-omitted) because
	// the sync-list scan could cross gaps (see bug:11632).  so this
	// version walks the display list rows instead.
	//
	// a side-effect of this is that a patch may not completely consume
	// a sync-node -- that is, it may start in the middle of one and end
	// in the middle of a later one.
	//
	// BTW, "first" is inclusive; "last" is not.

	long yRowFirst = yRowClick;
	long yRowLast  = yRowFirst + 1;
	if (!bDontExtend)
	{
		while (yRowFirst > 0)
		{
			const de_row & rDeRow = (*pDis)[yRowFirst];
			if (rDeRow.haveGap())		// a gap immediately prior to this row
				break;					// so don't cross it.

			const de_row & rDeRowPrev = (*pDis)[yRowFirst-1];
			const de_sync * pSyncPrev = rDeRowPrev.getSync();
			if (   pSyncPrev->isSameType(attrEQ)
				|| pSyncPrev->isMARK()
				|| (pSyncPrev->isOmitted() && !pSyncPrev->isMLMember()))
				break;

			yRowFirst--;
		}
		
		int nrRows = (int)pDis->size() - 1;
		while (yRowLast < nrRows)
		{
			const de_row & rDeRowLast = (*pDis)[yRowLast];
			if (rDeRowLast.haveGap())	// row begins a gap, so don't cross it.
				break;

			const de_sync * pSyncLast = rDeRowLast.getSync();
			if (   pSyncLast->isEND()
				|| pSyncLast->isSameType(attrEQ)
				|| pSyncLast->isMARK()
				|| (pSyncLast->isOmitted() && !pSyncLast->isMLMember()))
				break;

			yRowLast++;
		}
	}

#else
	// start by identifying the bounds of the sync node under the click.
	// 
	// we also need to find the bounds of the patch in display-row-space.
	// that is, find the display-rows which cover the patch.  for example, a
	// VOID can consume multiple display-rows covering multiple sync-nodes
	// and represent 0 lines in the document.
	//
	// BTW, "first" is inclusive; "last" is not.

	const de_row & rDeRow = (*pDis)[yRowClick];
	const de_sync * pSync = rDeRow.getSync();

	long yRowFirst = yRowClick - rDeRow.getOffsetInSync();
	long yRowLast  = yRowFirst + pSync->getMaxLen();
	const de_sync * pSyncFirst = pSync;
	const de_sync * pSyncLast = pSync->getNext();

	if (!bDontExtend)
	{
		// find complete range of the change (like a transitive closure).
		// that is, search both ways from the sync node containing the
		// line clicked on to get the end-points [first,last) of the change.
		//
		// we scan backwards and forewards until we get to an EQ line, MARK or
		// isolated omitted line.  this allows us to handle complex conflicts
		// (which consist of a series of adjacent sync-nodes).
		//
		// SEE ALSO, comment [1] about ML_MEMBER in isPatch().

		const de_sync * pSyncPrev  = pSyncFirst->getPrev();
		while (pSyncPrev)
		{
			if (   pSyncPrev->isSameType(attrEQ)
				|| pSyncPrev->isMARK()
				|| (pSyncPrev->isOmitted() && !pSyncPrev->isMLMember()))
				break;

			yRowFirst -= pSyncPrev->getMaxLen();
			pSyncFirst = pSyncPrev;
			pSyncPrev  = pSyncFirst->getPrev();
		}
		
		while (pSyncLast)
		{
			if (   pSyncLast->isEND()
				|| pSyncLast->isSameType(attrEQ)
				|| pSyncLast->isMARK()
				|| (pSyncLast->isOmitted() && !pSyncLast->isMLMember()))
				break;

			yRowLast += pSyncLast->getMaxLen();
			pSyncLast = pSyncLast->getNext();
		}
	}
#endif

//	wxLogTrace(wxTRACE_Messages, _T("de_de::setPatchHighlight: [yRowClick %d][starting lines %d %d %d]...[ending lines %d %d %d] [display-rows %d %d]"),
//			   yRowClick,
//			   pSyncFirst->getNdx(PANEL_T0)+1, pSyncFirst->getNdx(PANEL_T1)+1, pSyncFirst->getNdx(PANEL_T2)+1,
//			   pSyncLast->getNdx( PANEL_T0)+1, pSyncLast->getNdx( PANEL_T1)+1, pSyncLast->getNdx( PANEL_T2)+1,
//			   yRowFirst,yRowLast);

//	m_pSyncPatch[kSync][0] = pSyncFirst;
//	m_pSyncPatch[kSync][1] = pSyncLast;

	wxASSERT_MSG( (yRowLast > yRowFirst), _T("Coding Error") );

	m_yRowPatch[kSync][0] = yRowFirst;
	m_yRowPatch[kSync][1] = yRowLast;

	if (!bQuietly)
	{
		// broadcast message to tell views that the display lists have changed
		// (or rather, that we've changed which patch is highlighted).

		m_chgs |= ((kSync == SYNC_EDIT) ? DE_CHG_EDIT_DISP : DE_CHG_VIEW_DISP);
		m_cblChange.callAll( util_cbl_arg(this, m_chgs) );
	}
}

bool de_de::getPatchHighlight(long kSync, long * pyRowClick, long * pyRowFirst, long * pyRowLast)
{
	if (pyRowClick)
		*pyRowClick = m_yRowPatchClick[kSync];
	if (pyRowFirst)
		*pyRowFirst = m_yRowPatch[kSync][0];
	if (pyRowLast)
		*pyRowLast = m_yRowPatch[kSync][1];

	return m_bPatchHighlight[kSync];
}

bool de_de::isPatchAVoid(long kSync, PanelIndex kPanel, bool bAssert_kSync/*= true*/)
{
	// does the change/conflict represent a complete VOID in this panel?

	if (bAssert_kSync == true)
	{
		wxASSERT_MSG((m_bPatchHighlight[kSync]), _T("Coding Error"));
	}

#if 1
	const TVector_Display * pDis = getDisplayList(kSync);
	for (long kRow=m_yRowPatch[kSync][0]; kRow<m_yRowPatch[kSync][1]; kRow++)
	{
		const de_row & rDeRow = (*pDis)[kRow];
		if (rDeRow.getPanelLine(kPanel) != NULL)
			return false;
	}
	return true;
#else
	for (const de_sync * pSync=m_pSyncPatch[kSync][0]; pSync != m_pSyncPatch[kSync][1]; pSync=pSync->getNext())
		if (pSync->getLen(kPanel) > 0)
			return false;
	return true;
#endif
}

bool de_de::isPatchEqual(long kSync, PanelIndex kPanel, PanelIndex jPanel)
{
	// does the content of the entire patch match between
	// these two panels?

	bool bMerge = (m_pFsFs->getNrTops() == 3);

	// using this function for 2-way diffs doesn't make sense because
	// 2-way patches are different by definition.

	MY_ASSERT( (bMerge) );
	
	// [since a merge/3-way patch may span more than one sync node
	// (because of the adjacent NEQ globbing)(and because of node
	// splitting to get better sub-change VOID alignment), we need
	// to walk the set of sync nodes to compute the answer.]

	de_attr attrType;
	if      (kPanel==PANEL_T1  &&  jPanel==PANEL_T0)		attrType = DE_ATTR_MRG_T1T0EQ;
	else if (kPanel==PANEL_T1  &&  jPanel==PANEL_T2)		attrType = DE_ATTR_MRG_T1T2EQ;
	else if (kPanel==PANEL_T0  &&  jPanel==PANEL_T2)		attrType = DE_ATTR_MRG_T0T2EQ;
	else { wxASSERT_MSG( (0), _T("Codng Error") ); return false; }
		
#if 1
	const TVector_Display * pDis = getDisplayList(kSync);
	for (long kRow=m_yRowPatch[kSync][0]; kRow<m_yRowPatch[kSync][1]; kRow++)
	{
		const de_row & rDeRow = (*pDis)[kRow];
		const de_sync * pSync = rDeRow.getSync();
		if (!DE_ATTR__IS_TYPE(pSync->getAttr(),attrType))
			return false;
	}
	return true;
	
#else
	for (const de_sync * pSync=m_pSyncPatch[kSync][0]; pSync != m_pSyncPatch[kSync][1]; pSync=pSync->getNext())
		if (!DE_ATTR__IS_TYPE(pSync->getAttr(),attrType))
			return false;
	return true;
#endif
}

wxString de_de::getPatchSrcString(long kSync, PanelIndex kPanel)
{
	wxASSERT_MSG( (m_bPatchHighlight[kSync]), _T("Coding Error") );
	wxASSERT_MSG( ((kPanel==PANEL_T0) || (kPanel==PANEL_T2)), _T("Coding Error") );

	// get the contents of the change/conflict from this
	// (source) panel into a string buffer (and using the
	// EOL mode of the edit (destination) panel).

	wxString strEditEOL;
	fim_eol_mode modeEdit = getPTable(kSync,PANEL_EDIT)->getEolMode();
	switch (modeEdit)
	{
	default:					modeEdit = FIM_MODE_UNSET;	break;
	case FIM_MODE_LF:			strEditEOL = _T("\n");		break;
	case FIM_MODE_CRLF:			strEditEOL = _T("\r\n");	break;
	case FIM_MODE_CR:			strEditEOL = _T("\r");		break;
	}

	const TVector_Display * pDis = getDisplayList(kSync);

	wxString strPatchSrc;

	long rowStart = m_yRowPatch[kSync][0];
	long rowEnd   = m_yRowPatch[kSync][1];

	for (long row=rowStart; row<rowEnd; row++)
	{
		const de_row & rDeRow = (*pDis)[row];
		const de_line * pDeLine = rDeRow.getPanelLine(kPanel);

		if (!pDeLine)		// a void row
			continue;

		strPatchSrc += pDeLine->getStrLine();
		if (pDeLine->getStrEOL().Length() > 0)
			strPatchSrc += ((modeEdit==FIM_MODE_UNSET) ? pDeLine->getStrEOL() : strEditEOL);
	}

	return strPatchSrc;
}

//////////////////////////////////////////////////////////////////

bool de_de::getPatchStartDocPosition(long kSync, PanelIndex kPanel, fim_offset * pDocPos)
{
	wxASSERT_MSG( (m_bPatchHighlight[kSync]), _T("Coding Error") );
	
	// get the starting bounds of the entire patch -- in display-list row-space.
	// this is independent of any panel.

	long rowStart = m_yRowPatch[kSync][0];
	long rowEnd   = m_yRowPatch[kSync][1];

	// now find the actual starting row on this panel.
	// WARNING: this panel may be entirely or partially VOID, so we need to
	// WARNING: find the actual bounds ON THIS PANEL.

	long rowActualStart = -1;
	const TVector_Display * pDis = getDisplayList(kSync);

	for (long row=rowStart; row<rowEnd; row++)
	{
		const de_row & rDeRow = (*pDis)[row];
		const de_line * pDeLine = rDeRow.getPanelLine(kPanel);

		if (!pDeLine)		// a void row
			continue;

		if (rowActualStart == -1)
			rowActualStart = row;	// remember first non-void row
	}
	
	if (rowActualStart == -1)		// completely VOID
		return false;

	int col = 0;
	int colTabWidth = 8;			// this is bogus and unused (since we only care about column 0)
	
	return mapCoordToDocPosition(kSync,kPanel,rowActualStart,col,colTabWidth,pDocPos);
}

bool de_de::getPatchEndDocPosition(long kSync, PanelIndex kPanel, fim_offset * pDocPos)
{
	// get the docPosition of the end of the currently highlighted patch.

	wxASSERT_MSG( (m_bPatchHighlight[kSync]), _T("Coding Error") );
	
	long rowEnd   = m_yRowPatch[kSync][1];

	// if a VOID immediately follows the patch (possible when
	// we didn't extend the patch to the closure), we need to
	// get the first non-VOID following it in order to compute
	// the document-position.

	const TVector_Display * pDis = getDisplayList(kSync);
	int nrRows = (int)pDis->size() - 1;

	long row;
	for (row=rowEnd; row<nrRows; row++)
	{
		const de_row & rDeRow = (*pDis)[row];
		const de_line * pDeLine = rDeRow.getPanelLine(kPanel);

		if (pDeLine)	// found non-VOID
			break;
	}

	int col = 0;
	int colTabWidth = 8;			// this is bogus and unused (since we only care about column 0)
	
	return mapCoordToDocPosition(kSync,kPanel,row,col,colTabWidth,pDocPos);
}

//////////////////////////////////////////////////////////////////

bool de_de::getPatchLineNrs(long kSync, PanelIndex kPanel, long * pLineNrFirst, long * pLineNrLast)
{
	// return the document line number of the first/last line of the patch.
	// 
	// WARNING: this is not the display list row number.
	// 
	// WARNING: this will return false if this patch contains only voids
	// WARNING: for this patch.

	wxASSERT_MSG( (m_bPatchHighlight[kSync]), _T("Coding Error") );

	// get the starting bounds of the entire patch -- in display-list row-space.
	// this is independent of any panel.

	long rowStart = m_yRowPatch[kSync][0];		// first row included in patch
	long rowEnd   = m_yRowPatch[kSync][1];		// first row past end of patch

	const de_line * pDeLineFirst = NULL;
	const de_line * pDeLineLast = NULL;
	
	const TVector_Display * pDis = getDisplayList(kSync);

	for (long row=rowStart; row<rowEnd; row++)
	{
		const de_row & rDeRow = (*pDis)[row];
		const de_line * pDeLine = rDeRow.getPanelLine(kPanel);

		if (!pDeLine)		// a void row
			continue;

		if (!pDeLineFirst)
			pDeLineFirst = pDeLine;		// find the first non-void line within patch
		pDeLineLast = pDeLine;			// first the last non-void line within patch
	}

	if (!pDeLineFirst)		// completely void within the patch
		return false;

	if (pLineNrFirst)
		*pLineNrFirst = pDeLineFirst->getFlLine()->getLineNr();
	if (pLineNrLast)
		*pLineNrLast = pDeLineLast->getFlLine()->getLineNr();

	return true;
}

bool de_de::getPatchLineNrAfter(long kSync, PanelIndex kPanel, long * pLineNrAfter)
{
	// return line number of the first line following the patch in this panel.
	// returns false when at end of file.

	wxASSERT_MSG( (m_bPatchHighlight[kSync]), _T("Coding Error") );

	long rowEnd   = m_yRowPatch[kSync][1];		// first row past end of patch

	const TVector_Display * pDis = getDisplayList(kSync);

	int nrRows = (int)pDis->size();

	for (long row=rowEnd; row<nrRows; row++)
	{
		const de_row & rDeRow = (*pDis)[row];
		const de_line * pDeLine = rDeRow.getPanelLine(kPanel);

		if (!pDeLine)		// a void row
			continue;

		if (pLineNrAfter)
			*pLineNrAfter = pDeLine->getFlLine()->getLineNr();
		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////

void de_de::_get_patch_line_nrs_allowing_for_voids_or_eof(long kSync, PanelIndex kPanel, long * pLineNrFirst, long * pLineNrLast)
{
	bool bValidLineNrs;

	bValidLineNrs = getPatchLineNrs(kSync,kPanel,pLineNrFirst,pLineNrLast);
	if (bValidLineNrs)		// non-void (return [+,+])
		return;

	// void or eof

	*pLineNrLast = -1;

	bValidLineNrs = getPatchLineNrAfter(kSync,kPanel,pLineNrFirst);
	if (bValidLineNrs)		// first line following void
		return;				// void (return [+,-])

	// eof

	*pLineNrFirst = -1;		// eod (return [-,-])
}

de_patch * de_de::createPatch(fim_patch_op op, int kSync)
{
	// create a de_patch from the info in the currently highlighted patch

	long rowStart, rowEnd;
	getPatchHighlight(kSync,NULL,&rowStart,&rowEnd);

	fim_length lenDelete;
	fim_offset docPosEditStart, docPosEditEnd;
	bool bValidStartPos = getPatchStartDocPosition(kSync,PANEL_EDIT,&docPosEditStart);
	bool bValidEndPos   = getPatchEndDocPosition(kSync,PANEL_EDIT,&docPosEditEnd);

	long lineNrEditFirst,lineNrEditLast;
	long lineNrT0First,lineNrT0Last;
	long lineNrT2First,lineNrT2Last;

	wxString strNewTextT0, strNewTextT2;

	_get_patch_line_nrs_allowing_for_voids_or_eof(kSync,PANEL_EDIT, &lineNrEditFirst,&lineNrEditLast);
	_get_patch_line_nrs_allowing_for_voids_or_eof(kSync,PANEL_T0,&lineNrT0First,&lineNrT0Last);
	_get_patch_line_nrs_allowing_for_voids_or_eof(kSync,PANEL_T2,&lineNrT2First,&lineNrT2Last);
	
	switch (op)
	{
	default:
		//case POP_IGNORE:
		wxASSERT_MSG( (0), _T("Coding Error") );
		lenDelete = 0;
		break;

	case POP_DELETE:
		MY_ASSERT( (bValidStartPos && bValidEndPos && (docPosEditEnd > docPosEditStart)) );
		lenDelete = docPosEditEnd - docPosEditStart;
		MY_ASSERT( ((lineNrEditFirst != -1) && (lineNrEditLast != -1)) );	// edit panel must not be void
		break;

	case POP_REPLACE_L:
		MY_ASSERT( (bValidStartPos && bValidEndPos && (docPosEditEnd > docPosEditStart)) );
		lenDelete = docPosEditEnd - docPosEditStart;
		MY_ASSERT( ((lineNrEditFirst != -1) && (lineNrEditLast != -1)) );	// edit panel must not be void
		MY_ASSERT( ((lineNrT0First != -1) && (lineNrT0Last != -1)) );		// T0 must not be void
		strNewTextT0 = getPatchSrcString(kSync,PANEL_T0);
		break;

	case POP_REPLACE_R:
		MY_ASSERT( (bValidStartPos && bValidEndPos && (docPosEditEnd > docPosEditStart)) );
		lenDelete = docPosEditEnd - docPosEditStart;
		MY_ASSERT( ((lineNrEditFirst != -1) && (lineNrEditLast != -1)) );	// edit panel must not be void
		MY_ASSERT( ((lineNrT2First != -1) && (lineNrT2Last != -1)) );		// T2 must not be void
		strNewTextT2 = getPatchSrcString(kSync,PANEL_T2);
		break;

	case POP_INSERT_L:
		MY_ASSERT( (bValidEndPos && !bValidStartPos) );
		docPosEditStart = docPosEditEnd;
		lenDelete = 0;
		MY_ASSERT( (lineNrEditLast == -1) );								// edit panel must be void
		MY_ASSERT( ((lineNrT0First != -1) && (lineNrT0Last != -1)) );		// T0 must not be void
		strNewTextT0 = getPatchSrcString(kSync,PANEL_T0);
		break;
		
	case POP_INSERT_R:
		MY_ASSERT( (bValidEndPos && !bValidStartPos) );
		docPosEditStart = docPosEditEnd;
		lenDelete = 0;
		MY_ASSERT( (lineNrEditLast == -1) );								// edit panel must be void
		MY_ASSERT( ((lineNrT2First != -1) && (lineNrT2Last != -1)) );		// T2 must not be void
		strNewTextT2 = getPatchSrcString(kSync,PANEL_T2);
		break;
		
	case POP_CONFLICT:
		wxASSERT_MSG( (bValidEndPos), _T("Coding Error") );
		if (!bValidStartPos)	// conflict is completely void in edit panel
			docPosEditStart = docPosEditEnd;
		lenDelete = docPosEditEnd - docPosEditStart;
		strNewTextT0 = getPatchSrcString(kSync,PANEL_T0);
		strNewTextT2 = getPatchSrcString(kSync,PANEL_T2);
		break;
	}

	if ((op == POP_INSERT_L) || (op == POP_INSERT_R))
	{
		if (!getPTable(kSync,PANEL_EDIT)->hasFinalEOL() && !getPatchLineNrAfter(kSync,PANEL_EDIT,NULL))
		{
			// bug:11149
			// if the patch that we're about apply will insert text at the end
			// of the file and the edit buffer doesn't currently have a final EOL,
			// then we need to prepend an EOL to the text we insert so that it
			// won't start up on the end of the last partial line in the buffer.

			wxString strEditEOL;
			fim_eol_mode modeEdit = getPTable(kSync,PANEL_EDIT)->getEolMode();
			switch (modeEdit)
			{
			default:					strEditEOL = FIM_MODE_NATIVE_DISK_STR;	wxASSERT_MSG( (0), _T("Coding Error") ); break;		// should not happen
			case FIM_MODE_LF:			strEditEOL = _T("\n");					break;
			case FIM_MODE_CRLF:			strEditEOL = _T("\r\n");				break;
			case FIM_MODE_CR:			strEditEOL = _T("\r");					break;
			}

			strNewTextT0.Prepend(strEditEOL);
			strNewTextT2.Prepend(strEditEOL);
		}
	}
	
	return new de_patch(op,
						docPosEditStart,lenDelete,
						lineNrEditFirst,lineNrEditLast,
						strNewTextT0,FR_PROP__INSERTED, lineNrT0First,lineNrT0Last,
						strNewTextT2,FR_PROP__INSERTED, lineNrT2First,lineNrT2Last,
						rowStart,rowEnd);
}
