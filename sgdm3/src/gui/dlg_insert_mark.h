// dlg_insert_mark.h
// a modeless dialog to help the user insert a mark -- do manual alignment
//////////////////////////////////////////////////////////////////

#ifndef H_DLG_INSERT_MARK_H
#define H_DLG_INSERT_MARK_H

//////////////////////////////////////////////////////////////////

class dlg_insert_mark : public wxDialog
{
public:
	dlg_insert_mark(gui_frame * pFrame, de_mark * pDeMarkInitial=NULL);
	virtual ~dlg_insert_mark(void);

	void					OnApply(wxCommandEvent & e);
	void					OnCancel(wxCommandEvent & e);

	void					onButton_ShowHelp(wxCommandEvent & e);
	void					onButton_Delete(wxCommandEvent & e);
	void					onButton_DeleteAll(wxCommandEvent & e);

	void					onListBoxSelect(wxCommandEvent & e);

	void					externalUpdateLineNr(PanelIndex kPanel, const fl_line * pFlLine);

	inline void				frameDeleted(void)	{ m_pFrame = NULL; };

private:
	void					_preload_validated_fields(void);
	int						_get_spin_limit(int kPanel);
	int						_get_current_line(int kPanel);
	void					_enable_fields(void);
	int						_allocate_array_of_mark_descriptions(wxString ** array);
	void					_free_array_of_mark_descriptions(wxString * array);

	gui_frame *				m_pFrame;
	ViewFile *				m_pViewFile;
	int						m_nrFiles;
	int						m_iSpin[__NR_TOP_PANELS__];
	
	wxSpinCtrl *			m_pSpin[__NR_TOP_PANELS__];
	
	wxButton *				m_pButtonClose;
	wxButton *				m_pButtonHelp;
	wxButton *				m_pButtonInsert;
	wxButton *				m_pButtonDelete;
	wxButton *				m_pButtonDeleteAll;
	wxListBox *				m_pListBox;

	typedef enum {			ID_SPIN_LINE_NR_T0=100,
							ID_SPIN_LINE_NR_T1,
							ID_SPIN_LINE_NR_T2,

							ID_LISTBOX,
							ID_BUTTON_DELETE,
							ID_BUTTON_DELETE_ALL,

							ID_SHOW_HELP,
	} ID;
	
	DECLARE_EVENT_TABLE();
};

//////////////////////////////////////////////////////////////////

class dlg_insert_mark_help : public wxDialog
{
public:
	dlg_insert_mark_help(wxWindow * pParent);

	DECLARE_EVENT_TABLE();
};

//////////////////////////////////////////////////////////////////

#endif//H_DLG_INSERT_MARK_H
