// gui_frame__events_frame.cpp
// frame window events (excludes closing and menu/toolbar events)
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

void gui_frame::onActivateEvent(wxActivateEvent & e)
{
	bool bActive = e.GetActive();

	//wxLogTrace(wxTRACE_Messages, _T("Frame:Activate: [%p][b %d]"), this, bActive);

	if (m_pView)
		m_pView->activated(bActive);

	e.Skip();
}

void gui_frame::onSetFocusEvent(wxFocusEvent & e)
{
	//wxLogTrace(wxTRACE_Messages, _T("Frame:SetFocus: [%p] [previous focus %p] [trying to forward to %p]"),
	//		   this,e.GetWindow(),m_pView);

	if (m_pView)
		m_pView->specialSetFocus();
	else
		e.Skip();
}
