// MyStaticText.h
// a widget to replace wxStaticText in places where it's just
// stupid -or- behaves differently on all 3 platforms.
//////////////////////////////////////////////////////////////////

#ifndef H_MYSTATICTEXT_H
#define H_MYSTATICTEXT_H

//////////////////////////////////////////////////////////////////

typedef unsigned long MyStaticTextAttrs;

#define MY_STATIC_TEXT_ATTRS_ALIGN_LEFT		((MyStaticTextAttrs)0x00)		// pick one of these
#define MY_STATIC_TEXT_ATTRS_ALIGN_RIGHT	((MyStaticTextAttrs)0x01)
#define MY_STATIC_TEXT_ATTRS_ALIGN_CENTER	((MyStaticTextAttrs)0x02)
#define MY_STATIC_TEXT_ATTRS__ALIGN_MASK__	((MyStaticTextAttrs)0x0f)

#define MY_STATIC_TEXT_ATTRS_FLAGS_ELIDE	((MyStaticTextAttrs)0x10)		// elide (...) what we can, but then clip as necessary
#define MY_STATIC_TEXT_ATTRS__FLAGS_MASK__	((MyStaticTextAttrs)0xf0)

#define MY_STATIC_TEXT_ATTRS__DEFAULT__		(MY_STATIC_TEXT_ATTRS_ALIGN_LEFT)

//////////////////////////////////////////////////////////////////

class MyStaticText : public wxWindow
{
public:
	MyStaticText(wxWindow * pWinParent, long style, const wxString & strLabel,
				 MyStaticTextAttrs ta=MY_STATIC_TEXT_ATTRS__DEFAULT__);
	virtual ~MyStaticText(void) {};

	void				setText(const wxString & strLabel);
	void				setTextAttrs(MyStaticTextAttrs ta);

	MyStaticTextAttrs	getTextAttrs(void)					const	{ return m_ta; };

private:
	wxString			m_strLabel;
	wxString			m_strFormatted;
	MyStaticTextAttrs	m_ta;

private:
	void _format(void);
	
	void onSizeEvent(wxSizeEvent & e);
	void onPaintEvent(wxPaintEvent & e);

	DECLARE_EVENT_TABLE();

	//////////////////////////////////////////////////////////////////
	// override wxWindow base class functions
	//////////////////////////////////////////////////////////////////

public:
    virtual bool		AcceptsFocus()				const	{ return false; };
    virtual bool		HasTransparentBackground()			{ return true; };
	virtual wxSize		DoGetBestSize(void)			const;
	virtual bool		SetFont(const wxFont & font);
};

//////////////////////////////////////////////////////////////////

#endif//H_MYSTATICTEXT_H
