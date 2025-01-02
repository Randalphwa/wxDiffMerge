// ViewFile.cpp
// View onto 2-way/3-way file set.
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
#include <rs.h>
#include <gui.h>

//////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(ViewFile, View)
	EVT_TEXT(      ID_FINDPANEL_TEXT,       ViewFile::onFindPanel_Text)
	EVT_TEXT_ENTER(ID_FINDPANEL_TEXT,       ViewFile::onFindPanel_TextEnter)
	EVT_BUTTON(    ID_FINDPANEL_CLOSE,      ViewFile::onFindPanel_Close)
	EVT_BUTTON(    ID_FINDPANEL_NEXT,       ViewFile::onFindPanel_Next)
	EVT_BUTTON(    ID_FINDPANEL_PREV,       ViewFile::onFindPanel_Prev)
	EVT_CHECKBOX(  ID_FINDPANEL_MATCHCASE,  ViewFile::onFindPanel_MatchCase)
	EVT_TEXT_ENTER(ID_FINDPANEL_GOTO,       ViewFile::onFindPanel_GoToTextEnter)
END_EVENT_TABLE();

//////////////////////////////////////////////////////////////////

static void s_cb_de_changed(void * pThis, const util_cbl_arg & arg)					{ ((ViewFile *)pThis)->cb_de_changed(arg); }
static void s_cb_EditPanelStatusChanged(void * pThis, const util_cbl_arg & arg)		{ ((ViewFile *)pThis)->cb_EditPanelStatusChanged(arg); }
static void s_cb_T1PanelStatusChanged(void * pThis, const util_cbl_arg & arg)		{ ((ViewFile *)pThis)->cb_T1PanelStatusChanged(arg); }

//////////////////////////////////////////////////////////////////

ViewFile::ViewFile(gui_frame * pFrame, Doc * pDoc, const cl_args * pArgs)
	: View(pFrame,pDoc,pArgs),
	  m_pNotebook(NULL),
	  m_pDeDe(NULL),
	  m_chg(0),
	  m_nQueueEventReason(VIEWFILE_QUEUE_EVENT_NOTHING),
	  m_bNeed_WarpSyncViewToFirstChange(false),
	  m_bNeed_WarpSyncEditToFirstChange(false),
	  m_bRaiseAutoMergeDialogAfterLoading(false),
	  m_bTemporarilySuppressActivationChecks(false),
	  m_ptStatEdit(PT_STAT_ZERO),
	  m_pPoiExportDiffsPathname(NULL),
	  m_lastExportToFileFormat(0),
	  m_pPanelFind(NULL), m_pTextFind(NULL),
	  m_pButtonFindClose(NULL), m_pButtonFindNext(NULL), m_pButtonFindPrev(NULL),
	  m_pCheckFindMatchCase(NULL),
	  m_uiDataGoTo(1), m_pTextGoTo(NULL)
{
	// the piecetables don't get created until later,
	// we just need to initialize stuff here and provide
	// a place for later link-up between the diff-engine
	// and panel windows.

	int k;
	for (int kPage=0; kPage<(int)NrElements(m_nbPage); kPage++)
	{
		m_nbPage[kPage].m_pNotebookPanel = NULL;
		m_nbPage[kPage].m_pWinScroll = NULL;
		m_nbPage[kPage].m_pWinGlance = NULL;
		for (k=0; k<(int)NrElements(m_nbPage[0].m_pWinSplitter);	k++)	{ m_nbPage[kPage].m_pWinSplitter[k] = NULL; }
		for (k=0; k<(int)NrElements(m_nbPage[0].m_pWinPanel);		k++)	{ m_nbPage[kPage].m_pWinPanel[k]    = NULL; }
		for (k=0; k<(int)NrElements(m_nbPage[0].m_pWinTitle);		k++)	{ m_nbPage[kPage].m_pWinTitle[k]    = NULL; }

		m_nbPage[kPage].m_panelWithFocus = -1;
	}
	
	m_dop[SYNC_VIEW]	= (de_display_ops)gpGlobalProps->getLong(GlobalProps::GPL_VIEW_FILE_DISPLAY_OP);
	m_dop[SYNC_EDIT]	= DE_DOP_ALL | ((m_dop[SYNC_VIEW] & DE_DOP_IGN_UNIMPORTANT));

	m_bPilcrow			= (gpGlobalProps->getLong(GlobalProps::GPL_VIEW_FILE_PILCROW) != 0);
	m_cTabStop			= gpGlobalProps->getLong(GlobalProps::GPL_VIEW_FILE_TABSTOP);
	m_bShowLineNumbers	= (gpGlobalProps->getLong(GlobalProps::GPL_VIEW_FILE_LINE_NUMBERS) != 0);
	m_currentNBSync		= -1;

	if (pArgs)
	{
		m_bRaiseAutoMergeDialogAfterLoading = pArgs->bMerge;
	}
}

ViewFileDiff::ViewFileDiff(gui_frame * pFrame, Doc * pDoc, const cl_args * pArgs)
	: ViewFile(pFrame,pDoc,pArgs)
{
}

ViewFileMerge::ViewFileMerge(gui_frame * pFrame, Doc * pDoc, const cl_args * pArgs)
	: ViewFile(pFrame,pDoc,pArgs)
{
}

//////////////////////////////////////////////////////////////////

ViewFile::~ViewFile(void)
{
	if (m_pDoc && m_pDoc->getFsFs() && getEditPanelPresent())
	{
		fim_ptable * pPTable = getPTable(SYNC_EDIT,PANEL_EDIT);
		if (pPTable)
			pPTable->delStatusCB(s_cb_EditPanelStatusChanged,this);
	}

	fim_ptable * pPTableT1 = getPTable(SYNC_VIEW,PANEL_T1);
	if (pPTableT1)
		pPTableT1->delStatusCB(s_cb_T1PanelStatusChanged,this);

	if (m_pDeDe)
		m_pDeDe->delChangeCB(s_cb_de_changed,this);

	for (int kPage=0; (kPage < (int)NrElements(m_nbPage)); kPage++)
	{
		for (int kPanel=0; (kPanel < (int)NrElements(m_nbPage[0].m_pWinPanel)); kPanel++)
			if (m_nbPage[kPage].m_pWinPanel[kPanel])
				m_nbPage[kPage].m_pWinPanel[kPanel]->bind_dede_flfl(NULL,NULL);

		if (m_nbPage[kPage].m_pWinGlance)
			m_nbPage[kPage].m_pWinGlance->bind_dede(NULL);
	}

	DELETEP(m_pDeDe);
}

//////////////////////////////////////////////////////////////////

void ViewFile::activated(bool bActive)
{
	// our frame window received an wxActivateEvent

	if (bActive)
	{
		// our frame is now the active window (with highlighted title bar).

		if (m_bTemporarilySuppressActivationChecks)
			return;

		// if our frame has a modeless dialog (like insert-mark, find, goto,
		// etc.) on top of us, suppress activation check until it goes away.
		// (we'll implicitly get one when the modeless dialog is closed and
		// focus returns to the frame.)

		if (m_pFrame->haveModelessDialog())
			return;

		// see if the documents we are presenting have been changed by
		// another application -- if so we should give them the standard
		// "file changed by another application, reload?" dialog...

		// check dtm's, have dlg, queue reload as necessary. [be careful
		// that modal dlg we raise (thus changing activation) doesn't
		// cause us to loop here.]

		// we post an event rather than doing all the work right now
		// so that the window gets fully activated -- and the mouse
		// event causing the activation gets cleared before we start
		// popping up message boxes.  when i did it here directly on XP,
		// the mouse was still captured (or something) because the after
		// the message box was dismissed the mouse still resized the
		// window and/or continued to drag a selection -- very odd.

		if (gpGlobalProps->getBool(GlobalProps::GPL_MISC_CHECK_FILES_ON_ACTIVATE))
			queueEvent(VIEWFILE_QUEUE_EVENT_CHECKFILES);

	}
	else			// our frame is no longer the active window
	{
		// this could be because another app (or another one of our frames)
		// became active -- OR -- it coulde be because we have a modal dialog
		// open on top of us.
	}
}

//////////////////////////////////////////////////////////////////

void ViewFile::cb_de_changed(const util_cbl_arg & arg)
{
	// note: arg.m_p contains nothing.
	// note: arg.m_l contains the DE_CHG_ code for this change.

	// something changed in the diff-engine.

	//wxLogTrace(wxTRACE_Messages,_T("ViewFile::cb_de_changed: [m_chg was %lx] [m_chg is %lx]"),m_chg,(m_chg|arg.m_l));
	m_chg |= arg.m_l;

	// we assume that the ViewFilePanel's are also watching and
	// will invalidate themselves (and cause a PaintEvent).

	_set_display_rs_name();
	if (m_chg & DE_CHG__VIEW_MASK)
		_set_stats_msg(SYNC_VIEW);
	if (m_chg & DE_CHG__EDIT_MASK)
		_set_stats_msg(SYNC_EDIT);
}

//////////////////////////////////////////////////////////////////

void ViewFile::_set_stats_msg(int kSync)
{
	if (!m_pDeDe)
		return;

	wxString strStats = (  (getNrTopPanels() == 3)
						 ? m_pDeDe->getStats3Msg(kSync)
						 : m_pDeDe->getStats2Msg(kSync));
#if 0
	// i moved the change-status-text from the frame's status bar to a text field
	// within the notebook panel.  the former causes flashing on MAC whenever the
	// the contents of the string change -- they're calling Refresh()/Update() on
	// the status bar which is causing the entire frame or viewfile to be redrawn.
	// this causes a massive flash on the mac.  it also causes an extra set of
	// redraws on the ViewFilePanels because of the way the cb_de_ and cb_fl_ events
	// happen.
	bool bNotEqual = (strStats != m_strLastStatsMsg);
	wxLogTrace(wxTRACE_Messages,_T("ViewFile::_set_stats_msg: [bNotEqual %d]"),(int)bNotEqual);
	if (bNotEqual)
	{
		m_pFrame->GetStatusBar()->SetStatusText(strStats, VIEWFILE_STATUSBAR_CELL_CHGS);
		m_strLastStatsMsg = strStats;
	}
#else
	m_nbPage[kSync].m_pWinChangeStatusText->setText(strStats);
#endif
}

//////////////////////////////////////////////////////////////////

void ViewFile::_set_display_rs_name(void)
{
	if (m_pDoc && m_pDoc->getFsFs() && m_pDoc->getFsFs()->getRuleSet())
	{
		wxString str = wxString::Format( _("Ruleset: %s"), m_pDoc->getFsFs()->getRuleSet()->getName().wc_str());
		if (str != m_strLastRuleSetMsg)
		{
			m_pFrame->GetStatusBar()->SetStatusText(str, VIEWFILE_STATUSBAR_CELL_RS);
			m_strLastRuleSetMsg = str;
		}
	}
}

void ViewFile::_set_display_encodings(void)
{
	// display character encoding in the status bar.
	// if all are the same, just display it once.
	// if any are different, list them all "utf-8(bom) : utf-8 : utf16le(bom)"

	if (!m_pDoc || !m_pDoc->getFsFs())
		return;

	fs_fs * pFsFs = m_pDoc->getFsFs();
	int nrTopPanels = getNrTopPanels();

	bool bAllSame = true;
	for (int kPanel=0; kPanel<nrTopPanels-1; kPanel++)
	{
		fim_ptable * pPTableA = pFsFs->getPTable(SYNC_VIEW,(PanelIndex)(kPanel  ));
		fim_ptable * pPTableB = pFsFs->getPTable(SYNC_VIEW,(PanelIndex)(kPanel+1));

		if (   (pPTableA->getEncoding() != pPTableB->getEncoding())
			|| (pPTableA->getHadUnicodeBOM() != pPTableB->getHadUnicodeBOM()))
		{
			bAllSame = false;
			break;
		}
	}

	if (bAllSame)			// if all the same, no need to list them all.
		nrTopPanels = 1;
	
	wxString strMsg;

	for (int kPanel=0; kPanel<nrTopPanels; kPanel++)
	{
		if (kPanel > 0)
			strMsg += _T(" : ");

		fim_ptable * pPTable = pFsFs->getPTable(SYNC_VIEW,(PanelIndex)kPanel);

		strMsg += wxFontMapper::GetEncodingName(pPTable->getEncoding());
		if (pPTable->getHadUnicodeBOM())
			strMsg += _T("(BOM)");
	}

	m_pFrame->GetStatusBar()->SetStatusText(strMsg, VIEWFILE_STATUSBAR_CELL_CHARENC);
}

//////////////////////////////////////////////////////////////////

void ViewFile::prePaint(void)
{
	//wxLogTrace(wxTRACE_Messages, _T("ViewFile:prePaint: [chg %lx]"), m_chg);

	if (!m_pDeDe)			// diff-engine not ready yet, just ignore.
		return;

	// the ViewFilePanel's (and maybe the glance window) all watch
	// their respective document layouts and the appropriate diff-engine
	// display list for changes -- so that they can repaint themselves
	// as necessary.  both the layouts and the display list run in
	// lazy mode -- they signal when they're dirty and wait until
	// the last possible moment to re-layout and/or re-build the display
	// list.
	//
	// the ViewFilePanel's continue that by noting the dirty status and
	// invalidating the client area -- and let the paint code trigger
	// the re-layout/diff.
	//
	// since ViewFile doesn't have anything to paint, we don't get hooked
	// into this sequence of events -- and even if we did, there's no
	// guarantee that we'd get to run first, since the notification cb's
	// can happen in any order and the sequence of paint events are out
	// of our control too.
	//
	// the only thing we need to do is to possibly re-calibrate the
	// scroll bars -- we can't do it during the notification cb's since
	// looking up the parameters would trigger the re-calcs and defeat
	// the lazy evaluation.
	//
	// so we remember what work needs to be done and ask that all of the
	// ViewFilePanel's call us in their OnPaint() routines so that we
	// take care of the dirty work.  and hopefully we can short-circuit
	// the re-calc's so that we only actually do it once and not for each
	// of the (possibly) 3 panels.

	// there are 2 distinct sets of DE_CHG_ flags:
	// [] the DE_CHG_*_CHG -- means that the diff-engine is marking
	//    the corresponding display list "dirty".
	// [] the DE_CHG_*_RUN -- means that the diff-engine is reporting
	//    that the corresponding display list has been re-run.
	//
	// we remember the chg status that we started with because some of
	// our actions may cause the de_de to re-signal and we want to stop
	// signals on signals.

	long inputChg = m_chg;

	// see if top panel's display list (view) is dirty and we haven't re-run engine.
	// see if bottom panel's display list (edit) is dirty and we haven't re-run engine.

	bool bNeedRun_REF  = ((inputChg & (DE_CHG_VIEW_CHG|DE_CHG_VIEW_RUN)) == DE_CHG_VIEW_CHG);
	bool bNeedRun_EDIT = ((inputChg & (DE_CHG_EDIT_CHG|DE_CHG_EDIT_RUN)) == DE_CHG_EDIT_CHG);

	// see if the de_de thinks it needs a run().  if we've captured all the right de notification events,
	// this should be true only when one of the _REF,_EDIT flags is true.  but perhaps when i tried to
	// minimize duplicate events/signals, we may have messed this up.  we can probably just use the _De
	// version and get rid of the others, but i'm not sure right now.

	bool bNeedRun_De = m_pDeDe->isRunNeeded();
	
	if (bNeedRun_REF || bNeedRun_EDIT || bNeedRun_De)
	{
		//wxLogTrace(wxTRACE_Messages,_T("ViewFile::prePaint: [%d == %d|%d]"), bNeedRun_De,bNeedRun_REF,bNeedRun_EDIT);

		// re-run the diff-engine.  this will re-build one or both display lists
		// as necessary.

		wxBusyCursor hourglass;	// cause hour-glass to appear
		m_pDeDe->run();
	}

	// if either display list WAS dirty, we may need to re-calibrate
	// the scroll bars.  i can't tell from the docs if updating the
	// scroll bars will cause wxWidgets to send any scroll events, but
	// hopefully, by doing it here before the ViewFilePanel's have
	// painted, we may get the scroll events before the paint events.
	
	if (inputChg & (DE_CHG_VIEW_CHG|DE_CHG_VIEW_RUN))
	{
		// the diff-engine re-ran on the top panels.  if this changed the
		// number of ROWS in the display list, we need to re-calibrate the
		// scroll bar.  remember: the number of ROWS in the display list
		// is determined by the current display mode (all, context only, etc)
		// and by how the sync lines things up between the panels.  these
		// can change the number of rows we have -- even though the number
		// of LINES in the various document may not have changed.

		long row = 0;
		if (m_bNeed_WarpSyncViewToFirstChange)
		{
			row = m_pDeDe->getNthChangeDisplayIndex(SYNC_VIEW,0);
			if (row == -1)
				row = 0;
			else
				m_pDeDe->setPatchHighlight(SYNC_VIEW,row,false);
			m_bNeed_WarpSyncViewToFirstChange = false;
			warpScrollCentered(SYNC_VIEW,row);
		}
		else
			adjustVerticalScrollbar(SYNC_VIEW);
		
		adjustHorizontalScrollbar(SYNC_VIEW,0);
	}

	if ((inputChg & (DE_CHG_EDIT_CHG|DE_CHG_EDIT_RUN))  &&  (getEditPanelPresent()))
	{
		long row = 0;
		if (m_bNeed_WarpSyncEditToFirstChange)
		{
			row = m_pDeDe->getNthChangeDisplayIndex(SYNC_EDIT,0);
			if (row == -1)
				row = 0;
			else
				m_pDeDe->setPatchHighlight(SYNC_EDIT,row,false);
			m_bNeed_WarpSyncEditToFirstChange = false;
			warpScrollCentered(SYNC_EDIT,row);
		}
		else
			adjustVerticalScrollbar(SYNC_EDIT);

		adjustHorizontalScrollbar(SYNC_EDIT);
	}

	m_chg = 0;

#ifdef DEBUGMEMORYSTATS
	gDebugMemoryStats.DumpStats();
#endif
}

//////////////////////////////////////////////////////////////////

void ViewFile::onEvent_SetDisplayMode(long kSync, de_display_ops dop)
{
	de_display_ops dopNew = (m_dop[kSync] & ~DE_DOP__MODE_MASK) | (dop & DE_DOP__MODE_MASK);

	_set_display_ops(kSync,dopNew);
}

void ViewFile::onEvent_SetDisplayBits(long kSync, de_display_ops dop, bool bOn)
{
	de_display_ops dopOldBits = m_dop[kSync] & ~DE_DOP__MODE_MASK;
	de_display_ops dopNewBits =          dop & ~DE_DOP__MODE_MASK;
	de_display_ops dopMode    = m_dop[kSync] &  DE_DOP__MODE_MASK;

	de_display_ops dopNew;

	if (bOn)
		dopNew = dopMode | (dopOldBits | dopNewBits);
	else
		dopNew = dopMode | (dopOldBits & ~dopNewBits);

	_set_display_ops(kSync,dopNew);
}

void ViewFile::_set_display_ops(long kSync, de_display_ops dop)
{
	wxASSERT_MSG( ((kSync==SYNC_VIEW)  ||  ((kSync==SYNC_EDIT) && getEditPanelPresent())), _T("Coding Error") );

	// update global props with the new value so that the next new
	// window will be created with this mode.

	gpGlobalProps->setLong(GlobalProps::GPL_VIEW_FILE_DISPLAY_OP,dop);

	if (m_dop[kSync] != dop)
	{
		// the display list is about to (possibly) radically change
		// because we are changing what type of lines we display.
		// therefore, the current patch-highlight and/or the caret
		// and/or the current selection is going to be invalid.
		// so, clear the patch-highlight, selection, and caret.

		m_pDeDe->unsetPatchHighlight(kSync);
		int kPanel = getPanelWithFocus();
		if (kPanel != -1)
		{
			ViewFilePanel * pPanelWithFocus = getPanel(kSync,(PanelIndex)kPanel);
			if (pPanelWithFocus)
				pPanelWithFocus->setBogusCaret();
		}
	}
	
	m_dop[kSync] = dop;
	
	m_pDeDe->setDisplayOps(kSync,dop);
	// I'm not going to explicitly call run() now, but rather wait for
	// the cb to do it in paint context.
	// TODO this call to setDisplayOps should cause the top display list
	// TODO change -- which should signal us via the cb.  so if all goes
	// TODO well, the scrollbars will be updated.  but we may also need
	// TODO to warp scroll to top, left.
}

//////////////////////////////////////////////////////////////////

void ViewFile::onEvent_SetPilcrow(bool bOn)
{
	// update global props with the new value so that the next new
	// window will be created with this mode.

	gpGlobalProps->setLong(GlobalProps::GPL_VIEW_FILE_PILCROW, (bOn==true));

	m_bPilcrow = bOn;

	// toggling this only affects the display (not the diff result
	// nor the display list), so we only need to invalidate the
	// view-file-panels so that they redraw.  and we only need to
	// invalidate the panels on the currently visible notebook page.

	long kSync = m_currentNBSync;
	if (kSync == -1)
		return;

	for (int kPanel=0; kPanel<__NR_TOP_PANELS__; kPanel++)
		if (m_nbPage[kSync].m_pWinPanel[kPanel])
			m_nbPage[kSync].m_pWinPanel[kPanel]->Refresh(true);
}

//////////////////////////////////////////////////////////////////

void ViewFile::onEvent_SetShowLineNumbers(bool bOn)
{
	// update global props with the new value so that the next new
	// window will be created with this mode.

	gpGlobalProps->setLong(GlobalProps::GPL_VIEW_FILE_LINE_NUMBERS, (bOn==true));

	m_bShowLineNumbers = bOn;

	long kSync;
	int kPanel;

	for (kSync=0; kSync<__NR_SYNCS__; kSync++)
		for (kPanel=0; kPanel<__NR_TOP_PANELS__; kPanel++)
			if (m_nbPage[kSync].m_pWinPanel[kPanel])
				m_nbPage[kSync].m_pWinPanel[kPanel]->setShowLineNumbers(bOn);

	// toggling this only affects the display (not the diff result
	// nor the display list), so we only need to invalidate the
	// view-file-panels so that they redraw.

	kSync = m_currentNBSync;
	if (kSync == -1)
		return;

	for (kPanel=0; kPanel<__NR_TOP_PANELS__; kPanel++)
		if (m_nbPage[kSync].m_pWinPanel[kPanel])
			m_nbPage[kSync].m_pWinPanel[kPanel]->Refresh(true);
}

//////////////////////////////////////////////////////////////////

void ViewFile::onEvent_setTabStop(int t)
{
	// update global props with the new value so that the next new
	// window will be created with this mode.

	gpGlobalProps->setLong(GlobalProps::GPL_VIEW_FILE_TABSTOP, t);

	// the selection is stored using the as-drawn column numbers
	// (after tabs have been expanded).  when we change the tab
	// stop, the selection (and anchor and etc) should be adjusted
	// to reflect the same set of characters in the document.
	//
	// this is kinda back-ass-wards, but lets us do other things
	// quickly.  and this doesn't happen too often.
	// 
	// since each panel gets its tabstop from us (via our getTabStop())
	// we need to convert the caret/anchor/selection-endpoints from
	// VFC's into absolute-document-positions, then change our tabstop
	// value, then update caret/anchor/selection-endpints.

	ViewFilePanel::caretState cs[__NR_SYNCS__][__NR_TOP_PANELS__];

	for (int kPage=0; kPage < __NR_SYNCS__; kPage++)
		for (int kPanel=0; kPanel<__NR_TOP_PANELS__; kPanel++)
			if (m_nbPage[kPage].m_pWinPanel[kPanel])
				m_nbPage[kPage].m_pWinPanel[kPanel]->beforeTabStopChange( &cs[kPage][kPanel] );

	// toggling this only affects the display (not the diff result
	// nor the display list), so we only need to update the caret/
	// selection and invalidate the view-file-panels so that they
	// redraw.

	m_cTabStop = t;

	for (int kPage=0; kPage < __NR_SYNCS__; kPage++)
		for (int kPanel=0; kPanel<__NR_TOP_PANELS__; kPanel++)
			if (m_nbPage[kPage].m_pWinPanel[kPanel])
				m_nbPage[kPage].m_pWinPanel[kPanel]->afterTabStopChange( &cs[kPage][kPanel] );

}

//////////////////////////////////////////////////////////////////

void ViewFile::deleteMark(long kSync, long yRowClick)
{
	de_mark * pDeMark = NULL;

	if (!m_pDeDe->isMark(kSync,yRowClick,&pDeMark))
		return;

	deleteMark(kSync,pDeMark);
}

void ViewFile::deleteMark(long kSync, de_mark * pDeMark)
{
	// delete the given mark.  try to preserve the selection/caret.
	//
	// the selection/caret is stored using the as-drawn column numbers
	// (after tabs have been expanded) and rows in the display list
	// (in VFC space).  when we add/remove a MARK, we may need to
	// adjust the row numbers in the caret/selection so that it maps
	// to the same content lines in the document.
	//
	// this is kinda back-ass-wards, but lets us do other things
	// quickly.  and this doesn't happen too often.
	//
	// fortunately, we can do the same thing that we do when the tab
	// stop changes -- map the caret/anchor/selection from VFC's to
	// absolute-document-positions, take care of the mark, then update
	// the caret/anchor/selection-endpoints.  unlike onEvent_setTabStop()
	// we only need to deal with the current view (kSync).

	ViewFilePanel::caretState cs[__NR_TOP_PANELS__];

	for (int kPanel=0; kPanel<__NR_TOP_PANELS__; kPanel++)
		if (m_nbPage[kSync].m_pWinPanel[kPanel])
			m_nbPage[kSync].m_pWinPanel[kPanel]->beforeTabStopChange( &cs[kPanel] );

	m_pDeDe->deleteMark(kSync,pDeMark);

	for (int kPanel=0; kPanel<__NR_TOP_PANELS__; kPanel++)
		if (m_nbPage[kSync].m_pWinPanel[kPanel])
			m_nbPage[kSync].m_pWinPanel[kPanel]->afterTabStopChange( &cs[kPanel] );
}

util_error ViewFile::createMark(int kSync, de_mark_type markType, int nrFiles, long * alLineNr, de_mark ** ppDeMark, PanelIndex * pPanelError)
{
	// create a mark.  try to preserve the selection/caret.
	// [see deleteMark() for reasons why we need all of this.

	ViewFilePanel::caretState cs[__NR_TOP_PANELS__];

	for (int kPanel=0; kPanel<__NR_TOP_PANELS__; kPanel++)
		if (m_nbPage[kSync].m_pWinPanel[kPanel])
			m_nbPage[kSync].m_pWinPanel[kPanel]->beforeTabStopChange( &cs[kPanel] );

	util_error ue = m_pDeDe->createMark(kSync,markType,nrFiles,alLineNr,ppDeMark,pPanelError);

	for (int kPanel=0; kPanel<__NR_TOP_PANELS__; kPanel++)
		if (m_nbPage[kSync].m_pWinPanel[kPanel])
			m_nbPage[kSync].m_pWinPanel[kPanel]->afterTabStopChange( &cs[kPanel] );

	return ue;
}

void ViewFile::deleteAllMark(int kSync)
{
	// call de_de::deleteAllMark() while preserving caret/selection

	ViewFilePanel::caretState cs[__NR_TOP_PANELS__];

	for (int kPanel=0; kPanel<__NR_TOP_PANELS__; kPanel++)
		if (m_nbPage[kSync].m_pWinPanel[kPanel])
			m_nbPage[kSync].m_pWinPanel[kPanel]->beforeTabStopChange( &cs[kPanel] );

	m_pDeDe->deleteAllMark(kSync);

	for (int kPanel=0; kPanel<__NR_TOP_PANELS__; kPanel++)
		if (m_nbPage[kSync].m_pWinPanel[kPanel])
			m_nbPage[kSync].m_pWinPanel[kPanel]->afterTabStopChange( &cs[kPanel] );
}

//////////////////////////////////////////////////////////////////

void ViewFile::_updateTitle(long kSync, PanelIndex kPanel)
{
	wxString strTitle;
	bool bPath = false;

	if (m_title[kPanel].Length() > 0)
	{
		strTitle = m_title[kPanel];
	}
	else
	{
		fs_fs * pFsFs = (m_pDoc) ? (m_pDoc->getFsFs()) : NULL;

		// we always want to use the titles in SYNC_VIEW for
		// all 2/3 panels -- [_T0 and _T2 are the same in all
		// cases] but for (SYNC_EDIT,PANEL_EDIT) its POI contains
		// the temporary edit buffer's pathname -- we always want
		// to show the reference source file in (SYNC_VIEW,PANEL_T1).

		if (pFsFs && pFsFs->getPoi(SYNC_VIEW,kPanel))
		{
			bPath = true;
			strTitle = pFsFs->getPoi(SYNC_VIEW,kPanel)->getFullPath();
		}
	}
	
	m_nbPage[kSync].m_pWinTitle[kPanel]->setText(strTitle);

	MyStaticTextAttrs taOld = m_nbPage[kSync].m_pWinTitle[kPanel]->getTextAttrs();
	MyStaticTextAttrs taNew;
	if (bPath)
		taNew = taOld | MY_STATIC_TEXT_ATTRS_FLAGS_ELIDE;
	else
		taNew = taOld & ~MY_STATIC_TEXT_ATTRS_FLAGS_ELIDE;
	if (taNew != taOld)
		m_nbPage[kSync].m_pWinTitle[kPanel]->setTextAttrs(taNew);
}

//////////////////////////////////////////////////////////////////

void ViewFileDiff::updateTitleWindows(long kSync)
{
	_updateTitle(kSync,PANEL_T0);
	_updateTitle(kSync,PANEL_T1);
}

void ViewFileMerge::updateTitleWindows(long kSync)
{
	_updateTitle(kSync,PANEL_T0);
	_updateTitle(kSync,PANEL_T1);
	_updateTitle(kSync,PANEL_T2);
}

//////////////////////////////////////////////////////////////////

void ViewFileDiff::setTopLevelWindowTitle(void)
{
	// construct and install a suitable title for our frame's titlebar.

	fs_fs * pFsFs = (m_pDoc ? m_pDoc->getFsFs() : NULL);
	if (!pFsFs)
		return;

	wxString strT0 = pFsFs->getPoi(SYNC_VIEW,(PanelIndex)0)->getFullName();
	wxString strT1 = pFsFs->getPoi(SYNC_VIEW,(PanelIndex)1)->getFullName();

	// we need to build the string the hard way.  if there are unicode (non-ascii)
	// characters in either pathname, wxString::Format() goes out to lunch on Mac
	// and maybe Linux.  bug:12326
	//
	//if (strT0.Cmp(strT1) != 0)
	//	m_pFrame->setCaption(wxString::Format( _("%s, %s - %s"),
	//										   strT0.wc_str(),
	//										   strT1.wc_str(),
	//										   VER_APP_TITLE));
	//else
	//	m_pFrame->setCaption(wxString::Format( _("%s - %s"),
	//										   strT0.wc_str(),
	//										   VER_APP_TITLE));

	wxString strCaption(strT0);
	if (strT0.Cmp(strT1) != 0)
	{
		strCaption += _T(", ");
		strCaption += strT1;
	}
	strCaption += _T(" - ");
	strCaption += VER_APP_TITLE;
	m_pFrame->setCaption(strCaption);

	// when editing and the edit panel is dirty, we should
	// put a "*" or something in the title somewhere.

	wxChar chStatus = ((getEditPanelPresent() && PT_STAT_TEST(m_ptStatEdit,PT_STAT_IS_DIRTY)) ? _T('*') : _T('-'));
	m_pFrame->setCaptionStatus(chStatus);
}

void ViewFileMerge::setTopLevelWindowTitle(void)
{
	// construct and install a suitable title for our frame's titlebar.

	fs_fs * pFsFs = (m_pDoc ? m_pDoc->getFsFs() : NULL);
	if (!pFsFs)
		return;

	wxString strT0 = pFsFs->getPoi(SYNC_VIEW,(PanelIndex)0)->getFullName();
	wxString strT1 = pFsFs->getPoi(SYNC_VIEW,(PanelIndex)1)->getFullName();
	wxString strT2 = pFsFs->getPoi(SYNC_VIEW,(PanelIndex)2)->getFullName();

	// we need to build the string the hard way.  if there are unicode (non-ascii)
	// characters in either pathname, wxString::Format() goes out to lunch on Mac
	// and maybe Linux.  bug:12326
	//
	//if ( (strT0.Cmp(strT1) == 0)  &&  (strT0.Cmp(strT2) == 0) )
	//	m_pFrame->setCaption(wxString::Format( _("%s - %s"),
	//										   strT0.wc_str(),
	//										   VER_APP_TITLE));
	//else
	//	m_pFrame->setCaption(wxString::Format( _("%s, %s, %s - %s"),
	//										   strT0.wc_str(),
	//										   strT1.wc_str(),
	//										   strT2.wc_str(),
	//										   VER_APP_TITLE));

	wxString strCaption(strT0);
	if ( (strT0.Cmp(strT1) != 0) || (strT0.Cmp(strT2) != 0) )
	{
		strCaption += _T(", ");
		strCaption += strT1;
		strCaption += _T(", ");
		strCaption += strT2;
	}
	strCaption += _T(" - ");
	strCaption += VER_APP_TITLE;
	m_pFrame->setCaption(strCaption);

	// when editing and the edit panel is dirty, we should
	// put a "*" or something in the title somewhere.

	wxChar chStatus = ((getEditPanelPresent() && PT_STAT_TEST(m_ptStatEdit,PT_STAT_IS_DIRTY)) ? _T('*') : _T('-'));
	m_pFrame->setCaptionStatus(chStatus);
}

//////////////////////////////////////////////////////////////////

void ViewFile::createEditPanel(void)
{
	wxASSERT_MSG( (getPTable(SYNC_EDIT,PANEL_EDIT) == NULL), _T("Coding Error") );
	wxASSERT_MSG( (m_pNotebook), _T("Coding Error") );
	
	fs_fs * pFsFs = (m_pDoc ? m_pDoc->getFsFs() : NULL);
	if (!pFsFs)									// should not happen
		return;

	if (pFsFs->getReadOnly())					// not allowed for this window
		return;									// should not happen

	m_bNeed_WarpSyncEditToFirstChange = true;

	bool bCreatedEditBuffer = pFsFs->setupEditing();			// cause (tempfile,clone_PTable) to be created if necessary
	pFsFs->getPTable(SYNC_EDIT,PANEL_EDIT)->addStatusCB(s_cb_EditPanelStatusChanged,this);	// we want to know when edit ptable changes

	m_pDeDe->enableEditing(bCreatedEditBuffer);		// update diff-engine to know of the new layout

	_layout_enable_edit_panel();	// create (but don't show) the "EDIT" notebook page

	for (int kPanel=0; (kPanel < __NR_TOP_PANELS__); kPanel++)
	{
		if (m_nbPage[SYNC_EDIT].m_pWinPanel[kPanel])
			m_nbPage[SYNC_EDIT].m_pWinPanel[kPanel]->bind_dede_flfl(m_pDeDe,getLayout(SYNC_EDIT,(PanelIndex)kPanel));
		
		if (m_nbPage[SYNC_EDIT].m_pWinGlance)
			m_nbPage[SYNC_EDIT].m_pWinGlance->bind_dede(m_pDeDe);
	}

	m_ptStatEdit = pFsFs->getPTable(SYNC_EDIT,PANEL_EDIT)->getPtStat();

	wxChar chStatus = (PT_STAT_TEST(m_ptStatEdit,PT_STAT_IS_DIRTY) ? _T('*') : _T('-'));
	m_pFrame->setCaptionStatus(chStatus);

	updateTitleWindows(SYNC_EDIT);

	wxString strTitle = ((getNrTopPanels() == 3) ? _("Edit View (Merge Result)") : _("Edit View (File as Edited)"));
	m_pNotebook->AddPage(m_nbPage[SYNC_EDIT].m_pNotebookPanel, strTitle);
	m_pNotebook->SetSelection(SYNC_EDIT);		// show now, now that all the pointers have been connected
	m_nbPage[SYNC_EDIT].m_pWinPanel[PANEL_EDIT]->SetFocus();	// put focus in the edit panel (rather than the notebook tab)
}

void ViewFile::cb_EditPanelStatusChanged(const util_cbl_arg & arg)
{
	// we get called anytime the status of the edit panel changes.
	
	pt_stat oldStat = m_ptStatEdit;
	pt_stat newStat	= (pt_stat)arg.m_l;

	//wxLogTrace(TRACE_PTABLE_DUMP, _T("STAT: [%x][%s][%s][%s][%s]"), newStat,
	//		   ((PT_STAT_TEST(newStat,PT_STAT_IS_DIRTY)) ? _T("DIRTY") : _T("")),
	//		   ((PT_STAT_TEST(newStat,PT_STAT_HAVE_AUTO_MERGED)) ? _T("MERGED") : _T("")),
	//		   ((PT_STAT_TEST(newStat,PT_STAT_CAN_UNDO)) ? _T("can UNDO") : _T("")),
	//		   ((PT_STAT_TEST(newStat,PT_STAT_CAN_REDO)) ? _T("can REDO") : _T("")));
	// ...add PT_STAT_PATHNAME_CHANGED to this...

	// since the toolbar buttons and menu items poll for status
	// (via the onUpdateUIEvent mechanism), we just store the
	// status and wait for them to poll again.

	m_ptStatEdit = newStat;

	// when the dirty state changes, we need to update the frame
	// window's title -- so that we set/clear the "*".  we defer
	// the update until we have installed the new state.

	bool bNeedToUpdateCaptionStatus = (PT_STAT_TEST(newStat,PT_STAT_IS_DIRTY) != PT_STAT_TEST(oldStat,PT_STAT_IS_DIRTY));
	if (bNeedToUpdateCaptionStatus)
	{
		wxChar chStatus = ((getEditPanelPresent() && PT_STAT_TEST(newStat,PT_STAT_IS_DIRTY)) ? _T('*') : _T('-'));
		m_pFrame->setCaptionStatus(chStatus);
	}

	// when auto-save runs/finishes/errors we stuff descriptive
	// text into the status bar.

	if (newStat & PT_STAT__STATE_EVENT)
	{
		if (PT_STAT_TEST(newStat,PT_STAT_AUTOSAVE_BEGIN))
		{
			m_pFrame->SetStatusText( _("Autosaving...") );
		}
		else if (PT_STAT_TEST(newStat,PT_STAT_AUTOSAVE_END))
		{
			m_pFrame->SetStatusText( _("Autosaving...Done.") );		// flash a ....Done
			// TODO consider adding a little delay between these.
			m_pFrame->SetStatusText( _("") );						// then clear.
		}
		else if (PT_STAT_TEST(newStat,PT_STAT_AUTOSAVE_ERROR))
		{
			m_pFrame->SetStatusText( _("Autosaving...Failed.  Turning off autosave in this window.") );
		}
	}
}

void ViewFile::cb_T1PanelStatusChanged(const util_cbl_arg & arg)
{
	// we get called anytime the status of the T1 panel changes.
	// this should be just pathname_change events.
	
	pt_stat newStat	= (pt_stat)arg.m_l;
	if (newStat & PT_STAT_PATHNAME_CHANGE)
	{
		//wxLogTrace(wxTRACE_Messages,_T("cb_T1PanelStatusChanged: pathname changed."));

		updateTitleWindows(SYNC_VIEW);
		updateTitleWindows(SYNC_EDIT);
		setTopLevelWindowTitle();
	}
}

//////////////////////////////////////////////////////////////////

void ViewFile::onTimerEvent_MyProgress(wxTimerEvent & /*e*/)
{
}

//////////////////////////////////////////////////////////////////

void ViewFile::queueEvent(int reason)
{
	// queue up a request to load/reload the files from disk and
	// populate our window.  we set a little delay to give the widget
	// creation a chance to complete (be realized/painted once) and
	// for events to settle down.

	if (m_nQueueEventReason != VIEWFILE_QUEUE_EVENT_NOTHING)
	{
		// OOPS, we already have an event queued.  we should
		// currently cannot push a second event.  see if it
		// is relatively harmless to ignore it.

		if (reason == VIEWFILE_QUEUE_EVENT_CHECKFILES)	// frame activated while waiting (to load/reload).
			return;										// this is safe to ignore (skip stat'ing files)..
		
		if (reason == m_nQueueEventReason)				// duplicate load/reload ??
			return;

		wxASSERT_MSG( (0), _T("Coding Error") );
	}
	
	// TODO investigate a proper value for this delay -- GTK seems
	// TODO to require more time than the other platforms.

	m_nQueueEventReason = reason;
	m_timer_MyQueue.Start(500,wxTIMER_ONE_SHOT);
}

//////////////////////////////////////////////////////////////////

void ViewFile::onTimerEvent_MyQueue(wxTimerEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages, _T("ViewFile:Timer: [%p][int %d ms]"), this, e.GetInterval());

	switch (m_nQueueEventReason)
	{
	default:
	case VIEWFILE_QUEUE_EVENT_NOTHING:
		m_nQueueEventReason = VIEWFILE_QUEUE_EVENT_NOTHING;
		return;

	case VIEWFILE_QUEUE_EVENT_LOAD:
		m_nQueueEventReason = VIEWFILE_QUEUE_EVENT_NOTHING;	// reset value before load() incase "this" gets deleted
		_timer_load_files();
		return;

	case VIEWFILE_QUEUE_EVENT_RELOAD:
		m_nQueueEventReason = VIEWFILE_QUEUE_EVENT_NOTHING;	// reset value before load() incase "this" gets deleted
		_timer_reload_files(false);		// reload but don't force it
		return;

	case VIEWFILE_QUEUE_EVENT_FORCE_RELOAD:
		m_nQueueEventReason = VIEWFILE_QUEUE_EVENT_NOTHING;	// reset value before load() incase "this" gets deleted
		_timer_reload_files(true);		// force full reload
		return;

	case VIEWFILE_QUEUE_EVENT_CHECKFILES:
		m_nQueueEventReason = VIEWFILE_QUEUE_EVENT_NOTHING;	// reset value before load() incase "this" gets deleted
		_checkForFilesChangedOnDisk();
		return;
	}
}

//////////////////////////////////////////////////////////////////

void ViewFile::_timer_reload_files(bool bForceReload)
{
	//wxLogTrace(wxTRACE_Messages, _T("ViewFile::Reload [%p]"), this);

	// reload already loaded files

	if (!m_pDeDe || !m_pDoc || !m_pDoc->getFsFs())		// should not happen
		return;

	// we need to check to see if any of the documents in T0, T1, or T2
	// are being edited (either in THIS WINDOW or in ANY OTHER WINDOW)
	// and if so, are dirty.  when a file is reloaded and it has an
	// associated edit-buffer, the edit-buffer is re-cloned and any
	// changes are discarded.

	fs_fs * pFsFs = (m_pDoc ? m_pDoc->getFsFs() : NULL);

	wxString strMsg = _("The following file(s) have been edited:\n\n");
	bool bDirty = false;

	for (int kPanel=0; (kPanel < __NR_TOP_PANELS__); kPanel++)
	{
		// screw-case: if they load the same file into multiple panels
		// we only want to test the file (and list it in strMsg) once.

		bool bDuplicate = false;
		for (int jPanel=0; (jPanel<kPanel); jPanel++)
			if (pFsFs->getPTable(SYNC_VIEW,(PanelIndex)kPanel) == pFsFs->getPTable(SYNC_VIEW,(PanelIndex)jPanel))
				bDuplicate = true;
		if (bDuplicate)
			continue;

		poi_item * pPoiItem_src = pFsFs->getPoi(SYNC_VIEW,(PanelIndex)kPanel);
		if (pPoiItem_src)
		{
			poi_item * pPoiItem_edit = pPoiItem_src->getPoiEditBuffer();
			if (pPoiItem_edit)		// if panel[k] is being edited somewhere
			{
				fim_ptable * pPTable_edit = pPoiItem_edit->getPTable();
				if (pPTable_edit)
				{
					pt_stat status = pPTable_edit->getPtStat();
					if (PT_STAT_TEST(status,PT_STAT_IS_DIRTY))	// edit-buffer behind src on kPanel is dirty
					{
						util_error ue;
						if (bForceReload || pPoiItem_src->getPTable()->hasChangedOnDiskSinceLoading(ue))
						{
							// the edit buffer for this file is dirty && we are going to
							// reload the source file (either because we are being forced
							// to reload it -or- because the source file has been changed
							// by another application).  so we list it.
							// this keeps us from asking them to discard their edits and
							// then we don't because T1 wasn't dirty.

							strMsg += pPoiItem_src->getFullPath();
							strMsg += _T("\n");

							bDirty = true;
						}
					}
				}
			}
		}
	}

	if (bDirty)
	{
		strMsg += _("\nDo you want to discard your edits and reload?");

		wxMessageDialog dlg(this,strMsg,_("Discard Your Changes?"),wxYES_NO|wxNO_DEFAULT|wxICON_QUESTION);
		int result = showModalDialogSuppressingActivationChecks(dlg);

		if (result != wxID_YES)
			return;
	}

	wxBusyCursor hourglass;

	m_pDoc->getFsFs()->reloadFiles(m_pFrame,bForceReload);

	//////////////////////////////////////////////////////////////////
	// TODO warp to first change ??
	//////////////////////////////////////////////////////////////////
}

//////////////////////////////////////////////////////////////////

void ViewFile::_timer_load_files(void)
{
	// initial loading of files

	wxASSERT_MSG( (m_pDeDe==NULL), _T("Coding Error") );

	util_error err = m_pDoc->getFsFs()->loadFiles(m_pFrame);
	switch (err.getErr())
	{
	default:
	case util_error::UE_CANCELED:
		m_pFrame->revertToEmptyFrame();		// THIS MIGHT DELETE US
		return;
		
	case util_error::UE_OK:
		_finishLoading();
		return;
	}
}

void ViewFile::_finishLoading(void)
{
	wxASSERT_MSG( (m_pDeDe==NULL), _T("Coding Error") );

	fs_fs * pFsFs = m_pDoc->getFsFs();

	pFsFs->getPTable(SYNC_VIEW,PANEL_T1)->addStatusCB(s_cb_T1PanelStatusChanged,this);	// we want to know when T1 ptable changes (for name changes)

	_set_display_encodings();		// stuff the character encoding labels in the status bar

	{
		wxBusyCursor hourglass;
		de_detail_level detailLevel = (de_detail_level)gpGlobalProps->getLong(GlobalProps::GPL_FILE_DETAIL_LEVEL);

		m_pDeDe = new de_de(pFsFs,m_dop[SYNC_VIEW],m_dop[SYNC_EDIT], detailLevel);
		m_pDeDe->run();
		m_pDeDe->addChangeCB(s_cb_de_changed,this);
	}

	// fake chg msg since we install cb after run()

	m_chg = DE_CHG__VIEW_MASK;

	// set the ruleset display (since we didn't actually receive the de chg cb)

	_set_display_rs_name();
	_set_stats_msg(SYNC_VIEW);

	// tell each panel window about the newly available layouts and diff-engine.

	for (int kPage=0; (kPage <= SYNC_VIEW); kPage++)
	{
		for (int kPanel=0; (kPanel < __NR_TOP_PANELS__); kPanel++)
			if (m_nbPage[kPage].m_pWinPanel[kPanel])
				m_nbPage[kPage].m_pWinPanel[kPanel]->bind_dede_flfl(m_pDeDe,getLayout(kPage,(PanelIndex)kPanel));

		if (m_nbPage[kPage].m_pWinGlance)
			m_nbPage[kPage].m_pWinGlance->bind_dede(m_pDeDe);
	}
	
	updateTitleWindows(SYNC_VIEW);

	// if there are no changes, gently inform them.
	// otherwise, warp scroll to the first change/conflict.
	// [we defer this until prePaint() gets called]

	long sum = 0;
	if (getNrTopPanels() == 2)
	{
		const de_stats2 * pStats2 = m_pDeDe->getStats2View();
		sum = pStats2->m_cImportantChanges + pStats2->m_cUnimportantChanges;
	}
	else
	{
		const de_stats3 * pStats3 = m_pDeDe->getStats3View();
		sum = pStats3->m_cImportantChanges + pStats3->m_cUnimportantChanges + pStats3->m_cImportantConflicts + pStats3->m_cUnimportantConflicts;
	}
	
	if (sum == 0)
	{
		// optionally display message box telling the user that these files are
		// equivalent (under the current ruleset).

		showModal_util_dont_show_again_msgbox_SuppressingActivationChecks(_("Notice"),
																		  _("Files are identical or equivalent under the current RuleSet."),
																		  GlobalProps::GPL_VIEW_FILE_DONT_SHOW_FILES_EQUIVALENT_MSGBOX);
	}
	else
	{
		m_bNeed_WarpSyncViewToFirstChange = true;
	}

	// if they didn't say /ro on the command line, we create the edit page.

	if (!pFsFs->getReadOnly())
		createEditPanel();

	// if they said /merge on the command line, we go ahead
	// and start the auto-merge dialog.
	// 
	// if sum==0, then we don't really need to do this.  don't
	// bother to raise a message box because we should have just
	// shown them the files-are-equivalent dialog.

	if ((sum > 0) && m_bRaiseAutoMergeDialogAfterLoading)
	{
		ViewFilePanel * pViewFilePanel = getPanel(SYNC_EDIT,PANEL_EDIT);
		if (pViewFilePanel)
			pViewFilePanel->autoMerge();
	}
}

//////////////////////////////////////////////////////////////////

long ViewFile::getNextChange(bool bConflict)
{
	// return row of next change/conflict relative to the currently
	// highlighted change, the caret, or the current thumb position.
	//
	// return -1 if there isn't a next change/conflict.
	//
	// when we don't have a highlighted change, we search forward
	// from the top-line in the window.

	if (!m_pDeDe)
		return -1;

	long kSync = m_currentNBSync;
	if (kSync == -1)
		return -1;

	switch ( m_dop[kSync] & DE_DOP__MODE_MASK )
	{
	case DE_DOP_EQL:		// we're only showing equal lines.  so we can't do this.
	default:				// bogus mode shouldn't ever happen.
		return -1;

	case DE_DOP_ALL:		// we're showing the entire file.
	case DE_DOP_DIF:		// we're showing diffs-without-context.
	case DE_DOP_CTX:		// we're showing diffs-with-context
		{
			// if we have a highlighted patch, get the bottom row of it.
			// if we have a caret, get that row.
			// if not, get the row displayed on the top line in the window.

			long rowReference;
			long rowPatchStart, rowPatchEnd;
			if (m_pDeDe->getPatchHighlight(kSync,NULL,&rowPatchStart,&rowPatchEnd))
				rowReference = rowPatchEnd - 1;		// -1 because we want the last row within the patch
			else if (   (getPanelWithFocus() != -1)
					 && (getPanel(kSync,(PanelIndex)getPanelWithFocus())->getCaretOrSelectionRow(true) != -1))
				rowReference = getPanel(kSync,(PanelIndex)getPanelWithFocus())->getCaretOrSelectionRow(true);
			else
				rowReference = getScrollThumbCharPosition(kSync,wxVERTICAL);

			const TVector_Display * pDis = m_pDeDe->getDisplayList(kSync);
			rowReference = MyMin(rowReference,(long)(pDis->size())-1);
			const de_row & rDeRowReference = (*pDis)[rowReference];

			//////////////////////////////////////////////////////////////////
			// TODO see if the next change/conflict could actually be scrolled
			// TODO to -- that is, if we've maxed-out the thumb, the next item
			// TODO is already on screen -- and because of the scroll limits,
			// TODO we can't move it to the top of the window.
			//////////////////////////////////////////////////////////////////

			return ((bConflict) ? rDeRowReference.getNextConflict() : rDeRowReference.getNextChange());
		}
	}
}

long ViewFile::getNextChange(bool bConflict, int rowRelative)
{
	// return row of next change/conflict relative to the given row.
	// return -1 if there isn't a next change/conflict.

	if (!m_pDeDe)
		return -1;

	long kSync = m_currentNBSync;
	if (kSync == -1)
		return -1;

	switch ( m_dop[kSync] & DE_DOP__MODE_MASK )
	{
	case DE_DOP_EQL:		// we're only showing equal lines.  so we can't do this.
	default:				// bogus mode shouldn't ever happen.
		return -1;

	case DE_DOP_ALL:		// we're showing the entire file.
	case DE_DOP_DIF:		// we're showing diffs-without-context.
	case DE_DOP_CTX:		// we're showing diffs-with-context
		{
			const TVector_Display * pDis = m_pDeDe->getDisplayList(kSync);
			const de_row & rDeRowReference = (*pDis)[rowRelative];

			return ((bConflict) ? rDeRowReference.getNextConflict() : rDeRowReference.getNextChange());
		}
	}
}

long ViewFile::getPrevChange(bool bConflict)
{
	// return row of prev change/conflict relative to the currently
	// highlighted change, the caret, or the current thumb position.
	// position.
	//
	// return -1 if there isn't a next change/conflict.

	if (!m_pDeDe)
		return -1;

	long kSync = m_currentNBSync;
	if (kSync == -1)
		return -1;

	switch ( m_dop[kSync] & DE_DOP__MODE_MASK )
	{
	case DE_DOP_EQL:		// we're only showing equal lines.  so we can't do this.
	default:				// bogus mode shouldn't ever happen.
		return -1;

	case DE_DOP_ALL:		// we're showing the entire file.
	case DE_DOP_DIF:		// we're showing diffs-without-context.
	case DE_DOP_CTX:		// we're showing diffs-with-context
		{
			// if we have a highlighted patch, get the top row of it.
			// if we have a caret, get that row.
			// if not, get the row displayed on the bottom line in the window.

			long rowReference;
			long rowPatchStart, rowPatchEnd;
			if (m_pDeDe->getPatchHighlight(kSync,NULL,&rowPatchStart,&rowPatchEnd))
				rowReference = rowPatchStart;
			else if (   (getPanelWithFocus() != -1)
					 && (getPanel(kSync,(PanelIndex)getPanelWithFocus())->getCaretOrSelectionRow(false) != -1))
				rowReference = getPanel(kSync,(PanelIndex)getPanelWithFocus())->getCaretOrSelectionRow(false);
			else
				rowReference = getScrollThumbCharPosition(kSync,wxVERTICAL) + getPanel(kSync,PANEL_T0)->getRowsDisplayable();

			const TVector_Display * pDis = m_pDeDe->getDisplayList(kSync);
			rowReference = MyMin(rowReference,(long)(pDis->size())-1);
			const de_row & rDeRow = (*pDis)[rowReference];

			return ((bConflict) ? rDeRow.getPrevConflict() : rDeRow.getPrevChange());
		}
	}
}

long ViewFile::getPrevChange(bool bConflict, int rowRelative)
{
	// return row of previous change/conflict relative to the given row.
	// return -1 if there isn't a previous change/conflict.

	if (!m_pDeDe)
		return -1;

	long kSync = m_currentNBSync;
	if (kSync == -1)
		return -1;

	switch ( m_dop[kSync] & DE_DOP__MODE_MASK )
	{
	case DE_DOP_EQL:		// we're only showing equal lines.  so we can't do this.
	default:				// bogus mode shouldn't ever happen.
		return -1;

	case DE_DOP_ALL:		// we're showing the entire file.
	case DE_DOP_DIF:		// we're showing diffs-without-context.
	case DE_DOP_CTX:		// we're showing diffs-with-context
		{
			const TVector_Display * pDis = m_pDeDe->getDisplayList(kSync);
			const de_row & rDeRowReference = (*pDis)[rowRelative];

			return ((bConflict) ? rDeRowReference.getPrevConflict() : rDeRowReference.getPrevChange());
		}
	}
}

//////////////////////////////////////////////////////////////////

ViewPrintout * ViewFileDiff::createPrintout(ViewPrintout * pPrintoutRelated)
{
	return new ViewPrintoutFileDiff(this,((pPrintoutRelated)
										  ? (static_cast<ViewPrintoutFile *>(pPrintoutRelated))->getData()
										  : NULL));
}

//////////////////////////////////////////////////////////////////

ViewPrintout * ViewFileMerge::createPrintout(ViewPrintout * pPrintoutRelated)
{
	return new ViewPrintoutFileMerge(this,((pPrintoutRelated)
										   ? (static_cast<ViewPrintoutFile *>(pPrintoutRelated))->getData()
										   : NULL));
}

//////////////////////////////////////////////////////////////////

int ViewFile::getPanelWithFocus(void)
{
	long kSync = m_currentNBSync;
	if (kSync == -1)			// if the notebook isn't set up yet,
		return PANEL_T1;		// just pick one.

	if (m_nbPage[kSync].m_panelWithFocus == -1)
	{
		wxWindow * pWinFocus = wxWindow::FindFocus();

		for (int kPanel=0; (kPanel < __NR_TOP_PANELS__); kPanel++)
			if (m_nbPage[kSync].m_pWinPanel[kPanel] && (m_nbPage[kSync].m_pWinPanel[kPanel]==pWinFocus))
			{
				m_nbPage[kSync].m_panelWithFocus = kPanel;
				return m_nbPage[kSync].m_panelWithFocus;
			}
	}
	
	return m_nbPage[kSync].m_panelWithFocus;	// this can still be -1 during window setup
}

void ViewFile::setPanelWithFocus(int newFocus)
{
	// This is called by ViewFilePanel::onSetFocus() to let us know which
	// panel currently has focus.  You probably don't want to call this.
	// You probably want to call SetFocus() on the panel window.
	
	long kSync = m_currentNBSync;
	if (kSync == -1)			// if the notebook isn't set up yet,
		return;

	int oldFocus = m_nbPage[kSync].m_panelWithFocus;

	//wxLogTrace(wxTRACE_Messages, _T("ViewFile::setPanelWithFocus: [oldFocus %d][newFocus %d]"), oldFocus,newFocus);

	if (newFocus == oldFocus)
		return;

	m_nbPage[kSync].m_panelWithFocus = newFocus;

	// clear selection and hide caret on the old focus panel.
	
	if (oldFocus != -1)
	{
		if (m_nbPage[kSync].m_pWinPanel[oldFocus])
			m_nbPage[kSync].m_pWinPanel[oldFocus]->clearSelection();
		if (m_nbPage[kSync].m_pWinTitle[oldFocus])
		{
			wxFont font(m_nbPage[kSync].m_pWinTitle[oldFocus]->GetFont());
			font.SetWeight(wxFONTWEIGHT_NORMAL);
			m_nbPage[kSync].m_pWinTitle[oldFocus]->SetFont(font);
		}
	}

	// refresh the new focus panel to show the caret.

	if (newFocus != -1)
	{
		if (m_nbPage[kSync].m_pWinPanel[newFocus])
			m_nbPage[kSync].m_pWinPanel[newFocus]->Refresh(true);
		if (m_nbPage[kSync].m_pWinTitle[newFocus])
		{
			wxFont font(m_nbPage[kSync].m_pWinTitle[newFocus]->GetFont());
			font.SetWeight(wxFONTWEIGHT_BOLD);
			m_nbPage[kSync].m_pWinTitle[newFocus]->SetFont(font);
		}
	}
	
}

void ViewFile::cycleFocus(bool bForward)
{
	long kSync = m_currentNBSync;
	if (kSync == -1)			// if the notebook isn't set up yet,
		return;

	int kNewFocus;

	// NOTE: when we have a top-level (SYNC-VIEW vs SYNC-EDIT) notebook up, Ctrl-TAB
	// NOTE: should do a page-change.  but we're using Ctrl-TAB to cycle between panels
	// NOTE: within a page.  perhaps, we should include the notebook tab in the loop??

	if (bForward)
	{
		switch (m_nbPage[kSync].m_panelWithFocus)
		{
		default:
		case PANEL_T0:														kNewFocus = PANEL_T1;	break;
		case PANEL_T1:	if (m_nbPage[kSync].m_pWinPanel[PANEL_T2]) 		{	kNewFocus = PANEL_T2;	break;	}	// else fall-thru-intended
		case PANEL_T2:														kNewFocus = PANEL_T0;	break;
		}
	}
	else
	{
		switch (m_nbPage[kSync].m_panelWithFocus)
		{
		default:
		case PANEL_T1:														kNewFocus = PANEL_T0;	break;
		case PANEL_T0:	if (m_nbPage[kSync].m_pWinPanel[PANEL_T2])		{	kNewFocus = PANEL_T2;	break;	}	// else fall-thru-intended
		case PANEL_T2:														kNewFocus = PANEL_T1;	break;
		}
	}

	m_nbPage[kSync].m_pWinPanel[kNewFocus]->SetFocus();
}

void ViewFile::specialSetFocus(void)
{
	// this is called by the frame when we need to put focus into the view.
	// this happens after the frame has been minimized and restored (at least
	// on Windows).

	long kSync = m_currentNBSync;
	if (kSync == -1)
		return;

	int kPanel = m_nbPage[kSync].m_panelWithFocus;
	if (kPanel == -1)
		kPanel = PANEL_T1;

	if (m_nbPage[kSync].m_pWinPanel[kPanel])
		m_nbPage[kSync].m_pWinPanel[kPanel]->SetFocus();
}

//////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(ViewFile::_my_notebook, wxNotebook)
	EVT_NOTEBOOK_PAGE_CHANGING(wxID_ANY,ViewFile::_my_notebook::on_notebook_page_changing)
	EVT_NOTEBOOK_PAGE_CHANGED(wxID_ANY,ViewFile::_my_notebook::on_notebook_page_changed)
	EVT_SET_FOCUS(ViewFile::_my_notebook::onSetFocusEvent)
	EVT_KEY_DOWN(ViewFile::_my_notebook::onKeyDownEvent)
END_EVENT_TABLE();

//////////////////////////////////////////////////////////////////

void ViewFile::onEvent_NotebookChanging(wxNotebookEvent & e)
{
	// prevent notebook page from changing between SYNC-VIEW and SYNC-EDIT
	// when the manual alignment dialog is up -- because the current-markers
	// listbox is specific to the page we're showing.
	//
	// as an alternative, we could allow this to happen and signal the dialog
	// to rebuild the listbox.

	if (m_pFrame->haveInsertMarkerDialog())
	{
		::wxBell();
		e.Veto();
	}
}

void ViewFile::onEvent_NotebookChanged(wxNotebookEvent & e)
{
	//wxLogTrace(wxTRACE_Messages, _T("ViewFile::onEvent_NotebookChanged: [was %d][new %d]"),
	//		   e.GetOldSelection(), e.GetSelection());

	// try to put focus into the ViewFilePanel that had it
	// the last time that this page was active.

	long kSync = (long)e.GetSelection();

	// set our cached version of m_pNotebook->GetSelection() before
	// we do anything else -- because the actual notebook widget may
	// or may not have updated yet -- it's documented as being platform
	// dependent.  we use this value everywhere else rather than talking
	// to the actual widget.
	
	m_currentNBSync = kSync;

	if (kSync == -1)
		return;

	int focus = ((m_nbPage[kSync].m_panelWithFocus == -1) ? PANEL_T1 : m_nbPage[kSync].m_panelWithFocus);

	m_nbPage[kSync].m_pWinPanel[focus]->SetFocus();

#if 0
	// stats are per-page-dependent; that is, there are stats for SYNC_VIEW and SYNC_EDIT.
	// we need to update the status bar when we change pages.

	_set_stats_msg();
#endif
}

//////////////////////////////////////////////////////////////////

bool ViewFile::_checkForFilesChangedOnDisk(void)
{
	// when our window is activated (brought to the front)
	// we should see if another application changed any of
	// them while we were not the active window.
	//
	// if any have changed, we should give the user a chance
	// to reload them.  this can get a little crazy with
	// (upto) 3 input files (PANEL_T0,T1,T2) and possibly
	// an edit-buffer (in PANEL_EDIT) and possibly a /RESULT:
	// pathname.
	//
	// further complicating this, is that any of these
	// files may also be open in another window.  and any
	// of these files may be in the T1 of the other window.
	//
	// further complicating this is that when we raise
	// a message box to ask them our window may be
	// de-activated and reactivated.

	//wxLogTrace(wxTRACE_Messages, _T("Checking for updated files....."));

	util_error ue;

	fs_fs * pFsFs = (m_pDoc ? m_pDoc->getFsFs() : NULL);
	if (!pFsFs)			// documents not completely loaded.  should not happen
		return false;

	bool bHasChanged[3] = { false, false, false };
	bool bAnyChanged = false;
	bool bAnyErrors = false;
	for (int kPanel=0; (kPanel < __NR_TOP_PANELS__); kPanel++)
	{
		// screw-case: if they load the same file into multiple panels
		// we only want to test the file (and list it in strMsg) once.

		bool bDuplicate = false;
		for (int jPanel=0; (jPanel<kPanel); jPanel++)
			if (pFsFs->getPTable(SYNC_VIEW,(PanelIndex)kPanel) == pFsFs->getPTable(SYNC_VIEW,(PanelIndex)jPanel))
				bDuplicate = true;
		if (bDuplicate)
			continue;

		// hasChangedOnDiskSinceLastChecked() will update the Date/Time
		// last checked with the file's current Date/Time, so we should
		// only detect a change in the first window (containing this
		// document) to be activated after the file is changed.  it should
		// also keep us from getting stuck in a reload loop (when the
		// modal "do you want to reload" dialog goes down and our window
		// get RE-activated.

		fim_ptable * pPTableK = pFsFs->getPTable(SYNC_VIEW,(PanelIndex)kPanel);
		if (pPTableK)
		{
			bHasChanged[kPanel] = pPTableK->hasChangedOnDiskSinceLastChecked(ue,true);

			// if we get an error on any of these checks, we just fail.
			// no sense trying to reload files (or asking to reload files)
			// that we can't even stat()....  ESPECIALLY when the user doesn't
			// even know that we checked.

			bAnyErrors |= (ue.isErr());
			bAnyChanged |= bHasChanged[kPanel];
		}
	}

	if (bAnyErrors || !bAnyChanged)
		return false;

	// ask them if they want to reload.
	
	wxString strMsg = _("The following file(s) have been changed by another application:\n\n");
	for (int kPanel=0; (kPanel < __NR_TOP_PANELS__); kPanel++)
	{
		if (bHasChanged[kPanel] && pFsFs->getPoi(SYNC_VIEW,(PanelIndex)kPanel))
		{
			strMsg += pFsFs->getPoi(SYNC_VIEW,(PanelIndex)kPanel)->getFullPath();
			strMsg += _T("\n");
		}
	}
	strMsg += _("\nWould you like to reload?");

	wxMessageDialog dlg(this,strMsg,_("Files Changed on Disk"),wxYES_NO|wxYES_DEFAULT|wxICON_QUESTION);
	int result = showModalDialogSuppressingActivationChecks(dlg);

	if (result != wxID_YES)
		return false;

	//wxLogTrace(wxTRACE_Messages, _T("Queueing reload....."));

	queueEvent(VIEWFILE_QUEUE_EVENT_RELOAD);

	//wxLogTrace(wxTRACE_Messages, _T("Queued reload for updated files....."));

	return true;
}

//////////////////////////////////////////////////////////////////

FindResult ViewFile::find(int kSync, PanelIndex kPanel,
						  const wxString & strPattern,
						  bool bIgnoreCase, bool bForward)
{
	ViewFilePanel * pViewFilePanel = getPanel(kSync,kPanel);

	return pViewFilePanel->find(strPattern, bIgnoreCase, bForward);
}

FindResult ViewFile::findAgain(bool bForward)
{
	// if the FindPanel is aleady visible, we interpret
	// an "Edit | Find Next/Prev" command (from the menu)
	// to mean use the values shown in the FindPanel.
	// if it is not visible, we load the search terms
	// from global props (into the FindPanel fields
	// without displaying the FindPanel).

	if (!m_pPanelFind->IsShown())
		preloadFindPanelFields();

	return onFindPanel__do_search(bForward);
}

//////////////////////////////////////////////////////////////////

int ViewFile::showModalDialogSuppressingActivationChecks(wxDialog & dlg)
{
	bool bOld = m_bTemporarilySuppressActivationChecks;

	m_bTemporarilySuppressActivationChecks = true;

	int result = dlg.ShowModal();

	m_bTemporarilySuppressActivationChecks = bOld;

	return result;
}

int ViewFile::showModal_util_dont_show_again_msgbox_SuppressingActivationChecks(const wxString & strTitle,
																				const wxString & strMessage,
																				int key,
																				long buttonFlags)
{
	bool bOld = m_bTemporarilySuppressActivationChecks;

	m_bTemporarilySuppressActivationChecks = true;

	int result = util_dont_show_again_msgbox::msgbox(this,strTitle,strMessage,key,buttonFlags);

	m_bTemporarilySuppressActivationChecks = bOld;

	return result;
}

//////////////////////////////////////////////////////////////////

void ViewFile::completeAutoMerge(fim_patchset * pPatchSet)
{
	int kSync = SYNC_EDIT;

	// apply the given patchset and inform the user of what we did.
		
	fim_ptable * pPTableEdit = getPTable(kSync,PANEL_EDIT);
	pPTableEdit->applyPatchSet( pPatchSet );

	dlg_auto_merge_result_summary dlg(getFrame(),pPatchSet);
	showModalDialogSuppressingActivationChecks(dlg);
	
	// don't delete patchset, we don't own it.
}

//////////////////////////////////////////////////////////////////

wxString ViewFileDiff::dumpSupportInfo(const wxString & strIndent) const
{
	wxString str;
	wxString strIndent2 = strIndent + _T("\t");

	fs_fs * pFsFs = m_pDoc->getFsFs();

	str += wxString::Format( _T("%sWindow Type: FileDiff\n"), strIndent.wc_str());
	str += wxString::Format( _T("%sEdit State: %s\n"),strIndent2.wc_str(), ((getEditPanelPresent()) ? _T("Editable") : _T("Read Only") ));
	str += wxString::Format( _T("%sDisplayOps[View]: 0x%08lx\n"),strIndent2.wc_str(),(unsigned long)getDisplayOps(SYNC_VIEW));
	str += wxString::Format( _T("%sDisplayOps[Edit]: 0x%08lx\n"),strIndent2.wc_str(),(unsigned long)getDisplayOps(SYNC_EDIT));
	str += wxString::Format( _T("%sPilcrow: %s\n"),strIndent2.wc_str(),((m_bPilcrow) ? _T("true") : _T("false")));
	str += wxString::Format( _T("%sTabStop: %d\n"),strIndent2.wc_str(),m_cTabStop);
	str += wxString::Format( _T("%sLineNumbers: %s\n"),strIndent2.wc_str(),((m_bShowLineNumbers) ? _T("true") : _T("false")));

	str += pFsFs->dumpSupportInfo(strIndent2);
	str += m_pDeDe->dumpSupportInfo(strIndent2);

	return str;
}

wxString ViewFileMerge::dumpSupportInfo(const wxString & strIndent) const
{
	wxString str;
	wxString strIndent2 = strIndent + _T("\t");

	fs_fs * pFsFs = m_pDoc->getFsFs();

	str += wxString::Format( _T("%sWindow Type: FileMerge\n"), strIndent.wc_str());
	str += wxString::Format( _T("%sEdit State: %s\n"),strIndent2.wc_str(), ((getEditPanelPresent()) ? _T("Editable") : _T("Read Only") ));
	str += wxString::Format( _T("%sDisplayOps[View]: 0x%08lx\n"),strIndent2.wc_str(),(unsigned long)getDisplayOps(SYNC_VIEW));
	str += wxString::Format( _T("%sDisplayOps[Edit]: 0x%08lx\n"),strIndent2.wc_str(),(unsigned long)getDisplayOps(SYNC_EDIT));
	str += wxString::Format( _T("%sPilcrow: %s\n"),strIndent2.wc_str(),((m_bPilcrow) ? _T("true") : _T("false")));
	str += wxString::Format( _T("%sTabStop: %d\n"),strIndent2.wc_str(),m_cTabStop);
	str += wxString::Format( _T("%sLineNumbers: %s\n"),strIndent2.wc_str(),((m_bShowLineNumbers) ? _T("true") : _T("false")));

	str += pFsFs->dumpSupportInfo(strIndent2);
	str += m_pDeDe->dumpSupportInfo(strIndent2);

	return str;
}

//////////////////////////////////////////////////////////////////

int ViewFile::computeDefaultAction(PanelIndex kPanelFrom)
{
	long kSync = m_currentNBSync;
	if (kSync != SYNC_EDIT)
		return ViewFilePanel::CTX__NO_MENU;

	bool bHavePatch = m_pDeDe->getPatchHighlight(kSync);
	if (!bHavePatch)
		return ViewFilePanel::CTX__NO_MENU;

	ViewFilePanel * pViewFilePanel = getPanel(kSync,kPanelFrom);

	return pViewFilePanel->computeDefaultPatchAction();
}

void ViewFile::applyDefaultAction(PanelIndex kPanelFrom, bool bAutoAdvance)
{
	// apply default patch from the given panel to the edit panel.
	// automatically advance to next change if requested.

	int defaultAction = computeDefaultAction(kPanelFrom);
	if (   (defaultAction == ViewFilePanel::CTX__NO_MENU)
		|| (defaultAction == ViewFilePanel::CTX__NO_DEFAULT))
		return;

	long kSync = m_currentNBSync;
	if (kSync != SYNC_EDIT)
		return;

	ViewFilePanel * pViewFilePanel = getPanel(kSync,kPanelFrom);

	pViewFilePanel->doPatchOperationSetCaret(defaultAction);

	if (bAutoAdvance)
		keyboardScroll_delta(true,false);
}

//////////////////////////////////////////////////////////////////

void ViewFile::doSave(void)
{
	// save the edit panel.  we may or may not cause a dialog to appear.
	// we do not worry about suppressing the activation event when the
	// dialog disappears.

	util_error ue = m_pDoc->getFsFs()->fileSave(m_pFrame);
	if (!ue.isOK() && (ue.getErr() != util_error::UE_CANCELED))
	{
		wxMessageDialog dlg(m_pFrame,ue.getMBMessage(),_("Error!"),wxOK|wxICON_ERROR);
		dlg.ShowModal();
	}
}

void ViewFile::doSaveAs(void)
{
	// save the edit panel.  we will cause at least 1 dialog to appear.
	// we do not worry about suppressing the activation event when the
	// dialog disappears.

	util_error ue = m_pDoc->getFsFs()->fileSaveAs(m_pFrame);
	if (!ue.isOK() && (ue.getErr() != util_error::UE_CANCELED))
	{
		wxMessageDialog dlg(m_pFrame,ue.getMBMessage(),_("Error!"),wxOK|wxICON_ERROR);
		dlg.ShowModal();
	}
}

//////////////////////////////////////////////////////////////////

/**
 * Ask the user for a pathname to export a file diff into.
 *
 * If we have a seed pathname, try to use it.
 * If we are wanting to export in the same format as
 * the seed, we can use the full path when seeding the
 * dialog.  If not, just seed the parent directory of
 * the last export (of this or any other window in this
 * or an earlier session).
 *
 * This version is not a member of the ViewFile class
 * so that we can be called from ViewFolder when right
 * clicking on a pair of files in a folder window, for
 * example.
 *
 */
static util_error _s_getExportPathnameFromUser(wxWindow * pParent,
											   poi_item * pPoiSeed,
											   int /*gui_frame::_iface_id*/ idPrev,
											   int /*gui_frame::_iface_id*/ id,
											   poi_item ** ppPoiReturned)
{
	util_error ue;
	wxString strDir;
	wxString strFile;

	if (pPoiSeed && (idPrev == id))
	{
		// if we already have done an export on this window
		// *AND* with this format,
		// seed the dialog with the pathname chosen last time.
		wxFileName fnSeed(pPoiSeed->getFullPath());
		strDir = fnSeed.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR);
		strFile = fnSeed.GetFullName();
	}
	else
	{
		// otherwise, seed the dialog to the directory of the
		// last export (of this or any other window in this or
		// an earlier session).
		// we do not want the filename portion.
		wxString strSeed(gpGlobalProps->getString(GlobalProps::GPS_DIALOG_CHOOSE_FILEDIFF_EXPORT_SEED));
		wxFileName fnSeed(strSeed);
		strDir = fnSeed.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR);
		// strFile = fnSeed.GetFullName();
	}

	wxString strPattern;
	if ((id == gui_frame::MENU_EXPORT__FILE_DIFFS__UNIFIED__HTML__FILE)
		||(id == gui_frame::MENU_EXPORT__FILE_DIFFS__TRADITIONAL__HTML__FILE)
		||(id == gui_frame::MENU_EXPORT__FILE_DIFFS__SXS__HTML__FILE))
		strPattern = _T("HTML (*.html)|*.html");
	else
		strPattern = _T("Text (*.txt)|*.txt");

	wxFileDialog dlg(pParent, _("Save As"),
					 strDir,strFile,
					 strPattern,
					 wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
	if (dlg.ShowModal() != wxID_OK)
	{
		ue.set(util_error::UE_CANCELED);
		return ue;
	}

	wxString strDialogResult = dlg.GetPath();
	gpGlobalProps->setString(GlobalProps::GPS_DIALOG_CHOOSE_FILEDIFF_EXPORT_SEED,strDialogResult);

	*ppPoiReturned = gpPoiItemTable->addItem(strDialogResult);

	return ue;
}

/**
 * public static version.  return null if destination is not a file (such a clipboard).
 */
/*static*/ util_error ViewFile::s_getExportPathnameFromUser(wxWindow * pParent,
															poi_item * pPoiSeed,
															int /*gui_frame::_iface_id*/ idPrev,
															int /*gui_frame::_iface_id*/ id,
															poi_item ** ppPoiReturned)
{
	util_error ue;
	
	switch (id)
	{
	default:
		ue.set(util_error::UE_UNSPECIFIED_ERROR, _T("Coding Error"));
		return ue;

	case gui_frame::MENU_EXPORT__FILE_DIFFS__UNIFIED__TEXT__FILE:
	case gui_frame::MENU_EXPORT__FILE_DIFFS__TRADITIONAL__TEXT__FILE:
	case gui_frame::MENU_EXPORT__FILE_DIFFS__UNIFIED__HTML__FILE:
	case gui_frame::MENU_EXPORT__FILE_DIFFS__TRADITIONAL__HTML__FILE:
	case gui_frame::MENU_EXPORT__FILE_DIFFS__SXS__HTML__FILE:
		ue = _s_getExportPathnameFromUser(pParent, pPoiSeed, idPrev, id, ppPoiReturned);
		return ue;

	case gui_frame::MENU_EXPORT__FILE_DIFFS__UNIFIED__TEXT__CLIPBOARD:
	case gui_frame::MENU_EXPORT__FILE_DIFFS__TRADITIONAL__TEXT__CLIPBOARD:
		// to clipboard, no file needed
		*ppPoiReturned = NULL;
		return ue;
	}
}

util_error ViewFile::_getExportPathnameFromUser(int /*gui_frame::_iface_id*/ id,
												poi_item ** ppPoiReturned)
{
	// ask the user for a pathname to export the diffs to.
	poi_item * pPoiReturned = NULL;
	util_error ue = ViewFile::s_getExportPathnameFromUser(m_pFrame,
														  m_pPoiExportDiffsPathname,
														  m_lastExportToFileFormat,
														  id,
														  &pPoiReturned);
	if (ue.isOK() && pPoiReturned)
	{
		// remember this path/id in this window so that
		// the next export is seeded properly, but only
		// if destination is a file.

		m_pPoiExportDiffsPathname = pPoiReturned;
		m_lastExportToFileFormat = id;
	}

	*ppPoiReturned = pPoiReturned;
	return ue;
}

//////////////////////////////////////////////////////////////////

/*static*/ util_error ViewFile::s_doExportToString(de_de * pDeDe, long kSync, int /*gui_frame::_iface_id*/ id,
												   wxString & strOutput, bool * pbHadChanges,
												   int cTabStop,
												   const wxString & strTitleA, const wxString & strTitleB)
{
	util_error ue;

	switch (id)
	{
	default:
		ue.set(util_error::UE_UNSPECIFIED_ERROR, _T("Coding Error"));
		return ue;

	case gui_frame::MENU_EXPORT__FILE_DIFFS__UNIFIED__TEXT__FILE:
	case gui_frame::MENU_EXPORT__FILE_DIFFS__UNIFIED__TEXT__CLIPBOARD:
		{
			wxString strLabelA, strLabelB;
			pDeDe->getFsFs()->makeLabelForExportUnifiedHeader(kSync, strLabelA, strLabelB, strTitleA, strTitleB);
			pDeDe->batchoutput_text_unified_diff2(kSync, strOutput, pbHadChanges, &strLabelA, &strLabelB);
		}
		break;

	case gui_frame::MENU_EXPORT__FILE_DIFFS__UNIFIED__HTML__FILE:
		{
			wxString strLabelA, strLabelB;
			pDeDe->getFsFs()->makeLabelForExportUnifiedHeader(kSync, strLabelA, strLabelB, strTitleA, strTitleB);
			pDeDe->batchoutput_html_unified_diff2(kSync, strOutput, pbHadChanges,
												  cTabStop,
												  &strLabelA, &strLabelB);
		}
		break;

	case gui_frame::MENU_EXPORT__FILE_DIFFS__TRADITIONAL__HTML__FILE:
		{
			wxString strLabelA, strLabelB;
			pDeDe->getFsFs()->makeLabelForExportUnifiedHeader(kSync, strLabelA, strLabelB, strTitleA, strTitleB);
			pDeDe->batchoutput_html_traditional_diff2(kSync, strOutput, pbHadChanges,
													  cTabStop,
													  &strLabelA, &strLabelB);
		}
		break;

	case gui_frame::MENU_EXPORT__FILE_DIFFS__SXS__HTML__FILE:
		{
			wxString strLabelA, strLabelB;
			pDeDe->getFsFs()->makeLabelForExportUnifiedHeader(kSync, strLabelA, strLabelB, strTitleA, strTitleB);
			pDeDe->batchoutput_html_sxs_diff2(kSync, strOutput, pbHadChanges,
											  cTabStop,
											  &strLabelA, &strLabelB);
		}
		break;

	case gui_frame::MENU_EXPORT__FILE_DIFFS__TRADITIONAL__TEXT__FILE:
	case gui_frame::MENU_EXPORT__FILE_DIFFS__TRADITIONAL__TEXT__CLIPBOARD:
		pDeDe->batchoutput_text_traditional_diff2(kSync, strOutput, pbHadChanges);
		break;
	}

	return ue;
}

/*static*/ util_error ViewFile::s_doExportStringToDestination(de_de * pDeDe, long kSync, int /*gui_frame::_iface_id*/ id,
															  wxString & strOutput,
															  poi_item * pPoiDestination)
{
	util_error ue;

	// strOutput is in Unicode.
	// When going to the clipboard we can just copy it as is.
	// When going to a file we need to convert it back to some
	// character encoding -- hopefully related to one of the
	// input files.  When writing an HTML file, we ALWAYS use
	// UTF-8 since we declared that in the meta-data.

	switch (id)
	{
	default:
		ue.set(util_error::UE_UNSPECIFIED_ERROR, _T("Coding Error"));
		return ue;

	case gui_frame::MENU_EXPORT__FILE_DIFFS__UNIFIED__TEXT__FILE:
	case gui_frame::MENU_EXPORT__FILE_DIFFS__TRADITIONAL__TEXT__FILE:
		ue = pDeDe->getFsFs()->writeStringInEncodingToFile(kSync, pPoiDestination, strOutput, false);
		return ue;

	case gui_frame::MENU_EXPORT__FILE_DIFFS__UNIFIED__HTML__FILE:
	case gui_frame::MENU_EXPORT__FILE_DIFFS__TRADITIONAL__HTML__FILE:
	case gui_frame::MENU_EXPORT__FILE_DIFFS__SXS__HTML__FILE:
		ue = pDeDe->getFsFs()->writeStringInEncodingToFile(kSync, pPoiDestination, strOutput, true);
		return ue;

	case gui_frame::MENU_EXPORT__FILE_DIFFS__UNIFIED__TEXT__CLIPBOARD:
	case gui_frame::MENU_EXPORT__FILE_DIFFS__TRADITIONAL__TEXT__CLIPBOARD:
		if (wxTheClipboard->Open())
		{
			wxTextDataObject * pTDO = new wxTextDataObject(strOutput);
			wxTheClipboard->SetData(pTDO);
			wxTheClipboard->Close();
		}
		return ue;
	}
}

//////////////////////////////////////////////////////////////////

util_error ViewFile::_doExport(int /*gui_frame::_iface_id*/ id, long kSync)
{
	util_error ue;
	poi_item * pPoiDestination = NULL;		// destination *IFF* to a file.

	ue = _getExportPathnameFromUser(id, &pPoiDestination);
	if (ue.isErr())
		return ue;

	de_de * pDeDe = getDE();
	wxString strOutput;
	bool bHadChanges = false;
	
	ue = ViewFile::s_doExportToString(pDeDe, kSync, id,
									  strOutput, &bHadChanges,
									  getTabStop(),
									  m_title[(PanelIndex)0], m_title[(PanelIndex)1]);
	if (ue.isErr())
		return ue;

	//////////////////////////////////////////////////////////////////
	//
	// Unlike BATCH MODE from the COMMAND LINE, we *ALWAYS* write
	// whatever diff data we have.  I think the interactive usage
	// here is conceptually different from the command line. W3956.
	//
	//////////////////////////////////////////////////////////////////

	ue = ViewFile::s_doExportStringToDestination(pDeDe, kSync, id, strOutput, pPoiDestination);
	return ue;
}

void ViewFile::doExport(int /*gui_frame::_iface_id*/ id, long kSync)
{
	// TODO 2013/08/02 This techically only handles a diff and not a merge.
	// TODO            Move this to ViewFileDiff and make this abstract.
	// TODO            I'm leaving it here for now to think about having a
	// TODO            3-way output.
	// 
	// export the contents of the diff of the current (reference or edit) view
	// to a file or clipboard.

	util_error ue = _doExport(id, kSync);
	if (!ue.isOK() && (ue.getErr() != util_error::UE_CANCELED))
	{
		wxMessageDialog dlg(m_pFrame,ue.getMBMessage(),_("Error!"),wxOK|wxICON_ERROR);
		dlg.ShowModal();
	}
	
}

//////////////////////////////////////////////////////////////////

void ViewFile::preloadFindPanelFields(void)
{
	wxString strText = gpMyFindData->get();
	bool bMatchCase  = (!gpGlobalProps->getBool(GlobalProps::GPL_DIALOG_FIND_ICASE));

	m_pTextFind->SetValue(strText);
	m_pCheckFindMatchCase->SetValue(bMatchCase);

	// Clear the GOTO field everytime we show the FindPanel.
	// I don't think it makes much sense to preload this field
	// with either the last thing they typed into it or the
	// row of the caret -- since they are already there.
	m_pTextGoTo->SetValue(_T(""));
}

void ViewFile::showFindPanel(bool bShow, bool bFocusFind, bool bFocusGoTo)
{
	wxASSERT_MSG( (m_pPanelFind), _T("Coding Error") );

	if (m_pPanelFind->IsShown() == bShow)
	{
		// if panel is already in the desired state
		// don't bother re-showing/re-hiding it.
	}
	else
	{
		if (bShow)
		{
			preloadFindPanelFields();
		}
		else
		{
			// we write the terms back to global props on each Find
			// so we don't need to do it when this window is hidden.
		}

		m_pPanelFind->Show(bShow);
		Layout();

#if defined(__WXMSW__)
		if (bShow)
		{
			// P0574 -- force a full repaint
			m_pPanelFind->Refresh();
			// m_pPanelFind->Update();
		}
#endif
	}

	if (bShow)
	{
		if (bFocusFind)
		{
			m_pTextFind->SetFocus();
			m_pTextFind->SetSelection(-1,-1);
		}
		else if (bFocusGoTo)
		{
			m_pTextGoTo->SetFocus();
			m_pTextFind->SetSelection(-1,-1);
		}
	}
	
}

void ViewFile::onFindPanel_Close(wxCommandEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages, _T("onFindPanel_Close"));

	showFindPanel(false, false, false);
}

void ViewFile::onFindPanel_Text(wxCommandEvent & /*e*/)
{
	wxString strText = m_pTextFind->GetValue();
	//wxLogTrace(wxTRACE_Messages, _T("onFindPanel_Text [%s]"), strText.wc_str());
}

void ViewFile::onFindPanel_TextEnter(wxCommandEvent & /*e*/)
{
	wxString strText = m_pTextFind->GetValue();
	bool bPrevious = ::wxGetKeyState(WXK_SHIFT);
	//wxLogTrace(wxTRACE_Messages, _T("onFindPanel_TextEnter [%s][%d]"), strText.wc_str(), bPrevious);

	onFindPanel__do_search( !bPrevious );
}

void ViewFile::onFindPanel_Next(wxCommandEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages, _T("onFindPanel_Next"));

	onFindPanel__do_search(true);
}

void ViewFile::onFindPanel_Prev(wxCommandEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages, _T("onFindPanel_Prev"));

	onFindPanel__do_search(false);
}

void ViewFile::onFindPanel_MatchCase(wxCommandEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages, _T("onFindPanel_MatchCase [%d]"), m_pCheckFindMatchCase->GetValue());
}

FindResult ViewFile::onFindPanel__do_search(bool bForward)
{
	int kSync             = getSync();
	int kPanelWindowFocus = getPanelWithFocus();
	wxString strPattern   = m_pTextFind->GetValue();
	bool bIgnoreCase      = (!m_pCheckFindMatchCase->GetValue());

	FindResult result = find(kSync, (PanelIndex)kPanelWindowFocus,
							 strPattern,
							 bIgnoreCase, bForward);
	
	switch (result)
	{
	default:
		break;

	case FindResult_NoMatch:
		::wxBell();
		break;
	}

	// Since we did a Find, write the terms to global props
	// for other windows.  We do it now rather than waiting
	// for the Find Panel to close.  This should also play
	// nice with the Command+E stuff.

	gpGlobalProps->setBool(GlobalProps::GPL_DIALOG_FIND_ICASE, bIgnoreCase);
	gpMyFindData->set(strPattern);

	// TODO 2013/08/19 Should we call:
	// TODO            m_nbPage[kSync].m_pWinPanel[kPanelWindowFocus]->SetFocus();
	// TODO
	// TODO            Which would immediately let them start editing in the file
	// TODO            but would break ENTER implicitly doing a find-next/-prev.
	// TODO

	return result;
}

void ViewFile::onFindPanel_GoToTextEnter(wxCommandEvent & /*e*/)
{
	wxString strText = m_pTextGoTo->GetValue();

	// I can't get the TransferFromWindow stuff to work
	// so I'm just going to get the integer value from
	// the string directly.

	unsigned long ulVal = 0;
	if (!strText.ToULong(&ulVal))
	{
		// This should not happen since we have an integer validator on the field.
		::wxBell();
		return;
	}

	int kSync  = getSync();
	int kPanel = getPanelWithFocus();

	ViewFilePanel * pViewFilePanel = getPanel(kSync, (PanelIndex)kPanel);

	unsigned int uiVal = (unsigned int)ulVal;
	if (uiVal < 1)
		uiVal = 1;
	unsigned int uiLimit = getDE()->getLayout(kSync, (PanelIndex)kPanel)->getFormattedLineNrs();
	if (uiVal > uiLimit)
		uiVal = uiLimit;

	uiVal--;	// 1-based back to 0-based
	
	//wxLogTrace(wxTRACE_Messages, _T("onFindPanel_GoToTextEnter [%s][%d]"), strText.wc_str(), uiVal);

	pViewFilePanel->gotoLine(uiVal);

	// After we do the goto using the field text, we should
	// clear the field and return focus to the file panel.
	// (Otherwise, we'd need for the goto field to track the
	// caret (and the current file panel and current sync and
	// I don't want to do that).

	m_pTextGoTo->SetValue( _T("") );
	m_nbPage[kSync].m_pWinPanel[kPanel]->SetFocus();

}
