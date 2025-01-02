// ViewFolder.cpp -- a folder diff view
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fs.h>
#include <fd.h>
#include <gui.h>

//////////////////////////////////////////////////////////////////
// we tweak the various margins for the different platforms.  there aren't any hard-n-fast
// rules here -- just what i thought looked better -- with respect to the OS's window
// decorations and how the individual platform widgets look.

#if defined(__WXMSW__)
#	define MY_L_TITLE_MARGINS	4
#	define MY_R_TITLE_MARGINS	4
#	define MY_T_TITLE_MARGINS	4
#	define MY_B_TITLE_MARGINS	4
#	define MY_T_STATUS_MARGIN	10
#	define MY_R_STATUS_MARGIN	0
#	define MY_B_STATUS_MARGIN	0
#elif defined(__WXGTK__)
#	define MY_L_TITLE_MARGINS	4
#	define MY_R_TITLE_MARGINS	4
#	define MY_T_TITLE_MARGINS	4
#	define MY_B_TITLE_MARGINS	4
#	define MY_T_STATUS_MARGIN	10
#	define MY_R_STATUS_MARGIN	0
#	define MY_B_STATUS_MARGIN	0
#elif defined(__WXMAC__)
#	define MY_L_TITLE_MARGINS	4
#	define MY_R_TITLE_MARGINS	4
#	define MY_T_TITLE_MARGINS	4
#	define MY_B_TITLE_MARGINS	4
#	define MY_T_STATUS_MARGIN	10
#	define MY_R_STATUS_MARGIN	20			/* extra space because of OS's lower-right corner window resizer */
#	define MY_B_STATUS_MARGIN	0
#endif

//////////////////////////////////////////////////////////////////

static void _cb_long(void * pThis, const util_cbl_arg & arg)
{
	// static callback to let us know when global props that are interested in change.

	GlobalProps::EnumGPL id = (GlobalProps::EnumGPL)arg.m_l;

	ViewFolder * pViewFolder = (ViewFolder *)pThis;
	pViewFolder->gp_cb_long(id);
}

static void _cb_string(void * pThis, const util_cbl_arg & arg)
{
	// static callback to let us know when global props that are interested in change.

	GlobalProps::EnumGPS id = (GlobalProps::EnumGPS)arg.m_l;

	ViewFolder * pViewFolder = (ViewFolder *)pThis;
	pViewFolder->gp_cb_string(id);
}

//////////////////////////////////////////////////////////////////

ViewFolder::ViewFolder(gui_frame * pFrame, Doc * pDoc, const cl_args * pArgs)
	: View(pFrame,pDoc,pArgs),
	  m_pListCtrl(NULL),
	  m_nQueueEventReason(VIEWFOLDER_QUEUE_EVENT_NOTHING),
	  m_bTemporarilySuppressActivationChecks(false),
	  m_pBGTH(NULL),
	  m_pBusyCursor(NULL)
{
	// we no longer watch the global GPL_FOLDER_SHOW_ flags
	// as these are now per-window flags.

	gpGlobalProps->addLongCB  (GlobalProps::GPL_FOLDER_IGNORE_SUBDIR_FILTER,        _cb_long,  this);
	gpGlobalProps->addLongCB  (GlobalProps::GPL_FOLDER_IGNORE_SUFFIX_FILTER,        _cb_long,  this);
	gpGlobalProps->addLongCB  (GlobalProps::GPL_FOLDER_IGNORE_FULL_FILENAME_FILTER, _cb_long,  this);
	gpGlobalProps->addLongCB  (GlobalProps::GPL_FOLDER_IGNORE_PATTERN_CASE,	        _cb_long,  this);
	gpGlobalProps->addLongCB  (GlobalProps::GPL_FOLDER_IGNORE_MATCHUP_CASE,	        _cb_long,  this);

	gpGlobalProps->addStringCB(GlobalProps::GPS_FOLDER_SUBDIR_FILTER,               _cb_string,this);
	gpGlobalProps->addStringCB(GlobalProps::GPS_FOLDER_SUFFIX_FILTER,               _cb_string,this);
	gpGlobalProps->addStringCB(GlobalProps::GPS_FOLDER_FULL_FILENAME_FILTER,        _cb_string,this);

	// soft match settings

	gpGlobalProps->addLongCB  (GlobalProps::GPL_FOLDER_SOFTMATCH_MODE,                    _cb_long,  this);

	gpGlobalProps->addStringCB(GlobalProps::GPS_FOLDER_SOFTMATCH_SIMPLE_SUFFIX,           _cb_string,this);
	gpGlobalProps->addLongCB  (GlobalProps::GPL_FOLDER_SOFTMATCH_SIMPLE_IGNORE_EOL,       _cb_long,  this);
	gpGlobalProps->addLongCB  (GlobalProps::GPL_FOLDER_SOFTMATCH_SIMPLE_IGNORE_WHITESPACE,_cb_long,  this);
	gpGlobalProps->addLongCB  (GlobalProps::GPL_FOLDER_SOFTMATCH_SIMPLE_IGNORE_TAB,       _cb_long,  this);

	gpGlobalProps->addStringCB(GlobalProps::GPS_FILE_RULESET_SERIALIZED,                  _cb_string,this);
	gpGlobalProps->addLongCB  (GlobalProps::GPL_FILE_RULESET_ENABLE_CUSTOM_RULESETS,      _cb_long,  this);
	gpGlobalProps->addLongCB  (GlobalProps::GPL_FILE_RULESET_ENABLE_AUTOMATIC_MATCH,      _cb_long,  this);
	gpGlobalProps->addLongCB  (GlobalProps::GPL_FILE_RULESET_IGNORE_SUFFIX_CASE,          _cb_long,  this);

	// Note 2013/08/13 Filtering only gets called for pairs of files in a pair of folders,
	// Note            so both filenames should be identical and therefore _REQUIRE_COMPLETE_MATCH
	// Note            doesn't matter.  But eventually I want to be able to re-align things to
	// Note            compensate for moves/renames -- especially if we do VCS integration.
	gpGlobalProps->addLongCB  (GlobalProps::GPL_FILE_RULESET_REQUIRE_COMPLETE_MATCH,      _cb_long,  this);

//  we need to use the rs_ruleset_table::findBestRuleSet_without_asking() because
//  we ***cannot*** ask for each pair of line items in a folder window -- the user
//	would go nuts.
//	gpGlobalProps->addLongCB  (GlobalProps::GPL_FILE_RULESET_ASK_IF_NO_MATCH,             _cb_long,  this);

	gpGlobalProps->addLongCB  (GlobalProps::GPL_FOLDER_SOFTMATCH_RULESET_FILE_LIMIT_MB,   _cb_long,  this);
	gpGlobalProps->addLongCB  (GlobalProps::GPL_FOLDER_SOFTMATCH_RULESET_ALLOW_DEFAULT,   _cb_long,  this);

	// quickmatch settings

	gpGlobalProps->addLongCB  (GlobalProps::GPL_FOLDER_QUICKMATCH_ENABLED, _cb_long,   this);
	gpGlobalProps->addStringCB(GlobalProps::GPS_FOLDER_QUICKMATCH_SUFFIX,  _cb_string, this);
		
}

ViewFolder::~ViewFolder(void)
{
	// we do not delete m_pListCtrl because it will get deleted
	// by wxWidgets when the frame is destroyed.

	// we no longer watch the global GPL_FOLDER_SHOW_ flags
	// as these are now per-window flags.

	gpGlobalProps->delLongCB  (GlobalProps::GPL_FOLDER_IGNORE_SUBDIR_FILTER,        _cb_long,  this);
	gpGlobalProps->delLongCB  (GlobalProps::GPL_FOLDER_IGNORE_SUFFIX_FILTER,        _cb_long,  this);
	gpGlobalProps->delLongCB  (GlobalProps::GPL_FOLDER_IGNORE_FULL_FILENAME_FILTER, _cb_long,  this);
	gpGlobalProps->delLongCB  (GlobalProps::GPL_FOLDER_IGNORE_PATTERN_CASE,         _cb_long,  this);
	gpGlobalProps->delLongCB  (GlobalProps::GPL_FOLDER_IGNORE_MATCHUP_CASE,	        _cb_long,  this);

	gpGlobalProps->delStringCB(GlobalProps::GPS_FOLDER_SUBDIR_FILTER,               _cb_string,this);
	gpGlobalProps->delStringCB(GlobalProps::GPS_FOLDER_SUFFIX_FILTER,               _cb_string,this);
	gpGlobalProps->delStringCB(GlobalProps::GPS_FOLDER_FULL_FILENAME_FILTER,        _cb_string,this);

	// softmatch settings.

	gpGlobalProps->delLongCB  (GlobalProps::GPL_FOLDER_SOFTMATCH_MODE,                    _cb_long,  this);

	gpGlobalProps->delStringCB(GlobalProps::GPS_FOLDER_SOFTMATCH_SIMPLE_SUFFIX,           _cb_string,this);
	gpGlobalProps->delLongCB  (GlobalProps::GPL_FOLDER_SOFTMATCH_SIMPLE_IGNORE_EOL,       _cb_long,  this);
	gpGlobalProps->delLongCB  (GlobalProps::GPL_FOLDER_SOFTMATCH_SIMPLE_IGNORE_WHITESPACE,_cb_long,  this);
	gpGlobalProps->delLongCB  (GlobalProps::GPL_FOLDER_SOFTMATCH_SIMPLE_IGNORE_TAB,       _cb_long,  this);

	gpGlobalProps->delStringCB(GlobalProps::GPS_FILE_RULESET_SERIALIZED,                  _cb_string,this);
	gpGlobalProps->delLongCB  (GlobalProps::GPL_FILE_RULESET_ENABLE_CUSTOM_RULESETS,      _cb_long,  this);
	gpGlobalProps->delLongCB  (GlobalProps::GPL_FILE_RULESET_ENABLE_AUTOMATIC_MATCH,      _cb_long,  this);
	gpGlobalProps->delLongCB  (GlobalProps::GPL_FILE_RULESET_IGNORE_SUFFIX_CASE,          _cb_long,  this);
	gpGlobalProps->delLongCB  (GlobalProps::GPL_FILE_RULESET_REQUIRE_COMPLETE_MATCH,      _cb_long,  this);
//	gpGlobalProps->delLongCB  (GlobalProps::GPL_FILE_RULESET_ASK_IF_NO_MATCH,             _cb_long,  this);
	gpGlobalProps->delLongCB  (GlobalProps::GPL_FOLDER_SOFTMATCH_RULESET_FILE_LIMIT_MB,   _cb_long,  this);
	gpGlobalProps->delLongCB  (GlobalProps::GPL_FOLDER_SOFTMATCH_RULESET_ALLOW_DEFAULT,   _cb_long,  this);

	// quickmatch settings

	gpGlobalProps->delLongCB  (GlobalProps::GPL_FOLDER_QUICKMATCH_ENABLED, _cb_long,   this);
	gpGlobalProps->delStringCB(GlobalProps::GPS_FOLDER_QUICKMATCH_SUFFIX,  _cb_string, this);
}

//////////////////////////////////////////////////////////////////

#define M_PROGRESS 3
#define FIX 0
#define VAR 1

void ViewFolder::createLayout(void)
{
	// populate the body of a folder-diff window.  that is, create all widgets/controls that
	// will appear in the client area (between the toolbar and status bar) of a frame window.
	// the View base class creates a single child properly positioned within the frame.
	// create the widgets for a folder-diff view and set up the wxSizer on our window.
	// the base class will tickle the sizer when the frame changes size.

	// create child widgets

	//////////////////////////////////////////////////////////////////

	m_pListCtrl = new ViewFolder_ListCtrl(this,wxDefaultSize);

	// create layout for content -- like a stretchable dialog.
	// 
	// [Title1]<<spacer>>[Title2]   (only present when titles given)
	// [------------------------]
	// [........................]
	// [........ListCtrl........]
	// [........................]
	// [------------------------]
	
	wxBoxSizer * pSizerContent = new wxBoxSizer(wxVERTICAL);

	if (!m_title[0].IsEmpty() || !m_title[1].IsEmpty())
	{
		//////////////////////////////////////////////////////////////////
		// TODO consider using MyStaticText for these 2 titles rather than wxStaticText.
		// TODO see ViewFile__iface_defs.cpp for details.
		//////////////////////////////////////////////////////////////////

		wxBoxSizer * pSizerTitles = new wxBoxSizer(wxHORIZONTAL);
		pSizerTitles->Add( MY_L_TITLE_MARGINS,1,                        /*fixed    width*/0);
		pSizerTitles->Add( new wxStaticText(this,wxID_ANY, m_title[0]), /*fixed    width*/0);
		pSizerTitles->Add( 1,1,                                         /*variable width*/1);		// a spacer to push next field to right edge
		pSizerTitles->Add( new wxStaticText(this,wxID_ANY, m_title[1]), /*fixed    width*/0, wxALIGN_RIGHT);
		pSizerTitles->Add( MY_R_TITLE_MARGINS,1,                        /*fixed    width*/0);

		pSizerContent->Add(1,MY_T_TITLE_MARGINS,    /*fixed height*/0);
		pSizerContent->Add(pSizerTitles,            /*fixed height*/0,wxEXPAND/*horizontally*/);
		pSizerContent->Add(1,MY_B_TITLE_MARGINS,    /*fixed height*/0);
	}
	pSizerContent->Add(m_pListCtrl,              /*variable height*/1,wxEXPAND/*horizontally*/);

	_create_progress_panel();
	pSizerContent->Add(m_pPanelProgress, FIX, wxEXPAND, 0);

	//////////////////////////////////////////////////////////////////
	// associate top-most layout with our window

	SetSizer(pSizerContent);
	Layout();					// forcing an initial layout helps the mac.

	// give intial keyboard focus to the list ctrl

	m_pListCtrl->SetFocus();
}

void ViewFolder::_create_progress_panel(void)
{
	m_pPanelProgress = new wxPanel(this);
	{
		wxBoxSizer * pSizerProgress_V = new wxBoxSizer(wxVERTICAL);
		{
			pSizerProgress_V->Add( new wxStaticLine(m_pPanelProgress),
								   FIX, wxEXPAND, 0);

			wxBoxSizer * pSizerProgress_H = new wxBoxSizer(wxHORIZONTAL);
			{
				m_pStaticTextProgress = new wxStaticText(m_pPanelProgress, wxID_ANY, _T(""));
				pSizerProgress_H->Add( m_pStaticTextProgress,
									   VAR, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, M_PROGRESS);

				m_pGaugeProgress = new wxGauge(m_pPanelProgress, wxID_ANY, 1,
											   wxDefaultPosition, wxSize(200,-1));
				pSizerProgress_H->Add( m_pGaugeProgress,
									   FIX, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, M_PROGRESS);
			}
			pSizerProgress_V->Add( pSizerProgress_H, FIX, wxEXPAND | wxALL, M_PROGRESS);
		}
		m_pPanelProgress->SetSizer(pSizerProgress_V);
	}

	m_pPanelProgress->Show(false);
}

//////////////////////////////////////////////////////////////////

void ViewFolder::_formatStats(void)
{
	// compose stats message for status bar.

	wxString msg = m_pDoc->getFdFd()->formatStatsString();

	m_pFrame->GetStatusBar()->SetStatusText(msg,VIEWFOLDER_STATUSBAR_CELL_STATS);
}

//////////////////////////////////////////////////////////////////

void ViewFolder::setTopLevelWindowTitle(void)
{
	// construct and install a suitable title for our frame's titlebar.

	fd_fd * pFdFd = (m_pDoc ? m_pDoc->getFdFd() : NULL);
	if (!pFdFd)
		return;

	// try to compute a sane set of paths eliding the common
	// part of the beginning of the path.

	wxString strT0, strT1;

	wxFileName fnT0 = pFdFd->getRootPoi(0)->getFileName();
	wxFileName fnT1 = pFdFd->getRootPoi(1)->getFileName();

#if defined(_WXMSW_)
	wxString strVolT0 = fnT0.GetVolume();
	wxString strVolT1 = fnT1.GetVolume();
	if (strVolT0.Cmp(strVolT1) != 0)
	{
		strT0 += strVolT0;
		strT0 += wxFileName::GetVolumeSeparator();
		strT1 += strVolT1;
		strT1 += wxFileName::GetVolumeSeparator();
	}
#endif

	const wxChar * szPathSep = _T("/");

	const wxArrayString astrDirsT0 = fnT0.GetDirs();
	const wxArrayString astrDirsT1 = fnT1.GetDirs();

	int limitT0 = (int)astrDirsT0.GetCount();
	int limitT1 = (int)astrDirsT1.GetCount();
	int limit = MyMin(limitT0,limitT1) - 1;		// -1 because we want at least one dir component in each path

	int k = 0;
	while ((k<limit) && (astrDirsT0[k].Cmp(astrDirsT1[k])==0))
		k++;

	if (k > 0)			// we found a common prefix, elide it.
	{
		strT0 += _T("...");
		strT1 += _T("...");
	}

	for (int j=k; j<limitT0; j++)		// build rest of pathname after common portion
	{
		strT0 += szPathSep;
		strT0 += astrDirsT0[j];
	}
	for (int j=k; j<limitT1; j++)		// build rest of pathname after common portion
	{
		strT1 += szPathSep;
		strT1 += astrDirsT1[j];
	}

	// if our results are equal, just list the pathname once.

	// we need to build the string the hard way.  if there are unicode (non-ascii)
	// characters in either pathname, wxString::Format() goes out to lunch on Mac
	// and maybe Linux.  bug:12326
	//
	// if (strT0.Cmp(strT1) != 0)
	//	m_pFrame->SetTitle(	wxString::Format( _("%s, %s - %s"), strT0.wc_str(), strT1.wc_str(), VER_APP_TITLE));
	// else
	//	m_pFrame->SetTitle(	wxString::Format( _("%s - %s"), strT0.wc_str(), VER_APP_TITLE));

	wxString strTitle(strT0);
	if (strT0.Cmp(strT1) != 0)
	{
		strTitle += _T(", ");
		strTitle += strT1;
	}
	strTitle += _T(" - ");
	strTitle += VER_APP_TITLE;
	m_pFrame->SetTitle(strTitle);
}

//////////////////////////////////////////////////////////////////

void ViewFolder::activated(bool bActive)
{
	// our frame window received an wxActivateEvent

	if (bActive)
	{
		// our frame is now the active window (with highlighted title bar).

		if (m_bTemporarilySuppressActivationChecks)
			return;

		// see if the root folders we are presenting have changed -- if so
		// we should re-scan them -- that is, we should redo the treewalk.
		// since this is ***VERY*** time-consuming, we try to limit how often
		// we do it -- that is if we get activated, and we raise a modal dialog
		// (causing us to get deactivated) and the dialog is dismissed (causing
		// us to get activated), we don't want to run a full treewalk again.
		//
		// TODO while we are the active window, should we automatically
		// TODO rescan every n seconds/minutes so that we are fresh ?
		// TODO or just wait for activation -- lots of apps do that.
		// TODO
		// TODO consider starting timer for auto-rescan.

		// we post an event rather than doing all the work right now
		// so that the window gets fully activated -- and the mouse
		// event causing the activation gets cleared before we start
		// popping up message boxes.

		if (gpGlobalProps->getBool(GlobalProps::GPL_MISC_CHECK_FOLDERS_ON_ACTIVATE))
			queueEvent(VIEWFOLDER_QUEUE_EVENT_LOAD);
		
	}
	else			// our frame is no longer the active window
	{
		// this could be because another app (or another one of our frames)
		// became active -- OR -- it coulde be because we have a modal dialog
		// open on top of us.

		// TODO if we created a timer for auto-rescan, stop it.
	}
}

//////////////////////////////////////////////////////////////////

void ViewFolder::gp_cb_long(GlobalProps::EnumGPL id)
{
	// one of the LONG global props has changed. set flag to indicate
	// that we need to update/verify something.
	//
	// when the options/preferences dialog is closed, we may get upto
	// n events, if the user changed all n props that we're interested
	// in.  so, we don't reload/rebuild immediately because we don't
	// want do n consecutive treewalks....  instead, we queue up a
	// request and turn on the interval timer (with a very short value),
	// so that as soon as our caller is done with the dialog and returns
	// to the event loop, our timer should happen -- sort of like
	// doing an InvalidateRect() and hanging off of the PAINT event
	// of a PostMessage(WM_USER,....).

	switch (id)
	{
	case GlobalProps::GPL_FOLDER_IGNORE_SUBDIR_FILTER:
	case GlobalProps::GPL_FOLDER_IGNORE_SUFFIX_FILTER:
	case GlobalProps::GPL_FOLDER_IGNORE_FULL_FILENAME_FILTER:
	case GlobalProps::GPL_FOLDER_IGNORE_PATTERN_CASE:
	case GlobalProps::GPL_FOLDER_IGNORE_MATCHUP_CASE:
		queueEvent(VIEWFOLDER_QUEUE_EVENT_LOAD);
		break;

	case GlobalProps::GPL_FOLDER_SOFTMATCH_MODE:
	case GlobalProps::GPL_FOLDER_SOFTMATCH_SIMPLE_IGNORE_EOL:
	case GlobalProps::GPL_FOLDER_SOFTMATCH_SIMPLE_IGNORE_WHITESPACE:
	case GlobalProps::GPL_FOLDER_SOFTMATCH_SIMPLE_IGNORE_TAB:

	case GlobalProps::GPL_FILE_RULESET_ENABLE_CUSTOM_RULESETS:
	case GlobalProps::GPL_FILE_RULESET_ENABLE_AUTOMATIC_MATCH:
	case GlobalProps::GPL_FILE_RULESET_IGNORE_SUFFIX_CASE:
	case GlobalProps::GPL_FILE_RULESET_REQUIRE_COMPLETE_MATCH:
//	case GlobalProps::GPL_FILE_RULESET_ASK_IF_NO_MATCH:

	case GlobalProps::GPL_FOLDER_SOFTMATCH_RULESET_FILE_LIMIT_MB:
	case GlobalProps::GPL_FOLDER_SOFTMATCH_RULESET_ALLOW_DEFAULT:
		queueEvent(VIEWFOLDER_QUEUE_EVENT_LOAD_SOFTMATCH);
		break;

	case GlobalProps::GPL_FOLDER_QUICKMATCH_ENABLED:
		queueEvent(VIEWFOLDER_QUEUE_EVENT_LOAD_SOFTMATCH);
		break;

	default:
		break;
	}
}

void ViewFolder::gp_cb_string(GlobalProps::EnumGPS id)
{
	// one of the STRING global props has changed. set flag to indicate
	// that we need to update/verify something.

	switch (id)
	{
	case GlobalProps::GPS_FOLDER_SUBDIR_FILTER:
	case GlobalProps::GPS_FOLDER_SUFFIX_FILTER:
	case GlobalProps::GPS_FOLDER_FULL_FILENAME_FILTER:
		queueEvent(VIEWFOLDER_QUEUE_EVENT_LOAD);
		break;

	case GlobalProps::GPS_FOLDER_SOFTMATCH_SIMPLE_SUFFIX:
	case GlobalProps::GPS_FILE_RULESET_SERIALIZED:
		queueEvent(VIEWFOLDER_QUEUE_EVENT_LOAD_SOFTMATCH);
		break;

	case GlobalProps::GPS_FOLDER_QUICKMATCH_SUFFIX:
		queueEvent(VIEWFOLDER_QUEUE_EVENT_LOAD_SOFTMATCH);
		break;

	default:
		break;
	}
}

//////////////////////////////////////////////////////////////////

//void ViewFolder::onPaintEvent(wxPaintEvent & /*e*/)
//{
//	wxPaintDC dc(this);		// required by wxWindows even if we don't use it.
//
//	// TODO do we need to draw/erase anything
//}

//////////////////////////////////////////////////////////////////

void ViewFolder::queueEvent(int reason)
{
	// VIEWFOLDER_QUEUE_EVENT_LOAD_SOFTMATCH
	// do _LOAD, but also invalidate the softmatch cache
	// and the quick-match data.
	//
	// 
	// VIEWFOLDER_QUEUE_EVENT_LOAD:
	// queue up a request to load run the full treewalk and
	// populate our window.  we set a little delay to give
	// the widget creation a chance to complete and events
	// to settle down.  *AND* to let all of the changes from
	// the Options Dialog to signal.
	//
	// TODO this delay let GTK finish painting the new widgets
	// TODO before we dove off into the directories.  See if
	// TODO this is really necessary.
	//
	// TODO investigate if this is annoying when they hit the
	// TODO File/Reload menu item.
	//
	//
	// VIEWFOLDER_QUEUE_EVENT_BUILD:
	// rebuild the vector from the current treewalk data.
	// this is used when toggling the visibility of options
	// (like show-files-without-peers).

	switch (reason)
	{
	default:
	case VIEWFOLDER_QUEUE_EVENT_NOTHING:
		return;

	case VIEWFOLDER_QUEUE_EVENT_LOAD_SOFTMATCH:
	case VIEWFOLDER_QUEUE_EVENT_LOAD:
		m_nQueueEventReason |= reason;
		m_timer_MyQueue.Start(500,wxTIMER_ONE_SHOT);	// medium delay to let widgets get painted
		return;

	case VIEWFOLDER_QUEUE_EVENT_BUILD:
		m_nQueueEventReason |= reason;
		m_timer_MyQueue.Start(1,wxTIMER_ONE_SHOT);		// very short timer
		return;
	}
}

void ViewFolder::onTimerEvent_MyQueue(wxTimerEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages, _T("ViewFolder_MyQueue:Timer: [%p][int %d ms]"), this, e.GetInterval());

	switch (m_nQueueEventReason)
	{
	default:
	case VIEWFOLDER_QUEUE_EVENT_NOTHING:
		m_nQueueEventReason = VIEWFOLDER_QUEUE_EVENT_NOTHING;
		return;

	case VIEWFOLDER_QUEUE_EVENT_LOAD_SOFTMATCH:
	case VIEWFOLDER_QUEUE_EVENT_LOAD:
	case VIEWFOLDER_QUEUE_EVENT_BUILD:
		_timer_reload_folders();
		m_nQueueEventReason = VIEWFOLDER_QUEUE_EVENT_NOTHING;
		return;
	}
	
	// TODO if appropriate, start one-shot timer depending on our activation.
	// TODO this would auto-reload in 5 or 10 minutes if desired.

}

//////////////////////////////////////////////////////////////////

// fix the selection to highlight the same item if possible
// and refresh the stats.
void ViewFolder::_postprocess_reload(const wxString & strSelected)
{
	long ndxToSelect = -1;

	if (strSelected.Length() > 0)
		m_pDoc->getFdFd()->findItemByRelativePath(strSelected,&ndxToSelect);

	m_pListCtrl->populateControl(ndxToSelect);

	_formatStats();
}

//////////////////////////////////////////////////////////////////

class scan_folders_in_background : public util_background_thread_helper
{
public:
	scan_folders_in_background(ViewFolder * pViewFolder,
							   bool bUsePreviousSoftmatchResult)
		: m_pViewFolder(pViewFolder),
		  m_bUsePreviousSoftmatchResult(bUsePreviousSoftmatchResult)
		{
		};
	virtual ~scan_folders_in_background(void)
		{
		};

	virtual wxThread::ExitCode DoWork(void)
		{
			fd_fd * pFdFd = m_pViewFolder->getDoc()->getFdFd();

			// run the scan without holding lock.
			util_error ue = pFdFd->loadFolders(this, m_bUsePreviousSoftmatchResult);
			switch (ue.getErr())
			{
			case util_error::UE_CANCELED:
				setThreadResult( MTS__FINISHED_ABORTED, ue );
				return (wxThread::ExitCode)MTEC__ABORT;

			default:
				setThreadResult( MTS__FINISHED_ERROR, ue );
				return (wxThread::ExitCode)MTEC__ERROR;

			case util_error::UE_OK:
				setThreadResult( MTS__FINISHED_OK, ue );
				return (wxThread::ExitCode)MTEC__OK;
			}
		};
	
public:
	ViewFolder *		m_pViewFolder;
	bool				m_bUsePreviousSoftmatchResult;
};

util_error ViewFolder::_run_scan_using_background_thread(bool bUsePreviousSoftmatchResult,
														 const wxString & strSelected)
{
	util_error ue;

	if (m_pBGTH)
	{
		// we already have a BG thread ?
		wxASSERT_MSG( (0), _T("Coding Error") );
		ue.set(util_error::UE_UNSPECIFIED_ERROR, _T("Already have a thread?"));
		return ue;
	}

	// since the fd_fd.m_vec is rebuilt by fd_fd::loadFolders() (which
	// is now in a background thread) and used by the list-control as
	// a display list (in the gui thread) and the individual fd_items
	// within it are in-transit as the background thread does its work,
	// we should clear the display list so that the list control doesn't
	// try to display stuff during re-paints (even though the window is
	// disabled, we still get exposes).  so truncate the vector first
	// and tell the list control that is is empty before we begin.
	m_pDoc->getFdFd()->clearVec();
	m_pListCtrl->populateControl(-1);

	m_pStaticTextProgress->SetLabelText(_T(""));	// clear previous progress state
	m_pGaugeProgress->Pulse();						// and force indeterminate mode.

	m_pBGTH = new scan_folders_in_background(this, bUsePreviousSoftmatchResult);
	m_BGTH_strSelected = strSelected;
	m_pBGTH->getThreadState(&m_BGTH_last_cProgress);

	ue = m_pBGTH->create_and_run();
	if (ue.isErr())
	{
		delete m_pBGTH;
		m_pBGTH = NULL;
	}
	else
	{
		m_timer_MyProgress.Start(200, wxTIMER_CONTINUOUS);
		m_pPanelProgress->Show(true);
		Layout();
		gpMyBusyInfo->Increment();
		m_pBusyCursor = new wxBusyCursor();
	}

	return ue;
}

void ViewFolder::onTimerEvent_MyProgress(wxTimerEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages, _T("onTimerEvent_MyProgress:"));
	if (!m_pBGTH)
	{
		m_timer_MyProgress.Stop();
		m_pPanelProgress->Show(false);
		Layout();
		gpMyBusyInfo->Decrement();
		delete m_pBusyCursor;
		m_pBusyCursor = NULL;

		// if we don't have a thread, why is this timer going off ?
		wxASSERT_MSG( (0), _T("Coding Error"));
		// who knows what state the fd_fd and/or the display vector is in?
		// TODO 2013/08/23 should we bail ?
		return;
	}

	int cProgress, n, d;
	wxString str;
	util_error ue;
	util_background_thread_helper::MyThreadState mts = m_pBGTH->getThreadState(&cProgress, &n, &d, &str, &ue);
	if (cProgress != m_BGTH_last_cProgress)
	{
//		wxLogTrace(wxTRACE_Messages, _T("MyProgress: [cP %d] [%d / %d] '%s'"),
//				   cProgress, n, d, str.wc_str());
		m_BGTH_last_cProgress = cProgress;

		m_pStaticTextProgress->SetLabelText( str );
		if ((d < 0) || (n < 0))
		{
			m_pGaugeProgress->Pulse();
		}
		else
		{
			m_pGaugeProgress->SetRange( d );
			m_pGaugeProgress->SetValue( n );
		}
	}

	if (mts & util_background_thread_helper::MTS__TERMINAL__MASK)
	{
		m_timer_MyProgress.Stop();
		m_pPanelProgress->Show(false);
		Layout();
		gpMyBusyInfo->Decrement();
		delete m_pBusyCursor;
		m_pBusyCursor = NULL;

		//wxLogTrace(wxTRACE_Messages, _T("MyProgress: thread finished: %d [err %d]"), mts, ue.getErr());

		_postprocess_reload(m_BGTH_strSelected);

		if (ue.isErr())
		{
			wxMessageDialog dlg(m_pFrame,ue.getMBMessage(),_("Error!"),wxOK|wxICON_ERROR);
			dlg.ShowModal();
		}

		delete m_pBGTH;
		m_pBGTH = NULL;
	}
		
}

//////////////////////////////////////////////////////////////////

void ViewFolder::_timer_reload_folders(void)
{
	util_error err;

	// see if we currently have a selected row (this index is in the listctrl's
	// "display list" (fd_fd::m_vec)).  map this into the actual fd_item (which
	// stores info for the individual pair of files/folders).  this item persists
	// in the fd_item_table::m_map but can get deleted (if the files/folders have
	// been deleted between runs).  so we cache the relative pathname here so that
	// we can try to re-select the same item after the display list is rebuilt.

	long rowSelected;
	bool bItemSelected = m_pListCtrl->getSelectedRowIndex(&rowSelected);
	wxString strSelected;
	if (bItemSelected)
	{
		fd_item * pFdItem = m_pDoc->getFdFd()->getItem(rowSelected);
		if (pFdItem)
		{
			strSelected = pFdItem->getRelativePathname(0);
			if (strSelected.Length() == 0)
				strSelected = pFdItem->getRelativePathname(1);
		}
	}

	switch (m_nQueueEventReason)
	{
	case VIEWFOLDER_QUEUE_EVENT_LOAD_SOFTMATCH:		// loadFolders (force new softmatch) and call buildVec
		// start the thread to do the work.
		err = _run_scan_using_background_thread(false, strSelected);
		// we can only report any thread start-up error.
		break;

	case VIEWFOLDER_QUEUE_EVENT_LOAD:				// loadFolders (using previous softmatch results) and call buildVec
		// start the thread to do the work.
		err = _run_scan_using_background_thread(true, strSelected);
		// we can only report any thread start-up error.
		break;

	case VIEWFOLDER_QUEUE_EVENT_BUILD:
		// no thread required.  just rebuild the vector and refresh the window.
		m_pDoc->getFdFd()->buildVec();
		_postprocess_reload(strSelected);
		break;

	default:	// quiets compiler
		wxASSERT_MSG( (0), _T("Coding Error"));
		break;
	}

	if (err.isErr())
	{
		wxMessageDialog dlg(m_pFrame,err.getMBMessage(),_("Error!"),wxOK|wxICON_ERROR);
		dlg.ShowModal();
	}

}

//////////////////////////////////////////////////////////////////

ViewPrintout * ViewFolder::createPrintout(ViewPrintout * pPrintoutRelated)
{
	return new ViewPrintoutFolder(this,((pPrintoutRelated)
										? (static_cast<ViewPrintoutFolder *>(pPrintoutRelated))->getData()
										: NULL));
}

//////////////////////////////////////////////////////////////////

wxString ViewFolder::dumpSupportInfo(const wxString & strIndent) const
{
	wxString str;
	wxString strIndent2 = strIndent + _T("\t");
	fd_fd * pFdFd = m_pDoc->getFdFd();
	
	str += wxString::Format( _T("%sWindow Type: Folder\n"), strIndent.wc_str());

	str += wxString::Format( _T("%sPath[0]: "), strIndent2.wc_str());
	str += pFdFd->getRootPoi(0)->getFullPath();
	str += _T("\n");

	str += wxString::Format( _T("%sPath[1]: "), strIndent2.wc_str());
	str += pFdFd->getRootPoi(1)->getFullPath();
	str += _T("\n");
	
	str += wxString::Format( _T("%sStats: %s\n"),   strIndent2.wc_str(), pFdFd->formatStatsString().wc_str());

	// TODO see TODO note in gui_frame__events.cpp -- toolbar buttons are global
	// TODO for folder windows. (show folders, show equal, show singles, etc)
	// TODO so we don't need to list them here.

	return str;
}

//////////////////////////////////////////////////////////////////

void ViewFolder::specialSetFocus(void)
{
	m_pListCtrl->SetFocus();
}

//////////////////////////////////////////////////////////////////

void ViewFolder::doSave(void)
{
	// we need this because the base class is abstract.
}

void ViewFolder::doSaveAs(void)
{
	// we need this because the base class is abstract.
}

//////////////////////////////////////////////////////////////////

void ViewFolder::doExportAs(FD_EXPORT_FORMAT expfmt)
{
	// export the contents of the current folder window
	// to a file, clipboard, or whereever.  This is a
	// view using the current show/hide settings.
	// 
	// if we cause a dialog to appear, we want to suppress
	// the activation event when the dialog disappears.

	bool bOld = m_bTemporarilySuppressActivationChecks;
	m_bTemporarilySuppressActivationChecks = true;
	{
		util_error ue = m_pDoc->getFdFd()->exportVisibleItems(m_pFrame, expfmt);
		if (!ue.isOK() && (ue.getErr() != util_error::UE_CANCELED))
		{
			wxMessageDialog dlg(m_pFrame,ue.getMBMessage(),_("Error!"),wxOK|wxICON_ERROR);
			dlg.ShowModal();
		}
	}
	m_bTemporarilySuppressActivationChecks = bOld;
}

#if 0	// not currently needed; not tested
int ViewFolder::showModalDialogSuppressingActivationChecks(wxDialog & dlg)
{
	bool bOld = m_bTemporarilySuppressActivationChecks;

	m_bTemporarilySuppressActivationChecks = true;

	int result = dlg.ShowModal();

	m_bTemporarilySuppressActivationChecks = bOld;

	return result;
}
#endif

