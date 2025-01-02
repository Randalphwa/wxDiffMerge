// AboutBox.h
// About Box dialog.
//////////////////////////////////////////////////////////////////

#ifndef H_ABOUTBOX_H
#define H_ABOUTBOX_H

//////////////////////////////////////////////////////////////////

class AboutBox : public wxDialog
{
public:
	AboutBox(wxFrame * pFrameParent, gui_frame * pGuiFrameActive);

	static wxString	build_version_string(void);


	typedef enum {	ID_BUTTON_SUPPORT = 100,
	} ID;

	DECLARE_EVENT_TABLE();

public:
	void			onEventButton_Support(wxCommandEvent & e);

private:
	wxFrame *		m_pFrameParent;		// parent window for stacking
	gui_frame *		m_pGuiFrameActive;	// active frame for support info detail
};

//////////////////////////////////////////////////////////////////

#endif//H_ABOUTBOX_H
