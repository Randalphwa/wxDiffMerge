// rs_context_dlg.h
//////////////////////////////////////////////////////////////////

#ifndef H_RS_CONTEXT_DLG_H
#define H_RS_CONTEXT_DLG_H

//////////////////////////////////////////////////////////////////

class rs_context_dlg : public wxDialog
{
public:
	rs_context_dlg(wxWindow * pParent, wxString strTitle,
				   bool bRSRespectEOL, const rs_context * pCTX=NULL);
	
	int					run(rs_context ** ppCTX_Result);

	void				onCheckEvent_EndsAtEOL(wxCommandEvent & e);
	void				onCheckEvent_Important(wxCommandEvent & e);
	void				onCheckEvent_White(wxCommandEvent & e);

	void				onButtonEvent_Help(wxCommandEvent & e);

protected:
	void				_enable_fields(void);

protected:
	const rs_context *	m_pCTX_InitialValue;

	wxButton *			m_pButtonHelp;

	wxTextCtrl *		m_pTextCtrlStartPattern;
	wxTextCtrl *		m_pTextCtrlEndPattern;
	wxTextCtrl *		m_pTextCtrlEscapeChar;
	wxCheckBox *		m_pCheckBoxEndsAtEOL;

	wxCheckBox *		m_pCheckBoxContext;
	wxCheckBox *		m_pCheckBoxEOL;
	wxCheckBox *		m_pCheckBoxCase;
	wxCheckBox *		m_pCheckBoxWhite;
	wxCheckBox *		m_pCheckBoxTab;
	
	wxString			m_strStartPattern;
	wxString			m_strEndPattern;
	wxString			m_strEscapeChar;
	bool				m_bEndsAtEOL;

	bool				m_bAllowBlankRegExStart;
	bool				m_bRSRespectEOL;

	bool				m_bContext;
	bool				m_bImportantEOL;
	bool				m_bImportantCase;
	bool				m_bImportantWhite;
	bool				m_bTabIsWhite;

	typedef enum {		ID_START_PATTERN=100,
						ID_END_PATTERN,
						ID_ESCAPE_CHARACTER,
						ID_ENDS_AT_EOL,
						ID_CONTEXT,
						ID_EOL,
						ID_CASE,
						ID_WHITE,
						ID_TAB,
						ID_SHOW_HELP,
	} ID;

	DECLARE_EVENT_TABLE();
};

//////////////////////////////////////////////////////////////////

#endif//H_RS_CONTEXT_DLG_H
