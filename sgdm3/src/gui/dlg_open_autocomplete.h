// dlg_open_autocomplete.h
//////////////////////////////////////////////////////////////////

#ifndef H_DLG_OPEN_AUTOCOMPLETE_H
#define H_DLG_OPEN_AUTOCOMPLETE_H

//////////////////////////////////////////////////////////////////

class dlg_open_autocomplete : public wxDialog
{
public:
	dlg_open_autocomplete(wxWindow * pParent, ToolBarType tbt, const cl_args * pArgs=NULL);

	void					onEventButtonSwap(wxCommandEvent & e);

	void					OnOK(wxCommandEvent & e);
	void					OnDirPickerChanged(wxFileDirPickerEvent & e);
	void					OnFilePickerChanged(wxFileDirPickerEvent & e);

	void					OnDropFiles(int ndx, const wxString & strPathname);
	int						dnd_hit_test(wxCoord xMouse, wxCoord yMouse);

	inline const wxString	getPath(int k) const { return m_strPath[k]; };

private:
	void					_enable_fields(void);

	wxWindow *				m_pParent;
	ToolBarType				m_tbt;

	wxFilePickerCtrl *		m_pFilePickerCtrl[3];
	wxDirPickerCtrl *		m_pDirPickerCtrl[2];

	wxString				_fetch_k(int k);
	void					_set_k(int k, const wxString & str);

	wxString				m_strPath[3];
	wxString				m_strBrowse[3];
	wxString				m_strCaption[3];

	typedef enum {			ID_PICKER_0,
							ID_PICKER_1,
							ID_PICKER_2,
							ID_SWAP,
	} ID;

	wxButton *				m_pButtonSwap;
	bool					m_bSwapped;

	DECLARE_EVENT_TABLE();
};

//////////////////////////////////////////////////////////////////

#endif//H_DLG_OPEN_AUTOCOMPLETE_H
