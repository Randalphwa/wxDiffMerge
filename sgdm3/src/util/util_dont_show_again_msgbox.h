// util_dont_show_again_msgbox.h
// a regular message box with an extra checkbox to suppress future showings.
//////////////////////////////////////////////////////////////////

#ifndef H_UTIL_DONT_SHOW_AGAIN_MSGBOX_H
#define H_UTIL_DONT_SHOW_AGAIN_MSGBOX_H

//////////////////////////////////////////////////////////////////

class util_dont_show_again_msgbox : public wxDialog
{
public:
	util_dont_show_again_msgbox(wxWindow * pParent, const wxString & strTitle, const wxString & strMessage, long buttonFlags=wxOK);
	int			run(bool * pbDontShowAgain);

	void		onButtonYes(wxCommandEvent & e);
	void		onButtonNo(wxCommandEvent & e);

	static int	msgbox(wxWindow * pParent, const wxString & strTitle, const wxString & strMessage, int key, long buttonFlags=wxOK);

protected:
	bool		m_bChecked;
	
	typedef enum {		ID_CHECKBOX=100,
	} ID;

	DECLARE_EVENT_TABLE();
};

//////////////////////////////////////////////////////////////////

#endif//H_UTIL_DONT_SHOW_AGAIN_MSGBOX_H
