// ViewFile__subwin.cpp
// a portion of class ViewFile that provides a simple wrapper
// around wxWindow that properly handles size events and causes
// a Layout().
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
#include <gui.h>

#if 0
//////////////////////////////////////////////////////////////////
			
BEGIN_EVENT_TABLE(ViewFile::_my_sub_win, wxWindow)
	EVT_SIZE (ViewFile::_my_sub_win::onSizeEvent)
END_EVENT_TABLE();

//////////////////////////////////////////////////////////////////

ViewFile::_my_sub_win::_my_sub_win(wxWindow * pParent, long style)
	: wxWindow(pParent,wxID_ANY,wxDefaultPosition,wxDefaultSize,style)
{
//	SetBackgroundColour(*wxBLUE);
}

//////////////////////////////////////////////////////////////////

void ViewFile::_my_sub_win::onSizeEvent(wxSizeEvent & /*e*/)
{
	Layout();
}
#endif
