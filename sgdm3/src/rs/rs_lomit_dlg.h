// rs_lomit_dlg.h
//////////////////////////////////////////////////////////////////

#ifndef H_RS_LOMIT_DLG_H
#define H_RS_LOMIT_DLG_H

//////////////////////////////////////////////////////////////////

class rs_lomit_dlg : public wxDialog
{
public:
	rs_lomit_dlg(wxWindow * pParent, wxString strTitle, const rs_lomit * pLOmit=NULL);

	int					run(rs_lomit ** ppLOmit_Result);

	void				onButtonEvent_InsBlank(wxCommandEvent & e);
	void				onButtonEvent_InsPage(wxCommandEvent & e);

protected:
	const rs_lomit *	m_pLOmit_InitialValue;

	wxTextCtrl *		m_pTextCtrlPattern;
	wxSpinCtrl *		m_pSpinCtrlSkip;

	wxString			m_strPattern;
	int					m_skip;

	bool				m_bAllowBlankRegEx;

	typedef enum {		ID_PATTERN=100,
						ID_SPIN_SKIP,
						ID_INS_BLANK,
						ID_INS_PAGE,
	} ID;

	DECLARE_EVENT_TABLE();
};

//////////////////////////////////////////////////////////////////

#endif//H_RS_LOMIT_DLG_H
