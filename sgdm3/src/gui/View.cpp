// View.cpp
// base class for gui-layer representation of a view set.
// Our window will be the sole child window in the frame
// (between the toolbar and status bar).
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

BEGIN_EVENT_TABLE(View, wxWindow)
	EVT_SIZE					(View::onSizeEvent)
	EVT_SYS_COLOUR_CHANGED				(View::onSysColourChangedEvent)
//	EVT_PAINT					(View::onPaintEvent)
	EVT_TIMER					(ID_TIMER_MY_QUEUE,    View::onTimerEvent_MyQueue)
	EVT_TIMER					(ID_TIMER_MY_PROGRESS, View::onTimerEvent_MyProgress)

END_EVENT_TABLE();

//////////////////////////////////////////////////////////////////
// NOTE we create our single child window that will completely
// NOTE cover the client area of the frame.  according to the
// NOTE docs, this should be [GetClientAreaOrigin(),GetClientSize()]
// NOTE (the client area offset by the toolbar).  but using these
// NOTE values causes some nasty flashing and resize fights between
// NOTE our child widgets and the status bar.  so we just use the
// NOTE default position (0,0).  the toolbar seems to get trashed
// NOTE either way, so this seems a little quicker.  and the frame
// NOTE will resize everything anyway in a minute, so it really
// NOTE doesn't matter...
//
// NOTE we use GetClientSize() rather than wxDefaultSize because
// NOTE of display problems on the mac.

View::View(gui_frame * pFrame, Doc * pDoc, const cl_args * pArgs)
	: wxWindow(pFrame,ID_VIEW_CLIENT,wxPoint(0,0),pFrame->GetClientSize(),wxNO_BORDER | wxCLIP_CHILDREN)
	, m_pFrame(pFrame)
	, m_pDoc(pDoc)
{
	m_timer_MyQueue.SetOwner(this,ID_TIMER_MY_QUEUE);
	m_timer_MyProgress.SetOwner(this,ID_TIMER_MY_PROGRESS);

	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));	// needed for Win32

	if (!pArgs)
		return;
	
	for (int k=0; (k < pArgs->nrParams); k++)
		if (pArgs->bTitle[k])
			m_title[k] = pArgs->title[k];
}


/* Destructor for the base view */

/* virtual */
View::~View(void)
{
}

//////////////////////////////////////////////////////////////////

void View::onSizeEvent(wxSizeEvent & /*e*/)
{
	// tell the wxSizer managing our children to adapt to our new size.

	Layout();
}

void View::onSysColourChangedEvent(wxSysColourChangedEvent & e)
{
	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));	// needed for Win32
	Refresh();
	e.Skip();
}

