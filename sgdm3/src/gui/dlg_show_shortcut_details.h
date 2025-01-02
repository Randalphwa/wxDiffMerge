// dlg_show_shortcut_details.h
// dialog to show selected fields from one or more Windows SHORTCUT
// .LNK files.
//////////////////////////////////////////////////////////////////

#ifndef H_DLG_SHOW_SHORTCUT_DETAILS_H
#define H_DLG_SHOW_SHORTCUT_DETAILS_H

#if defined(__WXMSW__)
//////////////////////////////////////////////////////////////////

class dlg_show_shortcut_details : public wxDialog
{
public:
	static void run_modal(wxWindow * pWindowParent,
						  poi_item * pPoiItem_0,
						  poi_item * pPoiItem_1);
	~dlg_show_shortcut_details(void);

private:
	dlg_show_shortcut_details(wxWindow * pWindowParent,
							  poi_item * pPoiItem_0,
							  poi_item * pPoiItem_1);

	wxStaticBoxSizer * add_item(wxString & strBoxLabel,
								poi_item * pPoiItem,
								util_shell_lnk * pLnk);

	void onEventOpenTargets(wxCommandEvent & /*e*/);

private:
	poi_item * m_pPoiItem_0;	// we do not own this
	poi_item * m_pPoiItem_1;	// we do not own this

	util_shell_lnk * m_pLnk_0;
	util_shell_lnk * m_pLnk_1;

	int m_nrFiles;
	int m_nrDirs;

	typedef enum { ID_OPEN_TARGETS = 100,
	} ID;

	DECLARE_EVENT_TABLE();

};

//////////////////////////////////////////////////////////////////
#endif//__WXMSW__

#endif//H_DLG_SHOW_SHORTCUT_DETAILS_H
