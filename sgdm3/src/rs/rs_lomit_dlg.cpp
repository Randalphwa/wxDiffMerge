// rs_lomit_dlg.cpp
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <poi.h>
#include <rs.h>

//////////////////////////////////////////////////////////////////

#define M 7
#define FIX 0
#define VAR 1

//////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(rs_lomit_dlg,wxDialog)
	EVT_BUTTON(ID_INS_BLANK,	rs_lomit_dlg::onButtonEvent_InsBlank)
	EVT_BUTTON(ID_INS_PAGE,		rs_lomit_dlg::onButtonEvent_InsPage)
END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////

rs_lomit_dlg::rs_lomit_dlg(wxWindow * pParent, wxString strTitle, const rs_lomit * pLOmit)
	: m_bAllowBlankRegEx(false)
{
	m_pLOmit_InitialValue = pLOmit;

	//////////////////////////////////////////////////////////////////
	// preload fields that the validators will use
	//////////////////////////////////////////////////////////////////

	if (pLOmit)
	{
		m_strPattern	= *pLOmit->getPattern();
		m_skip			= pLOmit->getNrLinesToSkip();
	}
	else
	{
		m_skip			= 1;
	}
	//////////////////////////////////////////////////////////////////

#if defined(__WXMAC__)
	SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif
	Create(pParent,-1, strTitle, wxDefaultPosition, wxDefaultSize,
		   wxCAPTION | wxSYSTEM_MENU | wxRESIZE_BORDER | wxCLOSE_BOX);

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		//////////////////////////////////////////////////////////////////

		wxStaticBoxSizer * staticBoxSizer1 = new wxStaticBoxSizer( new wxStaticBox(this,-1, _("&Pattern")), wxVERTICAL);
		{
			wxBoxSizer * hSizer = new wxBoxSizer(wxHORIZONTAL);
			{
				wxBoxSizer * vSizer = new wxBoxSizer(wxVERTICAL);
				{
					wxStaticText * pHelp = new wxStaticText(this,-1,
															wxGetTranslation( L"A Pattern must be a valid Regular Expression.  Each\n"
																			  L"document source line will be compared against the\n"
																			  L"pattern and when a match is found, that line will be\n"
																			  L"omitted from the Difference Analysis."
																));
					//pHelp->SetFont( *wxITALIC_FONT );
					vSizer->Add( pHelp, FIX, 0, 0);

					m_pTextCtrlPattern = new wxTextCtrl(this,ID_PATTERN,_T(""),
														wxDefaultPosition,wxDefaultSize,0,
														RegExpTextValidator(&m_strPattern,
																			wxString( _("Pattern cannot be blank.") ),
																			&m_bAllowBlankRegEx));
					vSizer->Add( m_pTextCtrlPattern, FIX, wxGROW|wxTOP, M);
				}
				hSizer->Add( vSizer, VAR, 0, 0);

				wxBoxSizer * vSizer2 = new wxBoxSizer(wxVERTICAL);
				{
					vSizer2->Add( new wxButton(this,ID_INS_BLANK,	_("&Blank Line")), FIX, wxGROW, 0);
					vSizer2->Add( new wxButton(this,ID_INS_PAGE,	_("Pa&ge Break")), FIX, wxGROW|wxTOP, M);
				}
				hSizer->Add( vSizer2, FIX, wxLEFT, M*2);
			}
			staticBoxSizer1->Add( hSizer, FIX, wxGROW|wxALL, M);
		}
		vSizerTop->Add(staticBoxSizer1, FIX, wxGROW|wxALL, M);

		//////////////////////////////////////////////////////////////////

		wxStaticBoxSizer * staticBoxSizer2 = new wxStaticBoxSizer( new wxStaticBox(this,-1, _("&Lines To Skip")), wxVERTICAL);
		{
			wxStaticText * pHelp = new wxStaticText(this,-1,
													wxGetTranslation( L"With each Pattern, you may specify a Skip Factor.  This is the total number of\n"
																	  L"lines (counting the line that matched) that should be omitted.  You might use\n"
																	  L"this, for example, to omit a hard Page Break and an N line Page Header/Margin."
														));
			//pHelp->SetFont( *wxITALIC_FONT );
			staticBoxSizer2->Add( pHelp, FIX, wxGROW|wxALL, M);

			m_pSpinCtrlSkip = new wxSpinCtrl(this,ID_SPIN_SKIP,_T(""),
											 wxDefaultPosition,wxDefaultSize,wxSP_ARROW_KEYS,
											 1,100,0);
			staticBoxSizer2->Add( m_pSpinCtrlSkip, FIX, wxALL, M);
		}
		vSizerTop->Add(staticBoxSizer2, FIX, wxGROW|wxALL, M);

		//////////////////////////////////////////////////////////////////

		vSizerTop->AddStretchSpacer(VAR);
		
		//////////////////////////////////////////////////////////////////

		vSizerTop->Add( new wxStaticLine(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxLI_HORIZONTAL), FIX, wxGROW|wxLEFT|wxRIGHT, M);

		//////////////////////////////////////////////////////////////////

		wxBoxSizer * hSizerButtons = new wxBoxSizer(wxHORIZONTAL);
		{
			hSizerButtons->AddStretchSpacer(VAR);
			hSizerButtons->Add( CreateButtonSizer(wxOK | wxCANCEL), FIX, 0, 0);
		}
		vSizerTop->Add(hSizerButtons,FIX,wxGROW|wxALL,M);
		
	}
	SetSizer(vSizerTop);
	vSizerTop->SetSizeHints(this);
	vSizerTop->Fit(this);

	// center dialog on parent window rather than arbitrary location
	// (screen-centered on Win32 & GTK, upper-left corner of window in MAC).
	// WXBUG: this doesn't appear to work on WIN32 -- oh well.

	Centre(wxBOTH);

	m_pSpinCtrlSkip->SetValue(m_skip);
	
}

//////////////////////////////////////////////////////////////////

int rs_lomit_dlg::run(rs_lomit ** ppLOmit_Result)
{
	int result = ShowModal();

	if (result != wxID_OK)
	{
		if (ppLOmit_Result) *ppLOmit_Result = NULL;
		return wxID_CANCEL;
	}

	if (ppLOmit_Result)
	{
		m_skip = m_pSpinCtrlSkip->GetValue();
		
		rs_lomit * pLOmit_New = new rs_lomit(m_strPattern,m_skip);

//		pLOmit_New->dump(10);

		*ppLOmit_Result = pLOmit_New;		// caller owns this now
	}

	return wxID_OK;
}

//////////////////////////////////////////////////////////////////

void rs_lomit_dlg::onButtonEvent_InsBlank(wxCommandEvent & /*e*/)
{
	m_strPattern = _T("^[[:blank:]]*$");
	m_pTextCtrlPattern->SetValue(m_strPattern);
	m_pTextCtrlPattern->SetSelection(-1,-1);
}

void rs_lomit_dlg::onButtonEvent_InsPage(wxCommandEvent & /*e*/)
{
	m_strPattern = _T("\\f");
	m_pTextCtrlPattern->SetValue(m_strPattern);
	m_pTextCtrlPattern->SetSelection(-1,-1);
}
