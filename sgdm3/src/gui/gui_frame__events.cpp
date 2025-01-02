// gui_frame__events.cpp
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fs.h>
#include <fd.h>
#include <rs.h>
#include <de.h>
#include <gui.h>

//////////////////////////////////////////////////////////////////

void gui_frame::onFileFolderDiff(wxCommandEvent & /*e*/)
{
	// file menu / open-folder-diff item
	// 
	// open a folder-diff window.  if we have don't have a
	// doc (and are just an empty window) use us.  otherwise,
	// create a new frame/window and let it do the work.

	//wxLogTrace(wxTRACE_Messages, _T("onFileFolderDiff"));

	gpFrameFactory->openFoldersFromDialogs(this);
}

void gui_frame::onFileFileDiff(wxCommandEvent & /*e*/)
{
	// file menu / open-file-diff item

	//wxLogTrace(wxTRACE_Messages, _T("onFileFileDiff"));

	gpFrameFactory->openFileDiffFromDialogs(this);
}

void gui_frame::onFileFileMerge(wxCommandEvent & /*e*/)
{
	// file menu / open-file-merge item

	//wxLogTrace(wxTRACE_Messages, _T("onFileFileMerge"));

	gpFrameFactory->openFileMergeFromDialogs(this);
}

//////////////////////////////////////////////////////////////////

void gui_frame::onFileReload(wxCommandEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages, _T("onFileReload"));

	if (m_pView && m_pDoc)
	{
		if (m_pDoc->getFdFd())
			m_pView->queueEvent(VIEWFOLDER_QUEUE_EVENT_LOAD);	// TODO should this be _EVENT_LOAD_SOFTMATCH ?
		else
			m_pView->queueEvent(VIEWFILE_QUEUE_EVENT_FORCE_RELOAD);
	}
}

void gui_frame::onUpdateFileReload(wxUpdateUIEvent & e)
{
	bool bEnable = (m_pView && m_pDoc && (m_pDoc->getFdFd() || m_pDoc->getFsFs()));

	e.Enable(bEnable);
}

//////////////////////////////////////////////////////////////////

void gui_frame::onFileChangeRuleset(wxCommandEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages, _T("onFileChangeRuleset"));

	fs_fs * pFsFs = m_pDoc->getFsFs();

	const rs_ruleset * pRSCurrent = pFsFs->getRuleSet();

	rs_choose_dlg__ruleset dlg(this,pRSCurrent);
	int nr = dlg.run();

	if (nr < -1)	// cancel or error
		return;

	const rs_ruleset * pRSChosen = ((nr == -1) ? gpRsRuleSetTable->getDefaultRuleSet() : gpRsRuleSetTable->getNthRuleSet(nr));

	if (pRSChosen == pRSCurrent)
		return;

	// change the ruleset on the FS_FS.  this will propagate a
	// change cb to all of the DE_DE's referencing it.  this
	// will cause them each invalidate the DE data and propagate
	// a cb to all of the views on them.  these will invalidate
	// their windows and redraw.

	pFsFs->changeRuleset(pRSChosen);
}

//////////////////////////////////////////////////////////////////

void gui_frame::onUpdateFileChangeRuleset(wxUpdateUIEvent & e)
{
	bool bEnable = (m_pView
					&& m_pDoc && m_pDoc->getFsFs()
					&& gpRsRuleSetTable && (gpRsRuleSetTable->getCountRuleSets() > 0));

	e.Enable(bEnable);
}

//////////////////////////////////////////////////////////////////

void gui_frame::onUpdateFileSave(wxUpdateUIEvent & e)
{
	//wxLogTrace(wxTRACE_Messages, _T("onUpdateFileSave"));

	bool bEnable = isEditableFileDirty();

	e.Enable(bEnable);
}

void gui_frame::onFileSave(wxCommandEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages, _T("onFileSave"));

	if (!isEditableFileDirty())
		return;

	m_pView->doSave();
}

//////////////////////////////////////////////////////////////////

void gui_frame::onUpdateFileSaveAs(wxUpdateUIEvent & e)
{
	//wxLogTrace(wxTRACE_Messages, _T("onUpdateFileSaveAs"));

	// allow Save-As in file diff/merge windows IFF we have an edit panel.

	bool bEnable = isEditableFileFrame();

	e.Enable(bEnable);
}

void gui_frame::onFileSaveAs(wxCommandEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages, _T("onFileSaveAs"));

	bool bEnable = isEditableFileFrame();
	if (!bEnable)
		return;

	m_pView->doSaveAs();
}

//////////////////////////////////////////////////////////////////

void gui_frame::onUpdateFileSaveAll(wxUpdateUIEvent & e)
{
	//wxLogTrace(wxTRACE_Messages, _T("onUpdateFileSaveAll"));

	bool bEnable = (gpFrameFactory->findFirstEditableFileFrame() != NULL);
	
	e.Enable(bEnable);
}

void gui_frame::onFileSaveAll(wxCommandEvent & e)
{
	//wxLogTrace(wxTRACE_Messages, _T("onFileSaveAll"));

	gpFrameFactory->saveAll(e);
}

//////////////////////////////////////////////////////////////////

void gui_frame::onUpdateEditUndo(wxUpdateUIEvent & e)
{
	//wxLogTrace(wxTRACE_Messages, _T("onUpdateEditUndo"));

	bool bEnable = false;

	if (m_pView  &&  m_pDoc  &&  m_pDoc->getFsFs())		// we are in a properly setup file window
	{
		ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);
		if (pViewFile->getCurrentNBSync()==SYNC_EDIT)
		{
			pt_stat s = pViewFile->getEditPanelStatus();
			bEnable = PT_STAT_TEST(s,PT_STAT_CAN_UNDO);
		}
	}

	e.Enable(bEnable);
}

void gui_frame::onEditUndo(wxCommandEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages, _T("onEditUndo"));

	if (m_pView  &&  m_pDoc  &&  m_pDoc->getFsFs())		// we are in a properly setup file window
	{
		ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);
		if (pViewFile->getCurrentNBSync()==SYNC_EDIT)
		{
			pt_stat s = pViewFile->getEditPanelStatus();
			if (PT_STAT_TEST(s,PT_STAT_CAN_UNDO))
			{
				ViewFilePanel * pViewFilePanel = pViewFile->getPanel( pViewFile->getCurrentNBSync(), PANEL_EDIT );
				if (pViewFilePanel)
					pViewFilePanel->undo();
			}
		}
	}
}

//////////////////////////////////////////////////////////////////

void gui_frame::onUpdateEditRedo(wxUpdateUIEvent & e)
{
	//wxLogTrace(wxTRACE_Messages, _T("onUpdateEditRedo"));

	bool bEnable = false;

	if (m_pView  &&  m_pDoc  &&  m_pDoc->getFsFs())		// we are in a properly setup file window
	{
		ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);
		if (pViewFile->getCurrentNBSync()==SYNC_EDIT)
		{
			pt_stat s = pViewFile->getEditPanelStatus();
			bEnable = PT_STAT_TEST(s,PT_STAT_CAN_REDO);
		}
	}

	e.Enable(bEnable);
}

void gui_frame::onEditRedo(wxCommandEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages, _T("onEditRedo"));

	if (m_pView  &&  m_pDoc  &&  m_pDoc->getFsFs())		// we are in a properly setup file window
	{
		ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);
		if (pViewFile->getCurrentNBSync()==SYNC_EDIT)
		{
			pt_stat s = pViewFile->getEditPanelStatus();
			if (PT_STAT_TEST(s,PT_STAT_CAN_REDO))
			{
				ViewFilePanel * pViewFilePanel = pViewFile->getPanel(pViewFile->getCurrentNBSync(), PANEL_EDIT );
				if (pViewFilePanel)
					pViewFilePanel->redo();
			}
		}
	}
}

//////////////////////////////////////////////////////////////////

void gui_frame::onUpdateEditCut(wxUpdateUIEvent & e)
{
	bool bEnable = false;
	
	if (m_pView  &&  m_pDoc  &&  m_pDoc->getFsFs())		// we are in a properly setup file window
	{
		ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);
		int kPanel = pViewFile->getPanelWithFocus();
		if ((pViewFile->getCurrentNBSync()==SYNC_EDIT) && (kPanel==PANEL_EDIT))
		{
			ViewFilePanel * pViewFilePanel = pViewFile->getPanel(pViewFile->getCurrentNBSync(), (PanelIndex)kPanel );
			if (pViewFilePanel)
				bEnable = pViewFilePanel->haveSelection();
		}
	}

	e.Enable(bEnable);
}

void gui_frame::onEditCut(wxCommandEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages, _T("onEditCut"));

	if (m_pView && m_pDoc && m_pDoc->getFsFs())
	{
		ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);
		int kPanel = pViewFile->getPanelWithFocus();
		if ((pViewFile->getCurrentNBSync()==SYNC_EDIT) && (kPanel==PANEL_EDIT))
		{
			ViewFilePanel * pViewFilePanel = pViewFile->getPanel(pViewFile->getCurrentNBSync(), (PanelIndex)kPanel );
			if (pViewFilePanel)
				pViewFilePanel->cutToClipboard();
		}
	}
}

//////////////////////////////////////////////////////////////////

void gui_frame::onUpdateEditCopy(wxUpdateUIEvent & e)
{
	bool bEnable = false;
	
	if (m_pView  &&  m_pDoc  &&  m_pDoc->getFsFs())		// we are in a properly setup file window
	{
		ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);
		int kPanel = pViewFile->getPanelWithFocus();
		if (kPanel != -1)
		{
			ViewFilePanel * pViewFilePanel = pViewFile->getPanel(pViewFile->getCurrentNBSync(), (PanelIndex)kPanel );
			if (pViewFilePanel)
				bEnable = pViewFilePanel->haveSelection();
		}
	}

	e.Enable(bEnable);
}

void gui_frame::onEditCopy(wxCommandEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages, _T("onEditCopy"));

	if (m_pView && m_pDoc && m_pDoc->getFsFs())
	{
		ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);
		int kPanel = pViewFile->getPanelWithFocus();
		if (kPanel != -1)
		{
			ViewFilePanel * pViewFilePanel = pViewFile->getPanel(pViewFile->getCurrentNBSync(), (PanelIndex)kPanel );
			if (pViewFilePanel)
				pViewFilePanel->copyToClipboard();
		}
	}
}

//////////////////////////////////////////////////////////////////

void gui_frame::onUpdateEditPaste(wxUpdateUIEvent & e)
{
	//wxLogTrace(wxTRACE_Messages, _T("onUpdateEditPaste"));
	bool bEnable = false;
	
	if (m_pView  &&  m_pDoc  &&  m_pDoc->getFsFs())		// we are in a properly setup file window
	{
		ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);
		int kPanel = pViewFile->getPanelWithFocus();
		if ((pViewFile->getCurrentNBSync()==SYNC_EDIT) && (kPanel==PANEL_EDIT))
		{
#if 0
			// the Open() causes an assert on GTK -- because somebody already
			// has the clipboard open ??
			//
			// also, on GTK the IsSupported() causes an async call w/ callback to
			// query the clipboard.  this seems a bit expensive (for us to use in
			// an idle cb) just to enable/disable the paste button when focus is
			// on the edit panel.
			//
			// so, for now, always enable the paste button when focus is on the
			// edit panel -- the actual paste code will test for TEXT content before
			// actually fetching it.

			if (wxTheClipboard->Open())
			{
				bEnable = wxTheClipboard->IsSupported(wxDF_TEXT);
				wxTheClipboard->Close();
			}
#else
			bEnable = true;
#endif
		}
	}
	
	e.Enable(bEnable);
}

void gui_frame::onEditPaste(wxCommandEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages, _T("onEditPaste"));

	if (m_pView && m_pDoc && m_pDoc->getFsFs())
	{
		ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);
		int kPanel = pViewFile->getPanelWithFocus();
		if ((pViewFile->getCurrentNBSync()==SYNC_EDIT) && (kPanel==PANEL_EDIT))
		{
			ViewFilePanel * pViewFilePanel = pViewFile->getPanel(pViewFile->getCurrentNBSync(), (PanelIndex)kPanel );
			if (pViewFilePanel)
				pViewFilePanel->pasteFromClipboard();
		}
	}
}

//////////////////////////////////////////////////////////////////

void gui_frame::onUpdateEditSelectAll(wxUpdateUIEvent & e)
{
	bool bEnable = (m_pView  &&  m_pDoc  &&  m_pDoc->getFsFs());

	e.Enable(bEnable);
}

void gui_frame::onEditSelectAll(wxCommandEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages, _T("onEditSelectAll"));

	if (m_pView && m_pDoc && m_pDoc->getFsFs())
	{
		ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);
		int kPanel = pViewFile->getPanelWithFocus();
		if (kPanel != -1)
		{
			ViewFilePanel * pViewFilePanel = pViewFile->getPanel(pViewFile->getCurrentNBSync(), (PanelIndex)kPanel );
			if (pViewFilePanel)
				pViewFilePanel->selectAll();
		}
	}
}

//////////////////////////////////////////////////////////////////

void gui_frame::onUpdateEditNextDelta   (wxUpdateUIEvent & e)		{ e.Enable( _onUpdateEdit__delta_or_conflict(true,  false) ); }
void gui_frame::onUpdateEditNextConflict(wxUpdateUIEvent & e)		{ e.Enable( _onUpdateEdit__delta_or_conflict(true,  true ) ); }
void gui_frame::onUpdateEditPrevDelta   (wxUpdateUIEvent & e)		{ e.Enable( _onUpdateEdit__delta_or_conflict(false, false) ); }
void gui_frame::onUpdateEditPrevConflict(wxUpdateUIEvent & e)		{ e.Enable( _onUpdateEdit__delta_or_conflict(false, true ) ); }

bool gui_frame::_onUpdateEdit__delta_or_conflict(bool bNext, bool bConflict)
{
	bool bEnable = false;

	if (m_pView  &&  m_pDoc  &&  m_pDoc->getFsFs())
	{
		ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);
		long row = ((bNext) ? pViewFile->getNextChange(bConflict) : pViewFile->getPrevChange(bConflict));
		bEnable = (row != -1);
	}

	return bEnable;
}

void gui_frame::onEditNextDelta   (wxCommandEvent & /*e*/)			{ _onEdit__delta_or_conflict(true,  false); }
void gui_frame::onEditNextConflict(wxCommandEvent & /*e*/)			{ _onEdit__delta_or_conflict(true,  true ); }
void gui_frame::onEditPrevDelta   (wxCommandEvent & /*e*/)			{ _onEdit__delta_or_conflict(false, false); }
void gui_frame::onEditPrevConflict(wxCommandEvent & /*e*/)			{ _onEdit__delta_or_conflict(false, true ); }

void gui_frame::_onEdit__delta_or_conflict(bool bNext, bool bConflict)
{
	if (!m_pView  ||  !m_pDoc  ||  !m_pDoc->getFsFs())
		return;

	ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);

	pViewFile->keyboardScroll_delta(bNext,bConflict);
}

//////////////////////////////////////////////////////////////////

bool gui_frame::_isEditAutoMergeEnabled(void)
{
	if (!m_pView  ||  !m_pDoc  ||  !m_pDoc->getFsFs())		// we are not in a properly setup file window
		return false;

	ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);
	if (pViewFile->getNrTopPanels() != 3)
		return false;
	
	if (pViewFile->getCurrentNBSync() != SYNC_EDIT)
		return false;

	pt_stat s = pViewFile->getEditPanelStatus();
	if (PT_STAT_TEST(s,PT_STAT_HAVE_AUTO_MERGED))	// can only do auto-merge once
		return false;

	return true;
}
	
void gui_frame::onUpdateEditAutoMerge(wxUpdateUIEvent & e)
{
	//wxLogTrace(wxTRACE_Messages, _T("onUpdateEditAutoMerge"));

	bool bEnable = _isEditAutoMergeEnabled();

	e.Enable(bEnable);
}

void gui_frame::onEditAutoMerge(wxCommandEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages, _T("onEditAutoMerge"));

	if (_isEditAutoMergeEnabled())
	{
		ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);
		ViewFilePanel * pViewFilePanel = pViewFile->getPanel(pViewFile->getCurrentNBSync(), PANEL_EDIT);
		pViewFilePanel->autoMerge();
	}
}

//////////////////////////////////////////////////////////////////

void gui_frame::onSettingsPreferences(wxCommandEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages, _T("onSettingsPreferences"));

	OptionsDlg dlg(this);
	dlg.ShowModal();
}

//////////////////////////////////////////////////////////////////

void gui_frame::onHelpContents(wxCommandEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages, _T("onHelpContents"));

	wxGetApp().ShowHelpContents();
}

void gui_frame::onHelpAbout(wxCommandEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages, _T("onHelpAbout"));

	AboutBox dlg(this, this);
	dlg.ShowModal();
}

//////////////////////////////////////////////////////////////////

void gui_frame::onUpdateFolderOpenFiles(wxUpdateUIEvent & e)
{
	bool bEnable = (m_pView && m_pView->isEnabled_FolderOpenFiles());

//	wxLogTrace(wxTRACE_Messages, _T("onUpdateFolderOpenFiles [bEnable %d]"), bEnable);

	e.Enable(bEnable);
}

void gui_frame::onFolderOpenFiles(wxCommandEvent & /*e*/)
{
	// open a file-diff window on the files in the currently selected row.

	//wxLogTrace(wxTRACE_Messages, _T("onFolderOpenFiles"));

	m_pView->onEvent_FolderOpenFiles();
}

//////////////////////////////////////////////////////////////////

void gui_frame::onUpdateFolderOpenFolders(wxUpdateUIEvent & e)
{
	bool bEnable = (m_pView && m_pView->isEnabled_FolderOpenFolders());

//	wxLogTrace(wxTRACE_Messages, _T("onUpdateFolderOpenFolders [bEnable %d]"), bEnable);

	e.Enable(bEnable);
}

void gui_frame::onFolderOpenFolders(wxCommandEvent & /*e*/)
{
	// open a folder-diff window on the subdirs in the currently selected row.

	//wxLogTrace(wxTRACE_Messages, _T("onFolderOpenFolders"));

	m_pView->onEvent_FolderOpenFolders();
}

//////////////////////////////////////////////////////////////////

#if defined(__WXMSW__)
void gui_frame::onUpdateFolderOpenShortcuts(wxUpdateUIEvent & e)
{
	bool bEnable = (m_pView && m_pView->isEnabled_FolderOpenShortcuts());

	e.Enable(bEnable);
}

void gui_frame::onFolderOpenShortcuts(wxCommandEvent & /*e*/)
{
	// open "show shortcut info" dialog on the current row

	m_pView->onEvent_FolderOpenShortcuts();
}
#endif

#if defined(__WXMAC__) || defined(__WXGTK__)
void gui_frame::onUpdateFolderOpenSymlinks(wxUpdateUIEvent & e)
{
	bool bEnable = (m_pView && m_pView->isEnabled_FolderOpenSymlinks());

	e.Enable(bEnable);
}

void gui_frame::onFolderOpenSymlinks(wxCommandEvent & /*e*/)
{
	// open "show symlink info" dialog on the current row

	m_pView->onEvent_FolderOpenSymlinks();
}
#endif

//////////////////////////////////////////////////////////////////

void gui_frame::onUpdateFolderShow__x(wxUpdateUIEvent & e, FD_SHOW_HIDE_FLAGS f_bit)
{
	if (m_pDoc && m_pDoc->getFdFd())
	{
		e.Enable(true);
		e.Check((m_pDoc->getFdFd()->getShowHideFlags() & f_bit) != 0);
	}
	else
	{
		e.Enable(false);
		e.Check(false);
	}
}

void gui_frame::onFolderShow__x(FD_SHOW_HIDE_FLAGS f_bit)
{
	if (!m_pView || !m_pDoc || !m_pDoc->getFdFd())
		return;	// probably should just assert these.

	// toggle value for bit "f_bit" in the current setings
	// for this window and update the global preference for
	// this bit in future windows.
	//
	// it doesn't look like wxWidgets gives us the current
	// value of the tb button/menu item.  so we must assume
	// that it is what we last set it to *ON THIS WINDOW*.

#define TOGGLE_BIT(v,b)	Statement( if (v & b) v &= ~b; else v |= b; )

	FD_SHOW_HIDE_FLAGS f_cur = m_pDoc->getFdFd()->getShowHideFlags();
	TOGGLE_BIT(f_cur, f_bit);
	m_pDoc->getFdFd()->setShowHideFlags(f_cur);

	// change the global default for this bit for future
	// folder windows.

	FD_SHOW_HIDE_FLAGS f_all = gpGlobalProps->getLong(GlobalProps::GPL_FOLDER_SHOW_HIDE_FLAGS);
	if ((f_all & f_bit) != (f_cur & f_bit))
	{
		// the current global setting for this bit
		// does not match the NEW current value of
		// this bit in this window.
		TOGGLE_BIT(f_all, f_bit);
		gpGlobalProps->setLong(GlobalProps::GPL_FOLDER_SHOW_HIDE_FLAGS, f_all);
	}

	// Folder windows used to have a single global set of
	// show/hide flags and all windows would update whenever
	// a toolbar button was toggled by listening for changes
	// to the global properties for the various GPL_FOLDER_SHOW_
	// values.  With the change to make these buttons per-window,
	// we need to schedule the refresh directly.

	m_pView->queueEvent(VIEWFOLDER_QUEUE_EVENT_BUILD);
}

//////////////////////////////////////////////////////////////////
	
void gui_frame::onUpdateFolderShowEqual(wxUpdateUIEvent & e)
{
	onUpdateFolderShow__x(e, FD_SHOW_HIDE_FLAGS__EQUAL);
}

void gui_frame::onFolderShowEqual(wxCommandEvent & /*e*/)
{
	onFolderShow__x(FD_SHOW_HIDE_FLAGS__EQUAL);
}

//////////////////////////////////////////////////////////////////

void gui_frame::onUpdateFolderShowEquivalent(wxUpdateUIEvent & e)
{
	onUpdateFolderShow__x(e, FD_SHOW_HIDE_FLAGS__EQUIVALENT);
}

void gui_frame::onFolderShowEquivalent(wxCommandEvent & /*e*/)
{
	onFolderShow__x(FD_SHOW_HIDE_FLAGS__EQUIVALENT);
}

//////////////////////////////////////////////////////////////////

void gui_frame::onUpdateFolderShowQuickMatch(wxUpdateUIEvent & e)
{
	if (gpGlobalProps->getLong(GlobalProps::GPL_FOLDER_QUICKMATCH_ENABLED))
	{
		onUpdateFolderShow__x(e, FD_SHOW_HIDE_FLAGS__QUICKMATCH);
	}
	else
	{
		e.Enable(false);
		e.Check(false);
	}
}

void gui_frame::onFolderShowQuickMatch(wxCommandEvent & /*e*/)
{
	onFolderShow__x(FD_SHOW_HIDE_FLAGS__QUICKMATCH);
}

//////////////////////////////////////////////////////////////////

void gui_frame::onUpdateFolderShowSingles(wxUpdateUIEvent & e)
{
	onUpdateFolderShow__x(e, FD_SHOW_HIDE_FLAGS__SINGLES);
}

void gui_frame::onFolderShowSingles(wxCommandEvent & /*e*/)
{
	onFolderShow__x(FD_SHOW_HIDE_FLAGS__SINGLES);
}

//////////////////////////////////////////////////////////////////

void gui_frame::onUpdateFolderShowFolders(wxUpdateUIEvent & e)
{
	onUpdateFolderShow__x(e, FD_SHOW_HIDE_FLAGS__FOLDERS);
}

void gui_frame::onFolderShowFolders(wxCommandEvent & /*e*/)
{
	onFolderShow__x(FD_SHOW_HIDE_FLAGS__FOLDERS);
}

//////////////////////////////////////////////////////////////////

void gui_frame::onUpdateFolderShowErrors(wxUpdateUIEvent & e)
{
	onUpdateFolderShow__x(e, FD_SHOW_HIDE_FLAGS__ERRORS);
}

void gui_frame::onFolderShowErrors(wxCommandEvent & /*e*/)
{
	onFolderShow__x(FD_SHOW_HIDE_FLAGS__ERRORS);
}

//////////////////////////////////////////////////////////////////

void gui_frame::_onUpdateViewFileShow__dop(wxUpdateUIEvent & e, de_display_ops dop)
{
	bool bEnable = false;
	bool bCheck  = false;

	// only allow _ALL,_CTX,... buttons to be used when looking at SYNC_VIEW.
	// it's very confusing to be editing a document while looking
	// at _CTX or _DIF's only -- things tend to get weird for the user.

	if (m_pView  &&  m_pDoc  &&  m_pDoc->getFsFs())
	{
		ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);
		long kSync = pViewFile->getSync();
		if (kSync == SYNC_VIEW)
		{
			bEnable = true;
			bCheck  = DE_DOP__IS_MODE(pViewFile->getDisplayOps(kSync), dop);
		}
		else
		{
			bCheck = (dop == DE_DOP_ALL);	// when SYNC_EDIT, force DE_DOP_ALL to be checked regardless
		}
	}
	
	e.Enable(bEnable);
	e.Check(bCheck);
}

void gui_frame::onUpdateViewFileShowAll(wxUpdateUIEvent & e)	{	_onUpdateViewFileShow__dop(e,DE_DOP_ALL);	}
void gui_frame::onUpdateViewFileShowDif(wxUpdateUIEvent & e)	{	_onUpdateViewFileShow__dop(e,DE_DOP_DIF);	}
void gui_frame::onUpdateViewFileShowCtx(wxUpdateUIEvent & e)	{	_onUpdateViewFileShow__dop(e,DE_DOP_CTX);	}
//void gui_frame::onUpdateViewFileShowEql(wxUpdateUIEvent & e)	{	_onUpdateViewFileShow__dop(e,DE_DOP_EQL);	}

void gui_frame::onViewFileShowAll(wxCommandEvent & /*e*/)		{	m_pView->onEvent_SetDisplayMode(SYNC_VIEW,DE_DOP_ALL);	}
void gui_frame::onViewFileShowDif(wxCommandEvent & /*e*/)		{	m_pView->onEvent_SetDisplayMode(SYNC_VIEW,DE_DOP_DIF);	}
void gui_frame::onViewFileShowCtx(wxCommandEvent & /*e*/)		{	m_pView->onEvent_SetDisplayMode(SYNC_VIEW,DE_DOP_CTX);	}
//void gui_frame::onViewFileShowEql(wxCommandEvent & /*e*/)		{	m_pView->onEvent_SetDisplayMode(SYNC_VIEW,DE_DOP_EQL);	}

//////////////////////////////////////////////////////////////////

void gui_frame::onUpdateViewFileIgnUnimportant(wxUpdateUIEvent & e)
{
	bool bEnable = false;
	bool bCheck  = false;
	
	// allow ignore-unimportant on both SYNC_VIEW and SYNC_EDIT.
	// but disallow when DOP is _EQL, since we're not showing any changes anyway.
	// also disallow when detail-level settings are too coarse.

	if (m_pView && m_pDoc && m_pDoc->getFsFs())
	{
		ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);
		de_display_ops dops = pViewFile->getDisplayOps( pViewFile->getSync() );

		if (   (DE_DOP__IS_MODE_EQL(dops))
			|| ( ! DE_DETAIL_LEVEL__USES_IGN_UNIMP(gpGlobalProps->getLong(GlobalProps::GPL_FILE_DETAIL_LEVEL))))
			bEnable = false;
		else
		{
			bEnable = true;
			bCheck = DE_DOP__IS_SET_IGN_UNIMPORTANT( dops );
		}
	}

	e.Enable(bEnable);
	e.Check(bCheck);
}

void gui_frame::onViewFileIgnUnimportant(wxCommandEvent & e)
{
	bool bOn = e.IsChecked();

	// allow ignore-unimportant on both SYNC_VIEW and SYNC_EDIT.

	if (m_pView && m_pDoc && m_pDoc->getFsFs())
	{
		ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);

		pViewFile->onEvent_SetDisplayBits(SYNC_VIEW,DE_DOP_IGN_UNIMPORTANT,bOn);
		if (pViewFile->getEditPanelPresent())
			pViewFile->onEvent_SetDisplayBits(SYNC_EDIT,DE_DOP_IGN_UNIMPORTANT,bOn);
	}
}

//////////////////////////////////////////////////////////////////

void gui_frame::onUpdateViewFileHideOmitted(wxUpdateUIEvent & e)
{
	bool bEnable = false;
	bool bCheck  = false;

	// only allow hide-omitted on SYNC_VIEW.
	// only allow hide-omitted when DOP is _ALL (because we always hide
	// them in the other modes).
	
	if (m_pView && m_pDoc && m_pDoc->getFsFs())
	{
		ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);
		long kSync = pViewFile->getSync();
		if (kSync == SYNC_VIEW)
		{
			de_display_ops dops = pViewFile->getDisplayOps(kSync);
			bEnable = DE_DOP__IS_MODE_ALL(dops);
			bCheck = DE_DOP__IS_SET(dops,DE_DOP_HIDE_OMITTED);
		}
	}

	e.Enable(bEnable);
	e.Check(bCheck);
}

void gui_frame::onViewFileHideOmitted(wxCommandEvent & e)
{
	//wxLogTrace(wxTRACE_Messages, _T("onViewFileHideOmitted"));

	bool bOn = e.IsChecked();

	m_pView->onEvent_SetDisplayBits(SYNC_VIEW,DE_DOP_HIDE_OMITTED,bOn);
}

//////////////////////////////////////////////////////////////////

void gui_frame::onUpdateViewFilePilcrow(wxUpdateUIEvent & e)
{
	bool bEnable = m_pDoc && m_pDoc->getFsFs();
	bool bCheck = m_pView && m_pView->getPilcrow();

	e.Enable(bEnable);
	e.Check(bCheck);
}

void gui_frame::onViewFilePilcrow(wxCommandEvent & e)
{
	//wxLogTrace(wxTRACE_Messages, _T("onViewFilePilcrow"));

	bool bOn = e.IsChecked();

	m_pView->onEvent_SetPilcrow(bOn);
}

//////////////////////////////////////////////////////////////////

void gui_frame::onUpdateViewFileLineNumbers(wxUpdateUIEvent & e)
{
	bool bEnable = m_pDoc && m_pDoc->getFsFs();
	bool bCheck = m_pView && m_pView->getShowLineNumbers();

	e.Enable(bEnable);
	e.Check(bCheck);
}

void gui_frame::onViewFileLineNumbers(wxCommandEvent & e)
{
	//wxLogTrace(wxTRACE_Messages, _T("onViewFileLineNumbers"));

	bool bOn = e.IsChecked();

	m_pView->onEvent_SetShowLineNumbers(bOn);
}

//////////////////////////////////////////////////////////////////

void gui_frame::onUpdateViewFileTab2(wxUpdateUIEvent & e)
{
	bool bEnable = m_pDoc && m_pDoc->getFsFs();
	bool bCheck  = m_pView && (m_pView->getTabStop() == 2);

	e.Enable(bEnable);
	e.Check(bCheck);
}

void gui_frame::onViewFileTab2(wxCommandEvent & /*e*/)
{
	m_pView->onEvent_setTabStop(2);
}

void gui_frame::onUpdateViewFileTab4(wxUpdateUIEvent & e)
{
	bool bEnable = m_pDoc && m_pDoc->getFsFs();
	bool bCheck  = m_pView && (m_pView->getTabStop() == 4);

	e.Enable(bEnable);
	e.Check(bCheck);
}

void gui_frame::onViewFileTab4(wxCommandEvent & /*e*/)
{
	m_pView->onEvent_setTabStop(4);
}

void gui_frame::onUpdateViewFileTab8(wxUpdateUIEvent & e)
{
	bool bEnable = m_pDoc && m_pDoc->getFsFs();
	bool bCheck  = m_pView && (m_pView->getTabStop() == 8);

	e.Enable(bEnable);
	e.Check(bCheck);
}

void gui_frame::onViewFileTab8(wxCommandEvent & /*e*/)
{
	m_pView->onEvent_setTabStop(8);
}

//////////////////////////////////////////////////////////////////

void gui_frame::onUpdatePrint(wxUpdateUIEvent & e)
{
	bool bEnable = (m_pDoc != NULL);

	e.Enable(bEnable);
}

void gui_frame::onPrint(wxCommandEvent & /*e*/)
{
	if (!gpPrintData)		gpPrintData     = gpGlobalProps->createPrintData();

	wxPrintDialogData dlgData(*gpPrintData);					// stuff the current print-data into a local print-dialog-data
	ViewPrintout * pPrintout = m_pView->createPrintout(NULL);	// create a printout from our view -- this manages the content

	wxPrinter printer(&dlgData);							// create a process manager to drive printing
	bool bResult = printer.Print(this,pPrintout,true);		// raise print-dialog and drive printing

	delete pPrintout;

	if (bResult)
	{
		// printing succeeded, save any changes to print-data.

		(*gpPrintData) = printer.GetPrintDialogData().GetPrintData();
		gpGlobalProps->savePrintData(gpPrintData);

		return;
	}
	
	if (wxPrinter::GetLastError() == wxPRINTER_ERROR)
	{
		wxMessageBox( _("There was a problem printing."), _("Printing"), wxOK);
		return;
	}

	// user cancelled
}

void gui_frame::onUpdatePrintPreview(wxUpdateUIEvent & e)
{
	bool bEnable = (m_pDoc != NULL);

	e.Enable(bEnable);
}

void gui_frame::onPrintPreview(wxCommandEvent & /*e*/)
{
	if (!gpPrintData)		gpPrintData     = gpGlobalProps->createPrintData();

	wxPrintDialogData dlgData(*gpPrintData);							// stuff the current print-data into a local print-dialog-data
	ViewPrintout * pPrintout1 = m_pView->createPrintout(NULL);			// one to draw scaled into preview frame
	ViewPrintout * pPrintout2 = m_pView->createPrintout(pPrintout1);	// and one to draw to printer if user clicks the print button

	wxPrintPreview * pPreview = new wxPrintPreview(pPrintout1, pPrintout2, &dlgData);	// pPreview now owns both pPrintout's
	if (!pPreview->Ok())
	{
		delete pPreview;
		wxMessageBox( _("There was a problem preparing preview."), _("Previewing"), wxOK);
		return;
	}

	wxPreviewFrame * pPreviewFrame = new MyPreviewFrame(pPreview,this,_("Print Preview"));	// frame now owns pPreview
//	pPreviewFrame->Centre(wxBOTH);
	pPreviewFrame->Initialize();

	// WXBUG: in 2.9.4 http://trac.wxwidgets.org/ticket/15104
	//        begin workaround for W3513 "blank print preview"
    wxPreviewCanvas* canvas = pPreview->GetCanvas();
    if (canvas)
        canvas->SetExtraStyle(wxWS_EX_PROCESS_IDLE);
	
	pPreviewFrame->Show();

	// this returns immediately -- preview is another top-level window
	// rather than being modal on our window like a dialog.
}

void gui_frame::onPageSetup(wxCommandEvent & /*e*/)
{
	if (!gpPrintData)		gpPrintData     = gpGlobalProps->createPrintData();
	if (!gpPageSetupData)	gpPageSetupData = gpGlobalProps->createPageSetupData();

	// stuff the current print-data into the print-data-copy
	// that's inside the page-setup-data.

	(*gpPageSetupData) = *gpPrintData;

	wxPageSetupDialog dlg(this,gpPageSetupData);
	int result = dlg.ShowModal();

	if (result != wxID_OK)
		return;

	// extract internal copies from dialog and copy to our persistent globals.

	(*gpPrintData)     = dlg.GetPageSetupData().GetPrintData();
	(*gpPageSetupData) = dlg.GetPageSetupData();

	// save new values to global props

	gpGlobalProps->savePrintData(gpPrintData);
	gpGlobalProps->savePageSetupData(gpPageSetupData);
}

//////////////////////////////////////////////////////////////////

bool gui_frame::isViewFileInsertMarkEnabled(void)
{
	bool bEnable = false;

	// don't allow if we aleady have a modeless dialog up for this frame.

	if (m_pDlgModelessInsertMark)
		return false;

	// only allow on SYNC_EDIT -or- when on SYNC_VIEW and DOP is _ALL (because we don't show MARKS in other modes).

	if (m_pView && m_pDoc && m_pDoc->getFsFs())
	{
		ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);
		long kSync = pViewFile->getSync();
		if (kSync == SYNC_VIEW)
		{
			de_display_ops dops = pViewFile->getDisplayOps(kSync);
			bEnable = DE_DOP__IS_MODE_ALL(dops);
		}
		else	// when SYNC_EDIT
			bEnable = true;
	}

	return bEnable;
}

void gui_frame::onUpdateViewFileInsertMark(wxUpdateUIEvent & e)
{
	bool bEnable = isViewFileInsertMarkEnabled();
	e.Enable(bEnable);
}

void gui_frame::onViewFileInsertMark(wxCommandEvent & /*e*/)
{
	bool bEnable = isViewFileInsertMarkEnabled();
	if (!bEnable)
		return;

	showInsertMarkDialog(NULL);
}

void gui_frame::showInsertMarkDialog(de_mark * pDeMarkInitial)
{
	// start modeless dialog on top of this frame window.

	wxASSERT_MSG( (isViewFileInsertMarkEnabled()), _T("Coding Error") );

	m_pDlgModelessInsertMark = new dlg_insert_mark(this,pDeMarkInitial);
	m_pDlgModelessInsertMark->Show(true);
}

//////////////////////////////////////////////////////////////////

void gui_frame::onUpdateViewFileDeleteAllMark(wxUpdateUIEvent & e)
{
	// enable if we have any marks (besides the trivial one)
	// and the insert-mark dialog is not up.

	bool bEnable = false;

	if (!m_pDlgModelessInsertMark && m_pView && m_pDoc && m_pDoc->getFsFs())
	{
		ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);
		if (pViewFile->getDE())		// incase toolbar updates before the diff-engine is fully instantiated
		{
			int kSync = pViewFile->getSync();
			int nrMarks = pViewFile->getDE()->getNrMarks(kSync);

			bEnable = (nrMarks > 1);	// always have trivial mark
		}
	}
	
	e.Enable(bEnable);
}

void gui_frame::onViewFileDeleteAllMark(wxCommandEvent & /*e*/)
{
	if (!m_pDlgModelessInsertMark && m_pView && m_pDoc && m_pDoc->getFsFs())
	{
		ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);
		int kSync = pViewFile->getSync();

		pViewFile->deleteAllMark(kSync);	// except for trivial mark
	}
}

//////////////////////////////////////////////////////////////////

void gui_frame::onUpdateViewFileEdit_Find_GoTo(wxUpdateUIEvent & e)
{
	// enable FindPanel (which includes new Find and GoTo fields)
	// when we are looking at 2/3 way file windows

	bool bEnable = (m_pView && m_pDoc && m_pDoc->getFsFs());

	e.Enable(bEnable);
}

void gui_frame::onViewFileEdit_Find_GoTo(wxCommandEvent & e)
{
	bool bFocusFind = false;
	bool bFocusGoTo = false;

	switch (e.GetId())
	{
	case wxID_FIND:
		bFocusFind = true;
		break;

	case MENU_EDIT_GOTO:
		bFocusGoTo = true;
		break;

	default:
		break;
	}

	if (m_pView && m_pDoc && m_pDoc->getFsFs())
	{
		ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);
		pViewFile->showFindPanel(true, bFocusFind, bFocusGoTo);
	}
}

void gui_frame::onUpdateViewFileEditFind__Next__Prev(wxUpdateUIEvent & e)
{
	// enable "Edit|FindNext" and "Edit|FindPrev" when we are looking at
	// 2/3 way file windows.

	bool bEnable = false;

	if (m_pView && m_pDoc && m_pDoc->getFsFs())
	{
#if defined(__WXMAC__)
		// always enable Find Next/Prev (without regard to whether we
		// have data to search for) because we don't want to poll
		// the system for NSFindPboard everytime the menu appears.
		bEnable = true;
#else
		// only enable Find Next/Prev if we have data to search for.
		if (gpMyFindData->haveData())
			bEnable = true;
#endif
	}

	e.Enable(bEnable);
}

void gui_frame::onViewFileEditFind__Next__Prev(wxCommandEvent & e)
{
	if (m_pView && m_pDoc && m_pDoc->getFsFs())
	{
#if defined(__WXMAC__)
		// normally we would only have enabled FindNext and FindPrev
		// if we have some data to search for), but with the NSFindPboard
		// changes, we let these commands raise the find dialog if
		// there's nothing to search for.
		wxString s = gpMyFindData->get();
		if (s.Length() == 0)
		{
			ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);
			pViewFile->showFindPanel(true, true, false);
			return;
		}
#else
		wxASSERT_MSG( (gpMyFindData->haveData()), _T("Coding Error") );
#endif

		ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);
		switch (e.GetId())
		{
		case MENU_EDIT_FIND_NEXT:
			pViewFile->findNext();
			return;
			
		case MENU_EDIT_FIND_PREV:
			pViewFile->findPrev();
			return;

		default:
			wxASSERT_MSG( (0), _T("Coding Error") );
			return;
		}
	}
}

void gui_frame::onUpdateViewFileEditUseSelectionForFind(wxUpdateUIEvent & e)
{
	bool bEnable = false;

	// enable "Edit|UseSelectionForFind" when we are looking at 2/3 way file windows.

	if (m_pView && m_pDoc && m_pDoc->getFsFs())
	{
		ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);

		if (pViewFile->isFindPanelVisible())
		{
			// TODO 2013/08/16 If the FindPanel is already open on
			// TODO            this File Window, do we want to allow them to
			// TODO            drag a new selection in one of the edit panels
			// TODO            and Command-E to replace the text field in the
			// TODO            FindPanel.
			// TODO
			// TODO            Likewise, if focus is in the FindPanel's text 
			// TODO            control, do we want it to grab that and replace
			// TODO            the field contents?  Feels weird.
			// TODO
			// TODO            For now, I'm going to say no.
		}
		else
		{
			int kPanel = pViewFile->getPanelWithFocus();
			if (kPanel != -1)
			{
				ViewFilePanel * pViewFilePanel = pViewFile->getPanel(pViewFile->getCurrentNBSync(), (PanelIndex)kPanel );
				if (pViewFilePanel)
					bEnable = pViewFilePanel->haveSelection();
			}
		}
	}

	e.Enable(bEnable);
}

void gui_frame::onViewFileEditUseSelectionForFind(wxCommandEvent & /*e*/)
{
	if (m_pView && m_pDoc && m_pDoc->getFsFs())
	{
		ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);
		int kPanel = pViewFile->getPanelWithFocus();
		if (kPanel != -1)
		{
			ViewFilePanel * pViewFilePanel = pViewFile->getPanel(pViewFile->getCurrentNBSync(), (PanelIndex)kPanel );
			if (pViewFilePanel)
				pViewFilePanel->copySelectionForFind();
		}
	}
}

//////////////////////////////////////////////////////////////////

int gui_frame::_getDefaultAction(PanelIndex kPanel)
{
	if (!m_pView || !m_pDoc || !m_pDoc->getFsFs())
		return ViewFilePanel::CTX__NO_MENU;

	ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);

	return pViewFile->computeDefaultAction(kPanel);
}

void gui_frame::_onUpdateViewFileEditApplyDefaultAction(wxUpdateUIEvent & e, PanelIndex kPanel)
{
	int defaultAction = _getDefaultAction(kPanel);
	bool bEnable = (   (defaultAction != ViewFilePanel::CTX__NO_MENU)
					&& (defaultAction != ViewFilePanel::CTX__NO_DEFAULT));

	e.Enable(bEnable);
}

void gui_frame::_onViewFileEditApplyDefaultAction(wxCommandEvent & /*e*/, PanelIndex kPanel)
{
	int defaultAction = _getDefaultAction(kPanel);
	if (   (defaultAction == ViewFilePanel::CTX__NO_MENU)
		|| (defaultAction == ViewFilePanel::CTX__NO_DEFAULT))
		return;	// should not happen

	ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);

	bool bAutoAdvance = gpGlobalProps->getBool(GlobalProps::GPL_MISC_AUTO_ADVANCE_AFTER_APPLY);

	pViewFile->applyDefaultAction(kPanel,bAutoAdvance);
}

//////////////////////////////////////////////////////////////////

void gui_frame::onUpdateViewFileEditApplyDefaultActionL(wxUpdateUIEvent & e)
{
	_onUpdateViewFileEditApplyDefaultAction(e,PANEL_T0);
}
void gui_frame::onUpdateViewFileEditApplyDefaultActionR(wxUpdateUIEvent & e)
{
	_onUpdateViewFileEditApplyDefaultAction(e,PANEL_T2);
}

//////////////////////////////////////////////////////////////////

void gui_frame::onViewFileEditApplyDefaultActionL(wxCommandEvent & e)
{
	_onViewFileEditApplyDefaultAction(e,PANEL_T0);
}

void gui_frame::onViewFileEditApplyDefaultActionR(wxCommandEvent & e)
{
	_onViewFileEditApplyDefaultAction(e,PANEL_T2);
}

//////////////////////////////////////////////////////////////////

void gui_frame::onWebhelp(wxCommandEvent & /*e*/)
{
	wxGetApp().ShowWebhelp();
}

//////////////////////////////////////////////////////////////////

void gui_frame::onVisitSG(wxCommandEvent & /*e*/)
{
	wxGetApp().ShowVisitSourceGear();
}

//////////////////////////////////////////////////////////////////

void gui_frame::onUpdateViewFileSplitVertically(wxUpdateUIEvent & e)
{
	bool bEnable = false;
	bool bCheck = false;
	long kSync = -1;

	if (m_pDoc && m_pDoc->getFsFs() && m_pView)
	{
		bEnable = true;
		ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);

		kSync = pViewFile->getCurrentNBSync();
		bCheck = pViewFile->areSplittersVertical(kSync);
	}

//	static long x = 0;
//	wxLogTrace(wxTRACE_Messages,
//			   _T("VSplitterUpdate: [sync %d][enable %d][check %d][counter %d]"),
//			   kSync,bEnable,bCheck,x++);

	e.Enable(bEnable);
	e.Check(bCheck);
}

void gui_frame::onViewFileSplitVertically(wxCommandEvent & /*e*/)
{
	if (m_pDoc && m_pDoc->getFsFs() && m_pView)
	{
		ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);
		pViewFile->setSplittersVertical(pViewFile->getCurrentNBSync(),true);
	}
}

void gui_frame::onUpdateViewFileSplitHorizontally(wxUpdateUIEvent & e)
{
	bool bEnable = false;
	bool bCheck = false;
	long kSync = -1;

	if (m_pDoc && m_pDoc->getFsFs() && m_pView)
	{
		bEnable = true;
		ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);

		kSync = pViewFile->getCurrentNBSync();
		bCheck = !pViewFile->areSplittersVertical(kSync);
	}
		
//	wxLogTrace(wxTRACE_Messages,
//			   _T("HSplitterUpdate: [sync %d][enable %d][check %d]"),
//			   kSync,bEnable,bCheck);

	e.Enable(bEnable);
	e.Check(bCheck);
}

void gui_frame::onViewFileSplitHorizontally(wxCommandEvent & /*e*/)
{
	if (m_pDoc && m_pDoc->getFsFs() && m_pView)
	{
		ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);
		pViewFile->setSplittersVertical(pViewFile->getCurrentNBSync(),false);
	}
}

//////////////////////////////////////////////////////////////////

void gui_frame::onUpdate__Export__FolderSummary__id(wxUpdateUIEvent & e)
{
	e.Enable( isFolder() );
}

void gui_frame::onExport__FolderSummary__id(wxCommandEvent & e)
{
	if (!isFolder())
		return;

	ViewFolder * pViewFolder = static_cast<ViewFolder *>(m_pView);

	switch (e.GetId())
	{
	case MENU_EXPORT__FOLDER_SUMMARY__HTML__FILE:
		pViewFolder->doExportAs(FD_EXPORT_FORMAT__HTML | FD_EXPORT__TO_FILE);
		return;

	case MENU_EXPORT__FOLDER_SUMMARY__CSV__FILE:
		pViewFolder->doExportAs(FD_EXPORT_FORMAT__CSV | FD_EXPORT__TO_FILE);
		return;

	case MENU_EXPORT__FOLDER_SUMMARY__RQ__FILE:
		pViewFolder->doExportAs(FD_EXPORT_FORMAT__RQ | FD_EXPORT__TO_FILE);
		return;
		
	case MENU_EXPORT__FOLDER_SUMMARY__RQ__CLIPBOARD:
		pViewFolder->doExportAs(FD_EXPORT_FORMAT__RQ | FD_EXPORT__TO_CLIPBOARD);
		return;

	default:
		return;
	}
}

//////////////////////////////////////////////////////////////////

void gui_frame::onUpdate__FileDiffExport__id(wxUpdateUIEvent & e)
{
	if (isFileDiff())
	{
		// in a file-diff window
		e.Enable(true);
		return;
	}
	if (isFolder())
	{
		if (m_pView && m_pView->isEnabled_FolderExportDiffFiles())
		{
			// in a folder window with a file pair highlighted
			e.Enable(true);
			return;
		}
	}

	e.Enable(false);
}

void gui_frame::onFileDiffExport__id(wxCommandEvent & e)
{
	if (isFileDiff())
	{
		if (m_pView && m_pDoc && m_pDoc->getFsFs())
		{
			ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);
			pViewFile->doExport( e.GetId(), pViewFile->getSync() );
		}
		return;
	}

	if (isFolder())
	{
		if (m_pView && m_pView->isEnabled_FolderExportDiffFiles())
		{
			m_pView->onEvent_FolderExportDiffFiles( e.GetId() );
		}
		return;
	}
}
