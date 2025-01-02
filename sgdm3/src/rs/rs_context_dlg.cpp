// rs_context_dlg.cpp
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

BEGIN_EVENT_TABLE(rs_context_dlg,wxDialog)
	EVT_CHECKBOX(ID_ENDS_AT_EOL,	rs_context_dlg::onCheckEvent_EndsAtEOL)
	EVT_CHECKBOX(ID_CONTEXT,		rs_context_dlg::onCheckEvent_Important)
	EVT_CHECKBOX(ID_WHITE,			rs_context_dlg::onCheckEvent_White)
	EVT_BUTTON(ID_SHOW_HELP,		rs_context_dlg::onButtonEvent_Help)
END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////

rs_context_dlg::rs_context_dlg(wxWindow * pParent, wxString strTitle,
							   bool bRSRespectEOL, const rs_context * pCTX)
{
	m_pCTX_InitialValue = pCTX;

	//////////////////////////////////////////////////////////////////
	// preload fields that the validators will use
	//////////////////////////////////////////////////////////////////

	m_bRSRespectEOL = bRSRespectEOL;

	rs_context_attrs attrs;

	if (pCTX)
	{
		m_strStartPattern	= *pCTX->getStartPatternString();
		m_strEndPattern		= *pCTX->getEndPatternString();
		m_strEscapeChar		= pCTX->getEscapeChar();
		m_bEndsAtEOL		= pCTX->getEndsAtEOL();

		attrs				= pCTX->getContextAttrs();
	}
	else
	{
		m_bEndsAtEOL		= true;

		attrs				= (( 0
								 //|RS_ATTRS_UNIMPORTANT
								 |RS_ATTRS_RESPECT_EOL
								 |RS_ATTRS_RESPECT_WHITE
								 |RS_ATTRS_RESPECT_CASE
								 //|RS_ATTRS_TAB_IS_WHITE
								   ));
	}

	m_bAllowBlankRegExStart = false;

	m_bContext				= ! RS_ATTRS_IsUnimportant(attrs);
	m_bImportantEOL			= RS_ATTRS_RespectEOL(attrs);
	m_bImportantCase		= RS_ATTRS_RespectCase(attrs);
	m_bImportantWhite		= RS_ATTRS_RespectWhite(attrs);
	m_bTabIsWhite			= RS_ATTRS_TabIsWhite(attrs);

	//////////////////////////////////////////////////////////////////
#if defined(__WXMAC__)
	SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif

	Create(pParent,-1, strTitle, wxDefaultPosition, wxDefaultSize,
		   wxCAPTION | wxSYSTEM_MENU | wxRESIZE_BORDER | wxCLOSE_BOX);

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		wxStaticBoxSizer * staticBoxSizer1 = new wxStaticBoxSizer( new wxStaticBox(this,-1, _("Context Boundaries")), wxVERTICAL);
		{
			wxFlexGridSizer * flexGridSizer = new wxFlexGridSizer(2,4,M,M);
			{

#define XLBL(_string_)	Statement(flexGridSizer->Add( new wxStaticText(this,-1, (_string_)),						\
													  FIX, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxTOP|wxLEFT, M);	)

				//////////////////////////////////////////////////////////////////
				// row 1: "start pattern: [___________]  escape character [___]"
				// row 2: "  end pattern: [___________]  [x] ends at EOL       "
				//////////////////////////////////////////////////////////////////
				
				XLBL( _("&Start Pattern:") );

				m_pTextCtrlStartPattern = new wxTextCtrl(this,ID_START_PATTERN,_T(""),
														 wxDefaultPosition,wxDefaultSize,0,
														 RegExpTextValidator(&m_strStartPattern,
																			 wxString( _("Start Pattern cannot be blank.") ),
																			 &m_bAllowBlankRegExStart));
				flexGridSizer->Add(m_pTextCtrlStartPattern, VAR, wxGROW|wxTOP|wxLEFT, M);

				XLBL( _("Esc&ape Character:") );

				m_pTextCtrlEscapeChar = new wxTextCtrl(this,ID_ESCAPE_CHARACTER,_T(""),
													   wxDefaultPosition,wxSize(20,-1),0,
													   wxTextValidator(wxFILTER_NONE, &m_strEscapeChar));
				m_pTextCtrlEscapeChar->SetMaxLength(1);
				flexGridSizer->Add(m_pTextCtrlEscapeChar, FIX, wxTOP|wxLEFT, M);

				//////////////////////////////////////////////////////////////////

				XLBL( _("End &Pattern:") );

				m_pTextCtrlEndPattern = new wxTextCtrl(this,ID_END_PATTERN,_T(""),
													   wxDefaultPosition,wxDefaultSize,0,
													   RegExpTextValidator(&m_strEndPattern,
																		   wxString( _("End Pattern cannot be blank or Ends at EOL must be checked.") ),
																		   &m_bEndsAtEOL));				// end-pattern can be blank if ends-at-eol set.
				flexGridSizer->Add(m_pTextCtrlEndPattern, VAR, wxGROW|wxTOP|wxLEFT, M);

				m_pCheckBoxEndsAtEOL = new wxCheckBox(this,ID_ENDS_AT_EOL, _("Ends at &EOL"),
													  wxDefaultPosition,wxDefaultSize,0,
													  wxGenericValidator(&m_bEndsAtEOL));
				flexGridSizer->Add(m_pCheckBoxEndsAtEOL, FIX, wxGROW|wxTOP|wxLEFT, M);
				
#undef XLBL
			}
			staticBoxSizer1->Add(flexGridSizer, VAR, wxGROW, M);
		}
		vSizerTop->Add(staticBoxSizer1, FIX, wxGROW|wxALL, M);

		wxStaticBoxSizer * staticBoxSizer2 = new wxStaticBoxSizer( new wxStaticBox(this,-1, _("Context Guidelines")), wxVERTICAL);
		{
			m_pCheckBoxContext = new wxCheckBox(this,ID_CONTEXT,_("Classify Differences as &Important"),
												wxDefaultPosition,wxDefaultSize,0,
												wxGenericValidator(&m_bContext));
			staticBoxSizer2->Add(m_pCheckBoxContext, FIX, wxLEFT|wxTOP|wxRIGHT, M);

			wxBoxSizer * vSizer1 = new wxBoxSizer(wxVERTICAL);
			{
				m_pCheckBoxEOL = new wxCheckBox(this,ID_EOL,_("&Line Termination is Important"),
												wxDefaultPosition,wxDefaultSize,0,
												wxGenericValidator(&m_bImportantEOL));
				vSizer1->Add(m_pCheckBoxEOL, FIX, wxLEFT|wxTOP|wxRIGHT, M);

				m_pCheckBoxCase = new wxCheckBox(this,ID_CASE,_("&Case is Important"),
												 wxDefaultPosition,wxDefaultSize,0,
												 wxGenericValidator(&m_bImportantCase));
				vSizer1->Add(m_pCheckBoxCase, FIX, wxLEFT|wxTOP|wxRIGHT, M);

				m_pCheckBoxWhite = new wxCheckBox(this,ID_WHITE,_("&Whitespace is Important"),
												  wxDefaultPosition,wxDefaultSize,0,
												  wxGenericValidator(&m_bImportantWhite));
				vSizer1->Add(m_pCheckBoxWhite, FIX, wxLEFT|wxTOP|wxRIGHT, M);

				wxBoxSizer * vSizer2 = new wxBoxSizer(wxVERTICAL);
				{
					m_pCheckBoxTab = new wxCheckBox(this,ID_TAB,_("Treat &TABs as Whitespace"),
													wxDefaultPosition,wxDefaultSize,0,
													wxGenericValidator(&m_bTabIsWhite));
					vSizer2->Add(m_pCheckBoxTab, FIX, wxALL, M);
				}
				vSizer1->Add(vSizer2, FIX, wxLEFT|wxRIGHT, M*4);
			}
			staticBoxSizer2->Add(vSizer1, FIX, wxLEFT|wxRIGHT, M*4);

		}
		vSizerTop->Add(staticBoxSizer2, FIX, wxGROW|wxALL, M);

		vSizerTop->AddStretchSpacer(VAR);
		
		vSizerTop->Add( new wxStaticLine(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxLI_HORIZONTAL), FIX, wxGROW|wxLEFT|wxRIGHT, M);

		wxBoxSizer * hSizerButtons = new wxBoxSizer(wxHORIZONTAL);
		{
			m_pButtonHelp = new wxButton(this,ID_SHOW_HELP,_("&Help..."));
			hSizerButtons->Add( m_pButtonHelp, FIX, wxALIGN_CENTER_VERTICAL, 0);
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

	_enable_fields();
}

//////////////////////////////////////////////////////////////////

int rs_context_dlg::run(rs_context ** ppCTX_Result)
{
	int result = ShowModal();

	if (result != wxID_OK)
	{
		if (ppCTX_Result) *ppCTX_Result = NULL;
		return wxID_CANCEL;
	}

	if (ppCTX_Result)
	{
		// set all attr bits from the dialog -- without regard to
		// whether a field was enabled or not -- so that they match
		// what's in the check boxes -- even though some of them
		// won't be used or won't make sense.
	
		rs_context_attrs 		attrs  = 0;
		if (!m_bContext)		attrs |= RS_ATTRS_UNIMPORTANT;
		if (m_bImportantEOL)	attrs |= RS_ATTRS_RESPECT_EOL;
		if (m_bImportantCase)	attrs |= RS_ATTRS_RESPECT_CASE;
		if (m_bImportantWhite)	attrs |= RS_ATTRS_RESPECT_WHITE;
		if (m_bTabIsWhite)		attrs |= RS_ATTRS_TAB_IS_WHITE;

		rs_context * pCTX_New = new rs_context(attrs,m_strStartPattern,m_strEndPattern,
											   ((m_strEscapeChar.Length() > 0) ? *(m_strEscapeChar.wc_str()) : 0),
											   m_bEndsAtEOL);

//		pCTX_New->dump(10);

		*ppCTX_Result = pCTX_New;			// caller owns this now
	}

	return wxID_OK;
}

//////////////////////////////////////////////////////////////////

void rs_context_dlg::_enable_fields(void)
{
	// when the overall context is unimportant, we can disable the individual
	// {case,whitespace,...} important/unimportant checkboxes.
	//
	// If the ruleset has respect-EOL set, then EOL chars are included
	// in the content when the context matching happens.  If the ruleset
	// has respect-EOL turned off, then the context matching never sees
	// the EOL chars.  Furthermore, if the context does not span multiple
	// lines (that is, it ends at the EOL), then EOL importance is not a
	// property of the context.  rather than hiding the context's
	// classify-EOL checkbox, we disable it.

	bool bCtxCanContainEOL = !m_bEndsAtEOL;

	m_pCheckBoxEOL->Enable(m_bContext && m_bRSRespectEOL && bCtxCanContainEOL);

	m_pCheckBoxCase->Enable(m_bContext);

	m_pCheckBoxWhite->Enable(m_bContext);
	
	// Tab-as-whitespace should only be enabled when whitespace-unimportant
	// with the thought being that if the number of spaces is significant,
	// then it's also significant whether it was a SP or a TAB.
	//
	// on the other hand, if the quantity of whitespace is unimportant, then
	// you may want to say that a SP vs a TAB is also unimportant.  (we don't
	// assume this because there may be some kinds of documents where a TAB
	// is not equivalent to a SP.)

	m_pCheckBoxTab->Enable( m_bContext && !m_bImportantWhite );
}

void rs_context_dlg::onCheckEvent_EndsAtEOL(wxCommandEvent & e)
{
	m_bEndsAtEOL = e.IsChecked();

	_enable_fields();
}

void rs_context_dlg::onCheckEvent_Important(wxCommandEvent & e)
{
	m_bContext = e.IsChecked();

	_enable_fields();
}

void rs_context_dlg::onCheckEvent_White(wxCommandEvent & e)
{
	m_bImportantWhite = e.IsChecked();

	_enable_fields();
}

//////////////////////////////////////////////////////////////////

void rs_context_dlg::onButtonEvent_Help(wxCommandEvent & /*e*/)
{
	util_help_dlg dlg(this,
					  wxGetTranslation(
						  L"A CONTEXT is mechanism for identifying portions of a document that should be\n"
						  L"specially handled.  For example, STRING and CHARACTER LITERALS and COMMENTS.\n"
						  L"\n"
						  L"Context Boundaries:\n"
						  L"\n"
						  L"A CONTEXT is defined as a START PATTERN and an optional END PATTERN.  All of\n"
						  L"the text between that matching the Start Pattern and that matching the End\n"
						  L"Pattern and/or the End of the Line will be considered to be in this Context.\n"
						  L"\n"
						  L"Patterns must be valid REGULAR EXPRESSIONS.  The End Pattern may be omitted if\n"
						  L"'Ends at EOL' is checked.  Set the ESCAPE CHARACTER if this Context has a Special\n"
						  L"Character (such as a backslash) to prevent premature matching of the End Pattern\n"
						  L"or EOL.\n"
						  L"\n"
						  L"Context Guidelines:\n"
						  L"\n"
						  L"Individual differences between documents may be classified as IMPORTANT or\n"
						  L"UNIMPORTANT.  Important differences are always highlighted; Unimportant ones may\n"
						  L"be highlighted or hidden using the 'View | Hide Unimportant Differences' menu command.\n"
						  L"\n"
						  L"Within Important Contexts, various commonly-occurring, minor differences can be\n"
						  L"marked Unimportant."
						  ));
	dlg.ShowModal();
}
