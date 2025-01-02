// dlg_auto_merge.h
//////////////////////////////////////////////////////////////////

#ifndef H_DLG_AUTO_MERGE_H
#define H_DLG_AUTO_MERGE_H

//////////////////////////////////////////////////////////////////
#if 0
class dlg_auto_merge : public wxDialog
{
public:
	dlg_auto_merge(gui_frame * pFrame, ViewFile * pViewFile, fim_patchset * pPatchSet, int nrConflicts);
	virtual ~dlg_auto_merge(void);

	void					onListItemSelected(wxListEvent & e);
	void					onListItemRightClick(wxListEvent & e);

	void					onCtxMenuEvent_Apply(wxCommandEvent & e);
	void					onCtxMenuEvent_Omit(wxCommandEvent & e);
	void					onCtxMenuEvent_Command(wxCommandEvent & e);

	void					onButton_ShowHelp(wxCommandEvent & e);

private:
	void					_build_image_list(void);
	void					_define_columns(void);
	void					_populate_list(void);

	gui_frame *				m_pFrame;
	ViewFile *				m_pViewFile;
	fim_patchset *			m_pPatchSet;
	wxImageList *			m_pImageList;
	wxListCtrl *			m_pListCtrl;
	wxStaticText *			m_pStaticTextSummary;
	wxButton *				m_pButtonHelp;

	long					m_itemContextMenu;
	int						m_nrConflicts;

	typedef enum {			ID_LIST_CTRL=100,
							ID_SUMMARY,
							ID_SHOW_HELP,
	} ID;

	DECLARE_EVENT_TABLE();
};

//////////////////////////////////////////////////////////////////

class dlg_auto_merge_help : public wxDialog
{
public:
	dlg_auto_merge_help(wxWindow * pParent);

	DECLARE_EVENT_TABLE();
};
#endif
//////////////////////////////////////////////////////////////////

#endif//H_DLG_AUTO_MERGE_H
