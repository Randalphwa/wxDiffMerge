// util_dont_show_again_msgbox.cpp
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>

//////////////////////////////////////////////////////////////////

#define M 7
#define FIX 0
#define VAR 1

//////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(util_dont_show_again_msgbox,wxDialog)
	EVT_BUTTON(wxID_YES,		util_dont_show_again_msgbox::onButtonYes)
	EVT_BUTTON(wxID_NO,			util_dont_show_again_msgbox::onButtonNo)
END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////

util_dont_show_again_msgbox::util_dont_show_again_msgbox(wxWindow * pParent, const wxString & strTitle, const wxString & strMessage, long buttonFlags)
{
	m_bChecked = false;
#if defined(__WXMAC__)
	SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif

	Create(pParent,-1,strTitle,wxDefaultPosition,wxDefaultSize,
		   wxCAPTION|wxSYSTEM_MENU|wxRESIZE_BORDER|wxCLOSE_BOX);

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		vSizerTop->Add( new wxStaticText(this,-1,strMessage),
						FIX, wxGROW|wxALL, M);

		vSizerTop->AddStretchSpacer(VAR);

		vSizerTop->Add( new wxStaticLine(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxLI_HORIZONTAL), FIX, wxGROW|wxTOP|wxLEFT|wxRIGHT, M);

		wxBoxSizer * hSizerButtons = new wxBoxSizer(wxHORIZONTAL);
		{
			hSizerButtons->Add( new wxCheckBox(this,ID_CHECKBOX,_("&Don't show this message again."),
											   wxDefaultPosition,wxDefaultSize,0,
											   wxGenericValidator(&m_bChecked)),
								FIX, wxRIGHT|wxALIGN_CENTER_VERTICAL, M*3);
			hSizerButtons->AddStretchSpacer(VAR);
			hSizerButtons->Add( CreateButtonSizer(buttonFlags), FIX, 0, 0);
		}
		vSizerTop->Add(hSizerButtons,FIX,wxGROW|wxALL,M);
		
	}
	SetSizer(vSizerTop);
	vSizerTop->SetSizeHints(this);
	vSizerTop->Fit(this);
	Centre(wxBOTH);
}

int util_dont_show_again_msgbox::run(bool * pbDontShowAgain)
{
	int result = ShowModal();

	if (pbDontShowAgain)
		*pbDontShowAgain = m_bChecked;

	return result;
}

//////////////////////////////////////////////////////////////////

void util_dont_show_again_msgbox::onButtonYes(wxCommandEvent & /*e*/)
{
	if (Validate() && TransferDataFromWindow())
		EndModal(wxID_OK);
}

void util_dont_show_again_msgbox::onButtonNo(wxCommandEvent & /*e*/)
{
	EndModal(wxID_CANCEL);
}

//////////////////////////////////////////////////////////////////

/*static*/ int util_dont_show_again_msgbox::msgbox(wxWindow * pParent,
												   const wxString & strTitle, const wxString & strMessage,
												   int key, long buttonFlags)
{
	// optionally show a message box dialog (if not already disabled in global props).
	// then update global props if they don't want to see it again.
	
	bool bKeyValue =  gpGlobalProps->getBool((GlobalProps::EnumGPL)key);

	if (bKeyValue)	// if don't show already set
		return wxID_OK;

	util_dont_show_again_msgbox dlg(pParent,strTitle,strMessage,buttonFlags);

	int result = dlg.run(&bKeyValue);

	if ((result == wxID_OK) && (bKeyValue))
		gpGlobalProps->setLong((GlobalProps::EnumGPL)key,1);

	return result;
}
