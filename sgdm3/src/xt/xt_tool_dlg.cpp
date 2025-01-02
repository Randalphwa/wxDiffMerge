// xt_tool_dlg.cpp
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <xt.h>

//////////////////////////////////////////////////////////////////

#define M 7
#define FIX 0
#define VAR 1
#define X_SIZE_ARGS_TEXT 500

//////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(xt_tool_dlg,wxDialog)
	EVT_CHECKBOX(ID_TOOL_GUI2ENABLE, xt_tool_dlg::onCheckEvent_EnableGui2)
	EVT_CHECKBOX(ID_TOOL_GUI3ENABLE, xt_tool_dlg::onCheckEvent_EnableGui3)
	EVT_BUTTON(ID_SHOW_HELP,         xt_tool_dlg::onButtonEvent_Help)
	EVT_BUTTON(ID_BROWSE_GUI2EXE,    xt_tool_dlg::onButtonEvent_BrowseGui2)
	EVT_BUTTON(ID_BROWSE_GUI3EXE,    xt_tool_dlg::onButtonEvent_BrowseGui3)

	EVT_BUTTON(ID_HELPER_LEFT_PATH,   xt_tool_dlg::onButtonEvent_LeftPath)
	EVT_BUTTON(ID_HELPER_LEFT_TITLE,  xt_tool_dlg::onButtonEvent_LeftTitle)
	EVT_BUTTON(ID_HELPER_RIGHT_PATH,  xt_tool_dlg::onButtonEvent_RightPath)
	EVT_BUTTON(ID_HELPER_RIGHT_TITLE, xt_tool_dlg::onButtonEvent_RightTitle)

	EVT_BUTTON(ID_HELPER_WORKING_PATH,      xt_tool_dlg::onButtonEvent_WorkingPath)
	EVT_BUTTON(ID_HELPER_BASELINE_PATH,     xt_tool_dlg::onButtonEvent_BaselinePath)
	EVT_BUTTON(ID_HELPER_OTHER_PATH,        xt_tool_dlg::onButtonEvent_OtherPath)
	EVT_BUTTON(ID_HELPER_DESTINATION_PATH,  xt_tool_dlg::onButtonEvent_DestinationPath)

	EVT_BUTTON(ID_HELPER_WORKING_TITLE,      xt_tool_dlg::onButtonEvent_WorkingTitle)
	EVT_BUTTON(ID_HELPER_OTHER_TITLE,        xt_tool_dlg::onButtonEvent_OtherTitle)
	EVT_BUTTON(ID_HELPER_DESTINATION_TITLE,  xt_tool_dlg::onButtonEvent_DestinationTitle)

END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////

void xt_tool_dlg::_init(wxWindow * pParent, const wxString & strTitle, bool bFirstPage)
{
	SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
#if defined(__WXMAC__)
	SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif

	Create(pParent,-1, strTitle, wxDefaultPosition, wxDefaultSize,
		   wxCAPTION | wxSYSTEM_MENU | wxRESIZE_BORDER | wxCLOSE_BOX);

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		m_pBookCtrl = new wxTreebook(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxCLIP_CHILDREN);

		// WARNING: if you re-order the pages on the notebook, update onButtonEvent_Help()

		m_pBookCtrl->AddPage( _createPanel_name(), _("Name") );
		m_pBookCtrl->AddPage( _createPanel_gui2(), _("Diff") );
		m_pBookCtrl->AddPage( _createPanel_gui3(), _("Merge") );
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


	_enable_fields();

	if (!bFirstPage)
	{
		// try to open the dialog on the same tab/page as they last saw it.

		long kPage = gpGlobalProps->getLong(GlobalProps::GPL_EXTERNAL_TOOLS_DLG_INITIAL_PAGE);
		long kLimit = (long)m_pBookCtrl->GetPageCount();
		if ( (kPage >= 0) && (kPage < kLimit) )
			m_pBookCtrl->SetSelection(kPage);
	}
}

xt_tool_dlg::~xt_tool_dlg(void)
{
	// regardless of whether OK or CANCEL was pressed, try to remember
	// the current tab/page for next time.
	
	long kPage = m_pBookCtrl->GetSelection();
	gpGlobalProps->setLong(GlobalProps::GPL_EXTERNAL_TOOLS_DLG_INITIAL_PAGE,kPage);

	delete m_pXTWorkingCopy;
}

//////////////////////////////////////////////////////////////////

wxPanel * xt_tool_dlg::_createPanel_name(void)
{
	wxPanel * pPanel = new wxPanel(m_pBookCtrl, ID_PANEL_NAME);

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		wxStaticBoxSizer * staticBoxNameSizer = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1, _("&Name")), wxVERTICAL);
		{
			staticBoxNameSizer->Add( new wxTextCtrl(pPanel,ID_TOOL_NAME,_T(""),
													wxDefaultPosition,wxDefaultSize,0,
													NonBlankTextValidator(&m_strName, _("Error: Tool Name Cannot be Blank"))),
									 FIX, wxGROW|wxALL, M);
		}
		vSizerTop->Add(staticBoxNameSizer, FIX, wxGROW|wxALL, M);
		
		wxStaticBoxSizer * staticBoxSuffixSizer = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1, _("&File Suffixes")), wxVERTICAL);
		{
			staticBoxSuffixSizer->Add( new wxTextCtrl(pPanel,ID_TOOL_SUFFIX,_T(""),
													  wxDefaultPosition,wxDefaultSize,0,
													  NonBlankTextValidator(&m_strSuffixes, _("Error: Suffix List Cannot be Blank"))),
									   FIX, wxGROW|wxALL, M);

			// TODO do we want to add a checkbox for a "also match files without suffixes" feature ??
		}
		vSizerTop->Add(staticBoxSuffixSizer, FIX, wxGROW|wxALL, M);
	}
	pPanel->SetSizer(vSizerTop);
	vSizerTop->Fit(pPanel);

	return pPanel;
}

#define XLBL(_flex_,_string_)												\
	Statement(																\
		(_flex_)->Add( new wxStaticText(pPanel,-1,(_string_)),				\
					   FIX, wxALIGN_CENTER_VERTICAL | wxTOP|wxLEFT, M);		)

#define XBTN(_flex_,_id_,_name_)											\
	Statement(																\
		(_flex_)->Add( new wxButton(pPanel,(_id_),(_name_)),				\
					   FIX, wxTOP|wxLEFT, M);								)

wxPanel * xt_tool_dlg::_createPanel_gui2(void)
{
	wxPanel * pPanel = new wxPanel(m_pBookCtrl, ID_PANEL_GUI2);

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		wxStaticBoxSizer * staticBoxSizerEnable = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1, _("&2-Way File Diffing")), wxVERTICAL);
		{
			staticBoxSizerEnable->Add( new wxCheckBox(pPanel,ID_TOOL_GUI2ENABLE,
													  _("&Enable External Tool for File Diffing"),
													  wxDefaultPosition,wxDefaultSize,0,
													  wxGenericValidator(&m_bEnableGui2)),
									   FIX, wxALL, M);
		}
		vSizerTop->Add(staticBoxSizerEnable, FIX, wxGROW|wxALL, M);

		wxStaticBoxSizer * staticBoxSizerExe = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1,_("&Pathname to External Tool")), wxHORIZONTAL);
		{
			staticBoxSizerExe->Add( new wxTextCtrl(pPanel,ID_TOOL_GUI2EXE,_T(""),
												   wxDefaultPosition,wxDefaultSize,0,
												   ConditionalNonBlankTextValidator(&m_strGui2Exe, _("Error: Diff Executable Pathname Cannot be Blank"))),
									VAR, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, M);

			staticBoxSizerExe->Add( new wxButton(pPanel,ID_BROWSE_GUI2EXE,_("...")),
									FIX, wxALIGN_CENTER_VERTICAL|wxALL, M);
		}
		vSizerTop->Add(staticBoxSizerExe, FIX, wxGROW|wxALL, M);
		
		wxStaticBoxSizer * staticBoxSizerArgs = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1,_("&Command Line Arguments for External Tool")), wxVERTICAL);
		{
			staticBoxSizerArgs->Add( new wxTextCtrl(pPanel,ID_TOOL_GUI2ARGS,_T(""),
													wxDefaultPosition,wxSize(X_SIZE_ARGS_TEXT,-1),0,
													wxTextValidator(wxFILTER_NONE, &m_strGui2Args)),
									 FIX, wxGROW|wxALL, M);

			staticBoxSizerArgs->Add( new wxStaticText(pPanel,-1,_("Insert Argument Placeholder Variables for:")),
									 FIX, wxTOP|wxLEFT|wxRIGHT, M);
			
			wxFlexGridSizer * flexGridSizer = new wxFlexGridSizer(2,3,M,M);
			{
				XLBL(flexGridSizer,_("Pathnames:"));
				XBTN(flexGridSizer,ID_HELPER_LEFT_PATH,_("Left"));
				XBTN(flexGridSizer,ID_HELPER_RIGHT_PATH,_("Right"));

				XLBL(flexGridSizer,_("Labels:"));
				XBTN(flexGridSizer,ID_HELPER_LEFT_TITLE,_("Left"));
				XBTN(flexGridSizer,ID_HELPER_RIGHT_TITLE,_("Right"));
			}
			staticBoxSizerArgs->Add(flexGridSizer, FIX, wxALL, M);

		}
		vSizerTop->Add(staticBoxSizerArgs, FIX, wxGROW|wxALL, M);
	}
	pPanel->SetSizer(vSizerTop);
	vSizerTop->Fit(pPanel);

	return pPanel;
}

wxPanel * xt_tool_dlg::_createPanel_gui3(void)
{
	wxPanel * pPanel = new wxPanel(m_pBookCtrl, ID_PANEL_GUI3);

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		wxStaticBoxSizer * staticBoxSizerEnable = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1, _("&3-Way File Merging")), wxVERTICAL);
		{
			staticBoxSizerEnable->Add( new wxCheckBox(pPanel,ID_TOOL_GUI3ENABLE,
													  _("&Enable External Tool for File Merging"),
													  wxDefaultPosition,wxDefaultSize,0,
													  wxGenericValidator(&m_bEnableGui3)),
									   FIX, wxALL, M);
		}
		vSizerTop->Add(staticBoxSizerEnable, FIX, wxGROW|wxALL, M);

		wxStaticBoxSizer * staticBoxSizerExe = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1,_("&Pathname to External Tool")), wxHORIZONTAL);
		{
			staticBoxSizerExe->Add( new wxTextCtrl(pPanel,ID_TOOL_GUI3EXE,_T(""),
												   wxDefaultPosition,wxDefaultSize,0,
												   ConditionalNonBlankTextValidator(&m_strGui3Exe, _("Error: Merge Executable Pathname Cannot be Blank"))),
									VAR, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, M);

			staticBoxSizerExe->Add( new wxButton(pPanel,ID_BROWSE_GUI3EXE,_("...")),
									FIX, wxALIGN_CENTER_VERTICAL|wxALL, M);
		}
		vSizerTop->Add(staticBoxSizerExe, FIX, wxGROW|wxALL, M);
		
		wxStaticBoxSizer * staticBoxSizerArgs = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1,_("&Command Line Arguments for External Tool")), wxVERTICAL);
		{
			staticBoxSizerArgs->Add( new wxTextCtrl(pPanel,ID_TOOL_GUI3ARGS,_T(""),
													wxDefaultPosition,wxSize(X_SIZE_ARGS_TEXT,-1),0,
													wxTextValidator(wxFILTER_NONE, &m_strGui3Args)),
									 FIX, wxGROW|wxALL, M);

			staticBoxSizerArgs->Add( new wxStaticText(pPanel,-1,_("Insert Argument Placeholder Variables for:")),
									 FIX, wxTOP|wxLEFT|wxRIGHT, M);
			
			wxFlexGridSizer * flexGridSizer = new wxFlexGridSizer(3,4,M,M);
			{
				XLBL(flexGridSizer,_("Pathnames:"));
				XBTN(flexGridSizer,ID_HELPER_WORKING_PATH,_("Working"));
				XBTN(flexGridSizer,ID_HELPER_BASELINE_PATH,_("Baseline"));
				XBTN(flexGridSizer,ID_HELPER_OTHER_PATH,_("Other"));

				XLBL(flexGridSizer,_T(""));
				XLBL(flexGridSizer,_T(""));
				XBTN(flexGridSizer,ID_HELPER_DESTINATION_PATH,_("Destination"));
				XLBL(flexGridSizer,_T(""));

				XLBL(flexGridSizer,_("Labels:"));
				XBTN(flexGridSizer,ID_HELPER_WORKING_TITLE,_("Working"));
				XBTN(flexGridSizer,ID_HELPER_DESTINATION_TITLE,_("Destination"));
				XBTN(flexGridSizer,ID_HELPER_OTHER_TITLE,_("Other"));
			}
			staticBoxSizerArgs->Add(flexGridSizer, FIX, wxALL, M);

		}
		vSizerTop->Add(staticBoxSizerArgs, FIX, wxGROW|wxALL, M);
	}
	pPanel->SetSizer(vSizerTop);
	vSizerTop->Fit(pPanel);

	return pPanel;
}

//////////////////////////////////////////////////////////////////

int xt_tool_dlg::run(xt_tool ** ppXT_Result)
{
	int result = ShowModal();

	if (result != wxID_OK)
	{
		if (ppXT_Result)
			*ppXT_Result = NULL;
		return wxID_CANCEL;
	}

	if (ppXT_Result)
	{
		m_pXTWorkingCopy->setName(m_strName);
		m_pXTWorkingCopy->setSuffixes(m_strSuffixes);
		m_pXTWorkingCopy->setEnabled2(m_bEnableGui2);
		m_pXTWorkingCopy->setGui2Exe(m_strGui2Exe);
		m_pXTWorkingCopy->setGui2Args(m_strGui2Args);
		m_pXTWorkingCopy->setEnabled3(m_bEnableGui3);
		m_pXTWorkingCopy->setGui3Exe(m_strGui3Exe);
		m_pXTWorkingCopy->setGui3Args(m_strGui3Args);

#ifdef DEBUG
		m_pXTWorkingCopy->dump(10);
#endif
		*ppXT_Result = m_pXTWorkingCopy;
		m_pXTWorkingCopy = NULL;
	}

	return wxID_OK;
}

//////////////////////////////////////////////////////////////////

void xt_tool_dlg::_preload_fields(const xt_tool * pxt)
{
	m_strName = pxt->getName();
	m_strSuffixes = pxt->getSuffixes();
	m_bEnableGui2 = pxt->getEnabled2();
	m_strGui2Exe = pxt->getGui2Exe();
	m_strGui2Args = pxt->getGui2Args();
	m_bEnableGui3 = pxt->getEnabled3();
	m_strGui3Exe = pxt->getGui3Exe();
	m_strGui3Args = pxt->getGui3Args();
}

//////////////////////////////////////////////////////////////////

xt_tool_dlg__add::xt_tool_dlg__add(wxWindow * pParent)
{
	m_pXTWorkingCopy = new xt_tool();
	_preload_fields(m_pXTWorkingCopy);
	_init(pParent,_("New External Tool"), true);
}

xt_tool_dlg__edit::xt_tool_dlg__edit(wxWindow * pParent, const xt_tool * pXT_Source)
{
	m_pXTWorkingCopy = new xt_tool(*pXT_Source);
	_preload_fields(m_pXTWorkingCopy);

	wxString strTitle = wxString::Format( _("Edit External Tool: %s"), pXT_Source->getName().wc_str() );
	_init(pParent, strTitle, false);
}

//////////////////////////////////////////////////////////////////

void xt_tool_dlg::_enable_fields(void)
{
	FindWindow(ID_TOOL_GUI2EXE       )->Enable(m_bEnableGui2);
	FindWindow(ID_BROWSE_GUI2EXE     )->Enable(m_bEnableGui2);
	FindWindow(ID_TOOL_GUI2ARGS      )->Enable(m_bEnableGui2);
	FindWindow(ID_HELPER_LEFT_PATH   )->Enable(m_bEnableGui2);
	FindWindow(ID_HELPER_RIGHT_PATH  )->Enable(m_bEnableGui2);
	FindWindow(ID_HELPER_LEFT_TITLE  )->Enable(m_bEnableGui2);
	FindWindow(ID_HELPER_RIGHT_TITLE )->Enable(m_bEnableGui2);

	FindWindow(ID_TOOL_GUI3EXE             )->Enable(m_bEnableGui3);
	FindWindow(ID_BROWSE_GUI3EXE           )->Enable(m_bEnableGui3);
	FindWindow(ID_TOOL_GUI3ARGS            )->Enable(m_bEnableGui3);
	FindWindow(ID_HELPER_WORKING_PATH      )->Enable(m_bEnableGui3);
	FindWindow(ID_HELPER_BASELINE_PATH     )->Enable(m_bEnableGui3);
	FindWindow(ID_HELPER_OTHER_PATH        )->Enable(m_bEnableGui3);
	FindWindow(ID_HELPER_DESTINATION_PATH  )->Enable(m_bEnableGui3);
	FindWindow(ID_HELPER_WORKING_TITLE     )->Enable(m_bEnableGui3);
	FindWindow(ID_HELPER_OTHER_TITLE       )->Enable(m_bEnableGui3);
	FindWindow(ID_HELPER_DESTINATION_TITLE )->Enable(m_bEnableGui3);
}

//////////////////////////////////////////////////////////////////

void xt_tool_dlg::onCheckEvent_EnableGui2(wxCommandEvent & e)
{
	m_bEnableGui2 = e.IsChecked();
	_enable_fields();
}

void xt_tool_dlg::onCheckEvent_EnableGui3(wxCommandEvent & e)
{
	m_bEnableGui3 = e.IsChecked();
	_enable_fields();
}

//////////////////////////////////////////////////////////////////

void xt_tool_dlg::onButtonEvent_Help(wxCommandEvent & /*e*/)
{
	long kPage = m_pBookCtrl->GetSelection();

	wxString strMsg;

	switch (kPage)
	{
	default:		// should not happen.  quiets compiler
		return;

	case 0:			// name
		strMsg = wxGetTranslation(
			L"External Tools allow different types of documents to be handled by an\n"
			L"external (third-party) application instead of using the normal DiffMerge\n"
			L"File Diff or File Merge window.\n"
			L"\n"
			L"External Tools can be automatically selected based upon file suffixes.\n"
			L"\n"
			L"Each Tool must have a name.  This can be anything you want.  It will be\n"
			L"displayed in Options dialog and in various Message dialogs.\n"
			L"\n"
			L"Each Tool must have a list of one or more suffixes separated by spaces.\n"
			L"These will be used to match against file suffixes when files are loaded."
			);
		break;

	case 1:			// 2-way diffs
		strMsg = wxGetTranslation(
			L"This page allows you to select the application executable and command line\n"
			L"arguments to be used for 2-way file diffs.\n"
			L"\n"
			L"The command line argument field should list the arguments expected by the\n"
			L"external tool.  Since different applications take arguments in various\n"
			L"orders and with various switches, please refer to the manual for your tool.\n"
			L"\n"
			L"The command line argument field may contain placeholder tokens to represent\n"
			L"the individual files.  These tokens will be replaced using values given to\n"
			L"DiffMerge on the command line or with values available when opening a new\n"
			L"window.\n"
			L"\n"
			L"IT IS HIGHLY RECOMMENDED THAT YOU PLACE QUOTES AROUND EACH TOKEN,\n"
			L"so that whitespace within pathnames and titles are properly handled by the\n"
			L"external tool.\n"
			L"\n"
			L"%LEFT_PATH%, %RIGHT_PATH% -- the left and right file pathnames.\n"
			L"%LEFT_LABEL%, %RIGHT_LABEL% -- optional titles for the left and right files.\n"
			L"        When titles are not available, the file pathnames will be used.\n"
			L"\n"
			L"For example, to configure DiffMerge to spawn itself (not a good idea), you\n"
			L"could set the argument field to:\n"
			L"\n"
			L"        /t1:\"%LEFT_LABEL%\"   /t2:\"%RIGHT_LABEL%\"\n"
			L"        \"%LEFT_PATH%\"   \"%RIGHT_PATH%\""
			);
		break;

	case 2:			// 3-way merges
		strMsg = wxGetTranslation(
			L"This page allows you to select the application executable and command line\n"
			L"arguments to be used for 3-way file merges.\n"
			L"\n"
			L"The command line argument field should list the arguments expected by the\n"
			L"external tool.  Since different applications take arguments in various\n"
			L"orders and with various switches, please refer to the manual for your tool.\n"
			L"\n"
			L"The command line argument field may contain placeholder tokens to represent\n"
			L"the individual files.  These tokens will be replaced using values given to\n"
			L"DiffMerge on the command line or with values available when opening a new\n"
			L"window.\n"
			L"\n"
			L"IT IS HIGHLY RECOMMENDED THAT YOU PLACE QUOTES AROUND EACH TOKEN,\n"
			L"so that whitespace within pathnames and titles are properly handled by the\n"
			L"external tool.\n"
			L"\n"
			L"%WORKING_PATH%, %WORKING_LABEL% -- the pathname and title for the\n"
			L"        WORKING version of the file.  In DiffMerge, this is displayed in the left\n"
			L"        panel.  In Vault, this file represents your working version.\n"
			L"%OTHER_PATH%, %OTHER_LABEL% -- the pathname and title for the OTHER\n"
			L"        version of the file.  In DiffMerge, this is displayed in the right panel.\n"
			L"        In Vault, this is usually the current repository version.\n"
			L"%BASELINE_PATH% -- the pathname of the COMMON ANCESTOR or baseline\n"
			L"        between WORKING and OTHER versions.  In DiffMerge, this is displayed in the\n"
			L"        center panel and forms the basis for the merge; changes from the other\n"
			L"        two versions are merged into this one.\n"
			L"%DEST_PATH% -- an optional DESTINATION pathname for saving the merge result\n"
			L"        (instead of overwriting the BASELINE version).\n"
			L"%DEST_LABEL% -- title for the baseline/destination panel.  In DiffMerge, this\n"
			L"        is displayed above the center panel.\n"
			L"\n"
			L"When titles are not available, the corresponding file pathnames will be used.\n"
			L"\n"
			L"For example, to configure DiffMerge to spawn itself (not a good idea), you\n"
			L"could set the argument field to:\n"
			L"\n"
			L"    /t1:\"%WORKING_LABEL%\"   /t2:\"%DEST_LABEL%\"   /t3:\"%OTHER_LABEL%\"\n"
			L"    /result:\"%DEST_PATH%\"\n"
			L"    \"%WORKING_PATH%\"   \"%BASELINE_PATH%\"   \"%OTHER_PATH%\""
			);
		break;
	}

	util_help_dlg dlg(this, strMsg);
	dlg.ShowModal();
	return;
}

//////////////////////////////////////////////////////////////////

void xt_tool_dlg::_handle_browse_button(int id, const wxString & strTitle, wxString * pStrExe)
{
	// NOTE: we don't use Validate()/TransferDataFromWindow() because
	// NOTE: that causes the field validation (and error dialog) for
	// NOTE: all fields on the dialog (like the name and suffix fields)
	// NOTE: which is a little disconcerting when you're only hitting
	// NOTE: the browse button rather than the main OK button.
	//	if (Validate())					// populate fields from
	//		TransferDataFromWindow();	// dialog controls.
	// all we really wanted was the current content of the exe text ctrl.
	wxTextCtrl * pTextCtrl = (wxTextCtrl *)FindWindow(id);
	*pStrExe = pTextCtrl->GetValue();
	
	wxFileName fn(*pStrExe);
	wxString strDir = fn.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR);
	wxString strFile = fn.GetFullName();

	// the wildcard type-list is a little tricky for executables.
	// it's one thing to look for *.exe or *.bat on windows, but
	// on linux stuff in /usr/bin/ doesn't have a suffix.
	// TODO i'm not sure what we need for the mac -- to get to the
	// TODO actual executable inside a .app directory thingy.
#if defined(__WXMAC__)
	wxString strTypeList = _T("*");
#endif
#if defined(__WXGTK__)
	wxString strTypeList = _T("*");
#endif
#if defined(__WXMSW__)
	wxString strTypeList = _T("*.*");
#endif

	wxFileDialog dlg(this,strTitle,strDir,strFile,strTypeList,wxFD_OPEN|wxFD_FILE_MUST_EXIST);
	if (dlg.ShowModal() != wxID_OK)
		return;

	*pStrExe = dlg.GetPath();
	// NOTE: we don't use TransferDataToWindow() because we didn't
	// NOTE: use Validate()/TransferDataFromWindow() and the other
	// NOTE: string fields haven't been updated.
	// TransferDataToWindow();
	pTextCtrl->ChangeValue(*pStrExe);
	_enable_fields();
}
	
void xt_tool_dlg::onButtonEvent_BrowseGui2(wxCommandEvent & /*e*/)
{
	_handle_browse_button(ID_TOOL_GUI2EXE, _T("Select Diff Application"), &m_strGui2Exe);
}
	
void xt_tool_dlg::onButtonEvent_BrowseGui3(wxCommandEvent & /*e*/)
{
	_handle_browse_button(ID_TOOL_GUI3EXE, _T("Select Merge Application"), &m_strGui3Exe);
}

//////////////////////////////////////////////////////////////////

void xt_tool_dlg::_handle_helper_insert(int id, const wxString & str)
{
	// insert the given string at the current insertion point (caret)
	// in the text ctrl with the given id.

	wxTextCtrl * pTextCtrl = (wxTextCtrl *)FindWindow(id);
	long selection_start, selection_end;
	pTextCtrl->GetSelection(&selection_start,&selection_end);
	if (selection_start != selection_end)
	{
		pTextCtrl->Remove(selection_start,selection_end);
		pTextCtrl->SetInsertionPoint(selection_start);
	}
	pTextCtrl->WriteText(str);
}
	
void xt_tool_dlg::onButtonEvent_LeftPath(wxCommandEvent & /*e*/)
{
	_handle_helper_insert(ID_TOOL_GUI2ARGS,_T("\"%LEFT_PATH%\" "));
}

void xt_tool_dlg::onButtonEvent_LeftTitle(wxCommandEvent & /*e*/)
{
	_handle_helper_insert(ID_TOOL_GUI2ARGS,_T("\"%LEFT_LABEL%\" "));
}

void xt_tool_dlg::onButtonEvent_RightPath(wxCommandEvent & /*e*/)
{
	_handle_helper_insert(ID_TOOL_GUI2ARGS,_T("\"%RIGHT_PATH%\" "));
}

void xt_tool_dlg::onButtonEvent_RightTitle(wxCommandEvent & /*e*/)
{
	_handle_helper_insert(ID_TOOL_GUI2ARGS,_T("\"%RIGHT_LABEL%\" "));
}

void xt_tool_dlg::onButtonEvent_WorkingPath(wxCommandEvent & /*e*/)
{
	_handle_helper_insert(ID_TOOL_GUI3ARGS,_T("\"%WORKING_PATH%\" "));
}

void xt_tool_dlg::onButtonEvent_BaselinePath(wxCommandEvent & /*e*/)
{
	_handle_helper_insert(ID_TOOL_GUI3ARGS,_T("\"%BASELINE_PATH%\" "));
}

void xt_tool_dlg::onButtonEvent_OtherPath(wxCommandEvent & /*e*/)
{
	_handle_helper_insert(ID_TOOL_GUI3ARGS,_T("\"%OTHER_PATH%\" "));
}

void xt_tool_dlg::onButtonEvent_DestinationPath(wxCommandEvent & /*e*/)
{
	_handle_helper_insert(ID_TOOL_GUI3ARGS,_T("\"%DEST_PATH%\" "));
}

void xt_tool_dlg::onButtonEvent_WorkingTitle(wxCommandEvent & /*e*/)
{
	_handle_helper_insert(ID_TOOL_GUI3ARGS,_T("\"%WORKING_LABEL%\" "));
}

void xt_tool_dlg::onButtonEvent_OtherTitle(wxCommandEvent & /*e*/)
{
	_handle_helper_insert(ID_TOOL_GUI3ARGS,_T("\"%OTHER_LABEL%\" "));
}

void xt_tool_dlg::onButtonEvent_DestinationTitle(wxCommandEvent & /*e*/)
{
	_handle_helper_insert(ID_TOOL_GUI3ARGS,_T("\"%DEST_LABEL%\" "));
}
