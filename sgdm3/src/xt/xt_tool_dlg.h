// xt_tool_dlg.h
//////////////////////////////////////////////////////////////////

#ifndef H_XT_TOOL_DLG_H
#define H_XT_TOOL_DLG_H

BEGIN_EXTERN_C

//////////////////////////////////////////////////////////////////

class xt_tool_dlg : public wxDialog
{
public:
	virtual ~xt_tool_dlg(void);

	int					run(xt_tool ** ppXT);

	void				onCheckEvent_EnableGui2(wxCommandEvent & e);
	void				onCheckEvent_EnableGui3(wxCommandEvent & e);
	void				onButtonEvent_Help(wxCommandEvent & e);
	void				onButtonEvent_BrowseGui2(wxCommandEvent & e);
	void				onButtonEvent_BrowseGui3(wxCommandEvent & e);

	void				onButtonEvent_LeftPath(wxCommandEvent & e);
	void				onButtonEvent_LeftTitle(wxCommandEvent & e);
	void				onButtonEvent_RightPath(wxCommandEvent & e);
	void				onButtonEvent_RightTitle(wxCommandEvent & e);
	void				onButtonEvent_WorkingPath(wxCommandEvent & e);
	void				onButtonEvent_BaselinePath(wxCommandEvent & e);
	void				onButtonEvent_OtherPath(wxCommandEvent & e);
	void				onButtonEvent_DestinationPath(wxCommandEvent & e);
	void				onButtonEvent_WorkingTitle(wxCommandEvent & e);
	void				onButtonEvent_OtherTitle(wxCommandEvent & e);
	void				onButtonEvent_DestinationTitle(wxCommandEvent & e);

protected:
	void				_init(wxWindow * pParent, const wxString & strTitle, bool bFirstPage=false);
	wxPanel *			_createPanel_name(void);
	wxPanel *			_createPanel_gui2(void);
	wxPanel *			_createPanel_gui3(void);
	void				_enable_fields(void);
	void				_preload_fields(const xt_tool * pxt);
	void				_handle_browse_button(int id, const wxString & strTitle, wxString * pStrExe);
	void				_handle_helper_insert(int id, const wxString & str);

protected:
	xt_tool *			m_pXTWorkingCopy;
	
	wxTreebook *		m_pBookCtrl;
	wxButton *			m_pButtonHelp;

	wxString			m_strName;
	wxString			m_strSuffixes;

	bool				m_bEnableGui2;
	wxString			m_strGui2Exe;
	wxString			m_strGui2Args;

	bool				m_bEnableGui3;
	wxString			m_strGui3Exe;
	wxString			m_strGui3Args;

protected:
	typedef enum {		ID_PANEL_NAME=100,
						ID_PANEL_GUI2,
						ID_PANEL_GUI3,

						ID_SHOW_HELP,

						ID_BROWSE_GUI2EXE,
						ID_BROWSE_GUI3EXE,

						ID_TOOL_NAME,
						ID_TOOL_SUFFIX,

						ID_TOOL_GUI2ENABLE,
						ID_TOOL_GUI2EXE,
						ID_TOOL_GUI2ARGS,

						ID_TOOL_GUI3ENABLE,
						ID_TOOL_GUI3EXE,
						ID_TOOL_GUI3ARGS,

						ID_HELPER_LEFT_PATH,
						ID_HELPER_RIGHT_PATH,
						ID_HELPER_LEFT_TITLE,
						ID_HELPER_RIGHT_TITLE,

						ID_HELPER_WORKING_PATH,
						ID_HELPER_BASELINE_PATH,
						ID_HELPER_OTHER_PATH,
						ID_HELPER_DESTINATION_PATH,
						ID_HELPER_WORKING_TITLE,
						ID_HELPER_OTHER_TITLE,
						ID_HELPER_DESTINATION_TITLE,

	} ID;

	DECLARE_EVENT_TABLE();
};

//////////////////////////////////////////////////////////////////

class xt_tool_dlg__add : public xt_tool_dlg
{
public:
	xt_tool_dlg__add(wxWindow * pParent);
};

class xt_tool_dlg__edit : public xt_tool_dlg
{
public:
	xt_tool_dlg__edit(wxWindow * pParent, const xt_tool * pXT);
};

//////////////////////////////////////////////////////////////////

END_EXTERN_C

#endif//H_XT_TOOL_DLG_H
