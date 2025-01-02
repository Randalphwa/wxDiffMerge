// dlg_show_symlink_details.cpp -- display selected fields
// from one or more symlinks.
//
// this is a modal dialog intended to be used by a folder
// window.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fs.h>
#include <de.h>
#include <fd.h>
#include <gui.h>

#if defined(__WXMAC__) || defined(__WXGTK__)
//////////////////////////////////////////////////////////////////

#define DLG_TITLE			_("Symlink Details")

#define M 7
#define FIX 0
#define VAR 1

//////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(dlg_show_symlink_details,wxDialog)
//	EVT_BUTTON(ID_OPEN_TARGETS, dlg_show_symlink_details::onEventOpenTargets)
END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////

/*static*/ void dlg_show_symlink_details::run_modal(wxWindow * pWindowParent,
													 poi_item * pPoiItem_0,
													 poi_item * pPoiItem_1)
{
	dlg_show_symlink_details dlg(pWindowParent, pPoiItem_0, pPoiItem_1);

	dlg.ShowModal();
}

//////////////////////////////////////////////////////////////////

wxStaticBoxSizer * dlg_show_symlink_details::add_item(wxString & strBoxLabel,
													   poi_item * pPoiItem,
													  const wxString & strTarget)
{
	wxString strEmpty;

	wxStaticBoxSizer * staticBoxSizer =
		new wxStaticBoxSizer(
			new wxStaticBox(this,-1,strBoxLabel),
			wxVERTICAL);
	
#define XLBL(_s_) Statement( flex->Add(new wxStaticText(this,-1,(_s_)), FIX, wxALIGN_RIGHT,0); )
#define XVAL(_s_) Statement( flex->Add(new wxStaticText(this,-1,(_s_)), FIX, wxALIGN_LEFT,0); )
	
	wxFlexGridSizer * flex = new wxFlexGridSizer(6,2,M,M);

	XLBL("Symlink:");
	XVAL(pPoiItem->getFullPath());

	XLBL("Target:");
	XVAL(strTarget);

	staticBoxSizer->Add(flex, VAR, wxGROW, M);
							
	return staticBoxSizer;
}

dlg_show_symlink_details::dlg_show_symlink_details(wxWindow * pWindowParent,
													 poi_item * pPoiItem_0,
													 poi_item * pPoiItem_1)
{
	m_pPoiItem_0 = pPoiItem_0;
	m_pPoiItem_1 = pPoiItem_1;

	if (m_pPoiItem_0)
		m_pPoiItem_0->get_symlink_target(m_strTarget_0);

	if (m_pPoiItem_1)
		m_pPoiItem_1->get_symlink_target(m_strTarget_1);

#if defined(__WXMAC__)
	SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif
	Create(pWindowParent, -1, DLG_TITLE, wxDefaultPosition, wxDefaultSize,
		   wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX);

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		if (m_pPoiItem_0)
		{
			wxString strLeft( _T("Left Symlink") );
			vSizerTop->Add( add_item( strLeft, m_pPoiItem_0, m_strTarget_0), VAR, wxGROW|wxALL, M);
		}

		if (m_pPoiItem_1)
		{
			wxString strRight( _T("Right Symlink") );
			vSizerTop->Add( add_item( strRight, m_pPoiItem_1, m_strTarget_1), VAR, wxGROW|wxALL, M);
		}

		// put horizontal line and close buttons across the bottom of the dialog

		vSizerTop->Add( new wxStaticLine(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxLI_HORIZONTAL),
						FIX, wxGROW|wxTOP|wxLEFT|wxRIGHT, M);
		wxBoxSizer * hSizerButtons = new wxBoxSizer(wxHORIZONTAL);
		{
//			if ((m_nrFiles > 1) || (m_nrDirs > 1))
//				hSizerButtons->Add( new wxButton(this, ID_OPEN_TARGETS, _T("Compare Targets")),
//									FIX, wxALIGN_CENTER_VERTICAL, 0);
			
			hSizerButtons->AddStretchSpacer(VAR);
			hSizerButtons->Add( CreateButtonSizer(wxOK), FIX, 0, 0);
		}
		vSizerTop->Add(hSizerButtons,FIX,wxGROW|wxALL,M);
	}
	SetSizer(vSizerTop);
	vSizerTop->SetSizeHints(this);
	vSizerTop->Fit(this);
	Centre(wxBOTH);
}

dlg_show_symlink_details::~dlg_show_symlink_details(void)
{
}

#if 0
void dlg_show_symlink_details::onEventOpenTargets(wxCommandEvent & /*e*/)
{
	if (m_nrFiles > 1)
		gpFrameFactory->openFileDiffFrameOrSpawnAsyncXT(*m_pLnk_0->pStrTargetPath,
														*m_pLnk_1->pStrTargetPath,
														NULL);
	else if (m_nrDirs > 1)
		gpFrameFactory->openFolderFrame(*m_pLnk_0->pStrTargetPath,
										*m_pLnk_1->pStrTargetPath,
										NULL);
}
#endif

#endif
