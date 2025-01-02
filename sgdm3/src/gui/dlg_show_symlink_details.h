// dlg_show_symlink_details.h
// dialog to show selected fields from one or more symlinks.
//////////////////////////////////////////////////////////////////

#ifndef H_DLG_SHOW_SYMLINK_DETAILS_H
#define H_DLG_SHOW_SYMLINK_DETAILS_H

#if defined(__WXMAC__) || defined(__WXGTK__)
//////////////////////////////////////////////////////////////////

class dlg_show_symlink_details : public wxDialog
{
public:
	static void run_modal(wxWindow * pWindowParent,
						  poi_item * pPoiItem_0,
						  poi_item * pPoiItem_1);
	~dlg_show_symlink_details(void);

private:
	dlg_show_symlink_details(wxWindow * pWindowParent,
							  poi_item * pPoiItem_0,
							  poi_item * pPoiItem_1);

	wxStaticBoxSizer * add_item(wxString & strBoxLabel,
								poi_item * pPoiItem,
								const wxString & strTarget);

//	void onEventOpenTargets(wxCommandEvent & /*e*/);

private:
	poi_item * m_pPoiItem_0;	// we do not own this
	poi_item * m_pPoiItem_1;	// we do not own this

	wxString m_strTarget_0;
	wxString m_strTarget_1;

	typedef enum { ID_OPEN_TARGETS = 100,
	} ID;

	DECLARE_EVENT_TABLE();

};

//////////////////////////////////////////////////////////////////
#endif//__WXMSW__

#endif//H_DLG_SHOW_SYMLINK_DETAILS_H
