// TaskListPanel.h
//////////////////////////////////////////////////////////////////

#ifndef H_TASKLISTPANEL_H
#define H_TASKLISTPANEL_H
#if 0	// we are not currently using the TaskList
//////////////////////////////////////////////////////////////////

#if defined(__WXMSW__)

#	define MY_PANEL_STYLE		0

#elif defined(__WXGTK__)

#	define MY_PANEL_STYLE		0

#elif defined(__WXMAC__)

#	define MY_PANEL_STYLE		0

#endif

//////////////////////////////////////////////////////////////////

class TaskListPanel : public wxPanel
{
public:
	TaskListPanel(wxWindow * pParent, ViewFile * pViewFile);
	virtual ~TaskListPanel(void);

	void					bind_dede(de_de * pDeDe);

protected:
	ViewFile *				m_pViewFile;
	de_de *					m_pDeDe;
};

//////////////////////////////////////////////////////////////////

class TaskListPanel_AutoMerge : public TaskListPanel
{
public:
	TaskListPanel_AutoMerge(wxWindow * pParent, ViewFile * pViewFile,
							fim_patchset * pPatchSet, int nrConflicts);
	virtual ~TaskListPanel_AutoMerge(void);

	void					createLayout(void);
	fim_patchset *			removePatchSet(void);

	void					onButton_Cancel(wxCommandEvent & e);
	void					onButton_Apply(wxCommandEvent & e);

	void					OnListItem_Selected(wxListEvent & e);
	void					OnListItem_RightClick(wxListEvent & e);
	void					OnMenuItem_Command(wxCommandEvent & e);

	void					cb_de_changed(const util_cbl_arg & arg);

	typedef enum {			ID_BTN_APPLY,
							ID_BTN_CANCEL,
							ID_LIST_CTRL,
	} ID;
	
private:
	void					_define_columns(void);
	void					_populate_list(void);
	void					_set_listctrl_fields(long ndxInList, de_patch * pDePatch, bool bSetStatusColumn);
	void					_enable_apply(void);
	void					_set_stale(void);

	wxButton *				m_pButtonApply;
	wxButton *				m_pButtonCancel;
	wxPanel *				m_pPanelBody;
	wxListCtrl *			m_pListCtrl;
	wxBoxSizer *			m_pSizerListCtrl;
	wxPanel *				m_pPanelError;

	fim_patchset *			m_pPatchSet;
	int						m_nrConflicts;

	long					m_ndxInList_for_Popup;
	bool					m_bPatchSetIsStale;

	DECLARE_EVENT_TABLE();
};

//////////////////////////////////////////////////////////////////
#endif//0
#endif//H_TASKLISTPANEL_H
