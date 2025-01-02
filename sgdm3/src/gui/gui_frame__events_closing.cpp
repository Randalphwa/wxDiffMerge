// gui_frame__events_closing.cpp
// window closing related events for frame.
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

bool gui_frame::_cleanupEditPanel(bool bCanVeto, bool bDirty)
{
	// return true if the user vetoes the close.
	// return false to allow the close to proceed.

	fs_fs * pFsFs = m_pDoc->getFsFs();

	if (!bDirty)
	{
		pFsFs->deleteAutoSaveFile();
		return false;
	}
	
	if (!bCanVeto)
	{
		// since we cannot veto the close, we must assume that
		// something is wrong and we're shutting down quickly.
		// force the auto-save to do it's thing if possible so
		// that the temp-file is at least complete.  this implies
		// that we **DO NOT** delete the auto-save file.

		pFsFs->forceAutoSaveNow();
		return false;
	}

	// otherwise, the edit panel is dirty and we have the choice
	// of closing the window or letting the user veto it.
	//
	// ask them if they want to save it before closing.

	// TODO see if we need to raise our window
	// TODO win32: see if we can get the dialog centered on the parent window rather than the screen.

	wxString strMessage = _("Do you want to save your changes in this window?");
				
	wxMessageDialog dlg(this,strMessage,_("File has been modified."),
						wxYES_NO | wxCANCEL | wxYES_DEFAULT | wxICON_QUESTION);
	int result = dlg.ShowModal();
	switch (result)
	{
	default:
	case wxID_CANCEL:		// abort close because user canceled (also prevents closeAll from continuing)
		return true;		// veto the close.

	case wxID_NO:			// close window without saving
		break;

	case wxID_YES:			// save it and close window
		{
			util_error ue = pFsFs->fileSave(this);
			switch (ue.getErr())
			{
			case util_error::UE_OK:
				break;			// file successfully saved, go on with closing.

			case util_error::UE_CANCELED:
				return true;	// if user aborted save at lower lever, abort closing.

			default:			// hard error trying to save file.  complain and abort window closing.
				{
					wxMessageDialog dlg2(this,ue.getMBMessage(),_("Error!"),wxOK|wxICON_ERROR);
					dlg2.ShowModal();
					return true;
				}
			}
		}
		break;

	}

	pFsFs->deleteAutoSaveFile();	// delete the auto-save file

	return false;
}

//////////////////////////////////////////////////////////////////

void gui_frame::onCloseEvent(wxCloseEvent & e)
{
	// EVT_CLOSE -- sent by window manager or system menu
	//              or by our calls to Close() from our menu.

	bool bCanVeto = e.CanVeto();
	//wxLogTrace(wxTRACE_Messages, _T("onCloseEvent: [canVeto %d]"),bCanVeto);

	if (m_pView && m_pDoc && m_pDoc->getFsFs())
	{
		ViewFile * pViewFile = static_cast<ViewFile *>(m_pView);
		if (pViewFile->getEditPanelPresent())
		{
			bool bDirty = PT_STAT_TEST(pViewFile->getEditPanelStatus(),PT_STAT_IS_DIRTY);

			bool bVetoed = _cleanupEditPanel(bCanVeto,bDirty);
			if (bVetoed)
			{
				e.Veto(true);
				return;
			}

			if ((wxGetApp().getExitStatusType() == MY_EXIT_STATUS_TYPE__MERGE)
				&& (wxGetApp().getExitStatusMergeFrame() == this))
			{
				// this window is the first window that we created and it is a
				// 3-way file merge and it is responsible for setting an exit status
				// for our invoker.

				// see if the user did a normal save to the original /result file
				// (as opposed to a save-as of some kind.)  if the user never saved
				// we want to abort the merge.

				poi_item * pPoiOriginalResult = wxGetApp().getExitStatusMergePoi();
				bool bDidSave = (pPoiOriginalResult->getSaveCount() > 0);

				// since we allow incremental/checkpoint saves as they are editing,
				// see if they have made any edits after the last checkpoint.  if
				// the edit buffer is dirty, we want to abort the merge (i'm ok with
				// this since we just gave them a chance to save).
				//
				// *BUT* we don't want to abort if they did a save to the original
				// result file, then continued editing using a Save As to a
				// different file.  Afterall, The purpose of the exit-status is
				// to inform invoking scripts what happened to the /result file
				// that they specified in the script.
				//
				// Note: we don't look at the poi(sync_edit,panel_edit) because we
				// don't want the poi of the edit buffer temp file.

				bDirty = PT_STAT_TEST(pViewFile->getEditPanelStatus(),PT_STAT_IS_DIRTY);
				fs_fs * pFsFs = m_pDoc->getFsFs();
				bool bNoSaveAs = ((pFsFs->getPoi(SYNC_VIEW,PANEL_T1) == pPoiOriginalResult)
								  || (pFsFs->getResultPOI() == pPoiOriginalResult));

				if (!bDidSave || (bDirty && bNoSaveAs))
				{
					// we think we should abort the merge, but let's ask for confirmation.
					
					long style = wxYES_NO | wxYES_DEFAULT | wxICON_QUESTION;
					if (bCanVeto)				// if they can stop the window from being
						style |= wxCANCEL;		// closed, we give them the option.

					wxString str(pPoiOriginalResult->getFullPath().wc_str());
					str += L"\n";
					str += L"\n";
					str += L"This window was given /result:<pathname> on the\n";
					str += L"command line.  This usually indicates that another\n";
					str += L"program is waiting for DiffMerge to write the merge\n";
					str += L"result to the above pathname and exit with the\n";
					str += L"appropriate status.\n";
					str += L"\n";
					if (!bDidSave)
						str += L"    DiffMerge has not written to this file.\n";
					else
					{
						str += L"    DiffMerge has written to the file, but\n";
						str += L"    you have unsaved edits.\n";
					}
					str += L"\n";
					str += L"Click YES to return ABORT-MERGE status (1).\n";
					str += L"Click NO to return MERGE-RESOLVED status (0).";
					if (bCanVeto)
						str += L"\nClick CANCEL to keep this window open.";

					wxMessageDialog dlg(this,str,_("Do You Want to Abort the Merge?"),style);
					int result = dlg.ShowModal();
					switch (result)
					{
					default:
					case wxID_CANCEL:
						e.Veto(true);
						return;

					case wxID_YES:
						wxGetApp().setExitStatusStatic(MY_EXIT_STATUS__ABORT_MERGE);
						break;

					case wxID_NO:
						wxGetApp().setExitStatusStatic(MY_EXIT_STATUS__OK);
						break;
					}
				}
				else
				{
					// the merge result was properly saved and all is well.
					wxGetApp().setExitStatusStatic(MY_EXIT_STATUS__OK);
				}
			}
		}
	}

	gpFrameFactory->unlinkFrame(this);		// remove us from list, but do not delete

	// gpFrameFactory only keeps track of gui_frames
	// and not dlg_nag frames.  if we closed the last
	// gui_frame and the dlg_nag is present
	// cancel/kill it.  this will block long enough for
	// the background thread to shut things down and die.
	int count = gpFrameFactory->countFrames();
	if (count == 0)
	{
		// W0562: Hide this window immediately so that
		// if we do block waiting on the background thread
		// we won't look like a dead window.  This has
		// been observed on Windows.
		Hide();
	}

	Destroy();								// let wxWidgets destroy the window and delete us
}

//////////////////////////////////////////////////////////////////

void gui_frame::onFileCloseWindow(wxCommandEvent & /*e*/)
{
	//wxLogTrace(wxTRACE_Messages, _T("onFileCloseWindow"));

	bool bForce = false;			// do not force us to close
	/*bool bClosed = */
	Close(bForce);	// send wxCloseEvent to our window

	//////////////////////////////////
	// 'this' may be deleted now if (bClosed).
	//////////////////////////////////

	//wxLogTrace(wxTRACE_Messages, _T("onFileCloseWindow: [closed %d]"), bClosed);
}

void gui_frame::onFileExit(wxCommandEvent & /*e*/)
{
	gpFrameFactory->onMenuFileExit(this);
}

//////////////////////////////////////////////////////////////////

void gui_frame::postEscapeCloseCommand(void)
{
	// See vault bug # 10312 -- some customers want SGDM to exit when they hit the ESC key
	// (instead or or in addition to the ALT+F4 (at least on Win32)) -- sort of like a dialog
	// rather than an app.  this violates the win32 user-interface guidelines, but oh well.

	// See vault bug # 12360 -- if user is dragging the vsplitter on a file window
	// and hits escape (without releasing the mouse), we eventually crash.  i've
	// reproduced this on all 3 platforms.
	// the cause varies by platform, but seems to be related to releasing capture
	// on a non-existant window and/or closing a window that's holding the mouse.
	// Since this is fairly obscure and there's no way to "properly" force a release,
	// let's just ignore this keystroke.

	if (GetCapture())
		return;

	// queue a File|Close (onFileCloseWindow()) menu event for when we are idle.

	wxCommandEvent eCmd(wxEVT_COMMAND_MENU_SELECTED, wxID_CLOSE);

	AddPendingEvent(eCmd);
}

