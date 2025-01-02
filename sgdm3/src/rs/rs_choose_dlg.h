// rs_choose_dlg.h
//////////////////////////////////////////////////////////////////

#ifndef H_RS_CHOOSE_DLG_H
#define H_RS_CHOOSE_DLG_H

//////////////////////////////////////////////////////////////////

class rs_choose_dlg : public wxDialog
{
public:
	void				onListBoxDClickEvent_listbox(wxCommandEvent & e);
	//void				onListBoxEvent_listbox(wxCommandEvent & e);
	void				onButtonEvent_use_default(wxCommandEvent & e);
	void				OnOK(wxCommandEvent & e);

	int					run(void);

protected:
	void				_init(wxWindow * pParent,
							  wxString strTitle, wxString strMessage, wxString strDefault,
							  int nrPoi, poi_item * pPoiTable[],
							  int nrChoices, wxString aChoices[],
							  int kPreSelect);
	void				_preload_preview_buffers(int nrPoi, poi_item * pPoiTable[]);

	wxWindow *			m_pParent;

	wxListBox *			m_pListBox;
	wxNotebook *		m_pNotebook;
	wxPanel *			m_pPanel[3];
	wxTextCtrl *		m_pTextPreview[3];
	wxString			m_strPreviewBuffer[3];

	int					m_nrPoi;
	util_error			m_uePreview[3];

protected:
	typedef enum {		ID_LISTBOX = 100,
						ID_NOTEBOOK,
						ID_PANEL_K0,	ID_PANEL_K1,	ID_PANEL_K2,
						ID_PREVIEW_K0,	ID_PREVIEW_K1,	ID_PREVIEW_K2,
						ID_USE_DEFAULT,
	} ID;
	
	DECLARE_EVENT_TABLE();
};

//////////////////////////////////////////////////////////////////

class rs_choose_dlg__ruleset : public rs_choose_dlg
{
public:
	rs_choose_dlg__ruleset(wxWindow * pParent, int kPreSelect, int nrPoi, poi_item * pPoiTable[], const rs_ruleset_table * pRST=NULL);
	rs_choose_dlg__ruleset(wxWindow * pParent, const rs_ruleset * pRSCurrent, const rs_ruleset_table * pRST=NULL);

private:
	void				_init(wxWindow * pParent, int kPreSelect, int nrPoi, poi_item * pPoiTable[], const rs_ruleset_table * pRST);
};

//////////////////////////////////////////////////////////////////

class rs_choose_dlg__charset : public rs_choose_dlg
{
public:
	rs_choose_dlg__charset(wxWindow * pParent, util_encoding encPreSelect, int nrPoi, poi_item * pPoiTable[]);
	
	bool				run(util_encoding * pEnc);

private:
	util_enc			m_encTable;
};

//////////////////////////////////////////////////////////////////

#endif//H_RS_CHOOSE_DLG_H
