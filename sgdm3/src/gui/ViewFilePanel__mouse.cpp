// ViewFilePanel__mouse.cpp
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

#define AUTO_SCROLL_TIMER_RATE_MS		100

//////////////////////////////////////////////////////////////////

bool ViewFilePanel::_mapMouseFindCrossing(wxDC & dc, int xMouse,
										  const wxChar * sz, long len,
										  bool bDisplayInvisibles, int cColTabWidth,
										  wxCoord & x0, wxCoord & x1,
										  int & col0, int & col1)
{
	wxArrayInt ai;
	wxString strPrep;
	_prepareStringForDrawing(sz,len,bDisplayInvisibles,cColTabWidth,col1,strPrep);	// this updates col1 and strPrep
	dc.GetPartialTextExtents(strPrep,ai);
	x1 = x0 + (int)ai.Last();

	if (x1 < xMouse)
	{
		// if entire string doesn't cross, update {x0,col0} so that
		// we start at this point with the next span of text.

		x0 = x1;
		col0 = col1;
		return false;
	}
	
	// somewhere within the run we crossed the mouse.
	// find the index of the character at the crossing.

	size_t kLimit = ai.GetCount();
	for (size_t k=0; (k<kLimit); k++)
	{
		if (x0 + (int)ai.Item(k) < xMouse)		// right edge of kth character has not yet reached mouse
			continue;

		x1 = x0 + (int)ai.Item(k);				// set right edge to the right edge of the kth character
		if (k > 0)
			x0 += (int)ai.Item(k-1);			// set our left edge to the right edge of k-1'th character

		col0 += (int)k;							// this is safe since we don't have tabs
		col1 = col0 + 1;
		return true;
	}
		
	wxASSERT_MSG( (0), _T("Coding Error") );
	return false;
}

void ViewFilePanel::_mapMouseEventToCoord(int xMouseGiven, int yMouseGiven,
										  int * pRow, int * pVScroll,
										  int * pCol, int * pHScroll)
{
	ViewFileCoord coord;

	int kSync = getSync();

	int yDocSize = _kb_getEOFRow();

	int xPixelsClientSize, yPixelsClientSize;
	GetClientSize(&xPixelsClientSize,&yPixelsClientSize);

	int yRowThumb = m_pViewFile->getScrollThumbCharPosition(m_kSync,wxVERTICAL);
	if (yMouseGiven <= 0)									// if mouse at or above top of window
	{
		*pRow = yRowThumb;							//    set to first row currently visible
		*pVScroll = ((yRowThumb > 0) ? -1 : 0);		//    if necessary, let auto-scroll scroll up.   TODO compute a better scroll factor.
	}
	else
	{
		int yPixelsPerRow = getPixelsPerRow();

		if (yMouseGiven < yPixelsClientSize)				// if mouse within client area of window
		{
			*pRow = yRowThumb + (yMouseGiven / yPixelsPerRow);
			if (*pRow > yDocSize)
				*pRow = yDocSize;
			*pVScroll = 0;
		}
		else										// otherwise, mouse at or below bottom of window
		{
			int yRowEnd = yRowThumb + getRowsDisplayable() + 1;		// +1 to get partially visible row
			int yRowThumbMax = m_pViewFile->getScrollThumbMax(m_kSync,wxVERTICAL);

			*pRow = MyMin(yDocSize, yRowEnd);
			*pVScroll = ((yRowThumb < yRowThumbMax) ? +1 : 0);
		}
	}

	if (*pRow > yDocSize)		// way past end of file, should not happen
		*pRow = yDocSize;
	
	if (*pRow == yDocSize)		// on the special EOF pseudo row 
	{
		// if the file is normal with a final EOL,
		// set col to 0 for the EOF-pseudo-line.
		// if we don't have a final EOL, try to set
		// col to the end of the last valid line
		// (hopefully, the user will get the hint
		// and add a final EOL).
		
		fim_ptable * pPTable = m_pViewFile->getPTable(m_kSync,m_kPanel);
		if (pPTable->hasFinalEOL()  || ! _kb_getPrevRow(*pRow,pRow))
		{
			*pCol = 0;
			*pHScroll = 0;
			return;
		}
		else	// no EOL char, force it onto the end of the last valid line
		{
			(void)_kb_getEOLCol(*pRow,pCol);

			// we need to possibly warp scroll so that the caret will be visible.
			// this may cause some confusion.

			int hThumbBest = _kb_ensureVisible_computeBestHThumb(*pRow,*pCol);
			int hThumbCurrent = m_pViewFile->getScrollThumbCharPosition(m_kSync,wxHORIZONTAL);

			*pHScroll = hThumbBest - hThumbCurrent;
			return;
		}
	}
	else
	{
		const TVector_Display * pDis = m_pDeDe->getDisplayList(kSync);
		const de_row & rDeRow = (*pDis)[*pRow];
		const de_line * pDeLine = rDeRow.getPanelLine(m_kPanel);
		if (!pDeLine)
		{
			*pCol = 0;			// a void row
			*pHScroll = 0;
		}
		else
		{
			int xPixelText = getXPixelText();		// x coordinate where document area begins
			int xColThumb = m_pViewFile->getScrollThumbCharPosition(m_kSync,wxHORIZONTAL);		// NOTE thumb is calibrated assuming fixed-pitch fonts.

			if ((xMouseGiven < xPixelText) && (xColThumb == 0))		// in line-number/left-margin area when not scrolled
			{
				*pCol = 0;
				*pHScroll = 0;
			}
			else
			{
				// since we don't assume a fixed-pitch font, walk the line and pretend to draw
				// each text run and measure it.  increment {x,col} as we go along the line.
				// stop when we find a run that intersects with the mouse coordinate.
				// 
				// use the layout (both the de and the fl versions should give the same result).
				// the layout version is easier for us here. (see onPaint())

				int xPixelsPerCol = getPixelsPerCol();
				int xPixelThumb = xColThumb * xPixelsPerCol;	// current scroll position in pixels -- as if a fixed-pitch font.
				int cColTabWidth = m_pViewFile->getTabStop();
				bool bDisplayInvisibles = m_pViewFile->getPilcrow();

				// create a temporary/scratch bitmap/dc so that we measure strings.

				wxBitmap bm(1,1);
				wxMemoryDC dcMem;
				dcMem.SelectObject(bm);

				int xMouse = MyMin(xPixelsClientSize,xMouseGiven);
			
				wxCoord x0 = xPixelText - xPixelThumb;		// for the "left" side of the run
				wxCoord x1 = x0;							// for the "right" side of the run
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
						// since they probably don't want to draw out a selection and just get the
						// CR in a CRLF sequence, we play some tricks here so that they always get
						// both (when present) -- that is, we always take the CR or LF and keep going
						// rather than breaking out if we've intersected the mouse coordinate.

						if (bDisplayInvisibles)
							_fakeDrawString(dcMem, sz,len, bDisplayInvisibles,cColTabWidth, x1,col1);
					}
					else if (pFlRun->isTAB())
					{
						// we draw a tab a n spaces (w/ or w/o the '>>')
						// 
						// we don't sub-divide our result when the mouse intersects within the area
						// that the tab expanded into.

						_fakeDrawString(dcMem, sz,len, bDisplayInvisibles,cColTabWidth, x1,col1);
						if (x1 >= xMouse)
							bStop = true;
						else
						{
							x0 = x1;
							col0 = col1;
						}
					}
					else
					{
						// a plain run with text and/or spaces.  deal with this in parts just
						// like the drawing code -- so that spaces are all treated the same
						// way and we get the same effective kerning.

						long begin = 0;
						long k     = begin;
						while (!bStop && (begin < len))
						{
							while ( (k < len) && (sz[k] != 0x0020) )
								k++;
							if (k != begin)
							{
								bStop = _mapMouseFindCrossing(dcMem, xMouse, &sz[begin],(k-begin), bDisplayInvisibles,cColTabWidth, x0,x1,col0,col1);
								if (bStop)
									break;
							}
							begin = k;

							while ( (k < len) && (sz[k] == 0x0020) )
								k++;
							if (k != begin)
							{
								bStop = _mapMouseFindCrossing(dcMem, xMouse, &sz[begin],(k-begin), bDisplayInvisibles,cColTabWidth, x0,x1,col0,col1);
								if (bStop)
									break;
							}
							begin = k;
						}
					}
				}

				wxCoord xMidPoint = (x0 + x1) / 2;
				*pCol = ( (xMouse <= xMidPoint) ? col0 : col1 );
				*pHScroll = 0;

				if (xMouse < xPixelText)
				{
					if (xColThumb > 0)
						*pHScroll = -1;
				}
				else if (xMouse >= xPixelsClientSize)
				{
					if (xColThumb < m_pViewFile->getScrollThumbMax(m_kSync,wxHORIZONTAL))
						*pHScroll = +1;
				}
				
				dcMem.SelectObject(wxNullBitmap);
			}
		}
	}
	
//	wxLogTrace(wxTRACE_Messages, _T("ViewFilePanel:_mapMouse: [row %d][vscroll %d][col %d][hscroll %d]"),
//			   *pRow,*pVScroll,
//			   *pCol, *pHScroll);
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::onMouseLeftDown(wxMouseEvent & e)
{
	if (!isBound())		// ignore click if we haven't been completely set up yet
		return;

//	wxLogTrace(wxTRACE_Messages, _T("ViewFilePanel:onMouseLeftDown: pos [%d,%d]"),e.m_x,e.m_y);

	CaptureMouse();
	m_bWeCapturedTheMouse = true;

	m_bDClick = false;
	m_vfcWordSelection0.clear();

	int row,col, vscroll,hscroll;
	_mapMouseEventToCoord(e.m_x,e.m_y,&row,&vscroll,&col,&hscroll);
	ViewFileCoord vfcClicked(row,col);

	if (e.ShiftDown() && haveSelection() && !_withinSelection(row,col))
	{
		// if extending an existing selection, move anchor to endpoint
		// furthest from the mouse.

		if (vfcClicked.compare(m_vfcSelection0) < 0)
			m_vfcAnchor.set(m_vfcSelection1);
		else
			m_vfcAnchor.set(m_vfcSelection0);
	}
		
	_doUpdateCaret(e.ShiftDown(),vfcClicked);
	_setGoal(vfcClicked);
	_rememberMousePosition(false,vfcClicked);

	Refresh(true);
	Update();
	
	e.Skip();		// this lets wxWidgets give us focus
}

void ViewFilePanel::onMouseLeftDClick(wxMouseEvent & e)
{
	if (!isBound())		// ignore click if we haven't been completely set up yet
		return;

//	wxLogTrace(wxTRACE_Messages, _T("ViewFilePanel:onMouseLeftDClick: pos [%d,%d]"),e.m_x,e.m_y);

	int row,col, vscroll,hscroll;
	_mapMouseEventToCoord(e.m_x,e.m_y,&row,&vscroll,&col,&hscroll);
	ViewFileCoord vfcClicked(row,col);

	de_mark * pDeMarkInitial = NULL;
	if (m_pDeDe->isMark(m_kSync,row,&pDeMarkInitial))
	{
		bool bCanRaiseDlg = m_pViewFile->getFrame()->isViewFileInsertMarkEnabled();
		if (bCanRaiseDlg)
			m_pViewFile->getFrame()->showInsertMarkDialog(pDeMarkInitial);

		return;
	}

	int rowResultStart, colResultStart;
	int rowResultEnd, colResultEnd;
	bool bHaveValid = _kb_getWordBoundsAtCol(row,col,
											 &rowResultStart,&colResultStart,
											 &rowResultEnd,&colResultEnd);
	if (!bHaveValid)
		return;

	CaptureMouse();
	m_bWeCapturedTheMouse = true;

	m_bDClick = true;

	ViewFileCoord vfcStart(rowResultStart,colResultStart);
	ViewFileCoord vfcEnd(rowResultEnd,colResultEnd);

#if 0
	if (e.ShiftDown() && haveSelection())
	{
		// extending an existing selection, extend by word.

		int resultStart = vfcStart.compare(m_vfcSelection0);
		if (resultStart < 0)		// start of clicked on word prior to existing selection
		{
			m_vfcAnchor.set(m_vfcSelection1);
			_doUpdateCaret(true,vfcStart);
		}
		else
		{
			int resultEnd = vfcEnd.compare(m_vfcSelection1);
			if (resultEnd > 0)		// end of clicked on word past end of existing selection
			{
				m_vfcAnchor.set(m_vfcSelection0);
				_doUpdateCaret(true,vfcEnd);
			}
		}
		_kb_ensureCaretVisible(true);
	}
	else
#endif
	{
		m_vfcWordSelection0.set(vfcStart);	// remember bound of word that
		m_vfcWordSelection1.set(vfcEnd);	// they initially clicked on.
		
		_doUpdateCaret(false,vfcStart);		// start selection here
		_doUpdateCaret(true,vfcEnd);		// extend to here
		_kb_ensureVisible(vfcEnd.getRow(),vfcEnd.getCol(), true);
	}
	return;
}

void ViewFilePanel::onMouseLeftUp(wxMouseEvent & e)
{
	if (!isBound())		// ignore click if we haven't been completely set up yet
		return;

//	wxLogTrace(wxTRACE_Messages, _T("ViewFilePanel:onMouseLeftUp: pos [%d,%d]"),e.m_x,e.m_y);

	if (m_timer_MyMouse.IsRunning())
	{
//		wxLogTrace(wxTRACE_Messages,_T("ViewFilePanel::onMouseLeftUp: stopping timer"));
		m_timer_MyMouse.Stop();
	}
	
	while (HasCapture())
		ReleaseMouse();
	m_bWeCapturedTheMouse = false;

	if (!e.ShiftDown() && !haveSelection() && m_vfcCaret.isSet() && m_pDeDe->isPatch(m_kSync,m_vfcCaret.getRow()))
	{
#if defined(__WXOSX__)
		bool bControlDown = e.RawControlDown();	// see note in __kb_mac.cpp
#else
		bool bControlDown = e.ControlDown();
#endif		
		m_pDeDe->setPatchHighlight(m_kSync,m_vfcCaret.getRow(),bControlDown);
	}

	e.Skip();
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::onMouseMiddleDown(wxMouseEvent & /*e*/)
{
	if (!isBound())		// ignore click if we haven't been completely set up yet
		return;

//	wxLogTrace(wxTRACE_Messages, _T("ViewFilePanel:onMouseMiddleDown: pos [%d,%d]"),e.m_x,e.m_y);

	// have middle-mouse (aka wheel-click) change focus without
	// moving caret within the panel receiving focus.
	//
	// we DO NOT set patch highlight.

	SetFocus();
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::_do_context_menu__mark(long yRow, wxMouseEvent & e)
{
	// right-click on a de_mark row.
	// 
	// if plain(unshifted) popup a context menu to ask if they want to delete the mark.
	// if shifted, assume they do.

	if (e.ShiftDown())
	{
		m_pViewFile->deleteMark(m_kSync,m_rowRightMouse);
		return;
	}
	
	// raise modal pop up menu.  we get an _onMenuEvent_CTX_{...}() callback
	// if the user selects anything.

	m_rowRightMouse = yRow;

	wxMenu menu;
	menu.Append(CTX_DIALOG_MARK, _("&Manual Alignment Markers..."));
	menu.Append(CTX_DELETE_MARK, _("&Delete Alignment Marker"));

	PopupMenu(&menu);
}

void ViewFilePanel::_do_context_menu__selection(void)
{
	// raise traditional context menu with cut/copy/paste when we have
	// a selection. contents change depending whether SYNC_VIEW or SYNC_EDIT.

	wxMenu menu;

	if ((m_kSync == SYNC_EDIT) && (m_kPanel == PANEL_EDIT))
	{
		menu.Append(CTX_CUT, _("Cu&t"));
		menu.Append(CTX_COPY, _("&Copy"));
		menu.Append(CTX_PASTE, _("&Paste"));
	}
	else
	{
		menu.Append(CTX_COPY, _("&Copy"));
	}
	menu.Append(CTX_SELECT_ALL, _("Select &All"));

	PopupMenu(&menu);
}

void ViewFilePanel::_do_context_menu__nonselection(int row, bool bOnPatch)
{
	// raise traditional context menu with cut/copy/paste
	// when we DO NOT have a selection.
	// contents change depending whether SYNC_VIEW or SYNC_EDIT.

	wxMenu menu;

	if ((m_kSync == SYNC_EDIT) && (m_kPanel == PANEL_EDIT))
		menu.Append(CTX_PASTE, _("&Paste"));
	menu.Append(CTX_SELECT_ALL, _("Select &All"));
	menu.AppendSeparator();

	if (bOnPatch)
	{
		menu.Append(CTX_SELECT_PATCH,_("&Select Change"));
		menu.AppendSeparator();
	}

	menu.Append(CTX_NEXT_CHANGE, _("&Next Change"));
	menu.Enable(CTX_NEXT_CHANGE, (m_pViewFile->getNextChange(false,row) != -1));
	menu.Append(CTX_PREV_CHANGE, _("P&revious Change"));
	menu.Enable(CTX_PREV_CHANGE, (m_pViewFile->getPrevChange(false,row) != -1));
	bool bMerge = (m_pViewFile->getNrTopPanels() == 3);
	if (bMerge)
	{
		menu.AppendSeparator();
		menu.Append(CTX_NEXT_CONFLICT, _("Ne&xt Conflict"));
		menu.Enable(CTX_NEXT_CONFLICT, (m_pViewFile->getNextChange(true,row) != -1));
		menu.Append(CTX_PREV_CONFLICT, _("Pre&vious Conflict"));
		menu.Enable(CTX_PREV_CONFLICT, (m_pViewFile->getPrevChange(true,row) != -1));
	}

	m_rowRightMouse = row;

	PopupMenu(&menu);
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::onMouseRightDown(wxMouseEvent & e)
{
	if (!isBound())		// ignore click if we haven't been completely set up yet
		return;

	SetFocus();
	m_bWeCapturedTheMouse = false;

	//wxLogTrace(wxTRACE_Messages, _T("ViewFilePanel:onMouseRightDown: pos [%d,%d]"),e.m_x,e.m_y);

	int row,col, vscroll,hscroll;
	_mapMouseEventToCoord(e.m_x,e.m_y,&row,&vscroll,&col,&hscroll);
	ViewFileCoord vfcClicked(row,col);

	if (haveSelection() && _withinSelection(row,col))
	{
		// if we have a selection (and the right-mouse click is
		// within it, show a cut/copy/paste type menu
		// (ignoring the fact that there may be patches here).

		_do_context_menu__selection();
		return;
	}

	// when no selection or not within the selection, we clear the
	// selection (if set) and move the caret to the click.  then
	// see what else we can do.
		
	_doUpdateCaret(false,vfcClicked);
	_setGoal(vfcClicked);
	_rememberMousePosition(false,vfcClicked);
	Refresh(true);
	Update();
		
	if (m_pDeDe->isMark(m_kSync,row))
	{
		bool bCanRaiseDlg = m_pViewFile->getFrame()->isViewFileInsertMarkEnabled();
		if (!bCanRaiseDlg)	// can't raise it because it's already up
			return;
		
		_do_context_menu__mark(row,e);
		return;
	}

	// otherwise, we have a content row.

	// e.ControlDown():
	// normally, we extend around the clicked row to get the complete
	// context of the patch (usually only significant when in a complex
	// conflict).  when the control key is down, we allow them to restrict
	// to just this sync-node (this should let them pick-n-choose within
	// the conflict, for example).
	//
	// e.ShiftDown():
	// if there is a default action go ahead and try to apply it without
	// raising the context menu.

#if defined(__WXOSX__)
	bool bControlDown = e.RawControlDown();	// see note in __kb_mac.cpp
#else
	bool bControlDown = e.ControlDown();
#endif		

	_raise_content_context_menu(row,bControlDown,e.ShiftDown());
}

void ViewFilePanel::_raise_content_context_menu(int row, bool bDontExtend, bool bAutoApply)
{
	// warning: this is not just used during a right-mouse.  we also use it
	// warning: when the windows-menu-key is used.

	if (!m_pDeDe->isPatch(m_kSync,row))
	{
		// the row they clicked on is not part of a change/conflict.
		// if we already had a patch highlighted, clear it.

		m_pDeDe->unsetPatchHighlight(m_kSync);

		_do_context_menu__nonselection(row,false);
		return;
	}

	// user clicked on a change/conflict row.

	// cause dotted lines to appear around bounds of patch
	// (we assume that when the popup menu appears that we'll get a
	// paint on all 2 or 3 panels (so that the dotted line gets shown)
	// as we wait for the user to pick something from the menu.

	m_pDeDe->setPatchHighlight(m_kSync,row,bDontExtend);

	// when on SYNC_VIEW, we only give them the traditional cut/copy/paste stuff.

	if (m_kSync == SYNC_VIEW)
	{
		bool bIsVoid = m_pDeDe->isPatchAVoid(m_kSync,m_kPanel);	// if non void also allow select-patch
		_do_context_menu__nonselection(row, !bIsVoid );
		return;
	}

	// compute the context menu (and possible default action) for a
	// click on this patch.

	wxMenu menu;
	int defaultAction = _build_context_menu(&menu);

	if (defaultAction == CTX__NO_MENU)			// they clicked on a type of void that doesn't have any operations.
	{
		_do_context_menu__nonselection(row,false);
		return;
	}

	if (bAutoApply && (defaultAction != CTX__NO_DEFAULT))
	{
		// when auto-apply, we try to go ahead and apply the
		// default patch-operation without raising the menu.

		doPatchOperation(defaultAction);
		return;
	}

	menu.AppendSeparator();
	if ((m_kSync==SYNC_EDIT) && (m_kPanel==PANEL_EDIT))
		menu.Append(CTX_PASTE, _("&Paste"));
	menu.Append(CTX_SELECT_ALL, _("Select &All"));
	menu.AppendSeparator();

	bool bIsVoid = m_pDeDe->isPatchAVoid(m_kSync,m_kPanel);
	if (!bIsVoid)
	{
		menu.Append(CTX_SELECT_PATCH,_("&Select Change"));
		menu.AppendSeparator();
	}
	
	menu.Append(CTX_NEXT_CHANGE, _("&Next Change"));
	menu.Enable(CTX_NEXT_CHANGE, (m_pViewFile->getNextChange(false,row) != -1));
	menu.Append(CTX_PREV_CHANGE, _("P&revious Change"));
	menu.Enable(CTX_PREV_CHANGE, (m_pViewFile->getPrevChange(false,row) != -1));
	bool bMerge = (m_pViewFile->getNrTopPanels() == 3);
	if (bMerge)
	{
		menu.AppendSeparator();
		menu.Append(CTX_NEXT_CONFLICT, _("Ne&xt Conflict"));
		menu.Enable(CTX_NEXT_CONFLICT, (m_pViewFile->getNextChange(true,row) != -1));
		menu.Append(CTX_PREV_CONFLICT, _("Pre&vious Conflict"));
		menu.Enable(CTX_PREV_CONFLICT, (m_pViewFile->getPrevChange(true,row) != -1));
	}
	
	// remember row during context menu so that next/prev can work.

	m_rowRightMouse = row;

	// raise modal pop up menu.  we get an _onMenuEvent_CTX_{...}() callback
	// if the user selects anything.

	PopupMenu(&menu);
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::onMouseMotion(wxMouseEvent & e)
{
	if (!isBound())		// ignore click if we haven't been completely set up yet
		return;

	//wxLogTrace(wxTRACE_Messages, _T("ViewFilePanel:onMouseMotion: pos [%d,%d]"),e.m_x,e.m_y);

	if (HasCapture() && m_bWeCapturedTheMouse)
	{
		int row,col, vscroll,hscroll;
		_mapMouseEventToCoord(e.m_x,e.m_y,&row,&vscroll,&col,&hscroll);

		wxASSERT_MSG( (m_vfcAnchor.isSet()), _T("Coding Error"));
		ViewFileCoord vfcMotion(row,col);

		//////////////////////////////////////////////////////////////////
		// TODO consider extending selection by "word" when m_bDClick is set.
		// TODO also need to update onTimerEvent if we do this.
		//////////////////////////////////////////////////////////////////

		if (!m_bHaveMotion || (m_vfcMouse.compare(vfcMotion) != 0) || (vscroll != 0) || (hscroll != 0))
		{
			if (m_bDClick)
			{
				// mouse motion following a double-click.
				// when we received the dclick, we did a word-selection.
				// now we want to extend the selection -- BUT always keep
				// the initial word selected.

				wxASSERT_MSG( (m_vfcWordSelection0.isSet()), _T("Coding Error"));
				wxASSERT_MSG( (m_vfcWordSelection1.isSet()), _T("Coding Error"));
				
				if (vfcMotion.compare(m_vfcWordSelection0) < 0)
				{
					// mouse is before start of initial word; select from
					// mouse to end of initial word.
					m_vfcAnchor.set(m_vfcWordSelection1);
					_doUpdateCaret(true,vfcMotion);
				} else if (vfcMotion.compare(m_vfcWordSelection1) > 0)
				{
					// mouse is past end of initial word; select from start
					// of initial word to current mouse position.
					m_vfcAnchor.set(m_vfcWordSelection0);
					_doUpdateCaret(true,vfcMotion);
				}
				else
				{
					// mouse is within the initial word.  keep the initial
					// word selected.
					m_vfcAnchor.set(m_vfcWordSelection0);
					_doUpdateCaret(true,m_vfcWordSelection1);
				}
			}
			else
			{
				// mouse motion following a single-click.  extend the
				// selection to the current mouse position from the
				// initial click position.

				_doUpdateCaret(true,vfcMotion);
			}

			_setGoal(vfcMotion);
			_rememberMousePosition(true,vfcMotion);

			if ((vscroll != 0) || (hscroll != 0))
			{
				// auto-scroll some.  we don't need to Refresh/Update here because
				// that will happen during the scroll event.

				m_pViewFile->mouseScroll( m_kSync, vscroll, hscroll);

				if (!m_timer_MyMouse.IsRunning())
				{
//					wxLogTrace(wxTRACE_Messages,_T("ViewFilePanel::onMouseMotion: starting timer because mouse outside client area"));
					m_timer_MyMouse.Start(AUTO_SCROLL_TIMER_RATE_MS,wxTIMER_CONTINUOUS);
				}
			}
			else
			{
				if (m_timer_MyMouse.IsRunning())	// if the mouse is within the document area proper
				{
//					wxLogTrace(wxTRACE_Messages,_T("ViewFilePanel::onMouseMotion: stopping timer because mouse inside client area (or can't scroll any more)"));
					m_timer_MyMouse.Stop();			// or we didn't need/can't scroll any turn off the timer.
				}
				
				Refresh(true);
				Update();
			}
		}
	}
	
	e.Skip();
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::onMouseEnterWindow(wxMouseEvent & e)
{
//	wxLogTrace(wxTRACE_Messages, _T("ViewFilePanel:onMouseEnterWindow: pos [%d,%d]"),e.m_x,e.m_y);
	e.Skip();
}

void ViewFilePanel::onMouseLeaveWindow(wxMouseEvent & e)
{
	e.Skip();
	if (!isBound())		// ignore click if we haven't been completely set up yet
		return;

//	wxLogTrace(wxTRACE_Messages, _T("ViewFilePanel:onMouseLeaveWindow: pos [%d,%d]"),e.m_x,e.m_y);

	// wxBUG: on the MAC, we get enter/leave events -- even when we have the mouse captured --
	// wxBUG: when the mouse leaves our containing frame window.  and after the leave event
	// wxBUG: we don't get mouse motion events again until the mouse re-enters our containing
	// wxBUG: frame window.
	//
	// this causes us the auto-scroll-while-dragging-out-a-selection to stop working.
	// we have to rely on the auto-scroll-timer to keep it going.
	//
	// [on the other platforms, we only get this event when the mouse leaves and we don't have
	// capture.]
	//
	// also, on Win32, we get a leave-window event when the popup context menu appears, so
	// don't start the timer unless we really mean to.

	if (HasCapture() && m_bWeCapturedTheMouse)
		if (!m_timer_MyMouse.IsRunning())
		{
//			wxLogTrace(wxTRACE_Messages, _T("ViewFilePanel::starting timer because of leave-window"));
			m_timer_MyMouse.Start(AUTO_SCROLL_TIMER_RATE_MS,wxTIMER_CONTINUOUS);
		}
	
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::onMouseEventWheel(wxMouseEvent & e)
{
	if (!isBound())		// ignore click if we haven't been completely set up yet
		return;

	int orientation;
	int sign;
	if (e.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL)
	{
		orientation = wxVERTICAL;
		sign = +1;
	}
	else
	{
		orientation = wxHORIZONTAL;
		sign = -1;
	}
		
//	wxLogTrace(wxTRACE_Messages,_T("ViewFilePanel::onMouseEventWheel: [axis %c][rotation %d][delta %d][lpa %d]"),
//			   ((orientation) ? _T('V') : _T('H')),
//			   e.GetWheelRotation(),e.GetWheelDelta(),e.GetLinesPerAction());

	int units = e.GetWheelRotation() / e.GetWheelDelta();
	int lines = sign * units * e.GetLinesPerAction();
	long yRowThumb = m_pViewFile->getScrollThumbCharPosition(m_kSync, orientation);
	long newThumb = yRowThumb - lines;
	if (newThumb < 0)
		newThumb = 0;

	m_pViewFile->adjustScrollbar(m_kSync, orientation, newThumb);
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::onTimerEvent_MyMouse(wxTimerEvent & /*e*/)
{
//	wxLogTrace(wxTRACE_Messages, _T("ViewFilePanel:Timer: [int %d ms]"), e.GetInterval());

	if (!HasCapture())
	{
//		wxLogTrace(wxTRACE_Messages,_T("ViewFilePanel::stopping timer because no capture"));
		m_timer_MyMouse.Stop();
		return;
	}

	// simmulate mouse-motion-auto-repeat
	
	wxPoint ptScreen = ::wxGetMousePosition();
	wxPoint ptClient = ScreenToClient(ptScreen);

	int row,col, vscroll,hscroll;
	_mapMouseEventToCoord(ptClient.x,ptClient.y,&row,&vscroll,&col,&hscroll);
	ViewFileCoord vfcTick(row,col);

	_doUpdateCaret(true,vfcTick);
	_setGoal(vfcTick);
	_rememberMousePosition(true,vfcTick);
	
	if ((vscroll != 0) || (hscroll != 0))
	{
		// auto-scroll some.  we don't need to Refresh/Update here because
		// that will happen during the scroll event.

		m_pViewFile->mouseScroll( m_kSync, vscroll, hscroll);
	}
	else
	{
		Refresh(true);
		Update();
	}
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::_rememberMousePosition(bool bMotionEvent, const ViewFileCoord & vfc)
{
	m_bHaveMotion = bMotionEvent;
	m_vfcMouse.set(vfc);
}

//////////////////////////////////////////////////////////////////

int ViewFilePanel::_build_context_menu__edit(wxMenu * pMenu)
{
	// create context menus when user clicks on PANEL_EDIT (aka _T1)
	// return value of default choice.

	wxASSERT_MSG( (m_kPanel==PANEL_EDIT), _T("Coding Error") );

	bool bHaveVoidED = m_pDeDe->isPatchAVoid(m_kSync,PANEL_T1);
	bool bHaveVoidT0 = m_pDeDe->isPatchAVoid(m_kSync,PANEL_T0);

	bool bMerge = (m_pViewFile->getNrTopPanels() == 3);
	
	if (!bMerge)	// a 2-way diff
	{
		wxASSERT_MSG( (!bHaveVoidED || !bHaveVoidT0), _T("Coding Error") );

		if (bHaveVoidED)
		{
			// we have
			// T0  T1
			// ==  ==
			// aa  ..

			if (pMenu)
				pMenu->Append(CTX_INSERT_L, _("Insert from Left (default)"));
			return CTX_INSERT_L;
		}

		if (bHaveVoidT0)
		{
			// we have
			// T0  T1
			// ==  ==
			// ..  aa

			if (pMenu)
				pMenu->Append(CTX_DELETE, _("Delete This (default)"));
			return CTX_DELETE;
		}

		// we have
		// T0  T1
		// ==  ==
		// aa  bb

		if (pMenu)
		{
			pMenu->Append(CTX_REPLACE_L,		_("Replace with Left (default)"));
			pMenu->Append(CTX_INSERT_BEFORE_L,	_("Prepend from Left"));
			pMenu->Append(CTX_INSERT_AFTER_L,	_("Append from Left"));
		}
		
		return CTX_REPLACE_L;
	}

	bool bHaveVoidT2 = m_pDeDe->isPatchAVoid(m_kSync,PANEL_T2);
	wxASSERT_MSG( (!bHaveVoidED || !bHaveVoidT0 || !bHaveVoidT2), _T("Coding Error") );

	if (bHaveVoidED)
	{
		// we have
		// T0  T1  T2    or    T0  T1  T2    or    T0  T1  T2    or    T0  T1  T2
		// ==  ==  ==          ==  ==  ==          ==  ==  ==          ==  ==  ==
		// aa  ..  ..          ..  ..  aa          aa  ..  aa          aa  ..  bb

		if (!bHaveVoidT0 && !bHaveVoidT2)
		{
			// we have
			// T0  T1  T2    or    T0  T1  T2
			// ==  ==  ==          ==  ==  ==
			// aa  ..  aa          aa  ..  bb

			bool bT0T2Eq = m_pDeDe->isPatchEqual(m_kSync,PANEL_T0,PANEL_T2);
			if (bT0T2Eq)
			{
				if (pMenu)
					pMenu->Append(CTX_INSERT_L, _("Insert (default)"));
				return CTX_INSERT_L;
			}

			if (pMenu)
			{
				pMenu->SetTitle(_("Conflict (no default)"));
				pMenu->Append(CTX_INSERT_L, _("Insert from Left"));
				pMenu->Append(CTX_INSERT_R, _("Insert from Right"));
			}
			return CTX__NO_DEFAULT;
		}

		// we have
		// T0  T1  T2    or    T0  T1  T2
		// ==  ==  ==          ==  ==  ==
		// aa  ..  ..          ..  ..  aa

		if (!bHaveVoidT0)
		{
			if (pMenu)
				pMenu->Append(CTX_INSERT_L, _("Insert from Left (default)"));
			return CTX_INSERT_L;
		}

		if (pMenu)
			pMenu->Append(CTX_INSERT_R, _("Insert from Right (default)"));
		return CTX_INSERT_R;
	}

	// T1 is non-void

	if (bHaveVoidT0 && bHaveVoidT2)
	{
		// we have
		// T0  T1  T2
		// ==  ==  ==
		// ..  aa  ..

		if (pMenu)
			pMenu->Append(CTX_DELETE, _("Delete This (default)"));
		return CTX_DELETE;
	}

	if (bHaveVoidT0)
	{
		// we have
		// T0  T1  T2    or    T0  T1  T2
		// ==  ==  ==          ==  ==  ==
		// ..  aa  aa          ..  aa  bb

		bool bT1T2Eq = m_pDeDe->isPatchEqual(m_kSync,PANEL_T1,PANEL_T2);
		if (bT1T2Eq)
		{
			if (pMenu)
				pMenu->Append(CTX_DELETE, _("Delete This (default)"));
			return CTX_DELETE;
		}
		
		// we have
		// T0  T1  T2
		// ==  ==  ==
		// ..  aa  bb

		if (pMenu)
		{
			pMenu->SetTitle(_("Conflict (no default)"));
			pMenu->Append(CTX_DELETE, _("Delete This"));
			pMenu->AppendSeparator();
			pMenu->Append(CTX_INSERT_BEFORE_R,	_("Prepend from Right"));
			pMenu->Append(CTX_REPLACE_R,		_("Replace with Right"));
			pMenu->Append(CTX_INSERT_AFTER_R,	_("Append from Right"));
		}
		return CTX__NO_DEFAULT;
	}
	
	if (bHaveVoidT2)
	{
		// we have
		// T0  T1  T2    or    T0  T1  T2
		// ==  ==  ==          ==  ==  ==
		// aa  aa  ..          bb  aa  ..

		bool bT1T0Eq = m_pDeDe->isPatchEqual(m_kSync,PANEL_T1,PANEL_T0);
		if (bT1T0Eq)
		{
			if (pMenu)
				pMenu->Append(CTX_DELETE, _("Delete This (default)"));
			return CTX_DELETE;
		}
		
		// we have
		// T0  T1  T2
		// ==  ==  ==
		// bb  aa  ..

		if (pMenu)
		{
			pMenu->SetTitle(_("Conflict (no default)"));
			pMenu->Append(CTX_DELETE, _("Delete This"));
			pMenu->AppendSeparator();
			pMenu->Append(CTX_INSERT_BEFORE_L,	_("Prepend from Left"));
			pMenu->Append(CTX_REPLACE_L,		_("Replace with Left"));
			pMenu->Append(CTX_INSERT_AFTER_L,	_("Append from Left"));
		}
		return CTX__NO_DEFAULT;
	}

	// all 3 panels have content

	bool bT1T0Eq = m_pDeDe->isPatchEqual(m_kSync,PANEL_T1,PANEL_T0);
	bool bT1T2Eq = m_pDeDe->isPatchEqual(m_kSync,PANEL_T1,PANEL_T2);
	bool bT0T2Eq = m_pDeDe->isPatchEqual(m_kSync,PANEL_T0,PANEL_T2);

	wxASSERT_MSG( (! (bT1T0Eq && bT1T2Eq && bT0T2Eq)), _T("Coding Error") );

	if (bT1T0Eq)
	{
		// we have
		// T0  T1  T2
		// ==  ==  ==
		// aa  aa  bb

		if (pMenu)
		{
			pMenu->Append(CTX_REPLACE_R,		_("Replace with Right (default)"));
			pMenu->Append(CTX_INSERT_BEFORE_R,	_("Prepend from Right"));
			pMenu->Append(CTX_INSERT_AFTER_R,	_("Append from Right"));
		}
		return CTX_REPLACE_R;
	}
	if (bT1T2Eq)
	{
		// we have
		// T0  T1  T2
		// ==  ==  ==
		// bb  aa  aa

		if (pMenu)
		{
			pMenu->Append(CTX_REPLACE_L,		_("Replace with Left (default)"));
			pMenu->Append(CTX_INSERT_BEFORE_L,	_("Prepend from Left"));
			pMenu->Append(CTX_INSERT_AFTER_L,	_("Append from Left"));
		}
		return CTX_REPLACE_L;
	}
	if (bT0T2Eq)
	{
		// we have
		// T0  T1  T2
		// ==  ==  ==
		// aa  bb  aa
		
		if (pMenu)
		{
			pMenu->Append(CTX_REPLACE_L,		_("Replace (default)"));
			pMenu->Append(CTX_INSERT_BEFORE_L,	_("Prepend"));
			pMenu->Append(CTX_INSERT_AFTER_L,	_("Append"));
		}
		return CTX_REPLACE_L;
	}
	
	// we have
	// T0  T1  T2
	// ==  ==  ==
	// aa  bb  cc

	if (pMenu)
	{
		pMenu->SetTitle(_("Conflict (no default)"));
		pMenu->Append(CTX_REPLACE_L,		_("Replace with Left"));
		pMenu->Append(CTX_INSERT_BEFORE_L,	_("Prepend from Left"));
		pMenu->Append(CTX_INSERT_AFTER_L,	_("Append from Left"));
		pMenu->AppendSeparator();
		pMenu->Append(CTX_REPLACE_R,		_("Replace with Right"));
		pMenu->Append(CTX_INSERT_BEFORE_R,	_("Prepend from Right"));
		pMenu->Append(CTX_INSERT_AFTER_R,	_("Append from Right"));
	}
	return CTX__NO_DEFAULT;
}

int ViewFilePanel::_build_context_menu__t0(wxMenu * pMenu)
{
	// create context menus when user clicks on PANEL_T0
	// returns value of default choice.

	wxASSERT_MSG( (m_kPanel==PANEL_T0), _T("Coding Error") );

	bool bHaveVoidED = m_pDeDe->isPatchAVoid(m_kSync,PANEL_T1);

	// NOTE as of OS X (El Capitan), the following call was causing an assertion.
	// The line was modified to avoid a check of the assertion until it can be 
	// determined what exactly this code is doing.
	bool bHaveVoidT0 = m_pDeDe->isPatchAVoid(m_kSync,PANEL_T0,false);
	// NOTE as of OS X (El Capitan), the previous line was causing an assertion.

	if (bHaveVoidT0)
	{
		// we have
		// T0  T1  [T2]   or   T0  T1  T2
		// ==  ==  [==]        ==  ==  ==
		// ..  aa  [??]        ..  ..  aa

		if (bHaveVoidED)
			return CTX__NO_MENU;
		
		if (pMenu)
			pMenu->Append(CTX_DELETE, _("Delete (default)"));
		return CTX_DELETE;
	}

	if (bHaveVoidED)
	{
		// we have
		// T0  T1  [T2]
		// ==  ==  [==]
		// aa  ..  [??]

		if (pMenu)
			pMenu->Append(CTX_INSERT_L, _("Insert This (default)"));
		return CTX_INSERT_L;
	}

	// we have
	// T0  T1  [T2]    or    T0  T1  T2
	// ==  ==  [==]          ==  ==  ==
	// aa  bb  [??]          aa  aa  ??
	//
	// that is T0 and T1 must be different when we have a 2-way.
	// they may or may not be equal when we have a 3-way.

	bool bMerge = (m_pViewFile->getNrTopPanels() == 3);
	if (bMerge)
	{
		bool bT1T0Eq = m_pDeDe->isPatchEqual(m_kSync,PANEL_T1,PANEL_T0);
		if (bT1T0Eq)
			return CTX__NO_MENU;
	}

	if (pMenu)
	{
		pMenu->Append(CTX_REPLACE_L,		_("Replace with This (default)"));
		pMenu->Append(CTX_INSERT_BEFORE_L,	_("Prepend This"));
		pMenu->Append(CTX_INSERT_AFTER_L,	_("Append This"));
	}
	return CTX_REPLACE_L;
}
		

int ViewFilePanel::_build_context_menu__t2(wxMenu * pMenu)
{
	// create context menus when user clicks on PANEL_T2
	// returns value of default choice.

	wxASSERT_MSG( (m_kPanel==PANEL_T2), _T("Coding Error") );

	bool bHaveVoidED = m_pDeDe->isPatchAVoid(m_kSync,PANEL_T1);
	bool bHaveVoidT2 = m_pDeDe->isPatchAVoid(m_kSync,PANEL_T2);

	if (bHaveVoidT2)
	{
		// we have
		// [T0]  T1  T2   or   T0  T1  T2
		// [==]  ==  ==        ==  ==  ==
		// [??]  aa  ..        aa  ..  ..

		if (bHaveVoidED)
			return CTX__NO_MENU;

		if (pMenu)
			pMenu->Append(CTX_DELETE, _("Delete (default)"));
		return CTX_DELETE;
	}

	if (bHaveVoidED)
	{
		// we have
		// [T0]  T1  T2
		// [==]  ==  ==
		// [??]  ..  aa

		if (pMenu)
			pMenu->Append(CTX_INSERT_R, _("Insert This (default)"));
		return CTX_INSERT_R;
	}

	// we have
	// [T0]  T1  T2    or    [T0]  T1  T2
	// [==]  ==  ==          [==]  ==  ==
	// [??]  bb  aa          [??]  aa  aa

	bool bT1T2Eq = m_pDeDe->isPatchEqual(m_kSync,PANEL_T1,PANEL_T2);
	if (bT1T2Eq)
		return CTX__NO_MENU;
	

	if (pMenu)
	{
		pMenu->Append(CTX_REPLACE_R,		_("Replace with This (default)"));
		pMenu->Append(CTX_INSERT_BEFORE_R,	_("Prepend This"));
		pMenu->Append(CTX_INSERT_AFTER_R,	_("Append This"));
	}
	return CTX_REPLACE_R;
}
		
int ViewFilePanel::_build_context_menu(wxMenu * pMenu)
{
	wxASSERT_MSG( (getSync() == SYNC_EDIT), _T("Coding Error") );

	int defaultAction;

	switch (m_kPanel)
	{
	case PANEL_EDIT:	defaultAction = _build_context_menu__edit(pMenu);		return defaultAction;
	case PANEL_T0:		defaultAction = _build_context_menu__t0(pMenu);			return defaultAction;
	case PANEL_T2:		defaultAction = _build_context_menu__t2(pMenu);			return defaultAction;
	default:			wxASSERT_MSG( (0), _T("Coding Error") );				return CTX__NO_MENU;
	}
}

//////////////////////////////////////////////////////////////////

int ViewFilePanel::computeDefaultPatchAction(void)
{
	return _build_context_menu(NULL);
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::_onMenuEvent_CTX_DELETE(wxCommandEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages,_T("ViewFilePanel::_onMenuEvent_CTX_DELETE:"));
	doPatchOperation(CTX_DELETE);
}

void ViewFilePanel::_onMenuEvent_CTX_INSERT_L(wxCommandEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages,_T("ViewFilePanel::_onMenuEvent_CTX_INSERT_L:"));
	doPatchOperation(CTX_INSERT_L);
}

void ViewFilePanel::_onMenuEvent_CTX_INSERT_BEFORE_L(wxCommandEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages,_T("ViewFilePanel::_onMenuEvent_CTX_INSERT_BEFORE_L:"));
	doPatchOperation(CTX_INSERT_BEFORE_L);
}

void ViewFilePanel::_onMenuEvent_CTX_INSERT_AFTER_L(wxCommandEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages,_T("ViewFilePanel::_onMenuEvent_CTX_INSERT_AFTER_L:"));
	doPatchOperation(CTX_INSERT_AFTER_L);
}

void ViewFilePanel::_onMenuEvent_CTX_REPLACE_L(wxCommandEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages,_T("ViewFilePanel::_onMenuEvent_CTX_REPLACE_L:"));
	doPatchOperation(CTX_REPLACE_L);
}

void ViewFilePanel::_onMenuEvent_CTX_INSERT_R(wxCommandEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages,_T("ViewFilePanel::_onMenuEvent_CTX_INSERT_R:"));
	doPatchOperation(CTX_INSERT_R);
}

void ViewFilePanel::_onMenuEvent_CTX_INSERT_BEFORE_R(wxCommandEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages,_T("ViewFilePanel::_onMenuEvent_CTX_INSERT_BEFORE_R:"));
	doPatchOperation(CTX_INSERT_BEFORE_R);
}

void ViewFilePanel::_onMenuEvent_CTX_INSERT_AFTER_R(wxCommandEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages,_T("ViewFilePanel::_onMenuEvent_CTX_INSERT_AFTER_R:"));
	doPatchOperation(CTX_INSERT_AFTER_R);
}

void ViewFilePanel::_onMenuEvent_CTX_REPLACE_R(wxCommandEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages,_T("ViewFilePanel::_onMenuEvent_CTX_REPLACE_R:"));
	doPatchOperation(CTX_REPLACE_R);
}

void ViewFilePanel::_onMenuEvent_CTX_DELETE_MARK(wxCommandEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages,_T("ViewFilePanel::_onMenuEvent_CTX_DELETE_MARK:"));

	if (m_rowRightMouse == -1)		// if bogus row, just ignore -- should not happen
		return;

	m_pViewFile->deleteMark(getSync(),m_rowRightMouse);
}

void ViewFilePanel::_onMenuEvent_CTX_DIALOG_MARK(wxCommandEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages,_T("ViewFilePanel::_onMenuEvent_CTX_DIALOG_MARK:"));

	if (m_rowRightMouse == -1)		// if bogus row, just ignore -- should not happen
		return;

	bool bCanRaiseDlg = m_pViewFile->getFrame()->isViewFileInsertMarkEnabled();
	if (!bCanRaiseDlg)	// can't raise it because it's already up
		return;

	de_mark * pDeMark = NULL;
	if (!m_pDeDe->isMark(m_kSync,m_rowRightMouse,&pDeMark))
		return;
	
	m_pViewFile->getFrame()->showInsertMarkDialog(pDeMark);
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::doPatchOperation(int op)
{
	// WARNING: because we allow context menus on all 3 panels, we can
	// WARNING: get here from any of them -- even though we only actually
	// WARNING: edit on the center panel.

//	wxLogTrace(wxTRACE_Messages,_T("ViewFilePanel::doPatchOperation: [%d]"), op);

	// get the patch bounds -- in display-list row-space
	// these are independent of panel.

	ViewFilePanel * pPanelEdit = m_pViewFile->getPanel(SYNC_EDIT,PANEL_EDIT);
	fim_ptable * pPTableEdit = m_pViewFile->getPTable(SYNC_EDIT,PANEL_EDIT);

	fim_offset docPosEditStart, docPosEditEnd;
	bool bValidStartPos = m_pDeDe->getPatchStartDocPosition(SYNC_EDIT,PANEL_EDIT,&docPosEditStart);
	bool bValidEndPos   = m_pDeDe->getPatchEndDocPosition(SYNC_EDIT,PANEL_EDIT,&docPosEditEnd);

	wxString strSource;

	switch (op)
	{
	default:
		MY_ASSERT( (0) );
		return;

	case CTX_DELETE:	// simple delete from the edit panel
		MY_ASSERT( (bValidStartPos && bValidEndPos &&  docPosEditEnd > docPosEditStart) );
		pPTableEdit->deleteText(docPosEditStart, docPosEditEnd - docPosEditStart);
		pPanelEdit->setBogusCaret();
		return;

	case CTX_INSERT_L:
	case CTX_INSERT_BEFORE_L:
	case CTX_INSERT_AFTER_L:
	case CTX_REPLACE_L:
		strSource = m_pDeDe->getPatchSrcString(SYNC_EDIT,PANEL_T0);
		break;

	case CTX_INSERT_R:
	case CTX_INSERT_BEFORE_R:
	case CTX_INSERT_AFTER_R:
	case CTX_REPLACE_R:
		strSource = m_pDeDe->getPatchSrcString(SYNC_EDIT,PANEL_T2);
		break;
	}

	switch (op)
	{
	default:
	//case CTX_DELETE:
		return;

	case CTX_INSERT_L:	// insert contents of change/conflict from T0 into a VOID in the edit panel
	case CTX_INSERT_R:	// insert contents of change/conflict from T2 into a VOID in the edit panel
		MY_ASSERT( (bValidEndPos && !bValidStartPos) );
	case CTX_INSERT_AFTER_L:	// insert contents of change/conflict from T0 after the contents in the edit panel
	case CTX_INSERT_AFTER_R:	// insert contents of change/conflict from T2 after the contents in the edit panel
		MY_ASSERT( (bValidEndPos) );
		if (!pPTableEdit->hasFinalEOL() && !m_pDeDe->getPatchLineNrAfter(SYNC_EDIT,PANEL_EDIT,NULL) && (docPosEditEnd > 0))
		{
			// bug:11149
			// if the patch that we're about apply will insert text at the end
			// of the file and the edit buffer doesn't currently have a final EOL,
			// then we need to prepend an EOL to the text we insert so that it
			// won't start up on the end of the last partial line in the buffer.
			//
			// bug:12309
			// require docPosEditEnd > 0 -- to ensure that we don't prepend a EOL
			// before inserting into an empty document.

			wxString strEditEOL;
			fim_eol_mode modeEdit = pPTableEdit->getEolMode();
			switch (modeEdit)
			{
			default:					strEditEOL = FIM_MODE_NATIVE_DISK_STR;	wxASSERT_MSG( (0), _T("Coding Error") ); 	break;	// should not happen
			case FIM_MODE_LF:			strEditEOL = _T("\n");					break;
			case FIM_MODE_CRLF:			strEditEOL = _T("\r\n");				break;
			case FIM_MODE_CR:			strEditEOL = _T("\r");					break;
			}

			strSource.Prepend(strEditEOL);
		}
		pPTableEdit->insertText(docPosEditEnd,FR_PROP__INSERTED,strSource);
		break;

	case CTX_INSERT_BEFORE_L:	// insert contents of change/conflict from T0 prior to the contents in the edit panel
	case CTX_INSERT_BEFORE_R:	// insert contents of change/conflict from T2 prior to the contents in the edit panel
		MY_ASSERT( (bValidStartPos) );
		pPTableEdit->insertText(docPosEditStart,FR_PROP__INSERTED,strSource);
		break;

	case CTX_REPLACE_L:			// replace the non-void contents in the edit panel with the the contents of T0
	case CTX_REPLACE_R:			// replace the non-void contents in the edit panel with the the contents of T2
		MY_ASSERT( (bValidStartPos && bValidEndPos &&  docPosEditEnd > docPosEditStart) );
		pPTableEdit->replaceText(docPosEditStart, docPosEditEnd - docPosEditStart, strSource, FR_PROP__INSERTED);
		break;
	}

	pPanelEdit->setBogusCaret();
}

void ViewFilePanel::doPatchOperationSetCaret(int op)
{
	fim_offset docPosEditEnd;
	bool bValidEndPos = m_pDeDe->getPatchEndDocPosition(SYNC_EDIT,m_kPanel,&docPosEditEnd);

	doPatchOperation(op);

	if (bValidEndPos)
	{
		int row,col;
		if (_mapDocPositionToRowCol(docPosEditEnd,&row,&col))
		{
			SetFocus();
			_doUpdateCaret(false,row,col);
		}
	}
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::_onMenuEvent_CTX_CUT(wxCommandEvent & /*e*/)
{
	wxASSERT_MSG( ((m_kSync==SYNC_EDIT) && (m_kPanel==PANEL_EDIT)), _T("Coding Error") );
	cutToClipboard();
}

void ViewFilePanel::_onMenuEvent_CTX_COPY(wxCommandEvent & /*e*/)
{
	copyToClipboard();
}

void ViewFilePanel::_onMenuEvent_CTX_PASTE(wxCommandEvent & /*e*/)
{
	wxASSERT_MSG( ((m_kSync==SYNC_EDIT) && (m_kPanel==PANEL_EDIT)), _T("Coding Error") );
	pasteFromClipboard();
}

void ViewFilePanel::_onMenuEvent_CTX_SELECT_ALL(wxCommandEvent & /*e*/)
{
	selectAll();
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::_doNextPrevOperation(int row)
{
	if (row == -1)
		return;

	m_pDeDe->setPatchHighlight(m_kSync,row,false);
	m_pViewFile->warpScrollCentered(m_kSync,row);
	setBogusCaret();
}
	
void ViewFilePanel::_onMenuEvent_CTX_NEXT_CHANGE(wxCommandEvent & /*e*/)
{
	_doNextPrevOperation(m_pViewFile->getNextChange(false,m_rowRightMouse));
}

void ViewFilePanel::_onMenuEvent_CTX_PREV_CHANGE(wxCommandEvent & /*e*/)
{
	_doNextPrevOperation(m_pViewFile->getPrevChange(false,m_rowRightMouse));
}

void ViewFilePanel::_onMenuEvent_CTX_NEXT_CONFLICT(wxCommandEvent & /*e*/)
{
	_doNextPrevOperation(m_pViewFile->getNextChange(true,m_rowRightMouse));
}

void ViewFilePanel::_onMenuEvent_CTX_PREV_CONFLICT(wxCommandEvent & /*e*/)
{
	_doNextPrevOperation(m_pViewFile->getPrevChange(true,m_rowRightMouse));
}

//////////////////////////////////////////////////////////////////

void ViewFilePanel::_onMenuEvent_CTX_SELECT_PATCH(wxCommandEvent & /*e*/)
{
	wxASSERT_MSG( (m_pDeDe->getPatchHighlight(m_kSync)), _T("Coding Error") );

	long rowStart, rowEnd;
	bool bHavePatch = m_pDeDe->getPatchHighlight(m_kSync,NULL,&rowStart,&rowEnd);
	if (!bHavePatch)
	{
		wxASSERT_MSG( (0), _T("Coding Error") );
		return;
	}
#ifdef DEBUG	
	fim_offset docPosStart, docPosEnd;
	if (!m_pDeDe->getPatchStartDocPosition(m_kSync,m_kPanel,&docPosStart)
		|| !m_pDeDe->getPatchEndDocPosition(m_kSync,m_kPanel,&docPosEnd)
		|| (docPosEnd <= docPosStart))
	{
		wxASSERT_MSG( (0), _T("Coding Error") );
		return;
	}
#endif

	_doUpdateCaret(false,rowStart,0);
	_doUpdateCaret(true,rowEnd,0);
	_setGoal(0);
}
