// rs_ruleset_dlg.cpp
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

BEGIN_EVENT_TABLE(rs_ruleset_dlg,wxDialog)
	EVT_CHECKBOX(ID_MATCH_STRIP_EOL,				rs_ruleset_dlg::onCheckEvent_Match_StripEOL)
	EVT_CHECKBOX(ID_MATCH_STRIP_CASE,				rs_ruleset_dlg::onCheckEvent_Match_StripCase)
	EVT_CHECKBOX(ID_MATCH_STRIP_WHITE,				rs_ruleset_dlg::onCheckEvent_Match_StripWhite)
	EVT_CHECKBOX(ID_MATCH_STRIP_TAB,				rs_ruleset_dlg::onCheckEvent_Match_StripTab)

	EVT_CHECKBOX(ID_EQUIVALENCE_STRIP_EOL,			rs_ruleset_dlg::onCheckEvent_Equivalence_StripEOL)
	EVT_CHECKBOX(ID_EQUIVALENCE_STRIP_CASE,			rs_ruleset_dlg::onCheckEvent_Equivalence_StripCase)
	EVT_CHECKBOX(ID_EQUIVALENCE_STRIP_WHITE,		rs_ruleset_dlg::onCheckEvent_Equivalence_StripWhite)
	EVT_CHECKBOX(ID_EQUIVALENCE_STRIP_TAB,			rs_ruleset_dlg::onCheckEvent_Equivalence_StripTab)

	EVT_CHECKBOX(ID_DEFAULT_CONTEXT_IMPORTANT,		rs_ruleset_dlg::onCheckEvent_DefaultContext_Important)
	EVT_CHECKBOX(ID_DEFAULT_CONTEXT_EOL,			rs_ruleset_dlg::onCheckEvent_DefaultContext_EOL)
	EVT_CHECKBOX(ID_DEFAULT_CONTEXT_CASE,			rs_ruleset_dlg::onCheckEvent_DefaultContext_Case)
	EVT_CHECKBOX(ID_DEFAULT_CONTEXT_WHITE,			rs_ruleset_dlg::onCheckEvent_DefaultContext_White)
	EVT_CHECKBOX(ID_DEFAULT_CONTEXT_TAB_IS_WHITE,	rs_ruleset_dlg::onCheckEvent_DefaultContext_TabIsWhite)

	EVT_BUTTON(ID_CONTEXT_ADD,						rs_ruleset_dlg::onButtonEvent_ContextAdd)
	EVT_BUTTON(ID_CONTEXT_EDIT,						rs_ruleset_dlg::onButtonEvent_ContextEdit)
	EVT_BUTTON(ID_CONTEXT_DELETE,					rs_ruleset_dlg::onButtonEvent_ContextDelete)

	EVT_BUTTON(ID_LOMIT_ADD,						rs_ruleset_dlg::onButtonEvent_LOmitAdd)
	EVT_BUTTON(ID_LOMIT_EDIT,						rs_ruleset_dlg::onButtonEvent_LOmitEdit)
	EVT_BUTTON(ID_LOMIT_DELETE,						rs_ruleset_dlg::onButtonEvent_LOmitDelete)

	EVT_BUTTON(ID_SHOW_HELP,						rs_ruleset_dlg::onButtonEvent_Help)

	EVT_RADIOBOX(ID_RADIO_ENCODING_STYLE,			rs_ruleset_dlg::onRadioEvent_EncodingStyle)

	EVT_LISTBOX(ID_CONTEXT_LISTBOX,					rs_ruleset_dlg::onListBoxSelect_Context)
	EVT_LISTBOX_DCLICK(ID_CONTEXT_LISTBOX,			rs_ruleset_dlg::onListBoxDClick_Context)

	EVT_LISTBOX(ID_LOMIT_LISTBOX,					rs_ruleset_dlg::onListBoxSelect_LOmit)
	EVT_LISTBOX_DCLICK(ID_LOMIT_LISTBOX,			rs_ruleset_dlg::onListBoxDClick_LOmit)

END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////

int _allocate_array_of_context_descriptions(const rs_ruleset * pRS, wxString ** array)
{
	int kLimit = pRS->getCountContexts();
	if (kLimit == 0)
	{
		*array = NULL;
		return 0;
	}

	*array = new wxString[kLimit];

	for (int k=0; k<kLimit; k++)
		(*array)[k] = pRS->getNthContext(k)->getSummaryDescription();

	return kLimit;
}

void _free_array_of_context_descriptions(wxString * array)
{
	delete [] array;
}

//////////////////////////////////////////////////////////////////

int _allocate_array_of_lomit_descriptions(const rs_ruleset * pRS, wxString ** array)
{
	int kLimit = pRS->getCountLOmit();
	if (kLimit == 0)
	{
		*array = NULL;
		return 0;
	}

	*array = new wxString[kLimit];

	for (int k=0; k<kLimit; k++)
		(*array)[k] = pRS->getNthLOmit(k)->getSummaryDescription();

	return kLimit;
}

void _free_array_of_lomit_descriptions(wxString * array)
{
	delete [] array;
}

//////////////////////////////////////////////////////////////////

void rs_ruleset_dlg::_init(wxWindow * pParent, const wxString & strTitle, bool bFirstPage)
{
	SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
#if defined(__WXMAC__)
	SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif

	Create(pParent,-1, strTitle, wxDefaultPosition, wxDefaultSize,
		   wxCAPTION | wxSYSTEM_MENU | wxRESIZE_BORDER | wxCLOSE_BOX);

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
	// TODO we can remove this #ifdef once we get the build system updated to 2.8.
#if wxCHECK_VERSION(2,8,0)
		m_pBookCtrl = new wxTreebook(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxCLIP_CHILDREN);
#else
		m_pBookCtrl = new util_treebook(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxCLIP_CHILDREN);
#endif

		// WARNING: if you re-order the pages on the notebook, update onButtonEvent_Help()
		if (!m_bHideNameAndSuffix)
			m_pBookCtrl->AddPage( _createPanel_name(), _("Name") );
		m_pBookCtrl->AddPage( _createPanel_char(), _("Character Encodings") );
		m_pBookCtrl->AddPage( _createPanel_lomit(),_("Lines to Omit") );
		m_pBookCtrl->AddPage( _createPanel_line(), _("Line Handling") );
		m_pBookCtrl->AddPage( _createPanel_ctxt(), _("Content Handling") );
		m_pBookCtrl->AddPage( _createPanel_equivalence(), _("Equivalence Mode") );
		vSizerTop->Add( m_pBookCtrl, VAR, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, M);

		// put horizontal line and set of ok/cancel buttons across the bottom of the dialog

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
	Centre(wxBOTH);

#if defined(__WXMAC__)
	// WXBUG 2013/05/07 On Mac (observed on 10.8.2) with wxWidgets 2.9.4
	//                  wxRadioBox widgets are too short -- by about 1 row.
	//                  W2330, W8701, W5873.  The problem goes away if you
	//                  resize the whole dialog.
	//
	//                  Force an extra re-fit on the panels with them.
	vSizerTop->Fit(m_pPanel_char);
#endif

	_enable_fields();

	if (!bFirstPage)
	{
		// try to open the dialog on the same tab/page as they last saw it.

		long kPage = gpGlobalProps->getLong(GlobalProps::GPL_RULESET_DLG_INITIAL_PAGE);	/*gui*/
		if (m_bHideNameAndSuffix)
			kPage--;
		long kLimit = (long)m_pBookCtrl->GetPageCount();
		if ( (kPage >= 0) && (kPage < kLimit) )
			m_pBookCtrl->SetSelection(kPage);
	}
}

rs_ruleset_dlg::~rs_ruleset_dlg(void)
{
	// regardless of whether OK or CANCEL was pressed, try to remember
	// the current tab/page for next time.
	
	long kPage = m_pBookCtrl->GetSelection();
	if (m_bHideNameAndSuffix)
		kPage++;
	gpGlobalProps->setLong(GlobalProps::GPL_RULESET_DLG_INITIAL_PAGE,kPage); /*gui*/

	delete m_pRSWorkingCopy;
}

//////////////////////////////////////////////////////////////////
// create widgets for each panel

wxPanel * rs_ruleset_dlg::_createPanel_name(void)
{
	m_pPanel_name = new wxPanel(m_pBookCtrl, ID_PANEL_NAME);

	wxPanel * pPanel = m_pPanel_name;

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		wxStaticBoxSizer * staticBoxNameSizer = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1, _("&Ruleset Name")), wxVERTICAL);
		{
			staticBoxNameSizer->Add( new wxTextCtrl(pPanel,ID_RULESET_NAME,_T(""),
													wxDefaultPosition,wxDefaultSize,0,
													NonBlankTextValidator(&m_strName, _("Error: Ruleset Name Cannot be Blank"))),
									 FIX, wxGROW|wxALL, M);
		}
		vSizerTop->Add(staticBoxNameSizer, FIX, wxGROW|wxALL, M);
		
		wxStaticBoxSizer * staticBoxSuffixSizer = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1, _("&File Suffixes")), wxVERTICAL);
		{
			staticBoxSuffixSizer->Add( new wxTextCtrl(pPanel,ID_RULESET_SUFFIX,_T(""),
													  wxDefaultPosition,wxDefaultSize,0,
													  NonBlankTextValidator(&m_strSuffixes, _("Error: Suffix List Cannot be Blank"))),
									   FIX, wxGROW|wxALL, M);

			// TODO do we want to add a checkbox for a "also match files without suffixes" feature ??
		}
		vSizerTop->Add(staticBoxSuffixSizer, FIX, wxGROW|wxALL, M);
	}
	m_pPanel_name->SetSizer(vSizerTop);
	vSizerTop->Fit(m_pPanel_name);

	return m_pPanel_name;
}

wxPanel * rs_ruleset_dlg::_createPanel_line(void)
{
	m_pPanel_line = new wxPanel(m_pBookCtrl, ID_PANEL_LINE);

	wxPanel * pPanel = m_pPanel_line;

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		wxStaticBoxSizer * staticBoxEOLSizer = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1, _("Line Termination")), wxVERTICAL);
		{
			staticBoxEOLSizer->Add( new wxCheckBox(pPanel,ID_MATCH_STRIP_EOL,
												   _("&Ignore/Strip All Line Termination Characters"),
												   wxDefaultPosition,wxDefaultSize,0,
												   wxGenericValidator(&m_bMatchStripEOL)),
									FIX, wxALL, M);
		}
		vSizerTop->Add(staticBoxEOLSizer, FIX, wxGROW|wxALL, M);

		wxStaticBoxSizer * staticBoxLineSizer = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1, _("Line Matching")), wxVERTICAL);
		{
			staticBoxLineSizer->Add( new wxCheckBox(pPanel,ID_MATCH_STRIP_CASE,
													_("Ignore/Fold &Case when Matching Lines"),
													wxDefaultPosition,wxDefaultSize,0,
													wxGenericValidator(&m_bMatchStripCase)),
									 FIX, wxLEFT|wxTOP|wxRIGHT, M);

			staticBoxLineSizer->Add( new wxCheckBox(pPanel,ID_MATCH_STRIP_WHITE,
													_("Ignore/Strip &Whitespace when Matching Lines"),
													wxDefaultPosition,wxDefaultSize,0,
													wxGenericValidator(&m_bMatchStripWhite)),
									 FIX, wxLEFT|wxTOP|wxRIGHT, M);

			wxBoxSizer * vSizer1 = new wxBoxSizer(wxVERTICAL);
			{
				// Tab-as-whitespace should only be enabled when strip-whitespace
				vSizer1->Add( new wxCheckBox(pPanel,ID_MATCH_STRIP_TAB,_("Also Ignore/Strip &TABs when Matching Lines"),
											 wxDefaultPosition,wxDefaultSize,0,
											 wxGenericValidator(&m_bMatchStripTab)),
							  FIX, wxALL, M);
			}
			staticBoxLineSizer->Add(vSizer1, FIX, wxLEFT|wxRIGHT, M*4);
		}
		vSizerTop->Add(staticBoxLineSizer, FIX, wxGROW|wxALL, M);
	}
	m_pPanel_line->SetSizer(vSizerTop);
	vSizerTop->Fit(m_pPanel_line);

	return m_pPanel_line;
}

wxPanel * rs_ruleset_dlg::_createPanel_char(void)
{
	m_pPanel_char = new wxPanel(m_pBookCtrl, ID_PANEL_CHAR);
	
	wxPanel * pPanel = m_pPanel_char;

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		wxStaticBoxSizer * staticBoxSniffSizer = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1,_("Automatic Character Encoding Detection")), wxVERTICAL);
		{
			staticBoxSniffSizer->Add( new wxCheckBox(pPanel,ID_CHECK_SNIFF_ENCODING_BOM,_("Search for Unicode BOM"),
													 wxDefaultPosition,wxDefaultSize,0,
													 wxGenericValidator(&m_bSniffEncodingBOM)),
									  FIX, wxALL, M);
		}
		vSizerTop->Add(staticBoxSniffSizer, FIX, wxGROW|wxALL, M);
		
		//////////////////////////////////////////////////////////////////

		wxString astrEncodingStyle[__RS_ENCODING_STYLE__NR__] = { _("Use System Local/&Default Encoding"),	// WARNING: order must match
																  _("Ask for Each &Window"),				// WARNING: RS_ENCODING_STYLE
																  _("Ask for Each &File in Each Window"),	// WARNING: enum in rs_dcl.h
																  _("Use &Named Encoding Below"),
																  _("Use &Named Encoding Below with 1 Alternate"),
																  _("Use &Named Encoding Below with 2 Alternates") };
		vSizerTop->Add( new wxRadioBox(pPanel,ID_RADIO_ENCODING_STYLE,_("Fallback Character Encoding Options"),
									   wxDefaultPosition,wxDefaultSize,
									   __RS_ENCODING_STYLE__NR__,astrEncodingStyle,
									   __RS_ENCODING_STYLE__NR__,wxRA_SPECIFY_ROWS,
									   wxGenericValidator(&m_encodingStyle)),
						FIX, wxGROW|wxALL, M);

		//////////////////////////////////////////////////////////////////

		wxStaticBoxSizer * staticBoxNamedSizer =
			new wxStaticBoxSizer(
				new wxStaticBox(pPanel,-1, _("Try Named &Character Encodings")),
				wxVERTICAL);
		{
			wxFlexGridSizer * pFlex = new wxFlexGridSizer(3,2,M,M);
			{
				pFlex->Add( new wxStaticText(pPanel, wxID_ANY, _T("")),
							FIX, wxALIGN_CENTER_VERTICAL, 0);
				pFlex->Add( new wxComboBox(pPanel, ID_COMBOBOX_ENCODING1,
										   m_encodingSettingStr1,
										   wxDefaultPosition,wxDefaultSize,
										   m_encTable.getCount(),m_encTable.getArrayNames(),wxCB_READONLY,
										   wxGenericValidator(&m_encodingSettingStr1)),
							FIX, 0, 0);

				pFlex->Add( new wxStaticText(pPanel, ID_LABEL_ENCODING2, _T("Alt 1:")),
							FIX, wxALIGN_CENTER_VERTICAL, 0);
				pFlex->Add( new wxComboBox(pPanel, ID_COMBOBOX_ENCODING2,
										   m_encodingSettingStr2,
										   wxDefaultPosition,wxDefaultSize,
										   m_encTable.getCount(),m_encTable.getArrayNames(),wxCB_READONLY,
										   wxGenericValidator(&m_encodingSettingStr2)),
							FIX, 0, 0);

				pFlex->Add( new wxStaticText(pPanel, ID_LABEL_ENCODING3, _T("Alt 2:")),
							FIX, wxALIGN_CENTER_VERTICAL, 0);
				pFlex->Add( new wxComboBox(pPanel, ID_COMBOBOX_ENCODING3,
										   m_encodingSettingStr3,
										   wxDefaultPosition,wxDefaultSize,
										   m_encTable.getCount(),m_encTable.getArrayNames(),wxCB_READONLY,
										   wxGenericValidator(&m_encodingSettingStr3)),
							FIX, 0, 0);
			}
			staticBoxNamedSizer->Add( pFlex, FIX, wxALL, M);
		}
		vSizerTop->Add(staticBoxNamedSizer, FIX, wxGROW|wxALL, M);
	}
	m_pPanel_char->SetSizer(vSizerTop);
	vSizerTop->Fit(m_pPanel_char);

	return m_pPanel_char;
}

wxPanel * rs_ruleset_dlg::_createPanel_ctxt(void)
{
	m_pPanel_ctxt = new wxPanel(m_pBookCtrl, ID_PANEL_CTXT);
	
	wxPanel * pPanel = m_pPanel_ctxt;

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		wxStaticBoxSizer * staticBoxContextSizer = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1, _("Matched Contexts")), wxVERTICAL);
		{
			wxBoxSizer * hSizerContext = new wxBoxSizer(wxHORIZONTAL);
			{
				wxString * array = NULL;
				int cContexts = _allocate_array_of_context_descriptions(m_pRSWorkingCopy,&array);

				m_pListBoxContext = new wxListBox(pPanel,ID_CONTEXT_LISTBOX,
												  wxDefaultPosition,wxDefaultSize,
												  cContexts,array,wxLB_SINGLE);
				_free_array_of_context_descriptions(array);
				
				hSizerContext->Add(m_pListBoxContext, VAR, wxGROW, 0);

				wxBoxSizer * vSizerContext = new wxBoxSizer(wxVERTICAL);
				{
					vSizerContext->Add( new wxButton(pPanel, ID_CONTEXT_ADD,      _("&Add..."   )), FIX, wxGROW, 0);
					vSizerContext->Add( new wxButton(pPanel, ID_CONTEXT_EDIT,     _("&Edit..."  )), FIX, wxGROW|wxTOP, M);
					vSizerContext->Add( new wxButton(pPanel, ID_CONTEXT_DELETE,   _("&Delete"   )), FIX, wxGROW|wxTOP, M);
				}
				hSizerContext->Add(vSizerContext, FIX, wxLEFT, M*2);
			}
			staticBoxContextSizer->Add(hSizerContext, VAR, wxGROW|wxALL, M);
		}
		vSizerTop->Add(staticBoxContextSizer, VAR, wxGROW|wxALL, M);

		//////////////////////////////////////////////////////////////////

		wxStaticBoxSizer * staticBoxDefaultContextSizer = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1, _("Default Context Guidelines")), wxVERTICAL);
		{
			staticBoxDefaultContextSizer->Add( new wxCheckBox(pPanel,ID_DEFAULT_CONTEXT_IMPORTANT,_("Classify Differences as &Important (Always Highlight)"),
															  wxDefaultPosition,wxDefaultSize,0,
															  wxGenericValidator(&m_bDefaultImportant)),
											   FIX, wxLEFT|wxTOP|wxRIGHT, M);

			wxBoxSizer * vSizer1 = new wxBoxSizer(wxVERTICAL);
			{
				vSizer1->Add( new wxCheckBox(pPanel,ID_DEFAULT_CONTEXT_EOL,
													_("&Line Termination is Important"),
													wxDefaultPosition,wxDefaultSize,0,
													wxGenericValidator(&m_bDefaultContextEOL)),
									 FIX, wxLEFT|wxTOP|wxRIGHT, M);

				vSizer1->Add( new wxCheckBox(pPanel,ID_DEFAULT_CONTEXT_CASE,
											 _("&Case is Important"),
											 wxDefaultPosition,wxDefaultSize,0,
											 wxGenericValidator(&m_bDefaultContextCase)),
							  FIX, wxLEFT|wxTOP|wxRIGHT, M);

				vSizer1->Add( new wxCheckBox(pPanel,ID_DEFAULT_CONTEXT_WHITE,
											 _("&Whitespace is Important"),
											 wxDefaultPosition,wxDefaultSize,0,
											 wxGenericValidator(&m_bDefaultContextWhite)),
							  FIX, wxLEFT|wxTOP|wxRIGHT, M);

				wxBoxSizer * vSizer2 = new wxBoxSizer(wxVERTICAL);
				{
					// Tab-as-whitespace should only be enabled when whitespace-not-important
					vSizer2->Add( new wxCheckBox(pPanel,ID_DEFAULT_CONTEXT_TAB_IS_WHITE,_("Treat &TABs as Whitespace"),
												 wxDefaultPosition,wxDefaultSize,0,
												 wxGenericValidator(&m_bDefaultTabIsWhite)),
								  FIX, wxALL, M);
				}
				vSizer1->Add(vSizer2, FIX, wxLEFT|wxRIGHT, M*4);
			}
			staticBoxDefaultContextSizer->Add(vSizer1, FIX, wxLEFT, M*4);

		}
		vSizerTop->Add(staticBoxDefaultContextSizer, FIX, wxGROW|wxALL, M);
		

	}
	m_pPanel_ctxt->SetSizer(vSizerTop);
	vSizerTop->Fit(m_pPanel_ctxt);

	return m_pPanel_ctxt;
}

wxPanel * rs_ruleset_dlg::_createPanel_lomit(void)
{
	m_pPanel_lomit = new wxPanel(m_pBookCtrl, ID_PANEL_LOMIT);
	
	wxPanel * pPanel = m_pPanel_lomit;

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		wxStaticBoxSizer * staticBoxLOmitSizer = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1, _("Patterns for Lines to Omit")), wxVERTICAL);
		{
			wxBoxSizer * hSizerLOmit = new wxBoxSizer(wxHORIZONTAL);
			{
				wxString * array = NULL;
				int cLOmit = _allocate_array_of_lomit_descriptions(m_pRSWorkingCopy,&array);

				m_pListBoxLOmit = new wxListBox(pPanel,ID_LOMIT_LISTBOX,
												wxDefaultPosition,wxDefaultSize,
												cLOmit,array,wxLB_SINGLE);
				_free_array_of_lomit_descriptions(array);
				
				hSizerLOmit->Add(m_pListBoxLOmit, VAR, wxGROW, 0);

				wxBoxSizer * vSizerLOmit = new wxBoxSizer(wxVERTICAL);
				{
					vSizerLOmit->Add( new wxButton(pPanel, ID_LOMIT_ADD,   _("&Add..." )), FIX, wxGROW, 0);
					vSizerLOmit->Add( new wxButton(pPanel, ID_LOMIT_EDIT,  _("&Edit...")), FIX, wxGROW|wxTOP, M);
					vSizerLOmit->Add( new wxButton(pPanel, ID_LOMIT_DELETE,_("&Delete" )), FIX, wxGROW|wxTOP, M);
				}
				hSizerLOmit->Add(vSizerLOmit, FIX, wxLEFT, M*2);
			}
			staticBoxLOmitSizer->Add(hSizerLOmit, VAR, wxGROW|wxALL, M);
		}
		vSizerTop->Add(staticBoxLOmitSizer, VAR, wxGROW|wxALL, M);


	}
	m_pPanel_lomit->SetSizer(vSizerTop);
	vSizerTop->Fit(m_pPanel_lomit);

	return m_pPanel_lomit;
}

wxPanel * rs_ruleset_dlg::_createPanel_equivalence(void)
{
	m_pPanel_equivalence = new wxPanel(m_pBookCtrl, ID_PANEL_EQUIVALENCE);

	wxPanel * pPanel = m_pPanel_equivalence;

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		wxStaticBoxSizer * staticBoxSizer = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1, _("Folder Window Equivalence Settings")), wxVERTICAL);
		{
			staticBoxSizer->Add( new wxCheckBox(pPanel,ID_EQUIVALENCE_STRIP_EOL,
												_("&Ignore/Strip All Line Termination Characters"),
												wxDefaultPosition,wxDefaultSize,0,
												wxGenericValidator(&m_bEquivalenceStripEOL)),
								 FIX, wxLEFT|wxTOP|wxRIGHT, M);

			staticBoxSizer->Add( new wxCheckBox(pPanel,ID_EQUIVALENCE_STRIP_CASE,
												_("Ignore/Fold &Case"),
												wxDefaultPosition,wxDefaultSize,0,
												wxGenericValidator(&m_bEquivalenceStripCase)),
								 FIX, wxLEFT|wxTOP|wxRIGHT, M);

			staticBoxSizer->Add( new wxCheckBox(pPanel,ID_EQUIVALENCE_STRIP_WHITE,
												_("Ignore/Strip &Whitespace"),
												wxDefaultPosition,wxDefaultSize,0,
												wxGenericValidator(&m_bEquivalenceStripWhite)),
								 FIX, wxLEFT|wxTOP|wxRIGHT, M);

			wxBoxSizer * vSizer1 = new wxBoxSizer(wxVERTICAL);
			{
				// Tab-as-whitespace should only be enabled when strip-whitespace
				vSizer1->Add( new wxCheckBox(pPanel,ID_EQUIVALENCE_STRIP_TAB,_("Also Ignore/Strip &TABs"),
											 wxDefaultPosition,wxDefaultSize,0,
											 wxGenericValidator(&m_bEquivalenceStripTab)),
							  FIX, wxALL, M);
			}
			staticBoxSizer->Add(vSizer1, FIX, wxLEFT|wxRIGHT, M*4);
		}
		vSizerTop->Add(staticBoxSizer, FIX, wxGROW|wxALL, M);
	}
	m_pPanel_equivalence->SetSizer(vSizerTop);
	vSizerTop->Fit(m_pPanel_equivalence);

	return m_pPanel_equivalence;
}

//////////////////////////////////////////////////////////////////

int rs_ruleset_dlg::run(rs_ruleset ** ppRS_Result)
{
	int result = ShowModal();

	if (result != wxID_OK)
	{
		if (ppRS_Result) *ppRS_Result = NULL;
		return wxID_CANCEL;
	}

	// TODO require ruleset name to be non-blank
	// TODO require suffixes to be non-blank

	if (ppRS_Result)
	{
		rs_context_attrs			attrsStrip  = 0;
		if (!m_bMatchStripEOL)		attrsStrip |= RS_ATTRS_RESPECT_EOL;
		if (!m_bMatchStripCase)		attrsStrip |= RS_ATTRS_RESPECT_CASE;
		if (!m_bMatchStripWhite)	attrsStrip |= RS_ATTRS_RESPECT_WHITE;
		if (m_bMatchStripTab)		attrsStrip |= RS_ATTRS_TAB_IS_WHITE;

		rs_context_attrs				attrsDefault  = 0;
		if (!m_bDefaultImportant)		attrsDefault |= RS_ATTRS_UNIMPORTANT;
		if (m_bDefaultContextEOL)		attrsDefault |= RS_ATTRS_RESPECT_EOL;
		if (m_bDefaultContextCase)		attrsDefault |= RS_ATTRS_RESPECT_CASE;
		if (m_bDefaultContextWhite)		attrsDefault |= RS_ATTRS_RESPECT_WHITE;
		if (m_bDefaultTabIsWhite)		attrsDefault |= RS_ATTRS_TAB_IS_WHITE;

		rs_context_attrs				attrsEquivalence  = 0;
		if (!m_bEquivalenceStripEOL)	attrsEquivalence |= RS_ATTRS_RESPECT_EOL;
		if (!m_bEquivalenceStripCase)	attrsEquivalence |= RS_ATTRS_RESPECT_CASE;
		if (!m_bEquivalenceStripWhite)	attrsEquivalence |= RS_ATTRS_RESPECT_WHITE;
		if (m_bEquivalenceStripTab)		attrsEquivalence |= RS_ATTRS_TAB_IS_WHITE;

		// we use the generic validator on the ID_COMBOBOX_ENCODING (which is
		// STRING based). so we need to extract the numerical value from it.

		m_encodingSetting1 = m_encTable.lookupEnc(m_encTable.findEnc(m_encodingSettingStr1));
		m_encodingSetting2 = m_encTable.lookupEnc(m_encTable.findEnc(m_encodingSettingStr2));
		m_encodingSetting3 = m_encTable.lookupEnc(m_encTable.findEnc(m_encodingSettingStr3));

		m_pRSWorkingCopy->setName(m_strName);
		m_pRSWorkingCopy->setSuffixes(m_strSuffixes);
		m_pRSWorkingCopy->setEncodingStyle((RS_ENCODING_STYLE)m_encodingStyle);
		m_pRSWorkingCopy->setEncoding1(m_encodingSetting1);
		m_pRSWorkingCopy->setEncoding2(m_encodingSetting2);
		m_pRSWorkingCopy->setEncoding3(m_encodingSetting3);
		m_pRSWorkingCopy->setSniffEncodingBOM(m_bSniffEncodingBOM);
		m_pRSWorkingCopy->setMatchStripAttrs(attrsStrip);
		m_pRSWorkingCopy->setDefaultContextAttrs(attrsDefault);
		m_pRSWorkingCopy->setEquivalenceAttrs(attrsEquivalence);
		
		// LOmit is handled by add/edit/delete-lomit buttons.
		// vector of contexts were handled by {add,edit,delete}-context buttons.

//		m_pRSWorkingCopy->dump(10);

		*ppRS_Result = m_pRSWorkingCopy;
		m_pRSWorkingCopy = NULL;
	}
	
	return wxID_OK;
}

//////////////////////////////////////////////////////////////////

void rs_ruleset_dlg::_preload_fields(const wxString & strName, const wxString & strSuffixes,
									 rs_context_attrs attrsStrip,
									 int encodingStyle,
									 util_encoding encodingSetting1,
									 util_encoding encodingSetting2,
									 util_encoding encodingSetting3,
									 bool bSniffEncodingBOM,
									 rs_context_attrs attrsDefault,
									 rs_context_attrs attrsEquivalence)
{
	m_strName				= strName;
	m_strSuffixes			= strSuffixes;

	m_bMatchStripEOL		= ! RS_ATTRS_RespectEOL(attrsStrip);
	m_bMatchStripCase		= ! RS_ATTRS_RespectCase(attrsStrip);
	m_bMatchStripWhite		= ! RS_ATTRS_RespectWhite(attrsStrip);
	m_bMatchStripTab		= RS_ATTRS_TabIsWhite(attrsStrip);

	m_bEquivalenceStripEOL		= ! RS_ATTRS_RespectEOL(attrsEquivalence);
	m_bEquivalenceStripCase		= ! RS_ATTRS_RespectCase(attrsEquivalence);
	m_bEquivalenceStripWhite	= ! RS_ATTRS_RespectWhite(attrsEquivalence);
	m_bEquivalenceStripTab		= RS_ATTRS_TabIsWhite(attrsEquivalence);

	m_encodingStyle			= encodingStyle;
	m_encodingSetting1		= encodingSetting1;
	m_encodingSetting2		= encodingSetting2;
	m_encodingSetting3		= encodingSetting3;
	m_encodingSettingStr1	= m_encTable.getName((wxFontEncoding)encodingSetting1);
	m_encodingSettingStr2	= m_encTable.getName((wxFontEncoding)encodingSetting2);
	m_encodingSettingStr3	= m_encTable.getName((wxFontEncoding)encodingSetting3);
	m_bSniffEncodingBOM		= bSniffEncodingBOM;

	m_bDefaultImportant		= ! RS_ATTRS_IsUnimportant(attrsDefault);
	m_bDefaultContextEOL	= RS_ATTRS_RespectEOL(attrsDefault);
	m_bDefaultContextCase	= RS_ATTRS_RespectCase(attrsDefault);
	m_bDefaultContextWhite	= RS_ATTRS_RespectWhite(attrsDefault);
	m_bDefaultTabIsWhite	= RS_ATTRS_TabIsWhite(attrsDefault);
}

//////////////////////////////////////////////////////////////////

rs_ruleset_dlg__add::rs_ruleset_dlg__add(wxWindow * pParent)
{
	m_bHideNameAndSuffix = false;
	m_pRSWorkingCopy = new rs_ruleset();

	wxString strBlank;

	_preload_fields(strBlank,strBlank,
					0 /*|RS_ATTRS_RESPECT_EOL*/ /*|RS_ATTRS_RESPECT_CASE*/ /*|RS_ATTRS_RESPECT_WHITE*/   |RS_ATTRS_TAB_IS_WHITE,
					RS_ENCODING_STYLE_LOCAL,wxFONTENCODING_DEFAULT,wxFONTENCODING_DEFAULT,wxFONTENCODING_DEFAULT,true,
					0   |RS_ATTRS_RESPECT_EOL     |RS_ATTRS_RESPECT_CASE     |RS_ATTRS_RESPECT_WHITE   /*|RS_ATTRS_TAB_IS_WHITE*/ /*|RS_ATTRS_UNIMPORTANT*/,
					0 /*|RS_ATTRS_RESPECT_EOL*/ /*|RS_ATTRS_RESPECT_CASE*/ /*|RS_ATTRS_RESPECT_WHITE*/   |RS_ATTRS_TAB_IS_WHITE
		);

	_init(pParent, _("New Ruleset"), true);
}

//////////////////////////////////////////////////////////////////

rs_ruleset_dlg__edit::rs_ruleset_dlg__edit(wxWindow * pParent, const rs_ruleset * pRS)
{
	m_bHideNameAndSuffix = false;
	m_pRSWorkingCopy = new rs_ruleset(*pRS);

	rs_context_attrs attrsStrip			= pRS->getMatchStripAttrs();
	rs_context_attrs attrsClassify		= pRS->getDefaultContextAttrs();
	rs_context_attrs attrsEquivalence	= pRS->getEquivalenceAttrs();

	_preload_fields(pRS->getName(), pRS->getSuffixes(),
					attrsStrip,
					pRS->getEncodingStyle(), pRS->getEncoding1(), pRS->getEncoding2(), pRS->getEncoding3(), pRS->getSniffEncodingBOM(),
					attrsClassify,
					attrsEquivalence);

	wxString strTitle = wxString::Format( _("Edit Ruleset: %s"), pRS->getName().wc_str() );

	_init(pParent, strTitle, false);
}

//////////////////////////////////////////////////////////////////

rs_ruleset_dlg__edit_default::rs_ruleset_dlg__edit_default(wxWindow * pParent, const rs_ruleset * pRS)
{
	// when editing the default ruleset, we can hide/disable a few fields.

	m_bHideNameAndSuffix = true;
	m_pRSWorkingCopy = new rs_ruleset(*pRS);

	rs_context_attrs attrsStrip			= pRS->getMatchStripAttrs();
	rs_context_attrs attrsClassify		= pRS->getDefaultContextAttrs();
	rs_context_attrs attrsEquivalence	= pRS->getEquivalenceAttrs();

	_preload_fields(pRS->getName(), pRS->getSuffixes(),
					attrsStrip,
					pRS->getEncodingStyle(), pRS->getEncoding1(),pRS->getEncoding2(), pRS->getEncoding3(), pRS->getSniffEncodingBOM(),
					attrsClassify,
					attrsEquivalence);

	wxString strTitle = _("Edit Default Ruleset");

	_init(pParent, strTitle, false);
}

//////////////////////////////////////////////////////////////////

void rs_ruleset_dlg::_enable_fields(void)
{
	// if default-context-not-important, then none of the default-context-{case,white,tab}-important
	// matter, because the whole context is unimportant.
	//
	// if match-strip-eol, then default-context-eol-important doesn't matter because EOLs are not
	// used in analysis.
	//
	// furthermore, only enable default-context-tab-important when default-context-white-not-important
	// (because if differences in the quantity of whitespace is important, then differences in the
	// actual chars (spaces vs tabs) should also be important).
	//
	// also only enable match-strip-tab when match-strip-white because if you're not stripping spaces
	// it doesn't make any sense to strip tabs.

	FindWindow(ID_DEFAULT_CONTEXT_EOL         )->Enable( m_bDefaultImportant  &&  !m_bMatchStripEOL );
	FindWindow(ID_DEFAULT_CONTEXT_CASE        )->Enable( m_bDefaultImportant );
	FindWindow(ID_DEFAULT_CONTEXT_WHITE       )->Enable( m_bDefaultImportant );
	FindWindow(ID_DEFAULT_CONTEXT_TAB_IS_WHITE)->Enable( m_bDefaultImportant  &&  !m_bDefaultContextWhite );
	
	FindWindow(ID_MATCH_STRIP_TAB             )->Enable( m_bMatchStripWhite );
	FindWindow(ID_EQUIVALENCE_STRIP_TAB       )->Enable( m_bEquivalenceStripWhite );

	// only enable encoding combo box when encoding-style radio is set to use-named

	int nrNamed;
	switch (m_encodingStyle)
	{
	default:
		nrNamed = 0;
		break;
	case RS_ENCODING_STYLE_NAMED1:
		nrNamed = 1;
		break;
	case RS_ENCODING_STYLE_NAMED2:
		nrNamed = 2;
		break;
	case RS_ENCODING_STYLE_NAMED3:
		nrNamed = 3;
		break;
	}
	FindWindow(ID_COMBOBOX_ENCODING1)->Enable( nrNamed >= 1 );
	FindWindow(ID_COMBOBOX_ENCODING2)->Enable( nrNamed >= 2 );
	FindWindow(ID_COMBOBOX_ENCODING3)->Enable( nrNamed >= 3 );

	FindWindow(ID_LABEL_ENCODING2)->Enable( nrNamed >= 2 );
	FindWindow(ID_LABEL_ENCODING3)->Enable( nrNamed >= 3 );

	// disable {edit,delete}-context when listbox is empty or nothing selected.

	bool bHaveContexts         = (m_pRSWorkingCopy->getCountContexts() > 0);
	bool bHaveContextSelection = (m_pListBoxContext->GetSelection() != wxNOT_FOUND);
	
	FindWindow(ID_CONTEXT_EDIT                )->Enable( bHaveContexts && bHaveContextSelection );
	FindWindow(ID_CONTEXT_DELETE              )->Enable( bHaveContexts && bHaveContextSelection );

	// disable {edit,delete}-lomit when listbox is empty or nothing selected.

	bool bHaveLOmits           = (m_pRSWorkingCopy->getCountLOmit() > 0);
	bool bHaveLOmitsSelection  = (m_pListBoxLOmit->GetSelection() != wxNOT_FOUND);
	
	FindWindow(ID_LOMIT_EDIT                  )->Enable( bHaveLOmits && bHaveLOmitsSelection );
	FindWindow(ID_LOMIT_DELETE                )->Enable( bHaveLOmits && bHaveLOmitsSelection );
}

//////////////////////////////////////////////////////////////////

void rs_ruleset_dlg::onCheckEvent_Match_StripEOL(wxCommandEvent & e)
{
	m_bMatchStripEOL = e.IsChecked();

	_enable_fields();
}

void rs_ruleset_dlg::onCheckEvent_Match_StripCase(wxCommandEvent & e)
{
	m_bMatchStripCase = e.IsChecked();

	_enable_fields();
}

void rs_ruleset_dlg::onCheckEvent_Match_StripWhite(wxCommandEvent & e)
{
	m_bMatchStripWhite = e.IsChecked();

	_enable_fields();
}

void rs_ruleset_dlg::onCheckEvent_Match_StripTab(wxCommandEvent & e)
{
	m_bMatchStripTab = e.IsChecked();

	_enable_fields();
}

//////////////////////////////////////////////////////////////////

void rs_ruleset_dlg::onCheckEvent_Equivalence_StripEOL(wxCommandEvent & e)
{
	m_bEquivalenceStripEOL = e.IsChecked();

	_enable_fields();
}

void rs_ruleset_dlg::onCheckEvent_Equivalence_StripCase(wxCommandEvent & e)
{
	m_bEquivalenceStripCase = e.IsChecked();

	_enable_fields();
}

void rs_ruleset_dlg::onCheckEvent_Equivalence_StripWhite(wxCommandEvent & e)
{
	m_bEquivalenceStripWhite = e.IsChecked();

	_enable_fields();
}

void rs_ruleset_dlg::onCheckEvent_Equivalence_StripTab(wxCommandEvent & e)
{
	m_bEquivalenceStripTab = e.IsChecked();

	_enable_fields();
}

//////////////////////////////////////////////////////////////////

void rs_ruleset_dlg::onCheckEvent_DefaultContext_Important(wxCommandEvent & e)
{
	m_bDefaultImportant = e.IsChecked();

	_enable_fields();
}

void rs_ruleset_dlg::onCheckEvent_DefaultContext_EOL(wxCommandEvent & e)
{
	m_bDefaultContextEOL = e.IsChecked();

	_enable_fields();
}
void rs_ruleset_dlg::onCheckEvent_DefaultContext_Case(wxCommandEvent & e)
{
	m_bDefaultContextCase = e.IsChecked();

	_enable_fields();
}

void rs_ruleset_dlg::onCheckEvent_DefaultContext_White(wxCommandEvent & e)
{
	m_bDefaultContextWhite = e.IsChecked();

	_enable_fields();
}

void rs_ruleset_dlg::onCheckEvent_DefaultContext_TabIsWhite(wxCommandEvent & e)
{
	m_bDefaultTabIsWhite = e.IsChecked();

	_enable_fields();
}

//////////////////////////////////////////////////////////////////

void rs_ruleset_dlg::onButtonEvent_ContextAdd(wxCommandEvent & /*e*/)
{
	rs_context_dlg dlg(this,_("Define New Context"),!m_bMatchStripEOL,NULL);

	rs_context * pNew = NULL;
	int result = dlg.run(&pNew);

	if (result != wxID_OK)
		return;

	m_pRSWorkingCopy->addContext(pNew);
	int index = m_pListBoxContext->Append( pNew->getSummaryDescription() );

	m_pListBoxContext->SetSelection(index);

	_enable_fields();
}

void rs_ruleset_dlg::onButtonEvent_ContextEdit(wxCommandEvent & /*e*/)
{
	int index = m_pListBoxContext->GetSelection();
	if (index == wxNOT_FOUND)	// should not happen because button should not be enabled
		return;

	int limit = m_pRSWorkingCopy->getCountContexts();
	if (index >= limit)			// should not happen
		return;

	const rs_context * pCTX_Current = m_pRSWorkingCopy->getNthContext(index);

	rs_context_dlg dlg(this,_("Edit Context"),!m_bMatchStripEOL,pCTX_Current);

	rs_context * pCTX_New = NULL;
	int result = dlg.run(&pCTX_New);

	if (result != wxID_OK)
		return;

	if (pCTX_Current->isEqual(pCTX_New))	// if they just clicked OK without
	{										// changing anything, we don't really
		delete pCTX_New;					// need to do anything.
		return;
	}

	m_pRSWorkingCopy->replaceContext(index,pCTX_New,true);
	//pCTX_Current=NULL;
	m_pListBoxContext->SetString(index, pCTX_New->getSummaryDescription() );
	m_pListBoxContext->SetSelection(index);

	_enable_fields();
}

void rs_ruleset_dlg::onButtonEvent_ContextDelete(wxCommandEvent & /*e*/)
{
	int index = m_pListBoxContext->GetSelection();
	if (index == wxNOT_FOUND)	// should not happen because button should not enabled
		return;

	int limit = m_pRSWorkingCopy->getCountContexts();
	if (index >= limit)			// should not happen
		return;

	m_pRSWorkingCopy->deleteContext(index);
	m_pListBoxContext->Delete(index);
	
	_enable_fields();
}

//////////////////////////////////////////////////////////////////

void rs_ruleset_dlg::onButtonEvent_LOmitAdd(wxCommandEvent & /*e*/)
{
	rs_lomit_dlg dlg(this,_("Add New Pattern"),NULL);

	rs_lomit * pNew = NULL;
	int result = dlg.run(&pNew);

	if (result != wxID_OK)
		return;

	m_pRSWorkingCopy->addLOmit(pNew);
	int index = m_pListBoxLOmit->Append( pNew->getSummaryDescription() );

	m_pListBoxLOmit->SetSelection(index);

	_enable_fields();
}

void rs_ruleset_dlg::onButtonEvent_LOmitEdit(wxCommandEvent & /*e*/)
{
	int index = m_pListBoxLOmit->GetSelection();
	if (index == wxNOT_FOUND)	// should not happen because button should not be enabled
		return;

	int limit = m_pRSWorkingCopy->getCountLOmit();
	if (index >= limit)			// should not happen
		return;

	const rs_lomit * pLOmit_Current = m_pRSWorkingCopy->getNthLOmit(index);

	rs_lomit_dlg dlg(this,_("Edit Pattern"),pLOmit_Current);

	rs_lomit * pNew = NULL;
	int result = dlg.run(&pNew);

	if (result != wxID_OK)
		return;

	if (pLOmit_Current->isEqual(pNew))		// if they just clicked OK without
	{										// changing anything, we don't really
		delete pNew;						// need to do anything.
		return;
	}

	m_pRSWorkingCopy->replaceLOmit(index,pNew,true);
	//pLOmit_Current = NULL;
	m_pListBoxLOmit->SetString( index, pNew->getSummaryDescription() );
	m_pListBoxLOmit->SetSelection(index);

	_enable_fields();
}

void rs_ruleset_dlg::onButtonEvent_LOmitDelete(wxCommandEvent & /*e*/)
{
	int index = m_pListBoxLOmit->GetSelection();
	if (index == wxNOT_FOUND)	// should not happen because button should not be enabled
		return;

	int limit = m_pRSWorkingCopy->getCountLOmit();
	if (index >= limit)			// should not happen
		return;

	m_pRSWorkingCopy->deleteLOmit(index);
	m_pListBoxLOmit->Delete(index);

	_enable_fields();
}

//////////////////////////////////////////////////////////////////

void rs_ruleset_dlg::onRadioEvent_EncodingStyle(wxCommandEvent & e)
{
	m_encodingStyle = e.GetInt();

	_enable_fields();
}

//////////////////////////////////////////////////////////////////

void rs_ruleset_dlg::onListBoxSelect_Context(wxCommandEvent & /*e*/)
{
	// current selection has changed -- enable {edit,delete}-context buttons if necessary
	//
	// wxBUG: we do not get called when the user clears the selection, only when making a selection.

	_enable_fields();
}

void rs_ruleset_dlg::onListBoxDClick_Context(wxCommandEvent & e)
{
	onButtonEvent_ContextEdit(e);
}

//////////////////////////////////////////////////////////////////

void rs_ruleset_dlg::onListBoxSelect_LOmit(wxCommandEvent & /*e*/)
{
	// current selection has changed -- enable {edit,delete}-lomit buttons if necessary
	//
	// wxBUG: we do not get called when the user clears the selection, only when making a selection.

	_enable_fields();
}

void rs_ruleset_dlg::onListBoxDClick_LOmit(wxCommandEvent & e)
{
	onButtonEvent_LOmitEdit(e);
}

//////////////////////////////////////////////////////////////////

void rs_ruleset_dlg::onButtonEvent_Help(wxCommandEvent & /*e*/)
{
	long kPage = m_pBookCtrl->GetSelection();
	if (m_bHideNameAndSuffix)
		kPage++;

	switch (kPage)
	{
	default:		// should not happen.  quiets compiler
		return;

	case 0:			// name
		{
			util_help_dlg dlg(this,
							  wxGetTranslation(
								  L"Custom Rulesets allow different types of documents (such as C/C++ source\n"
								  L"files versus Python source files) to be analyzed and displayed differently.\n"
								  L"Rulesets can be automatically selected based upon file suffixes.\n"
								  L"\n"
								  L"Each Ruleset must have a name.  This can be anything you want.  It will\n"
								  L"be displayed in the status bar and in the Choose Ruleset dialog.\n"
								  L"\n"
								  L"Each Ruleset must have a list of one or more suffixes separated by spaces.\n"
								  L"These will be used to match against file suffixes when files are loaded."
								  ));
			dlg.ShowModal();
			return;
		}
		
	case 1:			// char
		{
			util_help_dlg dlg(this,
							  wxGetTranslation(
								  L"All files are stored on disk in a specific CHARACTER ENCODING, such as ISO-8859-1 or\n"
								  L"UTF-8.  As files are loaded for difference analysis, they are first converted into\n"
								  L"UNICODE.  This allows files to be compared independent of their individual character\n"
								  L"encodings.\n"
								  L"\n"
								  L"This page lets you specify the Character Encoding for documents in this Ruleset.\n"
								  L"\n"
								  L"If desired, we can automatically detect the encoding of files that begin with a\n"
								  L"UNICODE Byte Order Mark (BOM).  (This should always be enabled.)\n"
								  L"\n"
								  L"For other files, you can choose a specific encoding here or request to be asked each\n"
								  L"time a document is loaded."
								  ));
			dlg.ShowModal();
			return;
		}

	case 2:			// lomit
		{
			util_help_dlg dlg(this,
							  wxGetTranslation(
								  L"Everything on this page is optional.\n"
								  L"\n"
								  L"This page lets you describe lines that should be omitted from the Difference Analysis.\n"
								  L"You might use this when comparing reports to omit page and column headers and help the\n"
								  L"synchronization match up the data in the report rather than matching up the headers.\n"
								  L"Or you might use it to ignore lines containing version control keywords, such as\n"
								  L"'$Revision: $' or '$Date: $'."
								  ));
			dlg.ShowModal();
			return;
		}

	case 3:			// line handling
		{
			util_help_dlg dlg(this,wxGetTranslation(
								  L"File Window Line Termination Handling:\n"
								  L"\n"
								  L"Different platforms use different Line Termination characters.  Such differences\n"
								  L"can cause two otherwise identical files to look like two completely different files.\n"
								  L"\n"
								  L"When enabled, this option changes all CR, LF, and CRLF characters to a generic EOL\n"
								  L"marker before the Difference Analysis begins; LINE TERMINATION DIFFERENCES\n"
								  L"WILL NOT BE INDICATED IN ANY WAY.\n"
								  L"\n"
								  L"When disabled, the original CR, LF, and CRLF characters are preserved and used in\n"
								  L"the analysis; the highlighting of Line Termination differences is context dependent.\n"
								  L"\n"
								  L"(Unless you have a specific need, this option should always be enabled.)"
								  L"\n"
								  L"\n"
								  L"Line Matching:\n"
								  L"\n"
								  L"These options control the line matching during the Difference Analysis.  By\n"
								  L"eliminating commonly-occurring, minor differences we can often achieve better file\n"
								  L"synchronization.  These options control only the matching; how such changes are\n"
								  L"highlighted is controlled by their context.\n"
								  L"\n"
								  L"(Unless you have a specific need, these options should always be enabled.)"
								  ));
			dlg.ShowModal();
			return;
		}

	case 4:			// ctxt
		{
			util_help_dlg dlg(this,
							  wxGetTranslation(
								  L"Individual differences between documents may be classified as Important or Unimportant.\n"
								  L"Important differences are always highlighted; Unimportant ones may be highlighted or hidden\n"
								  L"using the 'View | Hide Unimportant Differences' menu command.\n"
								  L"\n"
								  L"Matched Versus Default Contexts:\n"
								  L"\n"
								  L"You can use this page to describe portions of the syntax of documents using this Ruleset.\n"
								  L"For example, the format of STRING LITERALS and COMMENTS.  Content found within these\n"
								  L"CONTEXTS can then be classified as IMPORTANT or UNIMPORTANT.  STRING LITERALS should be\n"
								  L"considered Important because they may affect program behavior; COMMENTS can be considered\n"
								  L"Unimportant because they usually don't.\n"
								  L"\n"
								  L"All unmatched content will be classified using the DEFAULT CONTEXT Guidelines.  This will\n"
								  L"cover most of the content of most documents and should therefore be marked Important.\n"
								  L"\n"
								  L"Attributes within Important Contexts:\n"
								  L"\n"
								  L"Within Important Contexts, various commonly-occurring, minor differences can be marked\n"
								  L"Unimportant.  For example, in a C/C++ source file, Whitespace in the Default Context could\n"
								  L"be marked as Unimportant (because indentation changes are not (that) significant) when you\n"
								  L"want focus on actual code changes."
								  ));
			dlg.ShowModal();
			return;
		}

	case 5:			// equivalence
		{
			util_help_dlg dlg(this,wxGetTranslation(
								  L"Settings for use by Folder Windows when Ruleset-based Equivalence Mode is enabled.\n"
								  L"\n"
								  L"When the Equivalence Mode is set to Ruleset-based, Folder Windows will attempt to\n"
								  L"determine if non-identical files are 'equivalent'.  It will use the settings on the\n"
								  L"Character Encoding, Lines to Omit, and Equivalence pages to evaluate the files.\n"
								  L"\n"
								  L"The settings on this page direct DiffMerge to ignore minor differences.  Files that\n"
								  L"only differ in EOL characters, whitespace, and case would be reported as equivalent\n"
								  L"in a Folder Window.\n"
								  L"\n"
								  L"NOTE: The equivalence testing in Folder Windows is a little different from the\n"
								  L"difference analysis performed in File Windows.  The Folder Window version is\n"
								  L"designed to be a fast approximation suitable for quickly scanning an entire source tree.\n"
								  L"The File Window version uses the Line Handling settings to ignore minor differences\n"
								  L"to better match up lines and then the Content Handling settings to classify whitespace\n"
								  L"and case changes as important or unimportant depending on the context.  The Folder\n"
								  L"Window version does not distinguish between changes in comments or string literals\n"
								  L"because it does not use the Content Handling settings.\n"
								  L"\n"
								  L"NOTE: The settings on the Equivalence and Line Handling pages are essentially the\n"
								  L"same.  Having the 2 versions lets you adjust the granularity of the Folder Window\n"
								  L"Equivalence without affecting the File Window display."
								  ));
			dlg.ShowModal();
			return;
		}


	}
}
