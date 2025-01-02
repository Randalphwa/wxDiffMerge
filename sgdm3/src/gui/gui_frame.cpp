// gui_frame.cpp
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fd.h>
#include <fs.h>
#include <gui.h>

//////////////////////////////////////////////////////////////////

class gui_frame_dnd : public wxFileDropTarget
{
public:
	gui_frame_dnd(gui_frame * pOwner)
		{
			m_pOwner = pOwner;
		};

	virtual bool OnDropFiles(wxCoord /*x*/, wxCoord /*y*/,
							 const wxArrayString & asPathnames)
		{
#if 0 && defined(DEBUG)
			int kLimit = asPathnames.GetCount();
			int k;
			wxString strDebug = wxString::Format(_T("onDropFiles: [count %d]:\n"), kLimit);
			for (k=0; k<kLimit; k++)
				strDebug += wxString::Format( _T("\t[%d]: '%s'\n"), k, asPathnames[k].wc_str());
			wxLogTrace(wxTRACE_Messages, _T("%s"), strDebug.wc_str());
#endif

			// HACK 2013/10/23 On Windows and Mac we must RETURN from
			// HACK            this method to unblock Windows Explorer
			// HACK            and Finder.  This means that if we raise
			// HACK            the Open Dialog here, they will be blocked
			// HACK            until they time out or we dismiss the dialog.
			// HACK            See G0162.
			// HACK
			// HACK            I tried setting up a custom event, stuffing
			// HACK            a copy of the pathnames in it, and calling
			// HACK            either AddPendingEvent() or QueueEvent() so
			// HACK            that we can return here and raise the dialog
			// HACK            the next time we go idle.  This worked on
			// HACK            Windows, but did not on Mac.
			// HACK
			// HACK            On Mac, the QueueEvent() properly returned
			// HACK            immediately and allowed us to return, *BUT*
			// HACK            the code in DoMultipartDropMessage (in Cocoa)
			// HACK            caused a local/nested event loop to run (like
			// HACK            when a dialog is running) which meant that
			// HACK            the event we queued was non-synchronous from
			// HACK            the point of view of this function, but *VERY*
			// HACK            synchronous from the point of view of the
			// HACK            overall DROP.  (sigh)
			// HACK
			// HACK            So the following hack queues the event
			// HACK            using a timer with a 1ms alarm.  This lets
			// HACK            the overall DROP to return all the way (and
			// HACK            release the other process) and then we wait
			// HACK            for the timer to go off before raising the
			// HACK            dialog.

			m_pOwner->HACK_remember_drop_target(asPathnames);
			m_pOwner->HACK_start_drop_target_timer();

#if 0 && defined(DEBUG)
			wxLogTrace(wxTRACE_Messages, _T("onDropFiles: returning"));
#endif
			return true;
		};

	virtual wxDragResult OnDragOver(wxCoord /*x*/, wxCoord /*y*/,
									wxDragResult def)
		{
			return def;
		};

private:
	gui_frame * m_pOwner;
};


//////////////////////////////////////////////////////////////////

#ifdef __WXMSW__
	// win32 uses icons from the resource file (rather than a XPM)
	// so that we get our icon associated with us under WindowsExplorer
	// or other times when we're not running.
	//
	// wxWidgets provides a wxFrame::SetIcon() to load a bitmap for
	// the title bar and the taskbar.  we ifdef the call so for win32
	// to use the resource rather then the XPM (both ways work, however).
	//
	// NOTE: wxWidgets does not provide expose API to set both Large and
	// Small icons on a wxFrame, so we use the 32x32 Large ICON
	// and let MSwindows automatically shrink it to 16x16 when it
	// needs a small icon (for the title bar or start menu or etc).
#endif

//////////////////////////////////////////////////////////////////

gui_frame::gui_frame(const wxString & caption, ToolBarType tbt)
	: wxFrame()
	, m_strCaption(caption)
	, m_chCaptionStatus( _T('-') )
	, m_pDlgModelessInsertMark(NULL)
{
	HACK__m_pasPathnamesDropTarget = NULL;
	HACK__m_timerDropTarget.SetOwner(this, HACK__ID_DROP_TARGET);

#if defined(__WXMAC__)
	SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif
	
	//wxLogTrace(TRACE_GUI_DUMP, _T("gui_frame::gui_frame: [%p][%s]"),this,util_printable_s(caption).wc_str());

	m_pMenuBar = NULL;
	m_pMenu_File = NULL;
	m_pMenu_Edit = NULL;
	m_pMenu_View = NULL;
	m_pMenu_Settings = NULL;
	m_pMenu_Help = NULL;

	m_pToolBar = NULL;
	m_tbt = TBT_BASIC;
	m_tbtMenu = TBT_BASIC;

	m_pDoc = NULL;
	m_pView = NULL;

	wxPoint posLocal = m_posRestored = suggestInitialPosition(tbt);
	wxSize sizeLocal = m_sizeRestored = suggestInitialSize(tbt);
	bool bMaximized = suggestInitialMaximized(tbt);

	if (gpFrameFactory->haveFrameAtPosition(posLocal))
	{
		// if we already have a window open at this position, we should do a
		// little cascade trick.  we try to keep it on the same display if
		// possible.

		computeCascadedPosition(&posLocal,&sizeLocal);
	}

	verifyVisible(&posLocal,&sizeLocal);

	Create((wxFrame *)NULL, -1, caption, posLocal, sizeLocal);

	CreateStatusBar(1);

#ifdef __WXMSW__
	SetIcon(wxIcon(_T("DiffMergeIcon"))); // effectively ICON_BIG
#else
	SetIcon(wxIcon(DiffMergeIcon_xpm));
#endif
	
	_iface_define_menu(tbt);
	_iface_define_toolbar(tbt);

	m_pEmptyView = new EmptyView(this);

#ifdef __WXMAC__
	// on the mac, adding the toolbar causes a resize of the frame (sigh)
	// so we reset it to what we asked for before it is shown (and/or we
	// maximize it).
	SetSize(posLocal.x,posLocal.y,sizeLocal.GetWidth(),sizeLocal.GetHeight());
#endif

	// defer actually setting maximize bit until after everything is created
	// because adding menu/toolbar causes restore on windows.

	if (bMaximized)
		Maximize();

	// warning: between the call to Maximize() and Show(), calls to GetPosition() and GetSize()
	// report fake info -- screen geometry, not actual window position/size.  Once Show()
	// completes, we get actual info on the window.

	Show(TRUE);

	SetDropTarget(new gui_frame_dnd(this));

#ifdef __WXMSW__	// HACK HACK HACK
	if (bMaximized)
	{
		// WXBUG: on Windows (maybe just Win2000 and maybe just my stupid Matrox G400 video
		// card) when we have dual monitors, the system sometimes says we have 1 *very* wide
		// screen (wxDisplay only reports 1), but the system maximize function maximizes us
		// to only one screen (as if we had 2).  [normally, wxDisplay reports 2 and we can
		// get the geometry of each and everything is fine.]
		//
		// when the system is confused like this, creating a new maximized window causes
		// it to not appear (and we don't get any error indication and it doesn't appear
		// in the task bar) yet our data structures think it is there.
		//
		// the following is an attempt to see if the system is confused.  if so, hide it and
		// reshow it.  this seems to clear up the confusion.

		if (wxDisplay::GetCount() == 1)
		{
			// we have too many ways to get the size of the screen(s).
			
			wxDisplay dis((const wxWindow*) 0);
			wxRect rectDisplayGeometry = dis.GetGeometry();
			wxRect rectDisplayClientArea = dis.GetClientArea();
			
#if 0 && defined(DEBUG)
			int wds,hds;
			::wxDisplaySize(&wds,&hds);

			int wsm = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
			int hsm = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);
#endif

			int xcdr,ycdr,wcdr,hcdr;
			::wxClientDisplayRect(&xcdr,&ycdr,&wcdr,&hcdr);
			
#if 0 && defined(DEBUG)
			wxLogTrace(wxTRACE_Messages,_T("gui_frame::gui_frame: [rectDisplayGeometry %d,%d]"),
					   rectDisplayGeometry.width,rectDisplayGeometry.height);
			wxLogTrace(wxTRACE_Messages,_T("gui_frame::gui_frame: [rectDisplayClientArea %d,%d]"),
					   rectDisplayClientArea.width,rectDisplayClientArea.height);
			wxLogTrace(wxTRACE_Messages,_T("gui_frame::gui_frame: [wxDisplaySize %d,%d]"),wds,hds);
			wxLogTrace(wxTRACE_Messages,_T("gui_frame::gui_frame: [GetMetrics %d,%d]"),wsm,hsm);
			wxLogTrace(wxTRACE_Messages,_T("gui_frame::gui_frame: [ClientDisplayRect %d,%d]"),wcdr,hcdr);
#endif

			// the dimensions of the actual window after it was maximized and shown.
			wxPoint posFrame = GetPosition();
			wxSize sizeFrame = GetSize();
			
#if 0 && defined(DEBUG)
			wxLogTrace(wxTRACE_Messages,_T("gui_frame::gui_frame: [FramePos %d,%d][FrameSize %d,%d]"),
					   posFrame.x,posFrame.y,sizeFrame.GetWidth(),sizeFrame.GetHeight());
#endif

			int whack = sizeFrame.GetWidth() - wcdr;
			if (whack < 0)
				whack = -whack;
			int hhack = sizeFrame.GetHeight() - hcdr;
			if (hhack < 0)
				hhack = -hhack;

#define EPSILON 32
			if ((whack > EPSILON) || (hhack > EPSILON))
			{
				Show(FALSE);
				Show(TRUE);
			}
		}
	}
#endif			// HACK HACK HACK
}

gui_frame::~gui_frame(void)
{
	//wxLogTrace(TRACE_GUI_DUMP, _T("gui_frame::~gui_frame: [%p]"),this);

	if (m_pDlgModelessInsertMark)
	{
		dlg_insert_mark * p = m_pDlgModelessInsertMark;
		p->frameDeleted();
		m_pDlgModelessInsertMark = NULL;
		p->Destroy();
	}

	rememberOurSize();
	rememberOurPosition();

	// WARNING: we delete our view -- but they must not delete their
	// WARNING: window objects -- since they are now children of the
	// WARNING: frame window and wxWidgets will delete it as it does
	// WARNING: the toolbar/menu/etc.

	// TODO deleting the following variables can take a *VERY* long time
	// TODO because we have to free/delete a lot of data structures.
	// TODO meanwhile, to the user it looks like we've frozen.  i'd like
	// TODO to put up an hourglass, but that causes problems on some
	// TODO platforms.

	View * pView = m_pView;
	m_pView = NULL;
	delete pView;

	DELETEP(m_pDoc);

	//gpFdFdTable->dump(0);
	//gpPoiItemTable->dump(0);
}

//////////////////////////////////////////////////////////////////

bool gui_frame::haveModelessDialog(void) const
{
	if (m_pDlgModelessInsertMark) return true;

	return false;
}

//////////////////////////////////////////////////////////////////

bool gui_frame::isEmptyFrame(void) const
{
	return (m_pDoc == NULL);
}

//////////////////////////////////////////////////////////////////

void gui_frame::revertToEmptyFrame(void)
{
	if (m_pView)
	{
		View * p = m_pView;
		m_pView = NULL;
		p->Destroy();		// destroy the wxWindow and let it delete the object
	}

	DELETEP(m_pDoc);

	_iface_define_menu(TBT_BASIC);
	_iface_define_toolbar(TBT_BASIC);

	setCaption(VER_APP_TITLE);
	setCaptionStatus( _T('-') );

	wxStatusBar * pStatusBar = GetStatusBar();
	if (pStatusBar)
	{
		int aWidths[] = { -1 };
		pStatusBar->SetFieldsCount(NrElements(aWidths),aWidths);
	}
		
	m_pEmptyView = new EmptyView(this);
	m_pEmptyView->Refresh(true);
}

//////////////////////////////////////////////////////////////////

/*static*/ ToolBarType gui_frame::suggestInitialType(const cl_args * pArgs)
{
	if (!pArgs)					return TBT_BASIC;
	if (pArgs->nrParams<2)		return TBT_BASIC;
	if (pArgs->bFolders)		return TBT_FOLDER;
	if (pArgs->nrParams==3)		return TBT_MERGE;
	return TBT_DIFF;
}

//////////////////////////////////////////////////////////////////

void gui_frame::setCaption(const wxString & strCaption)
{
	m_strCaption = strCaption;
	_setTitle();
}

void gui_frame::setCaptionStatus(const wxChar chStatus)
{
	m_chCaptionStatus = chStatus;
	_setTitle();
}

void gui_frame::_setTitle(void)
{
	wxString str;

	if (m_chCaptionStatus != _T('-'))
	{
		str = m_chCaptionStatus;
		str += _T(" ");
		str += m_strCaption;
	}
	else
		str = m_strCaption;

	SetTitle(str);
}

//////////////////////////////////////////////////////////////////

void gui_frame::_loadFolders(const wxString & s0, const wxString & s1, const cl_args * pArgs)
{
	// start the work of loading the folders into this frame.
	// we initialize the pathnames in the Doc, create the
	// widget hierarchy, and set the window title immediately.
	// we queue a request to actually do the loading (think
	// PostMessage(WM_USER...)) so that the widgets have a
	// chance to get realized and painted once before we go
	// heads-down in what might be a lengthy filesystem
	// operation.

	wxASSERT_MSG( (isEmptyFrame()), _T("Coding Error: gui_frame::_loadFolders: already have doc") );

	m_pEmptyView->Destroy();
	m_pEmptyView = NULL;

	_iface_define_menu(TBT_FOLDER);
	_iface_define_toolbar(TBT_FOLDER);

	m_pDoc = new Doc();
	m_pDoc->initFolderDiff(s0,s1);

	m_pView = new ViewFolder(this,m_pDoc,pArgs);
	m_pView->createLayout();

	if (m_strCaption == VER_APP_TITLE)		// if we have the default titlebar (no CL caption)
		m_pView->setTopLevelWindowTitle();	// install a custom one using folder names.

	wxStatusBar * pStatusBar = GetStatusBar();
	if (pStatusBar)
	{
		int aWidths[] = { -1, VIEWFOLDER_STATUSBAR_CELL_STATS_WIDTH };
		pStatusBar->SetFieldsCount(NrElements(aWidths),aWidths);
	}

	m_pView->queueEvent(VIEWFOLDER_QUEUE_EVENT_LOAD);
}

bool gui_frame::isFolder(const fd_fd * pFdFd) const
{
	// if arg null, return true if we are a folderdiff window.
	// if arg non-null, return true if we are folderdiff window and contain the given folder pair.

	if (!m_pDoc)				return false;		// no doc of any kind.
	if (m_tbt != TBT_FOLDER)	return false;		// frame has wrong type
	if (!pFdFd)					return true;		// if no arg, any folderdiff doc will do
	return (m_pDoc->getFdFd() == pFdFd);			// else require exact match
}

//////////////////////////////////////////////////////////////////

void gui_frame::_loadFileDiff(const wxString & s0, const wxString & s1, const cl_args * pArgs)
{
	// start the work of loading the files into this frame.
	// we initialize the pathnames in the Doc, create the
	// widget hierarchy, and set the window title immediately.
	// we queue a request to actually do the loading (think
	// PostMessage(WM_USER...)) so that the widgets have a
	// chance to get realized and painted once before we go
	// heads-down in what might be a lengthy filesystem,
	// diff-engine, and format operation.

	wxASSERT_MSG( (isEmptyFrame()), _T("Coding Error: gui_frame::_loadFileDiff: already have doc") );

	wxBusyCursor hourglass;							// cause hour-glass to appear

	m_pEmptyView->Destroy();
	m_pEmptyView = NULL;

	_iface_define_menu(TBT_DIFF);
	_iface_define_toolbar(TBT_DIFF);

	m_pDoc = new Doc();
	m_pDoc->initFileDiff(s0,s1,pArgs);		// initialize fs_fs (ref existing fs_fs or create new one to load piecetables from disk)

	m_pView = new ViewFileDiff(this,m_pDoc,pArgs);	// create view (widgets only) while we wait
	m_pView->createLayout();						// create widgets only

	if (m_strCaption == VER_APP_TITLE)		// if we have the default titlebar (no CL caption)
		m_pView->setTopLevelWindowTitle();	// install a custom one using folder names.

	wxStatusBar * pStatusBar = GetStatusBar();
	if (pStatusBar)
	{
		int aWidths[] = { VIEWFILE_STATUSBAR_CELL_STATUS_WIDTH, VIEWFILE_STATUSBAR_CELL_RS_WIDTH, VIEWFILE_STATUSBAR_CELL_CHARENC_WIDTH };
		pStatusBar->SetFieldsCount(NrElements(aWidths),aWidths);
	}

	m_pView->queueEvent(VIEWFILE_QUEUE_EVENT_LOAD);
}

bool gui_frame::isFileDiff(const fs_fs * pFsFs) const
{
	// if arg null, return true if we are a file-diff window.
	// if arg non-null, return true if we are a file-diff window and contain the given file pair.

	if (!m_pDoc)			return false;		// no doc of any kind.
	if (m_tbt != TBT_DIFF)	return false;		// frame has wrong type
	if (!pFsFs)				return true;		// if no arg, any filediff doc will do
	return (m_pDoc->getFsFs() == pFsFs);		// else require exact match
}

//////////////////////////////////////////////////////////////////

void gui_frame::_loadFileMerge(const wxString & s0, const wxString & s1, const wxString & s2, const cl_args * pArgs)
{
	// see note in _loadFileDiff()

	wxASSERT_MSG( (isEmptyFrame()), _T("Coding Error: gui_frame::_loadFileMerge: already have doc") );

	wxBusyCursor hourglass;

	m_pEmptyView->Destroy();
	m_pEmptyView = NULL;

	_iface_define_menu(TBT_MERGE);
	_iface_define_toolbar(TBT_MERGE);

	m_pDoc = new Doc();
	m_pDoc->initFileMerge(s0,s1,s2,pArgs);

	m_pView = new ViewFileMerge(this,m_pDoc,pArgs);
	m_pView->createLayout();

	if (m_strCaption == VER_APP_TITLE)		// if we have the default titlebar (no CL caption)
		m_pView->setTopLevelWindowTitle();	// install a custom one using folder names.

	wxStatusBar * pStatusBar = GetStatusBar();
	if (pStatusBar)
	{
		int aWidths[] = { VIEWFILE_STATUSBAR_CELL_STATUS_WIDTH, VIEWFILE_STATUSBAR_CELL_RS_WIDTH, VIEWFILE_STATUSBAR_CELL_CHARENC_WIDTH };
		pStatusBar->SetFieldsCount(NrElements(aWidths),aWidths);
	}

	m_pView->queueEvent(VIEWFILE_QUEUE_EVENT_LOAD);
}

bool gui_frame::isFileMerge(const fs_fs * pFsFs) const
{
	// if arg null, return true if we are a file-merge window.
	// if arg non-null, return true if we are file-merge window and contain the given file set.

	if (!m_pDoc)			return false;		// no doc of any kind.
	if (m_tbt != TBT_MERGE)	return false;		// frame has wrong type
	if (!pFsFs)				return true;		// if no arg, any file-merge doc will do
	return (m_pDoc->getFsFs() == pFsFs);		// else require exact match
}

//////////////////////////////////////////////////////////////////

bool gui_frame::isEditableFileFrame(void) const
{
	if (!m_pView  ||  !m_pDoc  ||  !m_pDoc->getFsFs())
		return false;

	ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);

	return (pViewFile->getEditPanelPresent());
}

bool gui_frame::isEditableFileDirty(void) const
{
	if (!isEditableFileFrame())
		return false;

	ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);
	pt_stat stat = pViewFile->getEditPanelStatus();

	return (PT_STAT_TEST(stat,PT_STAT_IS_DIRTY));
}

//////////////////////////////////////////////////////////////////

wxString gui_frame::dumpSupportInfo(const wxString & strIndent) const
{
	wxString str;

	str += wxString::Format(_T("%sWindow Title: "),strIndent.wc_str());
	str += GetTitle();
	str += _T("\n");

	if (isEmptyFrame())
		str += wxString::Format(_T("%sWindow Type: Empty\n"),strIndent.wc_str());
	else
		str += m_pView->dumpSupportInfo(strIndent);
	str += _T("\n");

	return str;
}

void gui_frame::sendEventToView(wxEvent *pEvent) const
{
	if (m_pView != NULL)
		wxQueueEvent(m_pView, pEvent);
	else if (m_pEmptyView != NULL)
		wxQueueEvent(m_pEmptyView, pEvent);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
// Declarations for gui_frame::EmptyView
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(gui_frame::EmptyView, wxWindow)
	EVT_KEY_DOWN(gui_frame::EmptyView::onKeyDownEvent)
END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////

gui_frame::EmptyView::EmptyView(gui_frame * pFrame)
	: wxWindow(pFrame,ID_VIEW_CLIENT,wxDefaultPosition,pFrame->GetClientSize(),
			   wxSUNKEN_BORDER|wxCLIP_CHILDREN|wxFULL_REPAINT_ON_RESIZE)
	, m_pFrame(pFrame)
{
	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));	// needed for Win32

	SetFocus();
}

gui_frame::EmptyView::~EmptyView()
{
}

void gui_frame::EmptyView::onKeyDownEvent(wxKeyEvent & e)
{
	//wxLogTrace(wxTRACE_Messages, _T("EmptyView::KeyDown: [%p][keycode %d]"),
	//		   this, e.GetKeyCode());

	switch (e.GetKeyCode())
	{
	default:			{ e.Skip();	return; }

	case WXK_ESCAPE:	{ m_pFrame->postEscapeCloseCommand(); return; }
	}
}

//////////////////////////////////////////////////////////////////

void gui_frame::HACK_remember_drop_target(const wxArrayString & asPathnames)
{
	if (HACK__m_pasPathnamesDropTarget)	// already processing a DROP.
		return;							// this shouldn't happen, but don't crash/leak.

	HACK__m_pasPathnamesDropTarget = new wxArrayString(asPathnames);
	
}

void gui_frame::HACK_start_drop_target_timer(void)
{
	if (HACK__m_pasPathnamesDropTarget)
		HACK__m_timerDropTarget.Start(1,wxTIMER_ONE_SHOT);
}

void gui_frame::HACK_on_drop_target_timer(wxTimerEvent & /*e*/)
{
	if (!HACK__m_pasPathnamesDropTarget)
		return;

	gpFrameFactory->openFrameRequestedByFinderOrDND(this, *HACK__m_pasPathnamesDropTarget);

	delete HACK__m_pasPathnamesDropTarget;
	HACK__m_pasPathnamesDropTarget = NULL;
}

