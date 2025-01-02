// dlg_open.h
//////////////////////////////////////////////////////////////////

#ifndef H_DLG_OPEN_H
#define H_DLG_OPEN_H

//////////////////////////////////////////////////////////////////

class dlg_open : public wxDialog
{
public:
	dlg_open(wxWindow * pParent, ToolBarType tbt, const cl_args * pArgs=NULL);

	void					onEventButton_k(wxCommandEvent & e);
	void					onEventButtonSwap(wxCommandEvent & e);
	void					onEventTextChanged_k(wxCommandEvent & e);
	void					OnOK(wxCommandEvent & e);
	void					OnDropFiles(int ndx, const wxString & strPathname);
	bool					dnd_hit_test(wxCoord xMouse, wxCoord yMouse, int * pNdx);

	inline const wxString	getPath(int k) const { return m_strPath[k]; };

private:
	void					_enable_fields(void);

	wxWindow *				m_pParent;
	ToolBarType				m_tbt;

	wxButton *				m_pButtonBrowse[3];
	wxTextCtrl *			m_pTextCtrl[3];
	wxString				m_strPath[3];
	wxString				m_strBrowse[3];
	wxString				m_strCaption[3];

	typedef enum {			ID_BROWSE_0 = 100,
							ID_BROWSE_1,
							ID_BROWSE_2,
							ID_TEXT_0,
							ID_TEXT_1,
							ID_TEXT_2,
							ID_SWAP,
	} ID;

	wxButton *				m_pButtonSwap;
	bool					m_bSwapped;

	DECLARE_EVENT_TABLE();
};

//////////////////////////////////////////////////////////////////

#endif//H_DLG_OPEN_H
