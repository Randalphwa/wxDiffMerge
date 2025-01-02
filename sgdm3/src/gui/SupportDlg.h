// SupportDlg.h
//////////////////////////////////////////////////////////////////

#ifndef H_SUPPORTDLG_H
#define H_SUPPORTDLG_H

//////////////////////////////////////////////////////////////////

class SupportDlg : public wxDialog
{
public:
	SupportDlg(wxWindow * pWindowParent,
			   gui_frame * pGuiFrameActive);

	typedef enum {	ID_BUTTON_COPY_ALL = 100,
					ID_TEXT,
	} ID;

	DECLARE_EVENT_TABLE();

public:	
	void			onEventButton_CopyAll(wxCommandEvent & e);

private:
	wxString		_createMessage(void);

	wxWindow *		m_pWindowParent;
	gui_frame *		m_pGuiFrameActive;
	wxTextCtrl *	m_pTextCtrl;
};

//////////////////////////////////////////////////////////////////

#endif//H_SUPPORTDLG_H
