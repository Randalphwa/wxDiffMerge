// util_help_dlg.cpp -- a little help dialog
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>

//////////////////////////////////////////////////////////////////

#define M 7
#define FIX 0
#define VAR 1

#define DLG_TITLE		_("Help")

//////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(util_help_dlg,wxDialog)
END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////

util_help_dlg::util_help_dlg(wxWindow * pParent, const wxString & strText)
{
#if defined(__WXMAC__)
	SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif
	Create(pParent,-1,DLG_TITLE,wxDefaultPosition,wxDefaultSize,
		   wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX);

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		wxStaticText * pHelp = new wxStaticText(this,-1,strText);
		vSizerTop->Add( pHelp, FIX, wxGROW|wxALL, M);

		//////////////////////////////////////////////////////////////////
		// put horizontal line and set of ok/cancel buttons across the bottom of the dialog

//		vSizerTop->AddStretchSpacer(VAR);
		vSizerTop->Add( new wxStaticLine(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxLI_HORIZONTAL), FIX, wxGROW|wxLEFT|wxRIGHT, M);
		wxBoxSizer * hSizerButtons = new wxBoxSizer(wxHORIZONTAL);
		{
			hSizerButtons->AddStretchSpacer(VAR);
			hSizerButtons->Add( new wxButton(this,wxID_CANCEL,_("&Close")), FIX, wxALL, M);	// using wxID_CANCEL lets ESC key close dialog
		}
		vSizerTop->Add(hSizerButtons,FIX,wxGROW|wxALL,M);

	}
	SetSizer(vSizerTop);
	vSizerTop->SetSizeHints(this);
	vSizerTop->Fit(this);

	Centre(wxBOTH);
}
