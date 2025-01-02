// MyStaticText.cpp
// a widget to replace wxStaticText in places where it's just
// stupid -or- behaves differently on all 3 platforms.
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

//////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(MyStaticText, wxWindow)
	EVT_SIZE  (MyStaticText::onSizeEvent)
	EVT_PAINT (MyStaticText::onPaintEvent)
END_EVENT_TABLE();

//////////////////////////////////////////////////////////////////

MyStaticText::MyStaticText(wxWindow * pWinParent, long style, const wxString & strLabel,
						   MyStaticTextAttrs ta)
	: wxWindow(pWinParent,wxID_ANY,wxDefaultPosition,wxDefaultSize,style),
	  m_strLabel(strLabel),
	  m_ta(ta)
{
	_format();
}

//////////////////////////////////////////////////////////////////

void MyStaticText::setText(const wxString & strLabel)
{
	m_strLabel = strLabel;
	_format();
}

void MyStaticText::setTextAttrs(MyStaticTextAttrs ta)
{
	m_ta = ta;
	_format();
}

//////////////////////////////////////////////////////////////////

void MyStaticText::onSizeEvent(wxSizeEvent & e)
{
	_format();
	e.Skip();
}

//////////////////////////////////////////////////////////////////

void MyStaticText::onPaintEvent(wxPaintEvent & /*e*/)
{
	int xClient, yClient;
	GetClientSize(&xClient,&yClient);

#if defined(__WXGTK__)
	// WXBUG The GTK version crashes if the window has a negative
	// WXBUG size and we try to create a bitmap (via wxBufferedPaintDC)
	// WXBUG or sometimes if we try operate on it (dc.SetBackground).
	// WXBUG 
	// WXBUG One could ask why we're getting negative sizes on any
	// WXBUG window -- but that's a bigger question....
	// 
	// as a work-around, let's test it before try to use it.
	// even if negative, we need to create a trivial wxPaintDC
	// to satisfy the event.
	//
	// since this is fairly harmless, i'll let this code run on all
	// platforms -- rather than ifdef it.
#endif

	if ((xClient <= 0) || (yClient <= 0))
	{
		wxPaintDC dc(this);
		return;
	}

	// we use a simple wxPaintDC rather than a wxBufferedPaintDC so
	// that we get the default control background coloring/theme.

	wxPaintDC dc(this);

    wxFont font(GetFont());
    if (!font.Ok())
        font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);

    dc.SetFont(font);

	// TODO do we need to set fg colors ??

	wxCoord wText,hText;
	
	switch (m_ta & MY_STATIC_TEXT_ATTRS__ALIGN_MASK__)
	{
	default:
	case MY_STATIC_TEXT_ATTRS_ALIGN_LEFT:
		dc.DrawText(m_strFormatted,0,0);
		break;

	case MY_STATIC_TEXT_ATTRS_ALIGN_RIGHT:
		dc.GetTextExtent(m_strFormatted,&wText,&hText);
		dc.DrawText(m_strFormatted,xClient-wText,0);
		break;
	
	case MY_STATIC_TEXT_ATTRS_ALIGN_CENTER:
		dc.GetTextExtent(m_strFormatted,&wText,&hText);
		dc.DrawText(m_strFormatted,(xClient-wText)/2,0);
		break;
	}
	
}

//////////////////////////////////////////////////////////////////

bool MyStaticText::SetFont(const wxFont & font)
{
	bool bResult = wxWindow::SetFont(font);
	InvalidateBestSize();
//	DoSetSize(wxDefaultCoord,wxDefaultCoord,wxDefaultCoord,wxDefaultCoord,
//			  wxSIZE_AUTO_WIDTH | wxSIZE_AUTO_HEIGHT);
	Refresh();

	return bResult;
}

//////////////////////////////////////////////////////////////////

wxSize MyStaticText::DoGetBestSize(void) const
{
	// compute our "best" size -- since we are going to
	// clip (and optionally elide) the string, we only
	// care about the HEIGHT so that the overall widget
	// layout gets setup right.

    wxClientDC dc(wx_const_cast(MyStaticText *, this));
    wxFont font(GetFont());
    if (!font.Ok())
        font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);

    dc.SetFont(font);

    wxCoord w,h;
	dc.GetTextExtent(wxString(_T("X")), &w, &h);

    wxSize best(wxDefaultCoord,h);

    CacheBestSize(best);
    return best;
}

//////////////////////////////////////////////////////////////////

void MyStaticText::_format(void)
{
	m_strFormatted = m_strLabel.Strip(wxString::both);		// assume we can fit the entire string.
	if (m_ta & MY_STATIC_TEXT_ATTRS_FLAGS_ELIDE)			// if they requested that we try to elide the string (if necessary).
	{
		if (m_strFormatted.Length() > 3)					// if eliding wouldn't be just stupid.
		{
			int xClient,yClient;
			wxCoord wText,hText;

			GetClientSize(&xClient,&yClient);

			wxClientDC dc(this);
			wxFont font(GetFont());
			if (!font.Ok())
				font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
			dc.SetFont(font);
			dc.GetTextExtent(m_strFormatted,&wText,&hText);

			if (xClient <= wText)							// if window isn't wide enough, start eliding.
			{
				// we do a real stupid job -- just chop from the left as necessary.
				// this is intended for pathnames, and will elide the root/parent
				// directories first and save the individual file name for last.

				wxString strDot(_T("..."));
				wxCoord wDot,hDot;
				dc.GetTextExtent(strDot,&wDot,&hDot);
				
				wxString work = m_strFormatted;
				wxArrayInt ai;
				dc.GetPartialTextExtents(work,ai);
				wxCoord wLast = ai.Last();
				size_t nrLast = ai.GetCount() - 1;
				size_t nr;
				
				for (nr=0; (nr < nrLast); nr++)
					if (wDot + wLast - ai.Item(nr) < xClient)
						break;

				m_strFormatted = strDot + work.Mid(nr+1);
			}
		}
	}

	Refresh();
}
