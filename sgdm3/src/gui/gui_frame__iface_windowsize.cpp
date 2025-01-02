// gui_frame__iface_windowsize.cpp
// portion of gui_frame devoted to saving/restoring window position/size.
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

void gui_frame::onSizeEvent(wxSizeEvent & e)
{
	e.Skip(true);		// ensure that superclass does whatever it needs to do

	wxSize sizeFrame = GetSize();
	bool bMaximized = IsMaximized();
	bool bIconized = IsIconized();

	if (!bMaximized && !bIconized)
	{
		m_sizeRestored = sizeFrame;
	}
	
//	wxLogTrace(wxTRACE_Messages, _T("gui_frame:onSizeEvent: [now (size %d,%d)] [max %d] [restored (size %d,%d)]"),
//			   sizeFrame.GetWidth(),sizeFrame.GetHeight(),
//			   bMaximized,
//			   m_sizeRestored.GetWidth(),m_sizeRestored.GetHeight());

	rememberOurSize();
}

void gui_frame::onMoveEvent(wxMoveEvent & e)
{
	e.Skip(true);		// ensure that superclass does whatever it needs to do

	wxPoint posFrame = GetPosition();
	bool bMaximized = IsMaximized();
	bool bIconized = IsIconized();

	if (!bMaximized && !bIconized)
	{
		m_posRestored = posFrame;
	}
	
//	wxLogTrace(wxTRACE_Messages, _T("gui_frame:onMoveEvent: [now (pos %d,%d)] [max %d] [restored (pos %d,%d)]"),
//			   posFrame.x,posFrame.y,
//			   bMaximized,
//			   m_posRestored.x,m_posRestored.y);

	rememberOurPosition();
}

//////////////////////////////////////////////////////////////////

/*static*/ wxSize gui_frame::suggestInitialSize(ToolBarType tbt)
{
	GlobalProps::EnumGPL keyW, keyH;

	switch (tbt)
	{
	default:
	case TBT_BASIC:
		keyW = GlobalProps::GPL_WINDOW_SIZE_BLANK_W;
		keyH = GlobalProps::GPL_WINDOW_SIZE_BLANK_H;
		break;

	case TBT_FOLDER:
		keyW = GlobalProps::GPL_WINDOW_SIZE_FOLDER_W;
		keyH = GlobalProps::GPL_WINDOW_SIZE_FOLDER_H;
		break;

	case TBT_DIFF:
		keyW = GlobalProps::GPL_WINDOW_SIZE_FILE_DIFF_W;
		keyH = GlobalProps::GPL_WINDOW_SIZE_FILE_DIFF_H;
		break;

	case TBT_MERGE:
		keyW = GlobalProps::GPL_WINDOW_SIZE_FILE_MERGE_W;
		keyH = GlobalProps::GPL_WINDOW_SIZE_FILE_MERGE_H;
		break;
	}

	long w = gpGlobalProps->getLong(keyW);
	long h = gpGlobalProps->getLong(keyH);

//	wxLogTrace(wxTRACE_Messages, _T("gui_frame::suggestInitialSize: [tbt %d][w %d][h %d]"), tbt,w,h);

	return wxSize(w,h);
}

//////////////////////////////////////////////////////////////////

/*static*/ wxPoint gui_frame::suggestInitialPosition(ToolBarType tbt)
{
	GlobalProps::EnumGPL keyX, keyY;

	switch (tbt)
	{
	default:
	case TBT_BASIC:
		keyX = GlobalProps::GPL_WINDOW_SIZE_BLANK_X;
		keyY = GlobalProps::GPL_WINDOW_SIZE_BLANK_Y;
		break;

	case TBT_FOLDER:
		keyX = GlobalProps::GPL_WINDOW_SIZE_FOLDER_X;
		keyY = GlobalProps::GPL_WINDOW_SIZE_FOLDER_Y;
		break;

	case TBT_DIFF:
		keyX = GlobalProps::GPL_WINDOW_SIZE_FILE_DIFF_X;
		keyY = GlobalProps::GPL_WINDOW_SIZE_FILE_DIFF_Y;
		break;

	case TBT_MERGE:
		keyX = GlobalProps::GPL_WINDOW_SIZE_FILE_MERGE_X;
		keyY = GlobalProps::GPL_WINDOW_SIZE_FILE_MERGE_Y;
		break;
	}

	long x = gpGlobalProps->getLong(keyX);
	long y = gpGlobalProps->getLong(keyY);

//	wxLogTrace(wxTRACE_Messages, _T("gui_frame::suggestInitialPosition: [tbt %d][x %d][y %d]"), tbt,x,y);

	return wxPoint(x,y);
}

//////////////////////////////////////////////////////////////////

/*static*/ bool gui_frame::suggestInitialMaximized(ToolBarType tbt)
{
	GlobalProps::EnumGPL key;

	switch (tbt)
	{
	default:
	case TBT_BASIC:
		key = GlobalProps::GPL_WINDOW_SIZE_BLANK_MAXIMIZED;
		break;

	case TBT_FOLDER:
		key = GlobalProps::GPL_WINDOW_SIZE_FOLDER_MAXIMIZED;
		break;

	case TBT_DIFF:
		key = GlobalProps::GPL_WINDOW_SIZE_FILE_DIFF_MAXIMIZED;
		break;

	case TBT_MERGE:
		key = GlobalProps::GPL_WINDOW_SIZE_FILE_MERGE_MAXIMIZED;
		break;
	}

	bool b = gpGlobalProps->getBool(key);

//	wxLogTrace(wxTRACE_Messages, _T("gui_frame::suggestInitialMaximized: [tbt %d][b %d]"), tbt,b);

	return b;
}

//////////////////////////////////////////////////////////////////

/*static*/ void gui_frame::verifyVisible(wxPoint * pPosFrame, wxSize * pSizeFrame)
{
	// given a suggested frame (position,size) determine if it is visible on screen.
	// this can happen if they unplug a monitor or reconfigure their relative layout.

	wxRect rTest(*pPosFrame,*pSizeFrame);

	unsigned cDisplays = wxDisplay::GetCount();
	for (unsigned k=0; k<cDisplays; k++)
	{
//		wxLogTrace(wxTRACE_Messages,_T("Display[%d]:"),k);

		wxDisplay dis(k);

		wxRect rClientArea = dis.GetClientArea();
//		wxLogTrace(wxTRACE_Messages,_T("    ClientArea: [x %d][y %d][w %d][h %d]"),
//				   rClientArea.x,rClientArea.y,rClientArea.width,rClientArea.height);		

		if (rClientArea.Intersects(rTest))		// rTest is visible on this display
		{
//			wxLogTrace(wxTRACE_Messages,_T("gui_frame:verifyVisible: suggested rectangle [x %d][y %d][w %d][h %d] is visible on display [%d]."),
//			   rTest.x,rTest.y,rTest.width,rTest.height, k);
			
			return;
		}
	}

	// suggested rectangle is not visible on any of the displays,
	// so override suggestion with something on the primary display.

//	wxLogTrace(wxTRACE_Messages,_T("gui_frame:verifyVisible: suggested rectangle [x %d][y %d][w %d][h %d] not visible on any display."),
//			   rTest.x,rTest.y,rTest.width,rTest.height);

	wxDisplay dis0((unsigned) 0);
	wxRect rClientArea0 = dis0.GetClientArea();

	pPosFrame->x = rClientArea0.x;
	pPosFrame->y = rClientArea0.y;
	pSizeFrame->SetWidth( rClientArea0.GetWidth() / 2 );
	pSizeFrame->SetHeight( rClientArea0.GetHeight() / 2 );
}

//////////////////////////////////////////////////////////////////

/*static*/ void gui_frame::computeCascadedPosition(wxPoint * pPosFrame, const wxSize * pSizeFrame)
{
	// compute a cascaded initial position from the given input.
	// that is, take the input position and add a reasonable delta
	// to it.  if this causes the bottom right corner of the window
	// to be offscreen, reset the cascade to the top left corner of
	// the display that the window is on (rather than the top left
	// corner of the primary display).

#if defined(__WXMSW__)
	int xDelta = (  wxSystemSettings::GetMetric(wxSYS_EDGE_X)
				  + wxSystemSettings::GetMetric(wxSYS_VSCROLL_X));
	int yDelta = (  wxSystemSettings::GetMetric(wxSYS_EDGE_Y)
				  + wxSystemSettings::GetMetric(wxSYS_CAPTION_Y));
#endif
#if defined(__WXGTK__)
	int xDelta = 2*wxSystemSettings::GetMetric(wxSYS_VSCROLL_X);
	int yDelta = 2*wxSystemSettings::GetMetric(wxSYS_HSCROLL_Y);
#endif
#if defined(__WXMAC__)
	int xDelta = 2*wxSystemSettings::GetMetric(wxSYS_VSCROLL_X);
	int yDelta = 2*wxSystemSettings::GetMetric(wxSYS_HSCROLL_Y);
#endif

	// find the display containing this point.  if the point isn't on a display
	// (this can happen if the top edge of the window is slightly above the top
	// of the screen), try the bottom-right corner of the proposed window.  if
	// this isn't on a screen, just punt and assume the primary display.

	int ndxDisplay = wxDisplay::GetFromPoint(*pPosFrame);
	if (ndxDisplay == wxNOT_FOUND)
	{
		wxPoint posBottomRight(pPosFrame->x + pSizeFrame->GetWidth(),
							   pPosFrame->y + pSizeFrame->GetHeight());
		ndxDisplay = wxDisplay::GetFromPoint(posBottomRight);
		if (ndxDisplay == wxNOT_FOUND)
			ndxDisplay = 0;
	}
	wxDisplay dis(ndxDisplay);
	wxRect rectDisplay = dis.GetClientArea();

#define LOOP_LIMIT 10
	for (int k=0; k<LOOP_LIMIT; k++)
	{
		// ensure top-left corner of poposed window is on screen. but in
		// the normal case, we want to do a delta from the given position.
		// if we already have a window at the newly computed cascade
		// position, try again.  we cap this at n times just to be safe.
	
		if (pPosFrame->x < rectDisplay.x)
			pPosFrame->x = rectDisplay.x;
		else
			pPosFrame->x += xDelta;
		if (pPosFrame->y < rectDisplay.y)
			pPosFrame->y = rectDisplay.y;
		else
			pPosFrame->y += yDelta;

		// if the bottom or right edge of the proposed windows is offscreen,
		// force us back to the top or left edge of the screen.

		if (pPosFrame->x + pSizeFrame->GetWidth() >= rectDisplay.x + rectDisplay.width)
			pPosFrame->x = rectDisplay.x;
		if (pPosFrame->y + pSizeFrame->GetHeight() >= rectDisplay.y + rectDisplay.height)
			pPosFrame->y = rectDisplay.y;

		if (!gpFrameFactory->haveFrameAtPosition(*pPosFrame))
			return;
	}
}

//////////////////////////////////////////////////////////////////

void gui_frame::rememberOurSize(void)
{
	if (!gpGlobalProps)
		return;

	bool bMaximized = IsMaximized();					// are we maximized
	
//	wxLogTrace(wxTRACE_Messages,
//			   _T("gui_frame::rememberOurSize: [tbt %d][w %d][h %d][max %d]"),
//			   m_tbt,
//			   m_sizeRestored.GetWidth(),m_sizeRestored.GetHeight(),
//			   bMaximized);

	// regardless what type of window we have, update the values for blank windows

	gpGlobalProps->setLong( GlobalProps::GPL_WINDOW_SIZE_BLANK_W, m_sizeRestored.GetWidth());
	gpGlobalProps->setLong( GlobalProps::GPL_WINDOW_SIZE_BLANK_H, m_sizeRestored.GetHeight());
	gpGlobalProps->setBool( GlobalProps::GPL_WINDOW_SIZE_BLANK_MAXIMIZED, bMaximized);
	
	// also update the values for our type of window

	switch (m_tbt)
	{
	default:
	case TBT_BASIC:	// already handled
		break;

	case TBT_FOLDER:
		gpGlobalProps->setLong( GlobalProps::GPL_WINDOW_SIZE_FOLDER_W, m_sizeRestored.GetWidth());
		gpGlobalProps->setLong( GlobalProps::GPL_WINDOW_SIZE_FOLDER_H, m_sizeRestored.GetHeight());
		gpGlobalProps->setBool( GlobalProps::GPL_WINDOW_SIZE_FOLDER_MAXIMIZED, bMaximized);
		break;

	case TBT_DIFF:
		gpGlobalProps->setLong( GlobalProps::GPL_WINDOW_SIZE_FILE_DIFF_W, m_sizeRestored.GetWidth());
		gpGlobalProps->setLong( GlobalProps::GPL_WINDOW_SIZE_FILE_DIFF_H, m_sizeRestored.GetHeight());
		gpGlobalProps->setBool( GlobalProps::GPL_WINDOW_SIZE_FILE_DIFF_MAXIMIZED, bMaximized);
		break;

	case TBT_MERGE:
		gpGlobalProps->setLong( GlobalProps::GPL_WINDOW_SIZE_FILE_MERGE_W, m_sizeRestored.GetWidth());
		gpGlobalProps->setLong( GlobalProps::GPL_WINDOW_SIZE_FILE_MERGE_H, m_sizeRestored.GetHeight());
		gpGlobalProps->setBool( GlobalProps::GPL_WINDOW_SIZE_FILE_MERGE_MAXIMIZED, bMaximized);
		break;
	}
}

void gui_frame::rememberOurPosition(void)
{
	if (!gpGlobalProps)
		return;

//	wxLogTrace(wxTRACE_Messages,
//			   _T("gui_frame::rememberOurPosition: [tbt %d][x %d][y %d]"),
//			   m_tbt,
//			   m_posRestored.x,m_posRestored.y);

	// regardless what type of window we have, update the values for blank windows

	gpGlobalProps->setLong( GlobalProps::GPL_WINDOW_SIZE_BLANK_X, m_posRestored.x);
	gpGlobalProps->setLong( GlobalProps::GPL_WINDOW_SIZE_BLANK_Y, m_posRestored.y);
	
	// also update the values for our type of window

	switch (m_tbt)
	{
	default:
	case TBT_BASIC:	// already handled
		break;

	case TBT_FOLDER:
		gpGlobalProps->setLong( GlobalProps::GPL_WINDOW_SIZE_FOLDER_X, m_posRestored.x);
		gpGlobalProps->setLong( GlobalProps::GPL_WINDOW_SIZE_FOLDER_Y, m_posRestored.y);
		break;

	case TBT_DIFF:
		gpGlobalProps->setLong( GlobalProps::GPL_WINDOW_SIZE_FILE_DIFF_X, m_posRestored.x);
		gpGlobalProps->setLong( GlobalProps::GPL_WINDOW_SIZE_FILE_DIFF_Y, m_posRestored.y);
		break;

	case TBT_MERGE:
		gpGlobalProps->setLong( GlobalProps::GPL_WINDOW_SIZE_FILE_MERGE_X, m_posRestored.x);
		gpGlobalProps->setLong( GlobalProps::GPL_WINDOW_SIZE_FILE_MERGE_Y, m_posRestored.y);
		break;
	}
}
