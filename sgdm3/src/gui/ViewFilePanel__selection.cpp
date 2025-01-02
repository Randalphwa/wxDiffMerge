// ViewFilePanel__selection.cpp
// selection-related portion of ViewFilePanel.cpp
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fs.h>
#include <fl.h>
#include <rs.h>
#include <de.h>
#include <fd.h>
#include <gui.h>

//////////////////////////////////////////////////////////////////

#if defined(__WXMSW__)
// In http://trac.wxwidgets.org/ticket/14444 and
// commit [72259], they changed how wxTextDataObject
// gets data from the clipboard.  In ::SetData()
// they now *ALWAYS* convert it to LF from platform
// native CRLF -- as a convenience, since the app
// is probably using LF internally -- but i'm not.
//
// Oddly enough, they only "fixed" :SetData() and
// NOT the ctor.
class MY_TextDataObject : public wxTextDataObject
{
public:
	MY_TextDataObject(const wxString & text = wxEmptyString)
		: wxTextDataObject(text)
		{
		};

    virtual bool SetData(size_t len, const void *buf)
		{
			const wxString
				text = wxString(static_cast<const wxChar*>(buf), len/sizeof(wxChar));
			// SetText(wxTextBuffer::Translate(text, wxTextFileType_Unix));
			SetText(text);

			return true;
		};
};
#else
#define MY_TextDataObject wxTextDataObject
#endif

//////////////////////////////////////////////////////////////////

bool ViewFilePanel::haveSelection(void) const
{
	return (m_vfcSelection0.isSet());
}

void ViewFilePanel::clearSelection(void)
{
	m_vfcSelection0.clear();
	m_vfcWordSelection0.clear();
	m_vfcAnchor.clear();
	
	// don't update caret
	
//	wxLogTrace(wxTRACE_Messages,_T("ViewFilePanel::Selection cleared."));

	Refresh(true);
	Update();
}

void ViewFilePanel::selectAll(void)
{
	int kSync = getSync();

	const TVector_Display * pDis = m_pDeDe->getDisplayList(kSync);
	int nrRows = (int)pDis->size() - 1;

	wxASSERT_MSG( ((*pDis)[nrRows].isEOF()), _T("Coding Error") );	// last row in display list is the EOF marker.

	m_vfcAnchor.set(0,0);
	m_vfcCaret.set(nrRows,0);

	m_vfcSelection0.set(0,0);
	m_vfcSelection1.set(nrRows,0);

//	wxLogTrace(wxTRACE_Messages,_T("ViewFilePanel::SelectAll is (%d,%d) ... (%d,%d)"),
//			   m_vfcSelection0.getRow(),m_vfcSelection0.getCol(),
//			   m_vfcSelection1.getRow(),m_vfcSelection1.getCol());

	Refresh(true);
	Update();
}

//////////////////////////////////////////////////////////////////

long ViewFilePanel::getCaretOrSelectionRow(bool bEnd)
{
	// return the row of the caret or the beginning/ending row of the selection.

	if (haveSelection())
	{
		if (bEnd)
			return m_vfcSelection1.getRow();
		else
			return m_vfcSelection0.getRow();
	}

	if (m_vfcCaret.isSet())
		return m_vfcCaret.getRow();

	return -1;
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::_doUpdateCaret(bool bContinueSelection, int row, int col)
{
	ViewFileCoord vfc(row,col);
	_doUpdateCaret(bContinueSelection,vfc);
}

void ViewFilePanel::_doUpdateCaret(bool bContinueSelection, const ViewFileCoord & vfc)
{
	// use (bContinueSelection := true) when shifted so that we don't
	// update the anchor (if we have one) so that we can continue the
	// selection.
	//
	// when not shifted, we always update the anchor.  it should
	// always be equal to the caret when we don't have a selection.

	if (!bContinueSelection  ||  !m_vfcAnchor.isSet())
		m_vfcAnchor.set(vfc);

	m_vfcCaret.set(vfc);

	// if the new caret is not equal to the anchor, start a selection.
	// between the anchor and the caret.  if they are equal, clear the
	// selection.

	switch (m_vfcAnchor.compare(m_vfcCaret))
	{
	default:	// quiets compiler
	case 0:		// anchor == caret
		m_vfcSelection0.clear();
		break;

	case -1:	// anchor < caret
		m_vfcSelection0.set(m_vfcAnchor);
		m_vfcSelection1.set(m_vfcCaret);
		break;

	case +1:	// anchor > caret
		m_vfcSelection0.set(m_vfcCaret);
		m_vfcSelection1.set(m_vfcAnchor);
		break;
	}

#if 0
#if defined(DEBUG)
	if (haveSelection())
		wxLogTrace(wxTRACE_Messages,_T("ViewFilePanel::_doUpdateCaret: Selection is (%d,%d) ... (%d,%d)"),
				   m_vfcSelection0.getRow(),m_vfcSelection0.getCol(),
				   m_vfcSelection1.getRow(),m_vfcSelection1.getCol());
	else
		wxLogTrace(wxTRACE_Messages,_T("ViewFilePanel::Selection not set"));
#endif
#endif

	m_pDeDe->unsetPatchHighlight(getSync());

	// if the Manual Alignment Dialog is up on this frame, we post a new
	// line number into the spin box for this panel.  remember the caret
	// is in display-list-row-number space -- not line-number-space.

	dlg_insert_mark * pDlgInsertMark = m_pViewFile->getFrame()->getInsertMarkerDialog();
	if (pDlgInsertMark)
	{
		const fl_line * pFlLine = m_pDeDe->getFlLineFromDisplayListRow(getSync(),m_kPanel,m_vfcCaret.getRow());
		if (pFlLine)	// if have valid line -- that is, not void or mark or past EOF.
			pDlgInsertMark->externalUpdateLineNr(m_kPanel,pFlLine);
	}
}

void ViewFilePanel::setBogusCaret(void)
{
	m_vfcSelection0.clear();
	m_vfcWordSelection0.clear();
	m_vfcAnchor.clear();
	m_vfcCaret.clear();
	_setGoal(0);
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::_placeCaret(fim_offset docPos)
{
	// place caret at the (row,col) in the view
	// that corresponds to the given absolute
	// document position.

	int rowResult;
	int colResult;
	if (_mapDocPositionToRowCol(docPos,&rowResult,&colResult))
	{
		_doUpdateCaret(false,rowResult,colResult);
		_setGoal(m_vfcCaret.getCol());
		return;
	}

	// we could not compute the (row,col) where the caret
	// should be now.  i'm not sure there's a right answer
	// for this.  the important thing is that we clear the
	// selection.

	setBogusCaret();
}

void ViewFilePanel::_placeSelection(fim_offset docPos0,fim_offset docPos1)
{
	// select the region that corresponds to the 2 given
	// absolute document positions.

	int rowResult0;
	int colResult0;
	if (!_mapDocPositionToRowCol(docPos0,&rowResult0,&colResult0))
	{
		// we could not compute the (row,col) where the caret
		// should be now.  i'm not sure there's a right answer
		// for this.  the important thing is that we clear the
		// selection.

		setBogusCaret();
		return;
	}
	
	_doUpdateCaret(false,rowResult0,colResult0);		// set anchor & caret.  clear selection[01]
	_setGoal(m_vfcCaret.getCol());

	int rowResult1;
	int colResult1;
	if (!_mapDocPositionToRowCol(docPos1,&rowResult1,&colResult1))
		return;
	
	_doUpdateCaret(true, rowResult1,colResult1);		// set caret. extend selection from anchor to caret.
	_setGoal(m_vfcCaret.getCol());
}

//////////////////////////////////////////////////////////////////

bool ViewFilePanel::_withinSelection(int row, int col) const
{
	// is left-edge of (row,col) inside the selection.
	
	if (!haveSelection())
		return false;

	if ((row < m_vfcSelection0.getRow())  ||  (row > m_vfcSelection1.getRow()))
		return false;

	if ((row == m_vfcSelection0.getRow())  &&  (col < m_vfcSelection0.getCol()))
		return false;

	if ((row == m_vfcSelection1.getRow())  &&  (col >= m_vfcSelection1.getCol()))
		return false;

	return true;
}

bool ViewFilePanel::_intersectSelection(int row, int col0, int col1) const
{
	// does the span (with left-edge at column col0 and right edge at col1)
	// intersect the current selection.

	if (!haveSelection())
		return false;

	if ((row < m_vfcSelection0.getRow())  ||  (row > m_vfcSelection1.getRow()))
		return false;

	if ((row == m_vfcSelection0.getRow())  &&  (col1 <= m_vfcSelection0.getCol()))
		return false;

	if ((row == m_vfcSelection1.getRow())  &&  (col0 >= m_vfcSelection1.getCol()))
		return false;

	return true;
}

bool ViewFilePanel::_containsCaret(int row, int col0, int col1) const
{
	// does the span (with left-edge at column col0 and right edge at col1)
	// contain the (left-edge of the) caret.  that is, is the caret within
	// the half-open interval [col0,col1).

	if (!m_vfcCaret.isSet())
		return false;

	if (row != m_vfcCaret.getRow())
		return false;

	return ((col0 <= m_vfcCaret.getCol()) && (m_vfcCaret.getCol() < col1));
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::_setGoal(const ViewFileCoord & vfc)
{
	_setGoal(vfc.getCol());
}

void ViewFilePanel::_setGoal(int col)
{
	m_colGoal = col;
}

int ViewFilePanel::_getGoal(void) const
{
	return m_colGoal;
}

//////////////////////////////////////////////////////////////////

bool ViewFilePanel::prepare_to_copy_selection(wxString & strBuf) const
{
	if (!haveSelection())
		return false;

	strBuf.Empty();

	int kSync = getSync();

	const TVector_Display * pDis = m_pDeDe->getDisplayList(kSync);

	int cColTabWidth = m_pViewFile->getTabStop();

	for (int row=m_vfcSelection0.getRow(); (row<=m_vfcSelection1.getRow()); row++)
	{
		bool bFirstLine = (row == m_vfcSelection0.getRow());
		bool bLastLine  = (row == m_vfcSelection1.getRow());
		
		const de_row &  rDeRow  = (*pDis)[row];
		const de_line * pDeLine = rDeRow.getPanelLine(m_kPanel);

		if (!pDeLine)	// a void row
			continue;

		const fl_line * pFlLine = pDeLine->getFlLine();
		wxString strEOL, strLine;
		pFlLine->buildStringsFromRuns(false,&strLine,&strEOL);

		// deal with EOL styles.  we want to place text on the clipboard using
		// the platform-native EOL chars.  if the document is using the style
		// of a different platform, we need to change them.  so, we only look
		// to see if the line has an EOL char -- then we substitute the platform-
		// native one as we build the string.

		bool bHaveEOL = (strEOL.Length() > 0);

		if (bLastLine)
		{
			// on the last line of the selection, we may need to right-truncate the line if the selection doesn't
			// extend to the end of the line.
			// 
			// NOTE: the column number is the column after the tabs were expanded (and as seen by the user on the
			// NOTE: screen).  the buffer we have in strLine does not have tabs expanded (because we want to put
			// NOTE: the original tabs on the clipboard).
			//
			// a side effect of this is that we can't easily tell what the column number of the EOL is and whether
			// we should include/exclude it without counting the tabs.  [this is more complicated than it looks
			// because we also want to convert from doc-eol-style to platform-native-eol-style.]

			const wxChar * sz = strLine.wc_str();
			int len = (int)strLine.Length();

			int colEnd = m_vfcSelection1.getCol();
			int col_k = 0;
			int ndx = 0;
			while ((col_k < colEnd) && (ndx < len))
			{
				if (sz[ndx] == 0x0009)
				{
					int pad = cColTabWidth - (col_k % cColTabWidth);
					col_k += pad;
				}
				else
				{
					col_k++;
				}
				ndx++;
			}
			if (ndx < len)				// if we reached the destination column and didn't consume the entire string,
				strLine.Truncate(ndx);	//   then we only want the first portion of the last line in the selection.
			else if (col_k < colEnd)	// if the string ended before we reached the destination column,
				if (bHaveEOL)			//   then we want to include the EOL (if there was one).
					strLine += FIM_MODE_NATIVE_CLIPBOARD_STR;
		}
		else	// when not the last line in the selection, we always include the EOL
		{
			if (bHaveEOL)
				strLine += FIM_MODE_NATIVE_CLIPBOARD_STR;
		}

		if (bFirstLine && (m_vfcSelection0.getCol() > 0))
		{
			// on the first line of the selection, we may need to
			// left-truncate the line if the selection doesn't start
			// in column zero.
			// 
			// NOTE: the column number is the column after the tabs
			// NOTE: were expanded (and as seen by the user on the
			// NOTE: screen).  the buffer we have in strLine does
			// NOTE: not have tabs expanded (because we want to put
			// NOTE: the original tabs on the clipboard).

			const wxChar * sz = strLine.wc_str();
			int colStart = m_vfcSelection0.getCol();
			int col_k = 0;
			int ndx = 0;
			while (col_k < colStart)
			{
				if (sz[ndx] == 0x0009)
				{
					int pad = cColTabWidth - (col_k % cColTabWidth);
					col_k += pad;
				}
				else
				{
					col_k++;
				}
				ndx++;
			}
			strLine = strLine.Mid(ndx);
		}

		strBuf += strLine;
	}

	return true;
}

void ViewFilePanel::copyToClipboard(void) const
{
	wxString strBuf;

	if (!prepare_to_copy_selection(strBuf))
		return;

//	wxLogTrace(wxTRACE_Messages, _T("ViewFilePanel:Copy: [%s]"), util_printable_s(strBuf).wc_str());

	// wxBUG: there are a few problems with the clipboard:
	// wxBUG: 
	// wxBUG: on mac with 2.6.1 it crashes when there are chars in the buffer that can't be
	// wxBUG: converted to latin-1.  2.6.1 uses version 1.32 of common/dobjcmn.cpp and there's
	// wxBUG: a lot of changes in this area in the 1.38 in the 2.6.3rc2, so i expect this to
	// wxBUG: go away shortly.
	// wxBUG: 
	// wxBUG: on unix with 2.6.1 we get a trailing null in when the buffer is pasted into
	// wxBUG: emacs.  this was reported in bug #1210121 (when GTK & Unicode) (still open as
	// wxBUG: of 3/20/06).  this probably won't be fixed in 2.6.3.

	if (wxTheClipboard->Open())
	{
		wxTextDataObject * pTDO = new wxTextDataObject(strBuf);

		wxTheClipboard->SetData(pTDO);
		wxTheClipboard->Close();
	}
}

void ViewFilePanel::copySelectionForFind(void) const
{
	wxString strBuf;

	// TODO 2013/07/23 decide how/if we want to handle selections
	// TODO            spanning more than one line.

	if (!prepare_to_copy_selection(strBuf))
		return;

	gpMyFindData->set(strBuf);
}

//////////////////////////////////////////////////////////////////

bool ViewFilePanel::_getTextFromClipboard(wxTextDataObject & tdo)
{
	if (!wxTheClipboard->Open())
		return false;
	
	if (wxTheClipboard->IsSupported(wxDF_TEXT))
	{
		wxTheClipboard->GetData(tdo);
		wxTheClipboard->Close();
		return true;
	}
	else
	{
		wxTheClipboard->Close();
		return false;
	}
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::cutToClipboard(void)
{
	// delete the current selection from the document.

	if (!haveSelection())		// should not happen
		return;					// TODO should we clear the clipboard?

	copyToClipboard();

	if (!isEditPanel())
		return;

	fim_offset docPosBegin, docPosEnd;

	if (!_mapCoordToDocPosition2(&m_vfcSelection0, &docPosBegin, true))
		return;	// should not happen
	if (!_mapCoordToDocPosition2(&m_vfcSelection1, &docPosEnd, true))
		return;	// should not happen

	_deleteText(docPosBegin,docPosEnd);

	_kb_ensureCaretVisible(true);
}

void ViewFilePanel::pasteFromClipboard(void)
{
	if (!isEditPanel())
		return;

	MY_TextDataObject tdo;
	if (!_getTextFromClipboard(tdo))	// no text data on clipboard (or clipboard
		return;							// locked), so just ignore paste request.

	if ( tdo.GetText().Length() == 0 )
		return;

	wxString str = tdo.GetText();

	// we assume that we get text from the clipboard using platform-native EOL chars.
	// convert them to the document's EOL mode if necessary.

	convertEOLs(str,FIM_MODE_NATIVE_CLIPBOARD,m_pViewFile->getPTable(m_kSync,m_kPanel)->getEolMode());
	
	_insertText( str, false );

	_kb_ensureCaretVisible(true);
}

//////////////////////////////////////////////////////////////////

bool ViewFilePanel::_mapDocPositionToRowCol(fim_offset docPos,
											int * pRow, int * pCol)
{
	return m_pDeDe->mapDocPositionToRowCol(getSync(),m_kPanel,m_pViewFile->getTabStop(),docPos,pRow,pCol);
}

const fl_line * ViewFilePanel::_mapRowToFlLine(int row) const
{
	return m_pDeDe->getFlLineFromDisplayListRow(getSync(),m_kPanel,row);
}

bool ViewFilePanel::_mapCoordToDocPosition(const ViewFileCoord * pVFC, fim_offset * pDocPos) const
{
	if (!pVFC->isSet())
		return false;
	return m_pDeDe->mapCoordToDocPosition(getSync(),m_kPanel,pVFC->getRow(),pVFC->getCol(),m_pViewFile->getTabStop(),pDocPos);
}

bool ViewFilePanel::_mapCoordToDocPosition2(const ViewFileCoord * pVFC, fim_offset * pDocPos, bool bSkipVoids) const
{
	if (!pVFC->isSet())
		return false;
	return m_pDeDe->mapCoordToDocPosition2(getSync(),m_kPanel,pVFC->getRow(),pVFC->getCol(),m_pViewFile->getTabStop(),pDocPos,bSkipVoids);
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::_deleteText(fim_offset docPos0, fim_offset docPos1)
{
	if (docPos0 == docPos1)		// a delete of nothing ??
		return;
	
	wxASSERT_MSG( (docPos1 > docPos0), _T("Coding Error") );
	
	// actually delete the text from the document.  the piecetable
	// gets updated directly during the call.  the fl layout has
	// callbacks installed so it updates the lines/runs as the doc
	// changes.  the diff-engine gets notified that it needs to
	// re-run -- but this is done lazily when needed.

	fim_ptable * pPTable = m_pViewFile->getDoc()->getFsFs()->getPTable(m_kSync,m_kPanel);
	pPTable->deleteText(docPos0,(docPos1-docPos0));

	// the text following the delete is now touching the
	// text prior to the delete, so put the caret in the
	// middle.  we need to use the doc position to re-compute
	// the (row,col).   since one or more lines could have
	// been deleted, the fl_line references aren't necessarily
	// valid.  since frags may have been coalesced during the
	// delete the fim_frag references may not be valid.  also,
	// because the delete may change how the diff lines up,
	// the (row,col) values may have changed.
	//
	// TODO for now, we do this the *very* hard way.  in the
	// TODO future, we may be able to short-cut part of this.
	// TODO such as remembering the fl_line prior to the line
	// TODO we start the delete on and then seeing what its
	// TODO next value is.

	_placeCaret(docPos0);

	// caller needs to decide about warp-scrolling/refresh/update
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::_insertText(const wxString & str, bool bSelectNewText)
{
	// insert text at the caret or replace the current selection
	// (if we have one) in the document with the contents of str.
	// does not deal with the clipboard.

	if (!m_vfcCaret.isSet())	// should we beep ?
		return;

	wxASSERT_MSG( (str.Length() > 0), _T("Coding Error") );

	fim_ptable * pPTable = m_pViewFile->getDoc()->getFsFs()->getPTable(m_kSync,m_kPanel);

	// convert caret position (with tabs expanded) into doc position

	fim_offset docPosBegin;

	if (haveSelection())
	{
		fim_offset docPosEnd;

		if (!_mapCoordToDocPosition2(&m_vfcSelection0, &docPosBegin, true))
			return;	// should not happen
		if (!_mapCoordToDocPosition2(&m_vfcSelection1, &docPosEnd, true))
			return;	// should not happen

		// actually replace the text from the document.  the piecetable
		// gets updated directly during the call.  the fl layout has
		// callbacks installed so it updates the lines/runs as the doc
		// changes.  the diff-engine gets notified that it needs to
		// re-run -- but this is done lazily when needed.

		pPTable->replaceText(docPosBegin,(docPosEnd-docPosBegin),str, FR_PROP__INSERTED);
	}
	else
	{
		if (!_mapCoordToDocPosition2(&m_vfcCaret, &docPosBegin, true))
			return;		// should not happen

		// actually insert string into piecetable at frag/offset

		pPTable->insertText(docPosBegin, FR_PROP__INSERTED, str);
	}
	
	// now we need to move caret to end of the insertion.
	// if requested, we select the region that we inserted.

	if (bSelectNewText)
		_placeSelection(docPosBegin, docPosBegin + str.Length());
	else
		_placeCaret(docPosBegin + str.Length());

	// caller needs to decide about warp-scrolling/refresh/update
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::undo(void)
{
	pt_stat s = m_pViewFile->getEditPanelStatus();
	if (!PT_STAT_TEST(s,PT_STAT_CAN_UNDO))
		return;

	fim_ptable * pPTable = m_pViewFile->getDoc()->getFsFs()->getPTable(m_kSync,m_kPanel);

	fim_offset docPos;
	bool bSetDocPos = pPTable->undo(&docPos);

	if (bSetDocPos)
	{
		_placeCaret(docPos);
		_kb_ensureCaretVisible(true);
	}
	else
	{
		setBogusCaret();
	}
}

void ViewFilePanel::redo(void)
{
	pt_stat s = m_pViewFile->getEditPanelStatus();
	if (!PT_STAT_TEST(s,PT_STAT_CAN_REDO))
		return;

	fim_ptable * pPTable = m_pViewFile->getDoc()->getFsFs()->getPTable(m_kSync,m_kPanel);

	fim_offset docPos;
	bool bSetDocPos = pPTable->redo(&docPos);

	if (bSetDocPos)
	{
		_placeCaret(docPos);
		_kb_ensureCaretVisible(true);
	}
	else
	{
		setBogusCaret();
	}
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::beforeTabStopChange(ViewFilePanel::caretState * p)
{
	// prepare for a tab-stop change.  remember the absolute-document-positions
	// of the selection and caret (which are in already-expanded row,col values).

	p->bValidCaret      = m_vfcCaret.isSet() && _mapCoordToDocPosition(&m_vfcCaret,      &p->docPosCaret     );

	p->bValidSelection0 = m_vfcSelection0.isSet() && _mapCoordToDocPosition(&m_vfcSelection0, &p->docPosSelection0);
	p->bValidSelection1 = (p->bValidSelection0
						   && m_vfcSelection1.isSet()
						   && _mapCoordToDocPosition(&m_vfcSelection1, &p->docPosSelection1));

	// we do not attempt to preserve the goal column.
	// we do not save the anchor position, it can be computed
	// from the caret and the selection.

	setBogusCaret();
}

void ViewFilePanel::afterTabStopChange(ViewFilePanel::caretState * p)
{
	// restore the caret and selection after a tab-stop change.

	if (p->bValidSelection0 && p->bValidSelection1)
		_placeSelection(p->docPosSelection0, p->docPosSelection1);
	else if (p->bValidCaret)
		_placeCaret(p->docPosCaret);
	else
		setBogusCaret();

	Refresh(true);
}

//////////////////////////////////////////////////////////////////

fim_patchset * ViewFilePanel::_build_patchset(int kSync, int * pNrConflicts)
{
	// build a vector of operations to apply to the edit-buffer.
	// we start at the bottom and work backwards.  this lets the
	// piecetable apply them without messing up the absolute document
	// positions.  (that is, we apply patches from the bottom so that
	// the doc-offsets of earlier changes are still valid.)
	//
	// NOTE: we could walk the sync-list to get a set of patches to
	// NOTE: be applied. this might be quicker, but would require us
	// NOTE: to duplicate some code **and** has the risk of doing
	// NOTE: something slightly different in the merge.
	// NOTE:
	// NOTE: we want auto-merge to act like this:  apply the default
	// NOTE: patch operation on each patch.
	// NOTE:
	// NOTE: so, we start at the bottom of the display list and walk the
	// NOTE: prev change/conflict chains to quickly get to each change,
	// NOTE: use the patch identification code to get the bounds (doc
	// NOTE: positions), use the right-mouse-menu code to compute the
	// NOTE: default operation, and record the details in a list.
	// NOTE: these get handed to the piecetable in one step so that
	// NOTE: it can properly wrap them in a transaction.
	//
	// TODO this code should really be in class de_de, but because we use
	// TODO the _build_context_menu__edit() function here in the loop, we
	// TODO can't move it down.  maybe in the future, move the menu code
	// TODO down too.
	
	fim_patchset * pPatchSet = new fim_patchset();

	const TVector_Display * pDis = m_pDeDe->getDisplayList(kSync);
	long rowLimit = (long)pDis->size() - 1;			// the EOF row
	long row = (*pDis)[rowLimit].getPrevChange();	// get the beginning of the last change

	int nrConflicts = 0;

	while (row > -1)
	{
		m_pDeDe->setPatchHighlight(kSync,row,false,true);		// silently select/activate this patch
		de_patch * pDePatch = NULL;

		int op = _build_context_menu__edit(NULL);
		switch (op)
		{
		default:
			wxASSERT_MSG( (0), _T("Coding Error") );
			break;

		case CTX__NO_DEFAULT:		// a *CONFLICT*
			pDePatch = m_pDeDe->createPatch(POP_CONFLICT,kSync);
			nrConflicts++;
			break;

		case CTX_DELETE:
			pDePatch = m_pDeDe->createPatch(POP_DELETE,kSync);
			break;
			
		case CTX_INSERT_L:	// insert contents of change/conflict from T0 into a VOID in the edit panel
			pDePatch = m_pDeDe->createPatch(POP_INSERT_L,kSync);
			break;

		case CTX_INSERT_R:	// insert contents of change/conflict from T2 into a VOID in the edit panel
			pDePatch = m_pDeDe->createPatch(POP_INSERT_R,kSync);
			break;

		case CTX_REPLACE_L:			// replace the non-void contents in the edit panel with the the contents of T0
			pDePatch = m_pDeDe->createPatch(POP_REPLACE_L,kSync);
			break;

		case CTX_REPLACE_R:			// replace the non-void contents in the edit panel with the the contents of T2
			pDePatch = m_pDeDe->createPatch(POP_REPLACE_R,kSync);
			break;
		}

		if (pDePatch)
		{
			pPatchSet->appendPatch(pDePatch);
			row = (*pDis)[pDePatch->getRowStart()].getPrevChange();
		}
		else			// should not happen
			break;		// but go on if it does.
	}

	// since we cycled thru the patches, we should just unhighlight it now.

	m_pDeDe->unsetPatchHighlight(kSync);
	setBogusCaret();

	*pNrConflicts = nrConflicts;
	return pPatchSet;
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::autoMerge(void)
{
	int kSync = SYNC_EDIT;

	int nrConflicts = 0;
	fim_patchset * pPatchSet = _build_patchset(kSync,&nrConflicts);
	
	if (pPatchSet->getNrPatches() == 0)
	{
		// there are no patches/changes to merge.

		wxMessageDialog dlg(m_pViewFile->getFrame(),
							_("There are no changes to merge."),
							_("Notice"),
							wxOK|wxICON_EXCLAMATION);
		m_pViewFile->showModalDialogSuppressingActivationChecks(dlg);

		delete pPatchSet;
		return;
	}

	// apply what we can (the easy (non-conflict) changes) and
	// tell them what we did and let them deal with the rest.

	m_pViewFile->completeAutoMerge(pPatchSet);

	delete pPatchSet;
	return;
}

//////////////////////////////////////////////////////////////////

FindResult ViewFilePanel::find(const wxString & strPattern, bool bIgnoreCase, bool bForward)
{
	// TODO 2013/08/16 With the addition of the FindPanel to File Windows
	// TODO            I'm simplifying the interface and removing the
	// TODO            regex and wrap buttons.  It's just too many widgets
	// TODO            for the ribbon.  So I've removed them from ViewFile.cpp
	// TODO            but haven't yet removed them from here.
	bool bUseRegEx = false;
	bool bWrapAround = false;

	// search for the given pattern from the current selection or caret
	// (if bogus caret, complain without searching).
	//
	// if bUseRegEx, do reg-ex search.
	//
	// if we find a match, warp scroll it into view and select it.
	//
	// WARNING: we only search what is visible in the display-list.
	// WARNING: (so, for example, if they have diff-with-context set,
	// WARNING: we only search what's on screen; the same goes for
	// WARNING: lines hidden by lomit when hide-omitted is set.)

	wxString strPatternWork;
	wxRegEx rePatternWork;

	if (strPattern.length() == 0)
		return FindResult_EmptyString;

	if (bUseRegEx)
	{
		if (!bForward)
			return FindResult_CantRegExBackwards;

		int flags = wxRE_ADVANCED;
		if (bIgnoreCase)
			flags |= wxRE_ICASE;

		bool bValid = rePatternWork.Compile(strPattern,flags);
		if (!bValid)
			return FindResult_BadRegEx;
	}
	else
	{
		strPatternWork = strPattern;
		if (bIgnoreCase)
			strPatternWork.MakeLower();
	}

	if (bForward)
		return _find_forward(bUseRegEx,bIgnoreCase,bWrapAround,rePatternWork,strPatternWork);
	else
		return _find_backwards(bIgnoreCase,bWrapAround,strPatternWork);
}

FindResult ViewFilePanel::_find_forward(bool bUseRegEx, bool bIgnoreCase, bool bWrapAround, const wxRegEx & rePatternWork, const wxString & strPatternWork)
{
	// search forward 

	//////////////////////////////////////////////////////////////////
	// row,col are display-list values -- just like the selection/caret.

	int initialRowStart;
	int initialColStart;
	
	if (haveSelection())
	{
		initialRowStart = m_vfcSelection1.getRow();		// search forwards starting at the end of the selection
		initialColStart = m_vfcSelection1.getCol();
	}
	else if (m_vfcCaret.isSet())
	{
		initialRowStart = m_vfcCaret.getRow();				// search forwards starting at the caret
		initialColStart = m_vfcCaret.getCol();
	}
	else
	{
		initialRowStart = 0;								// silently start at beginning
		initialColStart = 0;
	}

	const TVector_Display * pDis = m_pDeDe->getDisplayList(m_kSync);

	// search from caret (initial starting point) to EOF.

	int rowLimit = (int)pDis->size();	// vector includes EOF row
	int rowStart = initialRowStart;
	int colStart = initialColStart;

	for (int kRow=rowStart; (kRow < rowLimit); kRow++, colStart = 0)
	{
		if (_find_forward_on_row(kRow,colStart,bUseRegEx,bIgnoreCase,rePatternWork,strPatternWork))
			return FindResult_Match;
	}

	if (!bWrapAround)
		return FindResult_NoMatch;	// no wrap-around, just fail

	if ((initialRowStart == 0) && (initialColStart == 0))
		return FindResult_NoMatch;	// wrap-around wanted, but we already did whole doc

	// if we started in the middle of the intial starting row, we need to include
	// it as the last line in the search.  if we started at the beginning of the
	// initial starting row, we don't.

	rowLimit = ((initialColStart > 0) ? initialRowStart+1 : initialRowStart);
	rowStart = 0;
	colStart = 0;

	for (int kRow=rowStart; (kRow < rowLimit); kRow++)
	{
		if (_find_forward_on_row(kRow,colStart,bUseRegEx,bIgnoreCase,rePatternWork,strPatternWork))
			return FindResult_Match;
	}

	return FindResult_NoMatch;
}

bool ViewFilePanel::_find_forward_on_row(int kRow, int colStart, bool bUseRegEx, bool bIgnoreCase,
										 const wxRegEx & rePatternWork, const wxString & strPatternWork)
{
	const TVector_Display * pDis = m_pDeDe->getDisplayList(m_kSync);
	const de_row &  rDeRow  = (*pDis)[kRow];

	if (rDeRow.isMARK())	// display-list has mark on this row, ignore
		return false;
	if (rDeRow.isEOF())		// reached the EOF row in the display-list, ignore
		return false;
		
	const de_line * pDeLine = rDeRow.getPanelLine(m_kPanel);
	if (!pDeLine)			// a void row, ignore
		return false;

	// get a raw-buffer of the line where tabs are *NOT* expanded. we need
	// this so that pattern matching can have access to the actual content
	// (and reg-ex's can match tabs, for example).

	wxString strTemp = pDeLine->getStrLine();

	// on the first line, we need to compensate for the caret being in the middle of
	// the line -- and exclude the portion to the left of the caret from the search.

	fim_offset docPosStart = 0;
	fim_offset docPosBOL = 0;
	int gap = 0;

	int cColTabWidth = m_pViewFile->getTabStop();

	m_pDeDe->mapCoordToDocPosition(m_kSync,m_kPanel,kRow,0,cColTabWidth,&docPosBOL);

	if (colStart > 0)
	{
		// take the column value (where tabs are already expanded) and map into
		// the raw buffer (where tabs are not expanded).  to do this we compute
		// the absolute-document-position of the column and the beginning of the
		// line.  subtracting the two values gives us the column's position in
		// the raw-buffer.
		
		m_pDeDe->mapCoordToDocPosition(m_kSync,m_kPanel,kRow,colStart,cColTabWidth,&docPosStart);
		gap = (int)(docPosStart - docPosBOL);
		if (gap > 0)
			strTemp = strTemp.Mid(gap);
	}

	if (bIgnoreCase)
		strTemp.MakeLower();

	int ndxMatch = -1;
	int lenMatch = 0;
		
	if (bUseRegEx)
	{
		size_t ndx,len;

		if (rePatternWork.Matches(strTemp) && rePatternWork.GetMatch(&ndx,&len))
		{
			//wxLogTrace(wxTRACE_Messages,_T("Find:RegExMatch [ndx %ld][len %ld][%s]"),ndx,len,strTemp.wc_str());
			ndxMatch = (int)ndx;
			lenMatch = (int)len;
		}
	}
	else
	{
		// find the first exact match within the string.

		ndxMatch = strTemp.Find(strPatternWork.wc_str());
		lenMatch = (int)strPatternWork.Length();
	}
		
	if (ndxMatch == -1)		// no match within line
		return false;

	// now, if colStart was set, map the matching ndx back to the entire raw-buffer

	ndxMatch += gap;

	// now convert the complete raw-buffer-ndx and map into a tab-expanded-column value
		
	int rowMatch,colMatch;
	m_pDeDe->mapDocPositionToRowCol(m_kSync,m_kPanel,cColTabWidth, docPosBOL+ndxMatch, &rowMatch,&colMatch);

	wxASSERT_MSG( (rowMatch==kRow), _T("Coding Error") );
	wxASSERT_MSG( (colMatch>=colStart), _T("Coding Error") );

	int rowMatchEnd, colMatchEnd;
	m_pDeDe->mapDocPositionToRowCol(m_kSync,m_kPanel,cColTabWidth, docPosBOL+ndxMatch+lenMatch, &rowMatchEnd,&colMatchEnd);

	wxASSERT_MSG( (rowMatchEnd==kRow), _T("Coding Error") );
	wxASSERT_MSG( (colMatchEnd>colMatch), _T("Coding Error") );
		
	//wxLogTrace(wxTRACE_Messages, _T("Plain text match found at [row %d] [col %d..%d]"),rowMatch,colMatch,colMatchEnd);

	_doUpdateCaret(false,rowMatch,   colMatch);		// set anchor & caret.  clear selection[01]
	_doUpdateCaret(true, rowMatchEnd,colMatchEnd);	// set caret. extend selection from anchor to caret.
	_setGoal(colMatchEnd);
	_kb_ensureCaretVisible(true);
		
	return true;
}

FindResult ViewFilePanel::_find_backwards(bool bIgnoreCase, bool bWrapAround, const wxString & strPatternWork)
{
	// search backward

	const TVector_Display * pDis = m_pDeDe->getDisplayList(m_kSync);
	int nrRows = (int)pDis->size() - 1;		// vector includes special EOF row

	//////////////////////////////////////////////////////////////////
	// row,col are display-list values -- just like the selection/caret.

	int initialRowStart;
	int initialColStart;
	
	if (haveSelection())
	{
		initialRowStart = m_vfcSelection0.getRow();		// search backwards starting at the beginning of the selection
		initialColStart = m_vfcSelection0.getCol();
	}
	else if (m_vfcCaret.isSet())
	{
		initialRowStart = m_vfcCaret.getRow();				// search backwards starting at the caret
		initialColStart = m_vfcCaret.getCol();
	}
	else
	{
		initialRowStart = nrRows;							// silently start at special EOF row (past the end of the document)
		initialColStart = 0;
	}

	// search from caret (initial starting point) to BOF.

	int rowLimit = (int)pDis->size();	// vector includes EOF row
	int rowStart = initialRowStart;
	int colStart = initialColStart;

	for (int kRow=rowStart; (kRow >= 0); kRow--, colStart = -1)
	{
		if (_find_backward_on_row(kRow,colStart,bIgnoreCase,strPatternWork))
			return FindResult_Match;
	}

	if (!bWrapAround)
		return FindResult_NoMatch;	// no wrap-around, just fail

	if ((initialRowStart == nrRows) && (initialColStart == 0))
		return FindResult_NoMatch;	// wrap-around wanted, but we already did whole doc

	rowLimit = initialRowStart;
	rowStart = nrRows;
	colStart = -1;

	for (int kRow=rowStart; (kRow >= rowLimit); kRow--)
	{
		if (_find_backward_on_row(kRow,colStart,bIgnoreCase,strPatternWork))
			return FindResult_Match;
	}

	return FindResult_NoMatch;
}

bool ViewFilePanel::_find_backward_on_row(int kRow, int colStart, bool bIgnoreCase, const wxString & strPatternWork)
{
	if (colStart == 0)		// search backwards from column 0 should always fail to find anything.
		return false;
	
	const TVector_Display * pDis = m_pDeDe->getDisplayList(m_kSync);
	const de_row &  rDeRow  = (*pDis)[kRow];

	if (rDeRow.isMARK())	// display-list has mark on this row, ignore
		return false;
	if (rDeRow.isEOF())		// reached the EOF row in the display-list, ignore
		return false;
		
	const de_line * pDeLine = rDeRow.getPanelLine(m_kPanel);
	if (!pDeLine)			// a void row, ignore
		return false;

	// get a raw-buffer of the line where tabs are *NOT* expanded. we need
	// this so that pattern matching can have access to the actual content
	// (and reg-ex's can match tabs, for example).

	wxString strTemp = pDeLine->getStrLine();

	fim_offset docPosStart = 0;
	fim_offset docPosBOL = 0;

	int cColTabWidth = m_pViewFile->getTabStop();
	int edge;	// edge is where we start looking backwards from

	m_pDeDe->mapCoordToDocPosition(m_kSync,m_kPanel,kRow,0,cColTabWidth,&docPosBOL);

	if (colStart > 0)
	{
		// take the column value (where tabs are already expanded) and map into
		// the raw buffer (where tabs are not expanded).  to do this we compute
		// the absolute-document-position of the column and the beginning of the
		// line.  subtracting the two values gives us the column's position in
		// the raw-buffer.
		
		m_pDeDe->mapCoordToDocPosition(m_kSync,m_kPanel,kRow,colStart,cColTabWidth,&docPosStart);
		edge = (int)(docPosStart - docPosBOL);
	}
	else /* colStart == -1 */ /* means entire line */
	{
		edge = (int)strTemp.Length();
	}

	if (bIgnoreCase)
		strTemp.MakeLower();

	int ndxMatch = -1;
	int lenMatch = 0;
		
	// find the last exact match within the string.
	// but since we don't have a FindLast() we need to loop.

	ndxMatch = util_string_find_last(strTemp,edge,strPatternWork.wc_str());
	lenMatch = (int)strPatternWork.Length();
		
	if (ndxMatch == -1)		// no match within line
		return false;

	// now convert the complete raw-buffer-ndx and map into a tab-expanded-column value
		
	int rowMatch,colMatch;
	m_pDeDe->mapDocPositionToRowCol(m_kSync,m_kPanel,cColTabWidth, docPosBOL+ndxMatch, &rowMatch,&colMatch);

	wxASSERT_MSG( (rowMatch==kRow), _T("Coding Error") );
	wxASSERT_MSG( ((colMatch<colStart) || (colStart==-1)), _T("Coding Error") );

	int rowMatchEnd, colMatchEnd;
	m_pDeDe->mapDocPositionToRowCol(m_kSync,m_kPanel,cColTabWidth, docPosBOL+ndxMatch+lenMatch, &rowMatchEnd,&colMatchEnd);

	wxASSERT_MSG( (rowMatchEnd==kRow), _T("Coding Error") );
	wxASSERT_MSG( (colMatchEnd>colMatch), _T("Coding Error") );
		
	//wxLogTrace(wxTRACE_Messages, _T("Plain text match found at [row %d] [col %d..%d]"),rowMatch,colMatch,colMatchEnd);

	_doUpdateCaret(false,rowMatchEnd,colMatchEnd);		// set anchor & caret.  clear selection[01]
	_doUpdateCaret(true, rowMatch,colMatch);			// set caret. extend selection from anchor to caret.
	_setGoal(colMatch);
	_kb_ensureCaretVisible(true);
		
	return true;
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::gotoLine(int lineNr)
{
	// goto document line-number NOT display-list-row-number

	// map line-number from layout to display-list.  line may
	// not actually be in the display list (depends upon DOPS
	// and etc), so if not go to the first row displayed after it.

	fl_fl * pFlFl = m_pDeDe->getLayout(m_kSync,m_kPanel);
	lineNr = MyMin(lineNr,pFlFl->getFormattedLineNrs());
	fl_line * pFlLine = pFlFl->getNthLine(lineNr);
	if (!pFlLine)							// should not happen, but let's not crash if it does
		return;

	// what row does this line think it's on -- this may be stale.

	de_line * pDeLine = m_pDeDe->getDeLineFromFlLine(m_kSync,m_kPanel,pFlLine);
	size_t row = pDeLine->getRowNr(m_kSync);

	const TVector_Display * pDis = m_pDeDe->getDisplayList(m_kSync);
	size_t nrRows = pDis->size() - 1;
	bool bStale = (row > nrRows);
	if (!bStale)
	{
		const de_row &  rDeRow  = (*pDis)[row];
		const de_line * pDeLineCheck = rDeRow.getPanelLine(m_kPanel);
		bStale = (pDeLine != pDeLineCheck);
	}
	if (bStale)
	{
		// the DeLine thought it was on row "row" in the display-list,
		// but the display-list thinks differently -- so the DeLine's
		// info is stale (probably because it was there, but the DOPs
		// changed and it is not presently visible).  do linear search
		// on the display-list for the first visible line on or after
		// the requested document-line-number.

		size_t kRow;
		for (kRow=0; kRow<nrRows; kRow++)
		{
			const de_row &  rDeRowK  = (*pDis)[kRow];
			const de_line * pDeLineK = rDeRowK.getPanelLine(m_kPanel);
			if (pDeLineK)								// if non-void row
			{
				const fl_line * pFlLineK = pDeLineK->getFlLine();
				if (pFlLineK->getLineNr() >= lineNr)
					break;
			}
		}
		row = (int)kRow;
	}

	//wxLogTrace(wxTRACE_Messages,_T("GoToLine: [line nr %d][row %d][bStale %d]"),lineNr,(int)row,bStale);

	if (bStale)
	{
		// the line they wanted is not visible - warp to next visible line following it.
		
		_doUpdateCaret(false,(int)row,0);		// set anchor & caret, clear selection.
	}
	else
	{
		// the line they wanted is visible - select the line.
		// start just past EOL and select back to BOL so that caret is actually on this line.

		_doUpdateCaret(false,(int)row+1,0);		// set anchor & caret just past EOL, clear selection
		_doUpdateCaret(true, (int)row, 0);		// select back to BOL on this line.
	}

	_kb_ensureCaretVisible(true);
}

//////////////////////////////////////////////////////////////////

/*static*/void ViewFilePanel::convertEOLs(wxString & str, fim_eol_mode modeInBuf, fim_eol_mode modeDesired)
{
	if (modeInBuf == modeDesired)		return;		// no conversion needed
	if (modeInBuf == FIM_MODE_UNSET)	return;		// don't know what it is, just leave it alone
	if (modeDesired == FIM_MODE_UNSET)	return;		// don't know what it should be, just leave it alone

	wxRegEx re;
	switch (modeInBuf)
	{
	default:				// quiets compiler
	case FIM_MODE_LF:		re.Compile(_T("\n"  ),wxRE_ADVANCED);	break;
	case FIM_MODE_CRLF:		re.Compile(_T("\r\n"),wxRE_ADVANCED);	break;
	case FIM_MODE_CR:		re.Compile(_T("\r"  ),wxRE_ADVANCED);	break;
	}
	
	wxString strReplacement;
	switch (modeDesired)
	{
	default:				// quiets compiler
	case FIM_MODE_LF:		strReplacement = _T("\n"  );	break;
	case FIM_MODE_CRLF:		strReplacement = _T("\r\n");	break;
	case FIM_MODE_CR:		strReplacement = _T("\r"  );	break;
	}
	
	re.ReplaceAll(&str,strReplacement);
}
