// dlg_auto_merge_result_summary.h
//////////////////////////////////////////////////////////////////

#ifndef H_DLG_AUTO_MERGE_RESULT_SUMMARY_H
#define H_DLG_AUTO_MERGE_RESULT_SUMMARY_H

//////////////////////////////////////////////////////////////////

class dlg_auto_merge_result_summary : public wxDialog
{
public:
	dlg_auto_merge_result_summary(wxWindow * pParent, const fim_patchset * pPatchSet);

private:
	void						_define_columns(void);
	long						_populate_list(void);

	const fim_patchset *		m_pPatchSet;

	wxListCtrl *				m_pListCtrl;
};

//////////////////////////////////////////////////////////////////

#endif//H_DLG_AUTO_MERGE_RESULT_SUMMARY_H
