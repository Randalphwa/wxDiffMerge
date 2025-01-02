// rs_ruleset_dlg.h
//////////////////////////////////////////////////////////////////

#ifndef H_RS_RULESET_DLG_H
#define H_RS_RULESET_DLG_H

//////////////////////////////////////////////////////////////////

class rs_ruleset_dlg : public wxDialog
{
public:
	virtual ~rs_ruleset_dlg(void);

	int					run(rs_ruleset ** ppRS_Ruleset);

protected:
	void				_init(wxWindow * pParent, const wxString & strTitle, bool bFirstPage=false);
	wxPanel *			_createPanel_name(void);
	wxPanel *			_createPanel_line(void);
	wxPanel *			_createPanel_equivalence(void);
	wxPanel *			_createPanel_char(void);
	wxPanel *			_createPanel_ctxt(void);
	wxPanel *			_createPanel_lomit(void);

	void				onCheckEvent_Match_StripEOL(wxCommandEvent & e);
	void				onCheckEvent_Match_StripCase(wxCommandEvent & e);
	void				onCheckEvent_Match_StripWhite(wxCommandEvent & e);
	void				onCheckEvent_Match_StripTab(wxCommandEvent & e);

	void				onCheckEvent_Equivalence_StripEOL(wxCommandEvent & e);
	void				onCheckEvent_Equivalence_StripCase(wxCommandEvent & e);
	void				onCheckEvent_Equivalence_StripWhite(wxCommandEvent & e);
	void				onCheckEvent_Equivalence_StripTab(wxCommandEvent & e);

	void				onCheckEvent_DefaultContext_Important(wxCommandEvent & e);
	void				onCheckEvent_DefaultContext_EOL(wxCommandEvent & e);
	void				onCheckEvent_DefaultContext_Case(wxCommandEvent & e);
	void				onCheckEvent_DefaultContext_White(wxCommandEvent & e);
	void				onCheckEvent_DefaultContext_TabIsWhite(wxCommandEvent & e);

	void				onButtonEvent_ContextAdd(wxCommandEvent & e);
	void				onButtonEvent_ContextEdit(wxCommandEvent & e);
	void				onButtonEvent_ContextDelete(wxCommandEvent & e);

	void				onButtonEvent_LOmitAdd(wxCommandEvent & e);
	void				onButtonEvent_LOmitEdit(wxCommandEvent & e);
	void				onButtonEvent_LOmitDelete(wxCommandEvent & e);

	void				onButtonEvent_Help(wxCommandEvent & e);

	void				onRadioEvent_EncodingStyle(wxCommandEvent & e);

	void				onListBoxSelect_Context(wxCommandEvent & e);
	void				onListBoxDClick_Context(wxCommandEvent & e);

	void				onListBoxSelect_LOmit(wxCommandEvent & e);
	void				onListBoxDClick_LOmit(wxCommandEvent & e);

	void				_enable_fields(void);
	
protected:
	// TODO we can remove this #ifdef once we get the build system updated to 2.8.
#if wxCHECK_VERSION(2,8,0)
	wxTreebook *		m_pBookCtrl;
#else
	wxBookCtrlBase *	m_pBookCtrl;
#endif

	wxPanel *			m_pPanel_name;
	wxPanel *			m_pPanel_line;
	wxPanel *			m_pPanel_equivalence;
	wxPanel *			m_pPanel_char;
	wxPanel *			m_pPanel_ctxt;
	wxPanel *			m_pPanel_lomit;
	
	wxListBox *			m_pListBoxContext;
	wxListBox *			m_pListBoxLOmit;
	wxButton *			m_pButtonHelp;

protected:
	void				_preload_fields(const wxString & strName, const wxString & strSuffixes,
										rs_context_attrs attrsStrip,
										int encodingStyle,
										util_encoding encodingSetting1,
										util_encoding encodingSetting2,
										util_encoding encodingSetting3,
										bool bSniffEncodingBOM,
										rs_context_attrs attrsDefaultContext,
										rs_context_attrs attrsEquivalence);

	rs_ruleset *		m_pRSWorkingCopy;

	wxString			m_strName;
	wxString			m_strSuffixes;

	bool				m_bMatchStripEOL;
	bool				m_bMatchStripCase;
	bool				m_bMatchStripWhite;
	bool				m_bMatchStripTab;

	bool				m_bEquivalenceStripEOL;
	bool				m_bEquivalenceStripCase;
	bool				m_bEquivalenceStripWhite;
	bool				m_bEquivalenceStripTab;

	int					m_encodingStyle;	// must be "int" not "RS_ENCODING_STYLE" for generic validator
	util_encoding		m_encodingSetting1;
	util_encoding		m_encodingSetting2;
	util_encoding		m_encodingSetting3;
	wxString			m_encodingSettingStr1;	// used for validator
	wxString			m_encodingSettingStr2;	// used for validator
	wxString			m_encodingSettingStr3;	// used for validator
	bool				m_bSniffEncodingBOM;

	bool				m_bDefaultImportant;
	bool				m_bDefaultContextEOL;
	bool				m_bDefaultContextWhite;
	bool				m_bDefaultContextCase;
	bool				m_bDefaultTabIsWhite;

	bool				m_bHideNameAndSuffix;

	util_enc			m_encTable;

protected:
	typedef enum {		ID_PANEL_NAME=100,
						ID_PANEL_LINE,
						ID_PANEL_CHAR,
						ID_PANEL_CTXT,
						ID_PANEL_LOMIT,
						ID_PANEL_EQUIVALENCE,

						ID_SHOW_HELP,

						ID_RULESET_NAME,
						ID_RULESET_SUFFIX,

						ID_MATCH_STRIP_EOL,
						ID_MATCH_STRIP_CASE,
						ID_MATCH_STRIP_WHITE,
						ID_MATCH_STRIP_TAB,

						ID_EQUIVALENCE_STRIP_EOL,
						ID_EQUIVALENCE_STRIP_CASE,
						ID_EQUIVALENCE_STRIP_WHITE,
						ID_EQUIVALENCE_STRIP_TAB,

						ID_CHECK_SNIFF_ENCODING_BOM,
						ID_RADIO_ENCODING_STYLE,
						ID_COMBOBOX_ENCODING1,
						ID_COMBOBOX_ENCODING2,
						ID_COMBOBOX_ENCODING3,
						ID_LABEL_ENCODING2,
						ID_LABEL_ENCODING3,

						ID_LOMIT_LISTBOX,
						ID_LOMIT_ADD,
						ID_LOMIT_EDIT,
						ID_LOMIT_DELETE,

						ID_CONTEXT_LISTBOX,
						ID_CONTEXT_ADD,
						ID_CONTEXT_EDIT,
						ID_CONTEXT_DELETE,

						ID_DEFAULT_CONTEXT_IMPORTANT,
						ID_DEFAULT_CONTEXT_EOL,
						ID_DEFAULT_CONTEXT_CASE,
						ID_DEFAULT_CONTEXT_WHITE,
						ID_DEFAULT_CONTEXT_TAB_IS_WHITE,

	} ID;

	DECLARE_EVENT_TABLE();
};

//////////////////////////////////////////////////////////////////

class rs_ruleset_dlg__add : public rs_ruleset_dlg
{
public:
	rs_ruleset_dlg__add(wxWindow * pParent);
};

//////////////////////////////////////////////////////////////////

class rs_ruleset_dlg__edit : public rs_ruleset_dlg
{
public:
	rs_ruleset_dlg__edit(wxWindow * pParent, const rs_ruleset * pRS);
};

//////////////////////////////////////////////////////////////////

class rs_ruleset_dlg__edit_default : public rs_ruleset_dlg
{
public:
	rs_ruleset_dlg__edit_default(wxWindow * pParent, const rs_ruleset * pRS);
};

//////////////////////////////////////////////////////////////////

#endif//H_RS_RULESET_DLG_H
