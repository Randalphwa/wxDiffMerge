// OptionsDlg.cpp
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fs.h>
#include <fd.h>
#include <rs.h>
#include <xt.h>
#include <gui.h>

//////////////////////////////////////////////////////////////////

#define OPTIONS_DLG_TITLE	_("DiffMerge Options")
#define M 7
#define FIX 0
#define VAR 1

//////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(OptionsDlg,wxDialog)
	EVT_BUTTON(ID_FOLDER_WINDOWS__RESTORE_DEFAULTS,	OptionsDlg::onButtonEvent_RestoreDefaults_folder_windows)
	EVT_BUTTON(ID_FOLDER_FILTERS__RESTORE_DEFAULTS,	OptionsDlg::onButtonEvent_RestoreDefaults_folder_filters)
	EVT_BUTTON(ID_FOLDER_COLORS__RESTORE_DEFAULTS,	OptionsDlg::onButtonEvent_RestoreDefaults_folder_colors)
	EVT_BUTTON(ID_FOLDER_SOFTMATCH__RESTORE_DEFAULTS,	OptionsDlg::onButtonEvent_RestoreDefaults_folder_softmatch)

	EVT_BUTTON(ID_FILE_WINDOWS__RESTORE_DEFAULTS,	OptionsDlg::onButtonEvent_RestoreDefaults_file_windows)
	EVT_BUTTON(ID_FILE_LINE_COLORS__RESTORE_DEFAULTS,	OptionsDlg::onButtonEvent_RestoreDefaults_file_line_colors)
	EVT_BUTTON(ID_FILE_IL_COLORS__RESTORE_DEFAULTS,		OptionsDlg::onButtonEvent_RestoreDefaults_file_il_colors)
	EVT_BUTTON(ID_FILE_OTHER_COLORS__RESTORE_DEFAULTS,	OptionsDlg::onButtonEvent_RestoreDefaults_file_other_colors)
	EVT_BUTTON(ID_FILE_RULESETS__RESTORE_DEFAULTS,	OptionsDlg::onButtonEvent_RestoreDefaults_file_rulesets)
	EVT_BUTTON(ID_MISC__RESTORE_DEFAULTS,			OptionsDlg::onButtonEvent_RestoreDefaults_misc)
	EVT_BUTTON(ID_MESG__RESTORE_DEFAULTS,			OptionsDlg::onButtonEvent_RestoreDefaults_messages)
#ifdef FEATURE_SHEX
	EVT_BUTTON(ID_SHEX__HELP,						OptionsDlg::onButtonEvent_Shex_Help)
	EVT_BUTTON(ID_SHEX__RESTORE_DEFAULTS,			OptionsDlg::onButtonEvent_RestoreDefaults_shex)
#endif
	EVT_BUTTON(ID_XT__RESTORE_DEFAULTS,				OptionsDlg::onButtonEvent_RestoreDefaults_ExternalTools)

	EVT_BUTTON(ID_FOLDER_COLORS__DIFFERENT_FG,  OptionsDlg::onButtonEvent_folder_color_different_fg)
	EVT_BUTTON(ID_FOLDER_COLORS__DIFFERENT_BG,  OptionsDlg::onButtonEvent_folder_color_different_bg)
	EVT_BUTTON(ID_FOLDER_COLORS__EQUAL_FG,      OptionsDlg::onButtonEvent_folder_color_equal_fg)
	EVT_BUTTON(ID_FOLDER_COLORS__EQUAL_BG,      OptionsDlg::onButtonEvent_folder_color_equal_bg)
	EVT_BUTTON(ID_FOLDER_COLORS__EQUIVALENT_FG, OptionsDlg::onButtonEvent_folder_color_equivalent_fg)
	EVT_BUTTON(ID_FOLDER_COLORS__EQUIVALENT_BG, OptionsDlg::onButtonEvent_folder_color_equivalent_bg)
	EVT_BUTTON(ID_FOLDER_COLORS__FOLDERS_FG,    OptionsDlg::onButtonEvent_folder_color_folders_fg)
	EVT_BUTTON(ID_FOLDER_COLORS__FOLDERS_BG,    OptionsDlg::onButtonEvent_folder_color_folders_bg)
	EVT_BUTTON(ID_FOLDER_COLORS__PEERLESS_FG,   OptionsDlg::onButtonEvent_folder_color_peerless_fg)
	EVT_BUTTON(ID_FOLDER_COLORS__PEERLESS_BG,   OptionsDlg::onButtonEvent_folder_color_peerless_bg)
	EVT_BUTTON(ID_FOLDER_COLORS__ERROR_FG,      OptionsDlg::onButtonEvent_folder_color_error_fg)
	EVT_BUTTON(ID_FOLDER_COLORS__ERROR_BG,      OptionsDlg::onButtonEvent_folder_color_error_bg)

	// colors on "line colors" tab

	EVT_BUTTON(ID_FILE_COLORS__ALL_EQ_FG,			OptionsDlg::onButtonEvent_file_color_all_eq_fg)
	EVT_BUTTON(ID_FILE_COLORS__ALL_EQ_BG,			OptionsDlg::onButtonEvent_file_color_all_eq_bg)
	EVT_BUTTON(ID_FILE_COLORS__NONE_EQ_FG,			OptionsDlg::onButtonEvent_file_color_none_eq_fg)
	EVT_BUTTON(ID_FILE_COLORS__NONE_EQ_BG,			OptionsDlg::onButtonEvent_file_color_none_eq_bg)
	EVT_BUTTON(ID_FILE_COLORS__SUB_EQUAL_FG,		OptionsDlg::onButtonEvent_file_color_sub_equal_fg)
	EVT_BUTTON(ID_FILE_COLORS__SUB_EQUAL_BG,		OptionsDlg::onButtonEvent_file_color_sub_equal_bg)
	EVT_BUTTON(ID_FILE_COLORS__SUB_NOTEQUAL_FG,		OptionsDlg::onButtonEvent_file_color_sub_notequal_fg)
	EVT_BUTTON(ID_FILE_COLORS__SUB_NOTEQUAL_BG,		OptionsDlg::onButtonEvent_file_color_sub_notequal_bg)
	EVT_BUTTON(ID_FILE_COLORS__CONFLICT_FG,			OptionsDlg::onButtonEvent_file_color_conflict_fg)
	EVT_BUTTON(ID_FILE_COLORS__CONFLICT_BG,			OptionsDlg::onButtonEvent_file_color_conflict_bg)

	// colors on "intra-line colors" tab

	EVT_BUTTON(ID_FILE_COLORS__ALL_EQ_UNIMP_FG,		OptionsDlg::onButtonEvent_file_color_all_eq_unimp_fg)
	EVT_BUTTON(ID_FILE_COLORS__NONE_EQ_UNIMP_FG,	OptionsDlg::onButtonEvent_file_color_none_eq_unimp_fg)
	EVT_BUTTON(ID_FILE_COLORS__SUB_UNIMP_FG,		OptionsDlg::onButtonEvent_file_color_sub_unimp_fg)
	EVT_BUTTON(ID_FILE_COLORS__CONFLICT_UNIMP_FG,	OptionsDlg::onButtonEvent_file_color_conflict_unimp_fg)
	EVT_BUTTON(ID_FILE_COLORS__NONE_EQ_IL_BG,		OptionsDlg::onButtonEvent_file_color_none_eq_il_bg)
	EVT_BUTTON(ID_FILE_COLORS__CONFLICT_IL_BG,		OptionsDlg::onButtonEvent_file_color_conflict_il_bg)
	EVT_BUTTON(ID_FILE_COLORS__SUB_EQUAL_IL_BG,		OptionsDlg::onButtonEvent_file_color_sub_equal_il_bg)
	EVT_BUTTON(ID_FILE_COLORS__SUB_NOTEQUAL_IL_BG,	OptionsDlg::onButtonEvent_file_color_sub_notequal_il_bg)

	// colors on "other colors" tab

	EVT_BUTTON(ID_FILE_COLORS__WINDOW_BG,			OptionsDlg::onButtonEvent_file_color_window_bg)
	EVT_BUTTON(ID_FILE_COLORS__OMIT_FG,				OptionsDlg::onButtonEvent_file_color_omit_fg)
	EVT_BUTTON(ID_FILE_COLORS__OMIT_BG,				OptionsDlg::onButtonEvent_file_color_omit_bg)
	EVT_BUTTON(ID_FILE_COLORS__EOL_UNKNOWN_FG,		OptionsDlg::onButtonEvent_file_color_eol_unknown_fg)
	EVT_BUTTON(ID_FILE_COLORS__VOID_FG,				OptionsDlg::onButtonEvent_file_color_void_fg)
	EVT_BUTTON(ID_FILE_COLORS__VOID_BG,				OptionsDlg::onButtonEvent_file_color_void_bg)
	EVT_BUTTON(ID_FILE_COLORS__LINENR_FG,			OptionsDlg::onButtonEvent_file_color_linenr_fg)
	EVT_BUTTON(ID_FILE_COLORS__LINENR_BG,			OptionsDlg::onButtonEvent_file_color_linenr_bg)
	EVT_BUTTON(ID_FILE_COLORS__CARET_FG,			OptionsDlg::onButtonEvent_file_color_caret_fg)
	EVT_BUTTON(ID_FILE_COLORS__SELECTION_FG,		OptionsDlg::onButtonEvent_file_color_selection_fg)
	EVT_BUTTON(ID_FILE_COLORS__SELECTION_BG,		OptionsDlg::onButtonEvent_file_color_selection_bg)

	EVT_CHECKBOX(ID_FILE_RULESETS__ENABLE_CUSTOM_RULESETS,	OptionsDlg::onCheckEvent_enable_custom_rulesets)
	EVT_CHECKBOX(ID_FILE_RULESETS__ENABLE_AUTOMATIC_MATCH,	OptionsDlg::onCheckEvent_enable_automatic_match)
	EVT_LISTBOX(ID_FILE_RULESETS__LISTBOX,					OptionsDlg::onListBoxEvent_listbox)
	EVT_LISTBOX_DCLICK(ID_FILE_RULESETS__LISTBOX,			OptionsDlg::onListBoxDClickEvent_listbox)
	EVT_BUTTON(ID_FILE_RULESETS__RULESET_ADD,				OptionsDlg::onButtonEvent_ruleset_add)
	EVT_BUTTON(ID_FILE_RULESETS__RULESET_EDIT,				OptionsDlg::onButtonEvent_ruleset_edit)
	EVT_BUTTON(ID_FILE_RULESETS__RULESET_DELETE,			OptionsDlg::onButtonEvent_ruleset_delete)
	EVT_BUTTON(ID_FILE_RULESETS__RULESET_CLONE,				OptionsDlg::onButtonEvent_ruleset_clone)
	EVT_BUTTON(ID_FILE_RULESETS__RULESET_MOVEUP,			OptionsDlg::onButtonEvent_ruleset_moveup)
	EVT_BUTTON(ID_FILE_RULESETS__RULESET_MOVEDOWN,			OptionsDlg::onButtonEvent_ruleset_movedown)
	EVT_BUTTON(ID_FILE_RULESETS__EDIT_DEFAULT_RULESET,		OptionsDlg::onButtonEvent_ruleset_edit_default_ruleset)

	EVT_BUTTON(ID_FONTS__EDIT_FILE_FONT,			OptionsDlg::onButtonEvent_fonts_file)
#if VIEW_FOLDER_LIST_CTRL_USE_FONTS
	EVT_BUTTON(ID_FONTS__EDIT_FOLDER_FONT,			OptionsDlg::onButtonEvent_fonts_folder)
#endif
	EVT_BUTTON(ID_FONTS__EDIT_PRINTER_FILE_FONT,	OptionsDlg::onButtonEvent_fonts_printer_file)
	EVT_BUTTON(ID_FONTS__EDIT_PRINTER_FOLDER_FONT,	OptionsDlg::onButtonEvent_fonts_printer_folder)

	EVT_CHECKBOX(ID_MISC__ENABLE_AUTOSAVE,			OptionsDlg::onCheckEvent_enable_autosave)
	EVT_RADIOBOX(ID_MISC__RADIO_DETAIL_LEVEL,		OptionsDlg::onRadioEvent_DetailLevel)

	EVT_CHECKBOX(ID_FOLDER_FILTERS__USE_FILE_FILTER,	OptionsDlg::onCheckEvent_use_file_filters)
	EVT_CHECKBOX(ID_FOLDER_FILTERS__USE_FOLDER_FILTER,	OptionsDlg::onCheckEvent_use_folder_filters)
	EVT_CHECKBOX(ID_FOLDER_FILTERS__USE_FULL_FILENAME_FILTER,	OptionsDlg::onCheckEvent_use_full_filename_filters)

	EVT_CHECKBOX(ID_FOLDER_SOFTMATCH__SIMPLE_IGNORE_WHITESPACE, OptionsDlg::onCheckEvent_SoftMatch_SimpleIgnoreWhitespace)
	EVT_RADIOBOX(ID_FOLDER_SOFTMATCH__RADIO_MODE, OptionsDlg::onRadioEvent_SoftMatch_Mode)
	EVT_BUTTON(ID_FOLDER_SOFTMATCH__HELP, OptionsDlg::onButtonEvent_SoftMatch_Help)

//	EVT_CHECKBOX(ID_FOLDER_QUICKMATCH__ENABLE, OptionsDlg::onCheckEvent_QuickMatch_Enable)

	EVT_CHECKBOX(ID_XT__ENABLE_EXTERNAL_TOOLS,		OptionsDlg::onCheckEvent_enable_external_tools)
	EVT_LISTBOX(ID_XT__LISTBOX,						OptionsDlg::onListBoxEvent_listbox_external_tools)
	EVT_LISTBOX_DCLICK(ID_XT__LISTBOX,				OptionsDlg::onListBoxDClickEvent_listbox_external_tools)
	EVT_BUTTON(ID_XT__ADD,							OptionsDlg::onButtonEvent_external_tools_add)
	EVT_BUTTON(ID_XT__EDIT,							OptionsDlg::onButtonEvent_external_tools_edit)
	EVT_BUTTON(ID_XT__DELETE,						OptionsDlg::onButtonEvent_external_tools_delete)
	EVT_BUTTON(ID_XT__CLONE,						OptionsDlg::onButtonEvent_external_tools_clone)
	EVT_BUTTON(ID_XT__MOVEUP,						OptionsDlg::onButtonEvent_external_tools_moveup)
	EVT_BUTTON(ID_XT__MOVEDOWN,						OptionsDlg::onButtonEvent_external_tools_movedown)

	EVT_TEXT(ID_FOLDER_FILTERS__FILE_FILTER, OptionsDlg::onTextEventChanged__FolderFilters__FileFilter)
	EVT_TEXT(ID_FOLDER_FILTERS__FULL_FILENAME_FILTER, OptionsDlg::onTextEventChanged__FolderFilters__FullFilenameFilter)
	EVT_TEXT(ID_FOLDER_FILTERS__FOLDER_FILTER, OptionsDlg::onTextEventChanged__FolderFilters__FolderFilter)

	EVT_BUTTON(wxID_OK, OptionsDlg::OnOK)

END_EVENT_TABLE()

BEGIN_EVENT_TABLE(OptionsDlg::_preview, wxWindow)
#if defined(__WXMAC__)
	EVT_PAINT(OptionsDlg::_preview::onPaintEvent)
#endif
	EVT_SIZE (OptionsDlg::_preview::onSizeEvent)
END_EVENT_TABLE();

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

OptionsDlg::OptionsDlg(wxFrame * pFrame)
	: m_bFontFileDirty(false),		m_pFont_File(NULL),
#if VIEW_FOLDER_LIST_CTRL_USE_FONTS
	  m_bFontFolderDirty(false),    m_pFont_Folder(NULL),
#endif
	  m_bFontPrinterFileDirty(false),	m_pFont_PrinterFile(NULL),
	  m_bFontPrinterFolderDirty(false),	m_pFont_PrinterFolder(NULL),
	  m_pRSTWorkingCopy(NULL),
	  m_pXTTWorkingCopy(NULL),
	  m_pPreviewFileLineColor2(NULL), m_pPreviewFileLineColor3(NULL),
	  m_pPreviewFileILColor2(NULL), m_pPreviewFileILColor3(NULL)
{
	SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
#if defined(__WXMAC__)
	SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif

	Create(pFrame, -1, OPTIONS_DLG_TITLE, wxDefaultPosition, wxDefaultSize,
		   wxCAPTION | wxSYSTEM_MENU | wxRESIZE_BORDER | wxCLOSE_BOX);

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		// wxTreebook is new in 2.8.0 -- we want to use it rather than my custom util_treebook
		// because theirs supports sub-pages -- an i want to get out of the business of
		// maintaining widgets.
		// 
		// TODO we can remove this #ifdef once we get the build system updated to 2.8 on
		// TODO all platforms.  Currently, 2.8.0 is unstable on MAC, so don't be too quick
		// TODO to get rid of this.

#if wxCHECK_VERSION(2,8,0)

		// WARNING: in 2.8.0 wxTreebook has an infinite recursion problem (causing stack overflow)
		// WARNING: when the user uses Ctrl-Tab to cycle pages.  This appears to have been fixed in
		// WARNING: src/common/containr.cpp between version 1.50 and 1.51.  so hopefully they'll
		// WARNING: a 2.8.1 before we ship.

		int kPage=0, kPageFile, kPageFolder, kPageMessages;
		
		m_pBookCtrl = new wxTreebook(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxCLIP_CHILDREN);

		m_pBookCtrl->AddPage( createPanel_file_windows(),  			_("File Windows") );			kPageFile = kPage++;
		m_pBookCtrl->AddSubPage( createPanel_file_rulesets(),			_("Rulesets") );			kPage++;
		m_pBookCtrl->AddSubPage( createPanel_misc(),           			_("Detail Level") );		kPage++;
		m_pBookCtrl->AddSubPage( createPanel_file_line_colors(),		_("Line Colors") );			kPage++;
		m_pBookCtrl->AddSubPage( createPanel_file_il_colors(),			_("Intra-Line Colors") );	kPage++;
		m_pBookCtrl->AddSubPage( createPanel_file_other_colors(),		_("Other Colors") );		kPage++;

		m_pBookCtrl->AddPage( createPanel_folder_windows(),			_("Folder Windows") );			kPageFolder = kPage++;
		m_pBookCtrl->AddSubPage( createPanel_folder_filters(),			_("Folder Filters") );		kPage++;
		m_pBookCtrl->AddSubPage( createPanel_folder_colors(),			_("Folder Colors") );		kPage++;
		m_pBookCtrl->AddSubPage( createPanel_folder_softmatch(),		_("Equivalence Mode") );	kPage++;

		m_pBookCtrl->AddPage( createPanel_messages(),       		_("Messages") );				kPageMessages = kPage++;
#ifdef FEATURE_SHEX
		m_pBookCtrl->AddPage( createPanel_shex(),					_("Explorer Integration") );	kPage++;
#endif
		m_pBookCtrl->AddPage( createPanel_external_tools(),			_("External Tools") );			kPage++;

		m_pBookCtrl->ExpandNode(kPageFile,true);
		m_pBookCtrl->ExpandNode(kPageFolder,true);
		m_pBookCtrl->ExpandNode(kPageMessages,true);
#else
		m_pBookCtrl = new util_treebook(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxCLIP_CHILDREN);

		m_pBookCtrl->AddPage( createPanel_file_windows(),  		_("File Windows") );
		m_pBookCtrl->AddPage( createPanel_file_rulesets(),  	_("....Rulesets") );
		m_pBookCtrl->AddPage( createPanel_misc(),           	_("....Detail Level") );
		m_pBookCtrl->AddPage( createPanel_file_line_colors(),   _("....Line Colors") );
		m_pBookCtrl->AddPage( createPanel_file_il_colors(),     _("....Intra-Line Colors") );
		m_pBookCtrl->AddPage( createPanel_file_other_colors(),  _("....Other Colors") );

		m_pBookCtrl->AddPage( createPanel_folder_windows(),		_("Folder Windows") );
		m_pBookCtrl->AddPage( createPanel_folder_filters(),		_("....Folder Filters") );
		m_pBookCtrl->AddPage( createPanel_folder_colors(),		_("....Folder Colors") );
		m_pBookCtrl->AddPage( createPanel_folder_softmatch(),	_("....Equivalence Mode") );

		m_pBookCtrl->AddPage( createPanel_messages(),       	_("Messages") );
#ifdef FEATURE_SHEX
		m_pBookCtrl->AddPage( createPanel_shex(),				_("Explorer Integration") );
#endif
		m_pBookCtrl->AddPage( createPanel_external_tools(),		_("External Tools") );
#endif

		vSizerTop->Add( m_pBookCtrl, VAR, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, M);

		// put horizontal line and set of ok/cancel buttons across the bottom of the dialog

		vSizerTop->Add( new wxStaticLine(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxLI_HORIZONTAL), FIX, wxGROW|wxLEFT|wxRIGHT, M);
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
	Centre(wxBOTH);

#if defined(__WXMAC__)
	// WXBUG 2013/05/07 On Mac (observed on 10.8.2) with wxWidgets 2.9.4
	//                  wxRadioBox widgets are too short -- by about 1 row.
	//                  W2330, W8701, W5873.  The problem goes away if you
	//                  resize the whole dialog.
	//
	//                  Force an extra re-fit on the panels with them.
	vSizerTop->Fit(m_pPanelMisc);
	vSizerTop->Fit(m_pPanelFolderSoftMatch);
#endif
	
	_enable_threshold_fields();
	_enable_softmatch_fields();
	_enable_file_ruleset_fields();
	_enable_external_tools_fields();
	_set_static_font_fields();

	// try to open the dialog on the same tab/page as they last saw it.

	long kPage = gpGlobalProps->getLong(GlobalProps::GPL_OPTIONS_DLG_INITIAL_PAGE);
	long kLimit = (long)m_pBookCtrl->GetPageCount();
	if ( (kPage >= 0) && (kPage < kLimit) )
		m_pBookCtrl->SetSelection(kPage);
}

OptionsDlg::~OptionsDlg(void)
{
	// regardless of whether OK or CANCEL was pressed, try to remember
	// the current tab/page for next time.
	
	long kPage = m_pBookCtrl->GetSelection();
	gpGlobalProps->setLong(GlobalProps::GPL_OPTIONS_DLG_INITIAL_PAGE,kPage);

	delete m_pRSTWorkingCopy;
	delete m_pXTTWorkingCopy;

	delete m_pFont_File;
#if VIEW_FOLDER_LIST_CTRL_USE_FONTS
	delete m_pFont_Folder;
#endif
	delete m_pFont_PrinterFile;
	delete m_pFont_PrinterFolder;
}

//////////////////////////////////////////////////////////////////
// before the dialog is initialized, load the initial values for each
// dialog field into these member variables (from global props).  these
// vars will be read by the validators during dialog initialization to
// seed the dialog controls and re-written during the OnOK processing to
// store the final values in the controls.

void OptionsDlg::preload_fields_folder_windows(bool bDefault)
{
#if VIEW_FOLDER_LIST_CTRL_USE_FONTS
	delete m_pFont_Folder;
	m_pFont_Folder = gpGlobalProps->createNormalFont(GlobalProps::GPS_VIEW_FOLDER_FONT, bDefault);
#endif

	delete m_pFont_PrinterFolder;
	m_pFont_PrinterFolder = gpGlobalProps->createNormalFont(GlobalProps::GPS_PRINTER_FOLDER_FONT, bDefault);

	m_bMessages_CheckFoldersOnActivate = gpGlobalProps->getBool(GlobalProps::GPL_MISC_CHECK_FOLDERS_ON_ACTIVATE, bDefault);
}

void OptionsDlg::preload_fields_folder_filters(bool bDefault)
{
	m_bFolderFilters_UseFileFilter			= ! gpGlobalProps->getBool(GlobalProps::GPL_FOLDER_IGNORE_SUFFIX_FILTER, bDefault);
	m_strFolderFilters_FileFilter			= gpGlobalProps->getString(GlobalProps::GPS_FOLDER_SUFFIX_FILTER,       bDefault);

	m_bFolderFilters_UseFolderFilter		= ! gpGlobalProps->getBool(GlobalProps::GPL_FOLDER_IGNORE_SUBDIR_FILTER, bDefault);
	m_strFolderFilters_FolderFilter			= gpGlobalProps->getString(GlobalProps::GPS_FOLDER_SUBDIR_FILTER,       bDefault);

	m_bFolderFilters_UseFullFilenameFilter	= ! gpGlobalProps->getBool(GlobalProps::GPL_FOLDER_IGNORE_FULL_FILENAME_FILTER, bDefault);
	m_strFolderFilters_FullFilenameFilter	= gpGlobalProps->getString(GlobalProps::GPS_FOLDER_FULL_FILENAME_FILTER,       bDefault);

	m_bFolderFilters_IgnorePatternCase = gpGlobalProps->getBool(GlobalProps::GPL_FOLDER_IGNORE_PATTERN_CASE, bDefault);
	m_bFolderFilters_IgnoreMatchupCase = gpGlobalProps->getBool(GlobalProps::GPL_FOLDER_IGNORE_MATCHUP_CASE, bDefault);

}

void OptionsDlg::preload_fields_folder_colors(bool bDefault)
{
	m_clrFolderColorDifferentFG  = gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_DIFFERENT_FG,   bDefault);
	m_clrFolderColorDifferentBG  = gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_DIFFERENT_BG,   bDefault);

	m_clrFolderColorEqualFG      = gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_EQUAL_FG,       bDefault);
	m_clrFolderColorEqualBG      = gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_EQUAL_BG,       bDefault);

	m_clrFolderColorEquivalentFG = gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_EQUIVALENT_FG,  bDefault);
	m_clrFolderColorEquivalentBG = gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_EQUIVALENT_BG,  bDefault);

	m_clrFolderColorFoldersFG    = gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_FOLDERS_FG,     bDefault);
	m_clrFolderColorFoldersBG    = gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_FOLDERS_BG,     bDefault);

	m_clrFolderColorPeerlessFG   = gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_PEERLESS_FG,   bDefault);
	m_clrFolderColorPeerlessBG   = gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_PEERLESS_BG,   bDefault);

	m_clrFolderColorErrorFG      = gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_ERROR_FG,       bDefault);
	m_clrFolderColorErrorBG      = gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_ERROR_BG,       bDefault);
}

void OptionsDlg::preload_fields_folder_softmatch(bool bDefault)
{
	m_iSoftMatch_Mode = gpGlobalProps->getLong(GlobalProps::GPL_FOLDER_SOFTMATCH_MODE,bDefault);

	m_strSoftMatchSimpleSuffix = gpGlobalProps->getString(GlobalProps::GPS_FOLDER_SOFTMATCH_SIMPLE_SUFFIX,bDefault);
	m_bSoftMatchSimpleIgnoreEOL = gpGlobalProps->getBool(GlobalProps::GPL_FOLDER_SOFTMATCH_SIMPLE_IGNORE_EOL,bDefault);
	m_bSoftMatchSimpleIgnoreWhitespace = gpGlobalProps->getBool(GlobalProps::GPL_FOLDER_SOFTMATCH_SIMPLE_IGNORE_WHITESPACE,bDefault);
	m_bSoftMatchSimpleIgnoreTAB = gpGlobalProps->getBool(GlobalProps::GPL_FOLDER_SOFTMATCH_SIMPLE_IGNORE_TAB,bDefault);

	m_bSoftMatchRulesetAllowDefault = gpGlobalProps->getBool(GlobalProps::GPL_FOLDER_SOFTMATCH_RULESET_ALLOW_DEFAULT,bDefault);
	m_iSoftMatchRulesetFileLimitMb = gpGlobalProps->getLong(GlobalProps::GPL_FOLDER_SOFTMATCH_RULESET_FILE_LIMIT_MB,bDefault);

	m_bEnableQuickMatch = gpGlobalProps->getBool(GlobalProps::GPL_FOLDER_QUICKMATCH_ENABLED, bDefault);
	m_strQuickMatchSuffix = gpGlobalProps->getString(GlobalProps::GPS_FOLDER_QUICKMATCH_SUFFIX, bDefault);
	
}

void OptionsDlg::preload_fields_file_windows(bool bDefault)
{
	delete m_pFont_File;
	m_pFont_File = gpGlobalProps->createNormalFont(GlobalProps::GPS_VIEW_FILE_FONT, bDefault);

	delete m_pFont_PrinterFile;
	m_pFont_PrinterFile	= gpGlobalProps->createNormalFont(GlobalProps::GPS_PRINTER_FILE_FONT, bDefault);

	m_bMessages_CheckFilesOnActivate	= gpGlobalProps->getBool(GlobalProps::GPL_MISC_CHECK_FILES_ON_ACTIVATE, bDefault);
	m_bMisc_PrintAcross					= gpGlobalProps->getBool(GlobalProps::GPL_MISC_PRINT_ACROSS,			bDefault);
	m_bMisc_RequireFinalEOL				= gpGlobalProps->getBool(GlobalProps::GPL_MISC_REQUIRE_FINAL_EOL,		bDefault);

	m_iMisc_AutoSaveInterval              = gpGlobalProps->getLong(GlobalProps::GPL_MISC_AUTOSAVE_INTERVAL,                      bDefault);
	m_bMisc_EnableAutoSave                = (m_iMisc_AutoSaveInterval >= 0);
	if (!m_bMisc_EnableAutoSave)
		m_iMisc_AutoSaveInterval          = gpGlobalProps->getLong(GlobalProps::GPL_MISC_AUTOSAVE_INTERVAL,                      true);

	m_bMisc_AutoAdvanceAfterApply         = gpGlobalProps->getBool(GlobalProps::GPL_MISC_AUTO_ADVANCE_AFTER_APPLY,               bDefault);
}

void OptionsDlg::preload_fields_file_line_colors(bool bDefault)
{
	m_clrFileColorAllEqFG			= gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_ALL_EQ_FG,			bDefault);
	m_clrFileColorAllEqBG			= gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_ALL_EQ_BG,			bDefault);
	
	m_clrFileColorNoneEqFG			= gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_NONE_EQ_FG,			bDefault);
	m_clrFileColorNoneEqBG			= gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_NONE_EQ_BG,			bDefault);

	m_clrFileColorSubEqualFG		= gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_SUB_EQUAL_FG,			bDefault);
	m_clrFileColorSubEqualBG		= gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_SUB_EQUAL_BG,			bDefault);

	m_clrFileColorSubNotEqualFG		= gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_SUB_NOTEQUAL_FG,		bDefault);
	m_clrFileColorSubNotEqualBG		= gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_SUB_NOTEQUAL_BG,		bDefault);

	m_clrFileColorConflictFG		= gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_CONFLICT_FG,			bDefault);
	m_clrFileColorConflictBG		= gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_CONFLICT_BG,			bDefault);
}

void OptionsDlg::preload_fields_file_il_colors(bool bDefault)
{
	m_clrFileColorAllEqUnimpFG		= gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_ALL_EQ_UNIMP_FG,		bDefault);
	m_clrFileColorNoneEqUnimpFG		= gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_NONE_EQ_UNIMP_FG,		bDefault);
	m_clrFileColorSubUnimpFG		= gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_SUB_UNIMP_FG,			bDefault);
	m_clrFileColorConflictUnimpFG	= gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_CONFLICT_UNIMP_FG,	bDefault);

	m_clrFileColorNoneEqIlBG		= gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_NONE_EQ_IL_BG,		bDefault);
	m_clrFileColorConflictIlBG		= gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_CONFLICT_IL_BG,		bDefault);
	m_clrFileColorSubEqualIlBG		= gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_SUB_EQUAL_IL_BG,		bDefault);
	m_clrFileColorSubNotEqualIlBG	= gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_SUB_NOTEQUAL_IL_BG,	bDefault);
}

void OptionsDlg::preload_fields_file_other_colors(bool bDefault)
{
	m_clrFileColorWindowBG			= gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_WINDOW_BG,			bDefault);

	m_clrFileColorOmitFG			= gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_OMIT_FG,				bDefault);
	m_clrFileColorOmitBG			= gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_OMIT_BG,				bDefault);

	m_clrFileColorEOLUnknownFG		= gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_EOL_UNKNOWN_FG,		bDefault);

	m_clrFileColorVoidFG			= gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_VOID_FG,				bDefault);
	m_clrFileColorVoidBG			= gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_VOID_BG,				bDefault);

	m_clrFileColorLineNrFG			= gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_LINENR_FG,			bDefault);
	m_clrFileColorLineNrBG			= gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_LINENR_BG,			bDefault);

	m_clrFileColorCaretFG			= gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_CARET_FG,				bDefault);

	m_clrFileColorSelectionFG		= gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_SELECTION_FG,			bDefault);
	m_clrFileColorSelectionBG		= gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_SELECTION_BG,			bDefault);
}

void OptionsDlg::preload_fields_file_rulesets(bool bDefault)
{
	delete m_pRSTWorkingCopy;
	if (bDefault)
	{
		m_pRSTWorkingCopy = new rs_ruleset_table();
		m_pRSTWorkingCopy->OnInit(true);
	}
	else
	{
		m_pRSTWorkingCopy = new rs_ruleset_table(*gpRsRuleSetTable);
	}

	m_bFileRulesets_EnableCustomRulesets = gpGlobalProps->getBool(GlobalProps::GPL_FILE_RULESET_ENABLE_CUSTOM_RULESETS, bDefault);
	m_bFileRulesets_EnableAutomaticMatch = gpGlobalProps->getBool(GlobalProps::GPL_FILE_RULESET_ENABLE_AUTOMATIC_MATCH, bDefault);
	m_bFileRulesets_IgnoreSuffixCase     = gpGlobalProps->getBool(GlobalProps::GPL_FILE_RULESET_IGNORE_SUFFIX_CASE,     bDefault);
	m_bFileRulesets_RequireCompleteMatch = gpGlobalProps->getBool(GlobalProps::GPL_FILE_RULESET_REQUIRE_COMPLETE_MATCH, bDefault);
	m_bFileRulesets_AskIfNoMatch         = gpGlobalProps->getBool(GlobalProps::GPL_FILE_RULESET_ASK_IF_NO_MATCH,        bDefault);
}

void OptionsDlg::preload_fields_misc(bool bDefault)
{
	m_iMisc_IntralineThreshold				= gpGlobalProps->getLong(GlobalProps::GPL_FILE_INTRALINE_SMOOTHING_THRESHOLD,        bDefault);
	m_iMisc_InterlineThreshold				= gpGlobalProps->getLong(GlobalProps::GPL_FILE_LINE_SMOOTHING_THRESHOLD,             bDefault);
	m_iMisc_DetailLevel						= gpGlobalProps->getLong(GlobalProps::GPL_FILE_DETAIL_LEVEL,                         bDefault);
	m_iMisc_MultiLineDetailLevel			= gpGlobalProps->getLong(GlobalProps::GPL_FILE_MULTILINE_DETAIL_LEVEL,               bDefault);
}

void OptionsDlg::preload_fields_messages(bool bDefault)
{
	m_bMessages_ShowFilesEquivalentMsgBox    = ! gpGlobalProps->getBool(GlobalProps::GPL_VIEW_FILE_DONT_SHOW_FILES_EQUIVALENT_MSGBOX, bDefault);
	m_bMessages_ShowAutoMergeConflictsMsgBox = ! gpGlobalProps->getBool(GlobalProps::GPL_VIEW_FILE_DONT_SHOW_AUTOMERGE_CONFLICTS_MSGBOX, bDefault);
	//m_bMessages_ShowAutoMergeResultMsgBox    = ! gpGlobalProps->getBool(GlobalProps::GPL_VIEW_FILE_DONT_SHOW_AUTOMERGE_RESULT_MSGBOX, bDefault);
	m_bMessages_ShowFindDialogEofMsgBox      = ! gpGlobalProps->getBool(GlobalProps::GPL_DIALOG_FIND_DONT_SHOW_EOF_MSGBOX, bDefault);
	m_bMessages_ShowSameFolderMsgBox         = ! gpGlobalProps->getBool(GlobalProps::GPL_VIEW_FOLDER_DONT_SHOW_SAME_FOLDER_MSGBOX, bDefault);
	m_bMessages_ShowSameFilesMsgBox          = ! gpGlobalProps->getBool(GlobalProps::GPL_VIEW_FILE_DONT_SHOW_SAME_FILES_MSGBOX, bDefault);
	m_bMessages_ShowExitMultipleWindowsMsgBox= ! gpGlobalProps->getBool(GlobalProps::GPL_VIEW_FILE_DONT_SHOW_EXIT_MULTIPLE_WINDOWS_MSGBOX, bDefault);
	m_bMessages_ShowExternalToolsMsgBox      = ! gpGlobalProps->getBool(GlobalProps::GPL_EXTERNAL_TOOLS_DONT_SHOW_EXEC_MSG, bDefault);
}

#ifdef FEATURE_SHEX
void OptionsDlg::preload_fields_shex(bool bDefault)
{
	m_lShexEnabled = gpGlobalProps->getLongUncached(GlobalProps::GPL_SHEX_ENABLED,bDefault);

	m_bShexEnabled = (m_lShexEnabled != 0);

	// the installed status comes directly from the registry.  this should return
	// true if the DLL has been installed.

	m_bShexInstalled = _is_shex_installed();
}
#endif

void OptionsDlg::preload_fields_external_tools(bool bDefault)
{
	delete m_pXTTWorkingCopy;
	if (bDefault)
	{
		m_pXTTWorkingCopy = new xt_tool_table();
		m_pXTTWorkingCopy->OnInit(true);
	}
	else
	{
		m_pXTTWorkingCopy = new xt_tool_table(*gpXtToolTable);
	}

	m_bExternalTools_EnableExternalTools	= gpGlobalProps->getBool(GlobalProps::GPL_EXTERNAL_TOOLS_ENABLE_TOOLS,				bDefault);
	m_bExternalTools_IgnoreSuffixCase		= gpGlobalProps->getBool(GlobalProps::GPL_EXTERNAL_TOOLS_IGNORE_SUFFIX_CASE,		bDefault);
	m_bExternalTools_RequireCompleteMatch	= gpGlobalProps->getBool(GlobalProps::GPL_EXTERNAL_TOOLS_REQUIRE_COMPLETE_MATCH,	bDefault);
}

//////////////////////////////////////////////////////////////////

void OptionsDlg::OnOK(wxCommandEvent & /*e*/)
{
	if (Validate() && TransferDataFromWindow())
	{
		updateGlobalProps();
		EndModal(wxID_OK);
	}
}

void OptionsDlg::updateGlobalProps(void)
{
	save_fields_folder_windows();
	save_fields_file_windows();

	save_fields_misc();
	save_fields_messages();

	save_fields_folder_filters();
	save_fields_folder_colors();
	save_fields_folder_softmatch();
	save_fields_file_line_colors();
	save_fields_file_il_colors();
	save_fields_file_other_colors();
	save_fields_file_rulesets();

#ifdef FEATURE_SHEX
	save_fields_shex();
#endif

	save_fields_external_tools();
}

//////////////////////////////////////////////////////////////////

#define SAVE_CHECK(_id_,_field_)													\
	Statement(																		\
		bool bPrev = gpGlobalProps->getBool(GlobalProps:: _id_ );					\
		if (bPrev != (_field_))														\
			gpGlobalProps->setLong(GlobalProps:: _id_ , (_field_));					)

#define SAVE_STRING(_id_,_field_)													\
	Statement(																		\
		wxString strPrev = gpGlobalProps->getString(GlobalProps:: _id_ );			\
		if (strPrev != (_field_))													\
			gpGlobalProps->setString(GlobalProps:: _id_ , (_field_));				)

#define SAVE_COLOR(_id_,_field_)													\
	Statement(																		\
		wxColor clrPrev = gpGlobalProps->getColor(GlobalProps:: _id_ );				\
		if (clrPrev != (_field_))													\
			gpGlobalProps->setColor(GlobalProps:: _id_ , (_field_));				)

#define SAVE_LONG(_id_,_field_)														\
	Statement(																		\
		long lPrev = gpGlobalProps->getLong(GlobalProps:: _id_ );					\
		if (lPrev != (_field_))														\
			gpGlobalProps->setLong(GlobalProps:: _id_, (_field_));					)

//////////////////////////////////////////////////////////////////

void OptionsDlg::save_fields_folder_windows(void)
{
#if VIEW_FOLDER_LIST_CTRL_USE_FONTS
	if (m_bFontFolderDirty)
		gpGlobalProps->saveFont(GlobalProps::GPS_VIEW_FOLDER_FONT,m_pFont_Folder);
#endif

	if (m_bFontPrinterFolderDirty)
		gpGlobalProps->saveFont(GlobalProps::GPS_PRINTER_FOLDER_FONT,m_pFont_PrinterFolder);

	SAVE_CHECK( GPL_MISC_CHECK_FOLDERS_ON_ACTIVATE,                  m_bMessages_CheckFoldersOnActivate        );
}

void OptionsDlg::save_fields_folder_filters(void)
{
	SAVE_CHECK(  GPL_FOLDER_IGNORE_SUFFIX_FILTER, ! m_bFolderFilters_UseFileFilter   );
	SAVE_STRING( GPS_FOLDER_SUFFIX_FILTER,        m_strFolderFilters_FileFilter       );

	SAVE_CHECK(  GPL_FOLDER_IGNORE_SUBDIR_FILTER, ! m_bFolderFilters_UseFolderFilter );
	SAVE_STRING( GPS_FOLDER_SUBDIR_FILTER,        m_strFolderFilters_FolderFilter     );

	SAVE_CHECK(  GPL_FOLDER_IGNORE_FULL_FILENAME_FILTER, ! m_bFolderFilters_UseFullFilenameFilter );
	SAVE_STRING( GPS_FOLDER_FULL_FILENAME_FILTER,        m_strFolderFilters_FullFilenameFilter     );

	SAVE_CHECK(  GPL_FOLDER_IGNORE_PATTERN_CASE, m_bFolderFilters_IgnorePatternCase );
	SAVE_CHECK(  GPL_FOLDER_IGNORE_MATCHUP_CASE, m_bFolderFilters_IgnoreMatchupCase );

}

void OptionsDlg::save_fields_folder_colors(void)
{
	SAVE_COLOR( GPL_FOLDER_COLOR_DIFFERENT_FG,   m_clrFolderColorDifferentFG );
	SAVE_COLOR( GPL_FOLDER_COLOR_DIFFERENT_BG,   m_clrFolderColorDifferentBG );

	SAVE_COLOR( GPL_FOLDER_COLOR_EQUAL_FG,       m_clrFolderColorEqualFG );
	SAVE_COLOR( GPL_FOLDER_COLOR_EQUAL_BG,       m_clrFolderColorEqualBG );

	SAVE_COLOR( GPL_FOLDER_COLOR_EQUIVALENT_FG, m_clrFolderColorEquivalentFG );
	SAVE_COLOR( GPL_FOLDER_COLOR_EQUIVALENT_BG, m_clrFolderColorEquivalentBG );

	SAVE_COLOR( GPL_FOLDER_COLOR_FOLDERS_FG,     m_clrFolderColorFoldersFG );
	SAVE_COLOR( GPL_FOLDER_COLOR_FOLDERS_BG,     m_clrFolderColorFoldersBG );

	SAVE_COLOR( GPL_FOLDER_COLOR_PEERLESS_FG,   m_clrFolderColorPeerlessFG );
	SAVE_COLOR( GPL_FOLDER_COLOR_PEERLESS_BG,   m_clrFolderColorPeerlessBG );

	SAVE_COLOR( GPL_FOLDER_COLOR_ERROR_FG,       m_clrFolderColorErrorFG );
	SAVE_COLOR( GPL_FOLDER_COLOR_ERROR_BG,       m_clrFolderColorErrorBG );
}

void OptionsDlg::save_fields_folder_softmatch(void)
{
	SAVE_LONG( GPL_FOLDER_SOFTMATCH_MODE, m_iSoftMatch_Mode );

	SAVE_STRING( GPS_FOLDER_SOFTMATCH_SIMPLE_SUFFIX, m_strSoftMatchSimpleSuffix );
	SAVE_CHECK( GPL_FOLDER_SOFTMATCH_SIMPLE_IGNORE_EOL, m_bSoftMatchSimpleIgnoreEOL );
	SAVE_CHECK( GPL_FOLDER_SOFTMATCH_SIMPLE_IGNORE_WHITESPACE, m_bSoftMatchSimpleIgnoreWhitespace );
	SAVE_CHECK( GPL_FOLDER_SOFTMATCH_SIMPLE_IGNORE_TAB, m_bSoftMatchSimpleIgnoreTAB );

	SAVE_CHECK( GPL_FOLDER_SOFTMATCH_RULESET_ALLOW_DEFAULT, m_bSoftMatchRulesetAllowDefault );

	m_iSoftMatchRulesetFileLimitMb = m_pSpinSoftMatchRulesetFileLimitMb->GetValue();

	SAVE_LONG( GPL_FOLDER_SOFTMATCH_RULESET_FILE_LIMIT_MB, m_iSoftMatchRulesetFileLimitMb );

	SAVE_CHECK( GPL_FOLDER_QUICKMATCH_ENABLED, m_bEnableQuickMatch );
	SAVE_STRING( GPS_FOLDER_QUICKMATCH_SUFFIX, m_strQuickMatchSuffix );
}

void OptionsDlg::save_fields_file_windows(void)
{
	if (m_bFontFileDirty)
		gpGlobalProps->saveFont(GlobalProps::GPS_VIEW_FILE_FONT,m_pFont_File);

	if (m_bFontPrinterFileDirty)
		gpGlobalProps->saveFont(GlobalProps::GPS_PRINTER_FILE_FONT,m_pFont_PrinterFile);

	SAVE_CHECK( GPL_MISC_CHECK_FILES_ON_ACTIVATE,                m_bMessages_CheckFilesOnActivate      );
	SAVE_CHECK( GPL_MISC_PRINT_ACROSS,                           m_bMisc_PrintAcross                   );
	SAVE_CHECK( GPL_MISC_REQUIRE_FINAL_EOL,                      m_bMisc_RequireFinalEOL               );

	int iAutoSaveIntervalPrev = gpGlobalProps->getLong(GlobalProps::GPL_MISC_AUTOSAVE_INTERVAL);
	int iAutoSaveIntervalNew;

	if (!m_bMisc_EnableAutoSave)
		iAutoSaveIntervalNew = -1;
	else
		iAutoSaveIntervalNew = m_pSpinAutoSaveInterval->GetValue();

	if (iAutoSaveIntervalNew != iAutoSaveIntervalPrev)
		gpGlobalProps->setLong(GlobalProps::GPL_MISC_AUTOSAVE_INTERVAL,iAutoSaveIntervalNew);

	SAVE_CHECK( GPL_MISC_AUTO_ADVANCE_AFTER_APPLY,               m_bMisc_AutoAdvanceAfterApply         );
}

void OptionsDlg::save_fields_file_line_colors(void)
{
	SAVE_COLOR( GPL_FILE_COLOR_ALL_EQ_FG,				m_clrFileColorAllEqFG );
	SAVE_COLOR( GPL_FILE_COLOR_ALL_EQ_BG,				m_clrFileColorAllEqBG );
	
	SAVE_COLOR( GPL_FILE_COLOR_NONE_EQ_FG,				m_clrFileColorNoneEqFG );
	SAVE_COLOR( GPL_FILE_COLOR_NONE_EQ_BG,				m_clrFileColorNoneEqBG );

	SAVE_COLOR( GPL_FILE_COLOR_SUB_EQUAL_FG,			m_clrFileColorSubEqualFG );
	SAVE_COLOR( GPL_FILE_COLOR_SUB_EQUAL_BG,			m_clrFileColorSubEqualBG );

	SAVE_COLOR( GPL_FILE_COLOR_SUB_NOTEQUAL_FG,			m_clrFileColorSubNotEqualFG );
	SAVE_COLOR( GPL_FILE_COLOR_SUB_NOTEQUAL_BG,			m_clrFileColorSubNotEqualBG );

	SAVE_COLOR( GPL_FILE_COLOR_CONFLICT_FG,				m_clrFileColorConflictFG );
	SAVE_COLOR( GPL_FILE_COLOR_CONFLICT_BG,				m_clrFileColorConflictBG );
}

void OptionsDlg::save_fields_file_il_colors(void)
{
	SAVE_COLOR( GPL_FILE_COLOR_ALL_EQ_UNIMP_FG,			m_clrFileColorAllEqUnimpFG );
	SAVE_COLOR( GPL_FILE_COLOR_NONE_EQ_UNIMP_FG,		m_clrFileColorNoneEqUnimpFG );
	SAVE_COLOR( GPL_FILE_COLOR_SUB_UNIMP_FG,			m_clrFileColorSubUnimpFG );
	SAVE_COLOR( GPL_FILE_COLOR_CONFLICT_UNIMP_FG,		m_clrFileColorConflictUnimpFG );

	SAVE_COLOR( GPL_FILE_COLOR_NONE_EQ_IL_BG,			m_clrFileColorNoneEqIlBG );
	SAVE_COLOR( GPL_FILE_COLOR_CONFLICT_IL_BG,			m_clrFileColorConflictIlBG );
	SAVE_COLOR( GPL_FILE_COLOR_SUB_EQUAL_IL_BG,			m_clrFileColorSubEqualIlBG );
	SAVE_COLOR( GPL_FILE_COLOR_SUB_NOTEQUAL_IL_BG,		m_clrFileColorSubNotEqualIlBG );
}

void OptionsDlg::save_fields_file_other_colors(void)
{
	SAVE_COLOR( GPL_FILE_COLOR_WINDOW_BG,				m_clrFileColorWindowBG );

	SAVE_COLOR( GPL_FILE_COLOR_OMIT_FG,					m_clrFileColorOmitFG );
	SAVE_COLOR( GPL_FILE_COLOR_OMIT_BG,					m_clrFileColorOmitBG );

	SAVE_COLOR( GPL_FILE_COLOR_EOL_UNKNOWN_FG,			m_clrFileColorEOLUnknownFG );

	SAVE_COLOR( GPL_FILE_COLOR_VOID_FG,					m_clrFileColorVoidFG );
	SAVE_COLOR( GPL_FILE_COLOR_VOID_BG,					m_clrFileColorVoidBG );

	SAVE_COLOR( GPL_FILE_COLOR_LINENR_FG,				m_clrFileColorLineNrFG );
	SAVE_COLOR( GPL_FILE_COLOR_LINENR_BG,				m_clrFileColorLineNrBG );

	SAVE_COLOR( GPL_FILE_COLOR_CARET_FG,				m_clrFileColorCaretFG );

	SAVE_COLOR( GPL_FILE_COLOR_SELECTION_FG,			m_clrFileColorSelectionFG );
	SAVE_COLOR( GPL_FILE_COLOR_SELECTION_BG,			m_clrFileColorSelectionBG );
}

void OptionsDlg::save_fields_file_rulesets(void)
{
	bool bOld_EnableCustomRulesets = gpGlobalProps->getBool(GlobalProps::GPL_FILE_RULESET_ENABLE_CUSTOM_RULESETS);
	bool bNew_EnableCustomRulesets = m_bFileRulesets_EnableCustomRulesets;
	bool bToggled = (bOld_EnableCustomRulesets != bNew_EnableCustomRulesets);

	SAVE_CHECK( GPL_FILE_RULESET_ENABLE_CUSTOM_RULESETS, m_bFileRulesets_EnableCustomRulesets );
	SAVE_CHECK( GPL_FILE_RULESET_ENABLE_AUTOMATIC_MATCH, m_bFileRulesets_EnableAutomaticMatch );
	SAVE_CHECK( GPL_FILE_RULESET_IGNORE_SUFFIX_CASE,     m_bFileRulesets_IgnoreSuffixCase     );
	SAVE_CHECK( GPL_FILE_RULESET_REQUIRE_COMPLETE_MATCH, m_bFileRulesets_RequireCompleteMatch );
	SAVE_CHECK( GPL_FILE_RULESET_ASK_IF_NO_MATCH,        m_bFileRulesets_AskIfNoMatch         );

	// always save the rulesets to the registry (even if they haven't changed).
	// (this allows us to update the registry now to reflect what the user just
	// saw.  if another instance gets launched, it will see this set when it
	// starts up.

	m_pRSTWorkingCopy->doExport();

	// if the rulesets have changed -- OR if they've enabled/disabled custom-rulesets,
	// we need to update the file windows.

	bool bRSTEqual = gpRsRuleSetTable->isEqual(m_pRSTWorkingCopy);
	if (!bRSTEqual || bToggled)
	{
//		wxLogTrace(wxTRACE_Messages,_T("OptionsDlg::RulesetTable: [IsEqual %d][Toggled %d]"),
//				   (bRSTEqual ? 1 : 0),
//				   (bToggled ? 1 : 0));

		// convert all file-diff and file-merge windows to use the corresponding
		// rulesets in the new table.  if they've seriously modified the table,
		// they may get questioned for windows that we can't figure out.
		
		gpFsFsTable->applyNewRulesetTable(m_pRSTWorkingCopy);

		// now replace the global ruleset-table with the new one.

		delete gpRsRuleSetTable;
		gpRsRuleSetTable = m_pRSTWorkingCopy;

		m_pRSTWorkingCopy = NULL;
	}
}

void OptionsDlg::save_fields_misc(void)
{
	int valueIntraline = (int)m_pSpinIntralineThreshold->GetValue();
	SAVE_LONG(GPL_FILE_INTRALINE_SMOOTHING_THRESHOLD, valueIntraline);

	int valueInterline = (int)m_pSpinInterlineThreshold->GetValue();
	SAVE_LONG(GPL_FILE_LINE_SMOOTHING_THRESHOLD, valueInterline);

	SAVE_LONG(GPL_FILE_DETAIL_LEVEL, m_iMisc_DetailLevel);
	SAVE_LONG(GPL_FILE_MULTILINE_DETAIL_LEVEL, m_iMisc_MultiLineDetailLevel);
}

void OptionsDlg::save_fields_messages(void)
{
	SAVE_CHECK( GPL_VIEW_FILE_DONT_SHOW_FILES_EQUIVALENT_MSGBOX,     (! m_bMessages_ShowFilesEquivalentMsgBox   ) );
	SAVE_CHECK( GPL_VIEW_FILE_DONT_SHOW_AUTOMERGE_CONFLICTS_MSGBOX,  (! m_bMessages_ShowAutoMergeConflictsMsgBox) );
	//SAVE_CHECK( GPL_VIEW_FILE_DONT_SHOW_AUTOMERGE_RESULT_MSGBOX,     (! m_bMessages_ShowAutoMergeResultMsgBox   ) );
	SAVE_CHECK( GPL_DIALOG_FIND_DONT_SHOW_EOF_MSGBOX,                (! m_bMessages_ShowFindDialogEofMsgBox     ) );
	SAVE_CHECK( GPL_VIEW_FOLDER_DONT_SHOW_SAME_FOLDER_MSGBOX,        (! m_bMessages_ShowSameFolderMsgBox        ) );
	SAVE_CHECK( GPL_VIEW_FILE_DONT_SHOW_SAME_FILES_MSGBOX,           (! m_bMessages_ShowSameFilesMsgBox         ) );
	SAVE_CHECK( GPL_VIEW_FILE_DONT_SHOW_EXIT_MULTIPLE_WINDOWS_MSGBOX,(! m_bMessages_ShowExitMultipleWindowsMsgBox ) );
	SAVE_CHECK( GPL_EXTERNAL_TOOLS_DONT_SHOW_EXEC_MSG,               (! m_bMessages_ShowExternalToolsMsgBox       ) );
}

#ifdef FEATURE_SHEX
void OptionsDlg::save_fields_shex(void)
{
	SAVE_CHECK(GPL_SHEX_ENABLED, m_bShexEnabled);

	// TODO decide if we want to present the CLARGS in the dialog and save them here.
}
#endif

void OptionsDlg::save_fields_external_tools(void)
{
	SAVE_CHECK( GPL_EXTERNAL_TOOLS_ENABLE_TOOLS,			m_bExternalTools_EnableExternalTools);
	SAVE_CHECK( GPL_EXTERNAL_TOOLS_IGNORE_SUFFIX_CASE,		m_bExternalTools_IgnoreSuffixCase);
	SAVE_CHECK( GPL_EXTERNAL_TOOLS_REQUIRE_COMPLETE_MATCH,	m_bExternalTools_RequireCompleteMatch);

	m_pXTTWorkingCopy->doExport();

	// replace the global table.

	delete gpXtToolTable;
	gpXtToolTable = m_pXTTWorkingCopy;
	m_pXTTWorkingCopy = NULL;
}

//////////////////////////////////////////////////////////////////
// create widgets for each panel

wxPanel * OptionsDlg::createPanel_folder_windows(void)
{
	preload_fields_folder_windows(false);

	wxPanel * pPanel = new wxPanel(m_pBookCtrl, ID_FOLDER_WINDOWS__PANEL);

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		wxBoxSizer * vSizerBody = new wxBoxSizer(wxVERTICAL);
		{
#if VIEW_FOLDER_LIST_CTRL_USE_FONTS
			wxStaticBoxSizer * staticBoxFolder = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1,_("Window Font")),wxHORIZONTAL);
			{
				staticBoxFolder->Add( new wxStaticText(pPanel,ID_FONTS__STATIC_FOLDER,_T("")),       VAR, wxALIGN_CENTER_VERTICAL|wxALL, M);
				staticBoxFolder->Add( new wxButton(pPanel,ID_FONTS__EDIT_FOLDER_FONT,	_("&Choose...")), FIX, wxALL, M);
			}
			vSizerBody->Add(staticBoxFolder, FIX, wxGROW|wxALL, M);
#endif

			wxStaticBoxSizer * staticBoxPrinter = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1,_("Printer Font")),wxHORIZONTAL);
			{
				staticBoxPrinter->Add( new wxStaticText(pPanel,ID_FONTS__STATIC_PRINTER_FOLDER,_T("")),       VAR, wxALIGN_CENTER_VERTICAL|wxALL, M);
				staticBoxPrinter->Add( new wxButton(pPanel,ID_FONTS__EDIT_PRINTER_FOLDER_FONT,	_("&Choose..."  )), FIX, wxALL, M);
			}
			vSizerBody->Add(staticBoxPrinter, FIX, wxGROW|wxALL, M);

			wxStaticBoxSizer * staticBoxSizerContent2 = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1,_("Window Options")),wxVERTICAL);
			{
				staticBoxSizerContent2->Add( new wxCheckBox(pPanel,ID_MESG__CHECK_FOLDERS_ON_ACTIVATE,
															_("&Check for Modified Files/Folders when Folder Windows are Activated"),
															wxDefaultPosition,wxDefaultSize,0,
															wxGenericValidator(&m_bMessages_CheckFoldersOnActivate)),
											 FIX, wxALL, M);
			}
			vSizerBody->Add(staticBoxSizerContent2, FIX, wxGROW | wxALL, M);
		}
		vSizerTop->Add(vSizerBody, VAR, wxGROW | wxALIGN_CENTRE | wxALL, M);

		vSizerTop->Add( new wxButton(pPanel,ID_FOLDER_WINDOWS__RESTORE_DEFAULTS,_("Restore Defaults")), FIX, wxALIGN_RIGHT|wxALL, M);
	}
	pPanel->SetSizer(vSizerTop);
	vSizerTop->Fit(pPanel);

	m_pPanelFolderWindows = pPanel;
	
	return pPanel;
}

//////////////////////////////////////////////////////////////////

wxPanel * OptionsDlg::createPanel_folder_filters(void)
{
	preload_fields_folder_filters(false);

	wxPanel * pPanel = new wxPanel(m_pBookCtrl, ID_FOLDER_FILTERS__PANEL);

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		wxBoxSizer * vSizerBody = new wxBoxSizer(wxVERTICAL);
		{
			wxStaticBoxSizer * staticBoxFileSizer = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1,_("Filename Filters")),wxVERTICAL);
			{
				staticBoxFileSizer->Add( new wxCheckBox(pPanel,ID_FOLDER_FILTERS__USE_FILE_FILTER,_("Use File Suffi&x Filters"),
														wxDefaultPosition,wxDefaultSize,0,
														wxGenericValidator(&m_bFolderFilters_UseFileFilter)),
										 FIX, wxALL, M);

				m_pTextFileFilter =  new wxTextCtrl(pPanel,ID_FOLDER_FILTERS__FILE_FILTER,_T(""),
													wxDefaultPosition,wxDefaultSize,0,
													wxTextValidator(wxFILTER_NONE,&m_strFolderFilters_FileFilter));
				staticBoxFileSizer->Add( m_pTextFileFilter,
										 VAR, wxGROW|wxALL, M);
				m_pTextFileFilter->Enable(m_bFolderFilters_UseFileFilter);

				staticBoxFileSizer->Add( new wxCheckBox(pPanel,ID_FOLDER_FILTERS__USE_FULL_FILENAME_FILTER,_("Use &Filename Filters"),
														wxDefaultPosition,wxDefaultSize,0,
														wxGenericValidator(&m_bFolderFilters_UseFullFilenameFilter)),
										 FIX, wxALL, M);

				m_pTextFullFilenameFilter = new wxTextCtrl(pPanel,ID_FOLDER_FILTERS__FULL_FILENAME_FILTER,_T(""),
														   wxDefaultPosition,wxDefaultSize,0,
														   wxTextValidator(wxFILTER_NONE,&m_strFolderFilters_FullFilenameFilter));
				staticBoxFileSizer->Add( m_pTextFullFilenameFilter,
										 VAR, wxGROW|wxALL, M);
				m_pTextFullFilenameFilter->Enable(m_bFolderFilters_UseFullFilenameFilter);
			}
			vSizerBody->Add(staticBoxFileSizer, FIX, wxGROW|wxALL, M);

			
			wxStaticBoxSizer * staticBoxFolderSizer = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1,_("Folder Filters")),wxVERTICAL);
			{
				staticBoxFolderSizer->Add( new wxCheckBox(pPanel,ID_FOLDER_FILTERS__USE_FOLDER_FILTER,_("Use &Sub-folder Filters"),
														  wxDefaultPosition,wxDefaultSize,0,
														  wxGenericValidator(&m_bFolderFilters_UseFolderFilter)),
										   FIX, wxALL, M);
				m_pTextFolderFilter = new wxTextCtrl(pPanel,ID_FOLDER_FILTERS__FOLDER_FILTER,_T(""),
													 wxDefaultPosition,wxDefaultSize,0,
													 wxTextValidator(wxFILTER_NONE,&m_strFolderFilters_FolderFilter));
				staticBoxFolderSizer->Add( m_pTextFolderFilter,
										   VAR, wxGROW|wxALL, M);
				m_pTextFolderFilter->Enable(m_bFolderFilters_UseFolderFilter);
			}
			vSizerBody->Add(staticBoxFolderSizer, FIX, wxGROW|wxALL, M);

			vSizerBody->Add( new wxCheckBox(pPanel,ID_FOLDER_FILTERS__IGNORE_PATTERN_CASE,
											_("&Ignore Case in Patterns"),
											wxDefaultPosition,wxDefaultSize,0,
											wxGenericValidator(&m_bFolderFilters_IgnorePatternCase)),
							 FIX, wxALL, M);

			vSizerBody->Add( new wxCheckBox(pPanel,ID_FOLDER_FILTERS__IGNORE_MATCHUP_CASE,
											_("&Ignore Case when Matching Rows"),
											wxDefaultPosition,wxDefaultSize,0,
											wxGenericValidator(&m_bFolderFilters_IgnoreMatchupCase)),
							 FIX, wxALL, M);
		}
		vSizerTop->Add(vSizerBody, VAR, wxGROW | wxALIGN_CENTRE | wxALL, M);

		vSizerTop->Add( new wxButton(pPanel,ID_FOLDER_FILTERS__RESTORE_DEFAULTS,_("Restore Defaults")), FIX, wxALIGN_RIGHT|wxALL, M);
	}
	pPanel->SetSizer(vSizerTop);
	vSizerTop->Fit(pPanel);

	m_pPanelFolderFilters = pPanel;

	return pPanel;
}

//////////////////////////////////////////////////////////////////

wxPanel * OptionsDlg::createPanel_folder_colors(void)
{
	preload_fields_folder_colors(false);

	wxPanel * pPanel = new wxPanel(m_pBookCtrl, ID_FOLDER_COLORS__PANEL);

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		wxBoxSizer * vSizerBody = new wxBoxSizer(wxVERTICAL);
		{
			wxStaticBoxSizer * staticBoxSizerContent = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1,_("Color Scheme for Folder Windows")),wxVERTICAL);
			{
				wxFlexGridSizer * flexGridSizer = new wxFlexGridSizer(8,3,M,M);
				{

					//////////////////////////////////////////////////////////////////
					// populate the flex-grid with a row for each color pair.
					//////////////////////////////////////////////////////////////////
				
#define XLBL(_string_)																					\
	Statement(																							\
		flexGridSizer->Add( new wxStaticText(pPanel,-1, (_string_)),						\
											 FIX, wxALIGN_CENTER_VERTICAL | wxTOP|wxLEFT, M);			)

#define XBTN(_pButton_,_id_,_field_)																	\
	Statement(																							\
		(_pButton_) = new wxBitmapButton(pPanel,(_id_),wxNullBitmap,						\
										 wxDefaultPosition,wxButton::GetDefaultSize(),wxBU_AUTODRAW,	\
										 BitmapButtonValidator(&(_field_),M));							\
		flexGridSizer->Add( (_pButton_), FIX, wxTOP|wxLEFT, M);											)

					//////////////////////////////////////////////////////////////////

					XLBL( _T("") );
					XLBL( _("Foreground") );
					XLBL( _("Background") );

					//////////////////////////////////////////////////////////////////

					XLBL( _("&Different") );
					XBTN( m_pButtonFolderColorDifferentFG,ID_FOLDER_COLORS__DIFFERENT_FG,m_clrFolderColorDifferentFG );
					XBTN( m_pButtonFolderColorDifferentBG,ID_FOLDER_COLORS__DIFFERENT_BG,m_clrFolderColorDifferentBG );

					//////////////////////////////////////////////////////////////////

					XLBL( _("Equi&valent Files") );
					XBTN( m_pButtonFolderColorEquivalentFG,ID_FOLDER_COLORS__EQUIVALENT_FG,m_clrFolderColorEquivalentFG );
					XBTN( m_pButtonFolderColorEquivalentBG,ID_FOLDER_COLORS__EQUIVALENT_BG,m_clrFolderColorEquivalentBG );

					//////////////////////////////////////////////////////////////////

					XLBL( _("Identical") );
					XBTN( m_pButtonFolderColorEqualFG,ID_FOLDER_COLORS__EQUAL_FG,m_clrFolderColorEqualFG );
					XBTN( m_pButtonFolderColorEqualBG,ID_FOLDER_COLORS__EQUAL_BG,m_clrFolderColorEqualBG );

					//////////////////////////////////////////////////////////////////

					XLBL( _("&Folders") );
					XBTN( m_pButtonFolderColorFoldersFG,ID_FOLDER_COLORS__FOLDERS_FG,m_clrFolderColorFoldersFG );
					XBTN( m_pButtonFolderColorFoldersBG,ID_FOLDER_COLORS__FOLDERS_BG,m_clrFolderColorFoldersBG );

					//////////////////////////////////////////////////////////////////

					XLBL( _("&Peerless") );
					XBTN( m_pButtonFolderColorPeerlessFG,ID_FOLDER_COLORS__PEERLESS_FG,m_clrFolderColorPeerlessFG );
					XBTN( m_pButtonFolderColorPeerlessBG,ID_FOLDER_COLORS__PEERLESS_BG,m_clrFolderColorPeerlessBG );

					//////////////////////////////////////////////////////////////////

					XLBL( _("E&rrors") );
					XBTN( m_pButtonFolderColorErrorFG,ID_FOLDER_COLORS__ERROR_FG,m_clrFolderColorErrorFG );
					XBTN( m_pButtonFolderColorErrorBG,ID_FOLDER_COLORS__ERROR_BG,m_clrFolderColorErrorBG );

					//////////////////////////////////////////////////////////////////

#undef XLBL
#undef XBTN
				}
				staticBoxSizerContent->Add(flexGridSizer, FIX, wxALL, M);
			}
			vSizerBody->Add(staticBoxSizerContent, FIX, wxALL, M);
		}
		vSizerTop->Add(vSizerBody, VAR, wxGROW | wxALIGN_CENTRE | wxALL, M);

		vSizerTop->Add( new wxButton(pPanel,ID_FOLDER_COLORS__RESTORE_DEFAULTS,_("Restore Defaults")), FIX, wxALIGN_RIGHT|wxALL, M);
	}
	pPanel->SetSizer(vSizerTop);
	vSizerTop->Fit(pPanel);

	m_pPanelFolderColors = pPanel;
	
	return pPanel;
}

//////////////////////////////////////////////////////////////////

wxPanel * OptionsDlg::createPanel_folder_softmatch(void)
{
	preload_fields_folder_softmatch(false);

	wxPanel * pPanel = new wxPanel(m_pBookCtrl, ID_FOLDER_SOFTMATCH__PANEL);

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		wxBoxSizer * vSizerBody = new wxBoxSizer(wxVERTICAL);
		{

			wxStaticBoxSizer * staticBoxSizerQuickMatch = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1,_("Quick Matching")),
																				wxVERTICAL);
			{
				staticBoxSizerQuickMatch->Add( new wxCheckBox(pPanel, ID_FOLDER_QUICKMATCH__ENABLE,
															  _("Enable Quick Match (file size only) for suffixes:"),
															  wxDefaultPosition, wxDefaultSize, 0,
															  wxGenericValidator(&m_bEnableQuickMatch)),
											   FIX, wxALL, M);
				m_pTextQuickMatchSuffix = new wxTextCtrl(pPanel, ID_FOLDER_QUICKMATCH__SUFFIX, _T(""),
														 wxDefaultPosition, wxDefaultSize, 0,
														 wxTextValidator(wxFILTER_NONE, &m_strQuickMatchSuffix));
				staticBoxSizerQuickMatch->Add( m_pTextQuickMatchSuffix,
											   VAR, wxGROW|wxALL, M);
			}
			vSizerBody->Add(staticBoxSizerQuickMatch,FIX,wxGROW|wxALL,M);

			//////////////////////////////////////////////////////////////////

			wxString astrMode[__FD_SOFTMATCH_MODE__NR__] = { _("E&xact Match Only -- No File Equivalence Testing"),
															 _("Exact Match or &Simple File Equivalence Testing"),
															 _("Exact Match or &Ruleset-based File Equivalence Testing") };
			vSizerBody->Add( new wxRadioBox(pPanel,ID_FOLDER_SOFTMATCH__RADIO_MODE,_("File Equivalence Mode"),
											wxDefaultPosition,wxDefaultSize,
											__FD_SOFTMATCH_MODE__NR__,astrMode,
											__FD_SOFTMATCH_MODE__NR__,wxRA_SPECIFY_ROWS,
											wxGenericValidator(&m_iSoftMatch_Mode)),
							 FIX, wxGROW|wxALL, M);

			wxStaticBoxSizer * staticBoxSizerSimple = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1,_("Simple File Equivalence")),wxVERTICAL);
			{
				m_pTextSoftMatchSimpleSuffix = new wxTextCtrl(pPanel,ID_FOLDER_SOFTMATCH__SIMPLE_SUFFIX,_T(""),
															  wxDefaultPosition,wxDefaultSize,0,
															  wxTextValidator(wxFILTER_NONE,&m_strSoftMatchSimpleSuffix));
				staticBoxSizerSimple->Add( m_pTextSoftMatchSimpleSuffix,
										   VAR,wxGROW|wxALL,M);

				staticBoxSizerSimple->Add( new wxCheckBox(pPanel,ID_FOLDER_SOFTMATCH__SIMPLE_IGNORE_EOL,_("&Ignore Differences in Line Termination Characters"),
														  wxDefaultPosition,wxDefaultSize,0,
														  wxGenericValidator(&m_bSoftMatchSimpleIgnoreEOL)),
										   FIX, wxALL, M);

				staticBoxSizerSimple->Add( new wxCheckBox(pPanel,ID_FOLDER_SOFTMATCH__SIMPLE_IGNORE_WHITESPACE,_("Ignore &Whitespace Differences"),
														  wxDefaultPosition,wxDefaultSize,0,
														  wxGenericValidator(&m_bSoftMatchSimpleIgnoreWhitespace)),
										   FIX, wxALL, M);

				wxBoxSizer * vSizer1 = new wxBoxSizer(wxVERTICAL);
				{
					vSizer1->Add( new wxCheckBox(pPanel,ID_FOLDER_SOFTMATCH__SIMPLE_IGNORE_TAB,_("Treat &TABs as Whitespace"),
												 wxDefaultPosition,wxDefaultSize,0,
												 wxGenericValidator(&m_bSoftMatchSimpleIgnoreTAB)),
								  FIX, wxALL, M);
				}
				staticBoxSizerSimple->Add( vSizer1, FIX, wxLEFT|wxRIGHT, M*4);
			}
			vSizerBody->Add(staticBoxSizerSimple,FIX,wxGROW|wxALL,M);

			wxStaticBoxSizer * staticBoxSizerRuleset = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1,_("Ruleset-based File Equivalence")),wxVERTICAL);
			{
				staticBoxSizerRuleset->Add( new wxCheckBox(pPanel,ID_FOLDER_SOFTMATCH__RULESET_ALLOW_DEFAULT,_("&Allow Default Ruleset (See Help)"),
														   wxDefaultPosition,wxDefaultSize,0,
														   wxGenericValidator(&m_bSoftMatchRulesetAllowDefault)),
											FIX, wxALL, M);

				wxBoxSizer * hSizerLimit = new wxBoxSizer(wxHORIZONTAL);
				{
					m_pTextSoftMatchRulesetFileLimitMb = new wxStaticText(pPanel,-1,_("File Si&ze Limit (MB)"));
					hSizerLimit->Add( m_pTextSoftMatchRulesetFileLimitMb,
									  FIX, wxALIGN_CENTER_VERTICAL, M);
					
					m_pSpinSoftMatchRulesetFileLimitMb = new wxSpinCtrl(pPanel,ID_FOLDER_SOFTMATCH__RULESET_FILE_LIMIT_MB,wxEmptyString,
																		wxDefaultPosition,wxDefaultSize,wxSP_ARROW_KEYS,
																		1,99,
																		m_iSoftMatchRulesetFileLimitMb);	// spin ctrl does not use generic validator
					hSizerLimit->Add( m_pSpinSoftMatchRulesetFileLimitMb,
									  FIX, wxALIGN_CENTER_VERTICAL | wxLEFT, M);

					m_pSpinSoftMatchRulesetFileLimitMb->SetValue(m_iSoftMatchRulesetFileLimitMb);
				}
				staticBoxSizerRuleset->Add( hSizerLimit, FIX, wxGROW|wxALL, M);
			}
			vSizerBody->Add(staticBoxSizerRuleset,FIX,wxGROW|wxALL,M);

		}
		vSizerTop->Add(vSizerBody, VAR, wxGROW | wxALIGN_CENTRE | wxALL, M);

		// put [help]    [restore] buttons at bottom

		wxBoxSizer * hSizerPageButtons = new wxBoxSizer(wxHORIZONTAL);
		{
			hSizerPageButtons->Add( new wxButton(pPanel,ID_FOLDER_SOFTMATCH__HELP,_("&Help...")), FIX, 0, 0);
			hSizerPageButtons->AddStretchSpacer(VAR);
			hSizerPageButtons->Add( new wxButton(pPanel,ID_FOLDER_SOFTMATCH__RESTORE_DEFAULTS,_("Restore Defaults")), FIX, 0, 0);
		}
		vSizerTop->Add(hSizerPageButtons,FIX,wxGROW|wxALL,M);
			
//		vSizerTop->Add( new wxButton(pPanel,ID_FOLDER_SOFTMATCH__RESTORE_DEFAULTS,_("Restore Defaults")), FIX, wxALIGN_RIGHT|wxALL, M);
	}
	pPanel->SetSizer(vSizerTop);
	vSizerTop->Fit(pPanel);

	m_pPanelFolderSoftMatch = pPanel;
	
	return pPanel;
}

//////////////////////////////////////////////////////////////////

wxPanel * OptionsDlg::createPanel_file_windows(void)
{
	preload_fields_file_windows(false);

	wxPanel * pPanel = new wxPanel(m_pBookCtrl, ID_FILE_WINDOWS__PANEL);

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		wxBoxSizer * vSizerBody = new wxBoxSizer(wxVERTICAL);
		{
			wxStaticBoxSizer * staticBoxFile = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1,_("Window Font")),wxHORIZONTAL);
			{
				staticBoxFile->Add( new wxStaticText(pPanel,ID_FONTS__STATIC_FILE,_T("")),          VAR, wxALIGN_CENTER_VERTICAL|wxALL, M);
				staticBoxFile->Add( new wxButton(pPanel,ID_FONTS__EDIT_FILE_FONT,	_("&Choose..."  )),  FIX, wxALL, M);
			}
			vSizerBody->Add(staticBoxFile, FIX, wxGROW|wxALL, M);

			wxStaticBoxSizer * staticBoxPrinter = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1,_("Printer Font")),wxHORIZONTAL);
			{
				staticBoxPrinter->Add( new wxStaticText(pPanel,ID_FONTS__STATIC_PRINTER_FILE,_T("")),       VAR, wxALIGN_CENTER_VERTICAL|wxALL, M);
				staticBoxPrinter->Add( new wxButton(pPanel,ID_FONTS__EDIT_PRINTER_FILE_FONT,	_("&Choose..."  )), FIX, wxALL, M);
			}
			vSizerBody->Add(staticBoxPrinter, FIX, wxGROW|wxALL, M);

			wxStaticBoxSizer * staticBoxSizerContent2 = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1,_("Window Options")),wxVERTICAL);
			{
				staticBoxSizerContent2->Add( new wxCheckBox(pPanel,ID_MESG__CHECK_FILES_ON_ACTIVATE,
															_("&Check for Modified Files when File Diff/Merge Windows are Activated"),
															wxDefaultPosition,wxDefaultSize,0,
															wxGenericValidator(&m_bMessages_CheckFilesOnActivate)),
											 FIX, wxALL, M);

				staticBoxSizerContent2->Add( new wxCheckBox(pPanel,ID_MISC__PRINT_ACROSS,
															_("&Print Files Interleaved (1a, 1b, 1c, 2a, 2b, ...)"),
															wxDefaultPosition,wxDefaultSize,0,
															wxGenericValidator(&m_bMisc_PrintAcross)),
											 FIX, wxALL, M);

				staticBoxSizerContent2->Add( new wxCheckBox(pPanel,ID_MISC__REQUIRE_FINAL_EOL,
															_("&Require Final EOL when Saving Files"),
															wxDefaultPosition,wxDefaultSize,0,
															wxGenericValidator(&m_bMisc_RequireFinalEOL)),
											 FIX, wxALL, M);

				wxBoxSizer * hSizerAutoSave = new wxBoxSizer(wxHORIZONTAL);
				{
					hSizerAutoSave->Add( new wxCheckBox(pPanel,ID_MISC__ENABLE_AUTOSAVE,_("&Enable Auto Save"),
														wxDefaultPosition,wxDefaultSize,0,
														wxGenericValidator(&m_bMisc_EnableAutoSave)),
										 FIX, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8*M);

					m_pTextAutoSaveIntervalLabel = new wxStaticText(pPanel,-1,_("Edit &Interval"));

					hSizerAutoSave->Add( m_pTextAutoSaveIntervalLabel, FIX, wxALIGN_CENTER_VERTICAL, M);
				
					m_pSpinAutoSaveInterval = new wxSpinCtrl(pPanel,ID_MISC__AUTOSAVE_INTERVAL,wxEmptyString,
															 wxDefaultPosition,wxDefaultSize,wxSP_ARROW_KEYS,
															 1,999,
															 m_iMisc_AutoSaveInterval);	// spin ctrl does not use generic validator so this field won't be updated automatically.

					hSizerAutoSave->Add( m_pSpinAutoSaveInterval, FIX, wxALIGN_CENTER_VERTICAL | wxLEFT, M);

					m_pSpinAutoSaveInterval->SetValue(m_iMisc_AutoSaveInterval);

					m_pTextAutoSaveIntervalLabel->Enable(m_bMisc_EnableAutoSave);
					m_pSpinAutoSaveInterval->Enable(m_bMisc_EnableAutoSave);
				}
				staticBoxSizerContent2->Add(hSizerAutoSave, FIX, wxGROW|wxALL, M);

				staticBoxSizerContent2->Add( new wxCheckBox(pPanel,ID_MISC__AUTO_ADVANCE_AFTER_APPLY,
															_("Automatically Ad&vance to Next Change after Apply Change Command"),
															wxDefaultPosition,wxDefaultSize,0,
															wxGenericValidator(&m_bMisc_AutoAdvanceAfterApply)),
											 FIX, wxALL, M);
			}
			vSizerBody->Add(staticBoxSizerContent2, FIX, wxGROW | wxALL, M);
		}
		vSizerTop->Add(vSizerBody, VAR, wxGROW | wxALIGN_CENTRE | wxALL, M);

		vSizerTop->Add( new wxButton(pPanel,ID_FILE_WINDOWS__RESTORE_DEFAULTS,_("Restore Defaults")), FIX, wxALIGN_RIGHT|wxALL, M);
	}
	pPanel->SetSizer(vSizerTop);
	vSizerTop->Fit(pPanel);

	m_pPanelFileWindows = pPanel;

	return pPanel;
}

//////////////////////////////////////////////////////////////////

wxPanel * OptionsDlg::createPanel_file_line_colors(void)
{
	preload_fields_file_line_colors(false);

	wxPanel * pPanel = new wxPanel(m_pBookCtrl, ID_FILE_LINE_COLORS__PANEL);

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		wxBoxSizer * vSizerBody = new wxBoxSizer(wxVERTICAL);
		{
			wxStaticBoxSizer * staticBoxSizerContent = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1,_("Overall Line Colors")),wxVERTICAL);
			{
				wxFlexGridSizer * flexGridSizer = new wxFlexGridSizer(6,3,M,M);
				{

					//////////////////////////////////////////////////////////////////
					// populate the flex-grid with a row for each set of colors.
					//////////////////////////////////////////////////////////////////
				
#define XLBL(_string_)																					\
	Statement(																							\
		flexGridSizer->Add( new wxStaticText(pPanel,-1, (_string_)),						\
											 FIX, wxALIGN_CENTER_VERTICAL | wxTOP|wxLEFT, M);			)

#define XBTN(_pButton_,_id_,_field_)																	\
	Statement(																							\
		(_pButton_) = new wxBitmapButton(pPanel,(_id_),wxNullBitmap,						\
										 wxDefaultPosition,wxButton::GetDefaultSize(),wxBU_AUTODRAW,	\
										 BitmapButtonValidator(&(_field_),M));							\
		flexGridSizer->Add( (_pButton_), FIX, wxTOP|wxLEFT, M);											)

					//////////////////////////////////////////////////////////////////

					XLBL( _T("") );
					XLBL( _("Foreground") );
					XLBL( _("Background") );

					//////////////////////////////////////////////////////////////////
					// _DIF_2EQ and _MRG_3EQ

					XLBL( _("&Identical Lines") );
					XBTN( m_pButtonFileColorAllEqFG,     ID_FILE_COLORS__ALL_EQ_FG,      m_clrFileColorAllEqFG      );
					XBTN( m_pButtonFileColorAllEqBG,     ID_FILE_COLORS__ALL_EQ_BG,      m_clrFileColorAllEqBG      );

					//////////////////////////////////////////////////////////////////
					// _DIF_0EQ

					XLBL( _("Changes in\n&File Diff Windows") );
					XBTN( m_pButtonFileColorNoneEqFG,     ID_FILE_COLORS__NONE_EQ_FG,      m_clrFileColorNoneEqFG      );
					XBTN( m_pButtonFileColorNoneEqBG,     ID_FILE_COLORS__NONE_EQ_BG,      m_clrFileColorNoneEqBG      );

					//////////////////////////////////////////////////////////////////
					// _MRG_ TxTyEQ -- the matching parts (the 2 of a 2-vs-1)

					XLBL( _("&Matching Changes in\nFile Merge Windows") );
					XBTN( m_pButtonFileColorSubEqualFG,ID_FILE_COLORS__SUB_EQUAL_FG,m_clrFileColorSubEqualFG );
					XBTN( m_pButtonFileColorSubEqualBG,ID_FILE_COLORS__SUB_EQUAL_BG,m_clrFileColorSubEqualBG );

					//////////////////////////////////////////////////////////////////
					// _MRG_ TxTyEQ -- the non-matching part (the 1 of a 2-vs-1)

					XLBL( _("&Non-Matching Changes") );
					XBTN( m_pButtonFileColorSubNotEqualFG,ID_FILE_COLORS__SUB_NOTEQUAL_FG,m_clrFileColorSubNotEqualFG );
					XBTN( m_pButtonFileColorSubNotEqualBG,ID_FILE_COLORS__SUB_NOTEQUAL_BG,m_clrFileColorSubNotEqualBG );

					//////////////////////////////////////////////////////////////////
					// _MRG_0EQ

					XLBL( _("&Conflicts") );
					XBTN( m_pButtonFileColorConflictFG,     ID_FILE_COLORS__CONFLICT_FG,      m_clrFileColorConflictFG      );
					XBTN( m_pButtonFileColorConflictBG,     ID_FILE_COLORS__CONFLICT_BG,      m_clrFileColorConflictBG      );
				}
				staticBoxSizerContent->Add(flexGridSizer, FIX, wxALL, M);
			}
			vSizerBody->Add(staticBoxSizerContent, FIX, wxALL, M);

			//////////////////////////////////////////////////////////////////
			// preview pane
			//////////////////////////////////////////////////////////////////

			m_pNotebookPreviewFileLineColor = new wxNotebook(pPanel,wxID_ANY,
															 wxDefaultPosition,wxDefaultSize,
															 wxNB_TOP);
			{
				m_pPanelPreviewFileLineColor2 = createPanel_Preview(m_pNotebookPreviewFileLineColor,
																	NDX_PREVIEW_FILE_LINE_COLOR_2,
																	&m_pPreviewFileLineColor2);
				m_pNotebookPreviewFileLineColor->AddPage( m_pPanelPreviewFileLineColor2,
														  _("File Diff Window Preview"));

				m_pPanelPreviewFileLineColor3 = createPanel_Preview(m_pNotebookPreviewFileLineColor,
																	NDX_PREVIEW_FILE_LINE_COLOR_3,
																	&m_pPreviewFileLineColor3);
				m_pNotebookPreviewFileLineColor->AddPage( m_pPanelPreviewFileLineColor3,
														  _("File Merge Window Preview"));
			}
			vSizerBody->Add(m_pNotebookPreviewFileLineColor, VAR, wxGROW | wxALL, M);
		}
		vSizerTop->Add(vSizerBody, VAR, wxGROW | wxALIGN_CENTRE | wxALL, M);

		vSizerTop->Add( new wxButton(pPanel,ID_FILE_LINE_COLORS__RESTORE_DEFAULTS,_("Restore Defaults")), FIX, wxALIGN_RIGHT|wxALL, M);
	}
	pPanel->SetSizer(vSizerTop);
	vSizerTop->Fit(pPanel);

	m_pPanelFileLineColors = pPanel;

	return pPanel;

#undef XLBL
#undef XBTN
}

wxPanel * OptionsDlg::createPanel_file_il_colors(void)
{
	preload_fields_file_il_colors(false);

	wxPanel * pPanel = new wxPanel(m_pBookCtrl, ID_FILE_IL_COLORS__PANEL);

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		wxBoxSizer * vSizerBody = new wxBoxSizer(wxVERTICAL);
		{
			wxStaticBoxSizer * staticBoxSizerContent = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1,_("Intra-Line Colors within Changed Lines")),wxVERTICAL);
			{
				wxFlexGridSizer * flexGridSizer = new wxFlexGridSizer(6,5,M,M);
				{

					//////////////////////////////////////////////////////////////////
					// populate the flex-grid with a row for each set of colors.
					//////////////////////////////////////////////////////////////////
				
#define XLBL(_string_)																					\
	Statement(																							\
		flexGridSizer->Add( new wxStaticText(pPanel,-1, (_string_)),									\
											 FIX, wxALIGN_CENTER_VERTICAL | wxTOP|wxLEFT, M);			)

#define XBTN(_pButton_,_id_,_field_)																	\
	Statement(																							\
		(_pButton_) = new wxBitmapButton(pPanel,(_id_),wxNullBitmap,									\
										 wxDefaultPosition,wxButton::GetDefaultSize(),wxBU_AUTODRAW,	\
										 BitmapButtonValidator(&(_field_),M));							\
		flexGridSizer->Add( (_pButton_), FIX, wxALIGN_CENTER_VERTICAL | wxTOP|wxLEFT, M);				)

#define XBMP(_pBitmap_,_field_)																			\
	Statement(																							\
		(_pBitmap_) = new _my_static_bitmap(pPanel,this,&(_field_));									\
		flexGridSizer->Add( (_pBitmap_), FIX, wxALIGN_CENTER_VERTICAL | wxTOP|wxLEFT, M);				)
					
					//////////////////////////////////////////////////////////////////

					XLBL( _T("") );
					XLBL( _("Foreground\n(Important)") );
					XLBL( _("Foreground\n(Unimportant)") );
					XLBL( _("Intra-Line\nBackground") );
					XLBL( _("Overall Line\nBackground") );

					//////////////////////////////////////////////////////////////////
					// _DIF_2EQ and _MRG_3EQ

					XLBL( _("&Identical Content\nwithin Changed Lines") );
					XBMP( m_pBitmapFileColorAllEqFG,                                     m_clrFileColorAllEqFG      );
					XBTN( m_pButtonFileColorAllEqUnimpFG,ID_FILE_COLORS__ALL_EQ_UNIMP_FG,m_clrFileColorAllEqUnimpFG );
					XLBL( _T("") );
					XLBL( _T("") );

					//////////////////////////////////////////////////////////////////
					// _DIF_0EQ

					XLBL( _("Changes in\n&File Diff Windows") );
					XBMP( m_pBitmapFileColorNoneEqFG,                                      m_clrFileColorNoneEqFG      );
					XBTN( m_pButtonFileColorNoneEqUnimpFG,ID_FILE_COLORS__NONE_EQ_UNIMP_FG,m_clrFileColorNoneEqUnimpFG );
					XBTN( m_pButtonFileColorNoneEqIlBG,   ID_FILE_COLORS__NONE_EQ_IL_BG,   m_clrFileColorNoneEqIlBG    );
					XBMP( m_pBitmapFileColorNoneEqBG,                                      m_clrFileColorNoneEqBG      );

					//////////////////////////////////////////////////////////////////
					// _MRG_ TxTyEQ -- the matching parts (the 2 of a 2-vs-1)

					XLBL( _("&Matching Changes in\nFile Merge Windows") );
					XBMP( m_pBitmapFileColorSubEqualFG,                                    m_clrFileColorSubEqualFG );
					XBTN( m_pButtonFileColorSubUnimpFG,   ID_FILE_COLORS__SUB_UNIMP_FG,    m_clrFileColorSubUnimpFG );
					XBTN( m_pButtonFileColorSubEqualIlBG, ID_FILE_COLORS__SUB_EQUAL_IL_BG, m_clrFileColorSubEqualIlBG );
					XBMP( m_pBitmapFileColorSubEqualBG,                                    m_clrFileColorSubEqualBG );

					//////////////////////////////////////////////////////////////////
					// _MRG_ TxTyEQ -- the non-matching part (the 1 of a 2-vs-1)

					XLBL( _("&Non-Matching Changes") );
					XBMP( m_pBitmapFileColorSubNotEqualFG,                                 m_clrFileColorSubNotEqualFG );
					XLBL( _T("") );
					XBTN( m_pButtonFileColorSubNotEqualIlBG,ID_FILE_COLORS__SUB_NOTEQUAL_IL_BG,m_clrFileColorSubNotEqualIlBG );
					XBMP( m_pBitmapFileColorSubNotEqualBG,                                 m_clrFileColorSubNotEqualBG );

					//////////////////////////////////////////////////////////////////
					// _MRG_0EQ

					XLBL( _("&Conflicts") );
					XBMP( m_pBitmapFileColorConflictFG,                                       m_clrFileColorConflictFG      );
					XBTN( m_pButtonFileColorConflictUnimpFG,ID_FILE_COLORS__CONFLICT_UNIMP_FG,m_clrFileColorConflictUnimpFG );
					XBTN( m_pButtonFileColorConflictIlBG,   ID_FILE_COLORS__CONFLICT_IL_BG,   m_clrFileColorConflictIlBG    );
					XBMP( m_pBitmapFileColorConflictBG,                                       m_clrFileColorConflictBG      );
				}
				staticBoxSizerContent->Add(flexGridSizer, FIX, wxALL, M);
			}
			vSizerBody->Add(staticBoxSizerContent, FIX, wxALL, M);

			//////////////////////////////////////////////////////////////////
			// preview pane
			//////////////////////////////////////////////////////////////////

			m_pNotebookPreviewFileILColor = new wxNotebook(pPanel,wxID_ANY,
														   wxDefaultPosition,wxDefaultSize,
														   wxNB_TOP);
			{
				m_pPanelPreviewFileILColor2 = createPanel_Preview(m_pNotebookPreviewFileILColor,
																  NDX_PREVIEW_FILE_IL_COLOR_2,
																  &m_pPreviewFileILColor2);
				m_pNotebookPreviewFileILColor->AddPage( m_pPanelPreviewFileILColor2,
														_("File Diff Window Preview"));

				m_pPanelPreviewFileILColor3 = createPanel_Preview(m_pNotebookPreviewFileILColor,
																  NDX_PREVIEW_FILE_IL_COLOR_3,
																  &m_pPreviewFileILColor3);
				m_pNotebookPreviewFileILColor->AddPage( m_pPanelPreviewFileILColor3,
														_("File Merge Window Preview"));
			}
			vSizerBody->Add(m_pNotebookPreviewFileILColor, VAR, wxGROW | wxALL, M);

		}
		vSizerTop->Add(vSizerBody, VAR, wxGROW | wxALIGN_CENTRE | wxALL, M);

		vSizerTop->Add( new wxButton(pPanel,ID_FILE_IL_COLORS__RESTORE_DEFAULTS,_("Restore Defaults")), FIX, wxALIGN_RIGHT|wxALL, M);
	}
	pPanel->SetSizer(vSizerTop);
	vSizerTop->Fit(pPanel);

	m_pPanelFileILColors = pPanel;

	return pPanel;

#undef XLBL
#undef XBTN
#undef XBMP
}

wxPanel * OptionsDlg::createPanel_file_other_colors(void)
{
	preload_fields_file_other_colors(false);

	wxPanel * pPanel = new wxPanel(m_pBookCtrl, ID_FILE_OTHER_COLORS__PANEL);

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		wxBoxSizer * vSizerBody = new wxBoxSizer(wxVERTICAL);
		{
			wxStaticBoxSizer * staticBoxSizerOther = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1,_("Color Scheme of Other Colors")),wxVERTICAL);
			{
				wxFlexGridSizer * flexGridSizer = new wxFlexGridSizer(8,3,M,M);
				{
					//////////////////////////////////////////////////////////////////
					// populate the flex-grid with a row for each set of colors.
					//////////////////////////////////////////////////////////////////
				
#define XLBL(_string_)																					\
	Statement(																							\
		flexGridSizer->Add( new wxStaticText(pPanel,-1, (_string_)),						\
											 FIX, wxALIGN_CENTER_VERTICAL | wxTOP|wxLEFT, M);			)

#define XBTN(_pButton_,_id_,_field_)																	\
	Statement(																							\
		(_pButton_) = new wxBitmapButton(pPanel,(_id_),wxNullBitmap,						\
										 wxDefaultPosition,wxButton::GetDefaultSize(),wxBU_AUTODRAW,	\
										 BitmapButtonValidator(&(_field_),M));							\
		flexGridSizer->Add( (_pButton_), FIX, wxTOP|wxLEFT, M);											)

					//////////////////////////////////////////////////////////////////

					XLBL( _T("") );
					XLBL( _("Foreground") );
					XLBL( _("Background") );
					
					//////////////////////////////////////////////////////////////////
					// _Window

					XLBL( _("&Window") );
					XLBL( _T("") );
					XBTN( m_pButtonFileColorWindowBG,   ID_FILE_COLORS__WINDOW_BG,    m_clrFileColorWindowBG    );

					//////////////////////////////////////////////////////////////////
					// _OMITTED

					XLBL( _("&Lines Omitted\nfrom Analysis") );
					XBTN( m_pButtonFileColorOmitFG,     ID_FILE_COLORS__OMIT_FG,      m_clrFileColorOmitFG      );
					XBTN( m_pButtonFileColorOmitBG,     ID_FILE_COLORS__OMIT_BG,      m_clrFileColorOmitBG      );

					//////////////////////////////////////////////////////////////////
					// _OMITTED EOL's

					XLBL( _("&EOLs Omitted\nfrom Analysis") );
					XBTN( m_pButtonFileColorEOLUnknownFG,ID_FILE_COLORS__EOL_UNKNOWN_FG,m_clrFileColorEOLUnknownFG);
					XLBL( _T("") );

					//////////////////////////////////////////////////////////////////
					// Voids

					XLBL( _("&Voids") );
					XBTN( m_pButtonFileColorVoidFG,ID_FILE_COLORS__VOID_FG,m_clrFileColorVoidFG);
					XBTN( m_pButtonFileColorVoidBG,ID_FILE_COLORS__VOID_BG,m_clrFileColorVoidBG);

					//////////////////////////////////////////////////////////////////
					// Line numbers

					XLBL( _("Line N&umbers") );
					XBTN( m_pButtonFileColorLineNrFG,ID_FILE_COLORS__LINENR_FG,m_clrFileColorLineNrFG);
					XBTN( m_pButtonFileColorLineNrBG,ID_FILE_COLORS__LINENR_BG,m_clrFileColorLineNrBG);

					//////////////////////////////////////////////////////////////////
					// Caret color

					XLBL( _("&Caret") );
					XBTN( m_pButtonFileColorCaretFG,ID_FILE_COLORS__CARET_FG,m_clrFileColorCaretFG);
					XLBL( _T("") );

					//////////////////////////////////////////////////////////////////
					// Selection color

					XLBL( _("&Selection") );
					XBTN( m_pButtonFileColorSelectionFG,ID_FILE_COLORS__SELECTION_FG,m_clrFileColorSelectionFG);
					XBTN( m_pButtonFileColorSelectionBG,ID_FILE_COLORS__SELECTION_BG,m_clrFileColorSelectionBG);

					//////////////////////////////////////////////////////////////////
					//////////////////////////////////////////////////////////////////
				}
				staticBoxSizerOther->Add(flexGridSizer, FIX, wxALL, M);
			}
			vSizerBody->Add(staticBoxSizerOther, FIX, wxALL, M);
		}
		vSizerTop->Add(vSizerBody, VAR, wxGROW | wxALIGN_CENTRE | wxALL, M);

		vSizerTop->Add( new wxButton(pPanel,ID_FILE_OTHER_COLORS__RESTORE_DEFAULTS,_("Restore Defaults")), FIX, wxALIGN_RIGHT|wxALL, M);
	}
	pPanel->SetSizer(vSizerTop);
	vSizerTop->Fit(pPanel);

	m_pPanelFileOtherColors = pPanel;

	return pPanel;

#undef XLBL
#undef XBTN
}

//////////////////////////////////////////////////////////////////

wxPanel * OptionsDlg::createPanel_file_rulesets(void)
{
	preload_fields_file_rulesets(false);

	wxPanel * pPanel = new wxPanel(m_pBookCtrl, ID_FILE_RULESETS__PANEL);

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		wxBoxSizer * vSizerBody = new wxBoxSizer(wxVERTICAL);
		{
			wxStaticBoxSizer * staticBoxSizerContent = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1,_("Default Ruleset")),wxHORIZONTAL);
			{
				staticBoxSizerContent->Add( new wxStaticText(pPanel,wxID_ANY,_("The Default Ruleset is used when no Custom Ruleset matches.")),
											VAR, wxALIGN_CENTER_VERTICAL|wxALL, M);
				staticBoxSizerContent->Add( new wxButton(pPanel, ID_FILE_RULESETS__EDIT_DEFAULT_RULESET, _("Edi&t...")), FIX, wxALL, M);
			}
			vSizerBody->Add(staticBoxSizerContent, FIX, wxGROW|wxALL, M);
			
			wxStaticBoxSizer * staticBoxRulesetSizer = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1,_("Custom Rulesets")),wxVERTICAL);
			{
				staticBoxRulesetSizer->Add( new wxCheckBox(pPanel,ID_FILE_RULESETS__ENABLE_CUSTOM_RULESETS,_("Enable &Custom Rulesets"),
														   wxDefaultPosition,wxDefaultSize,0,
														   wxGenericValidator(&m_bFileRulesets_EnableCustomRulesets)),
											FIX, wxTOP|wxLEFT|wxRIGHT, M);

				wxBoxSizer * vSizer1 = new wxBoxSizer(wxVERTICAL);
				{
					vSizer1->Add( new wxCheckBox(pPanel,ID_FILE_RULESETS__ENABLE_AUTOMATIC_MATCH,_("Automatically &Match Suffixes (Instead of Asking Me)"),
												 wxDefaultPosition,wxDefaultSize,0,
												 wxGenericValidator(&m_bFileRulesets_EnableAutomaticMatch)),
								  FIX, wxTOP|wxLEFT|wxRIGHT, M);

					wxBoxSizer * vSizer2 = new wxBoxSizer(wxVERTICAL);
					{
						vSizer2->Add( new wxCheckBox(pPanel,ID_FILE_RULESETS__IGNORE_SUFFIX_CASE,_("&Ignore Case When Matching File Suffixes"),
													 wxDefaultPosition,wxDefaultSize,0,
													 wxGenericValidator(&m_bFileRulesets_IgnoreSuffixCase)),
									  FIX, wxTOP, M);
					
						vSizer2->Add( new wxCheckBox(pPanel,ID_FILE_RULESETS__REQUIRE_COMPLETE_MATCH,_("&Require Complete Match (Must Match All Files)"),
													 wxDefaultPosition,wxDefaultSize,0,
													 wxGenericValidator(&m_bFileRulesets_RequireCompleteMatch)),
									  FIX, wxTOP, M);
						vSizer2->Add( new wxCheckBox(pPanel,ID_FILE_RULESETS__ASK_IF_NO_MATCH,_("A&sk Me When Nothing Matches (Instead of Using Default)"),
													 wxDefaultPosition,wxDefaultSize,0,
													 wxGenericValidator(&m_bFileRulesets_AskIfNoMatch)),
									  FIX, wxTOP, M);
					}
					vSizer1->Add(vSizer2, FIX, wxLEFT|wxRIGHT, M*4);
				}
				staticBoxRulesetSizer->Add(vSizer1, FIX, wxLEFT|wxRIGHT, M*4);

				wxBoxSizer * hSizer1 = new wxBoxSizer(wxHORIZONTAL);
				{
					wxString * array = NULL;
					int cRulesets = m_pRSTWorkingCopy->allocateArrayOfNames(&array);	// this returns (0,NULL) if no rulesets

					m_pListBoxRulesets = new wxListBox(pPanel,ID_FILE_RULESETS__LISTBOX,
													   wxDefaultPosition,wxDefaultSize,
													   cRulesets,array,wxLB_SINGLE);	// do not sort the listbox
					hSizer1->Add(m_pListBoxRulesets, VAR, wxGROW, 0);

					m_pRSTWorkingCopy->freeArrayOfNames(array);

					wxBoxSizer * vSizerButtons = new wxBoxSizer(wxVERTICAL);
					{
						vSizerButtons->Add( new wxButton(pPanel, ID_FILE_RULESETS__RULESET_ADD,     _("&Add..."   )), FIX, 0, 0);
						vSizerButtons->Add( new wxButton(pPanel, ID_FILE_RULESETS__RULESET_EDIT,    _("&Edit..."  )), FIX, wxTOP, M);
						vSizerButtons->Add( new wxButton(pPanel, ID_FILE_RULESETS__RULESET_DELETE,  _("&Delete"   )), FIX, wxTOP, M);
						vSizerButtons->Add( new wxButton(pPanel, ID_FILE_RULESETS__RULESET_CLONE,   _("C&lone"    )), FIX, wxTOP, M);
						vSizerButtons->Add( new wxButton(pPanel, ID_FILE_RULESETS__RULESET_MOVEUP,  _("Move &Up"  )), FIX, wxTOP, M);
						vSizerButtons->Add( new wxButton(pPanel, ID_FILE_RULESETS__RULESET_MOVEDOWN,_("Move Do&wn")), FIX, wxTOP, M);
					}
					hSizer1->Add(vSizerButtons, FIX, wxLEFT, M*2);
				}
				staticBoxRulesetSizer->Add(hSizer1, VAR, wxGROW|wxALL, M);
			}
			vSizerBody->Add(staticBoxRulesetSizer, VAR, wxGROW|wxALL, M);
		}
		vSizerTop->Add(vSizerBody, VAR, wxGROW | wxALIGN_CENTRE | wxALL, M);

		vSizerTop->Add( new wxButton(pPanel,ID_FILE_RULESETS__RESTORE_DEFAULTS,_("Restore Defaults")), FIX, wxALIGN_RIGHT|wxALL, M);
	}
	pPanel->SetSizer(vSizerTop);
	vSizerTop->Fit(pPanel);

	m_pPanelFileRulesets = pPanel;

	return pPanel;
}

wxPanel * OptionsDlg::createPanel_misc(void)
{
	preload_fields_misc(false);

	wxPanel * pPanel = new wxPanel(m_pBookCtrl, ID_MISC__PANEL);
	
	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		wxBoxSizer * vSizerBody = new wxBoxSizer(wxVERTICAL);
		{
			wxString astrDetailLevel[__DE_DETAIL_LEVEL__NR__] = { _("&Lines Only"), _("Lines and C&haracter") };
			vSizerBody->Add( new wxRadioBox(pPanel,ID_MISC__RADIO_DETAIL_LEVEL,_("&Analysis Detail Level"),
											wxDefaultPosition,wxDefaultSize,
											__DE_DETAIL_LEVEL__NR__,astrDetailLevel,
											__DE_DETAIL_LEVEL__NR__,wxRA_SPECIFY_COLS,
											wxGenericValidator(&m_iMisc_DetailLevel)),
							 FIX, wxGROW|wxALL, M);

			wxString astrMultiLineDetailLevel[__DE_MULTILINE_DETAIL_LEVEL__NR__] = { _("&Disabled"), _("&Simple"), _("Com&plete") };
			m_pRadioMultiLineDetailLevel = new wxRadioBox(pPanel,ID_MISC__RADIO_MULTILINE_DETAIL_LEVEL,_("&Multi-Line Intra-Line Analysis Detail Level"),
														  wxDefaultPosition,wxDefaultSize,
														  __DE_MULTILINE_DETAIL_LEVEL__NR__,astrMultiLineDetailLevel,
														  __DE_MULTILINE_DETAIL_LEVEL__NR__,wxRA_SPECIFY_COLS,
														  wxGenericValidator(&m_iMisc_MultiLineDetailLevel));
			vSizerBody->Add(m_pRadioMultiLineDetailLevel, FIX, wxGROW|wxALL, M);
			
			wxStaticBoxSizer * staticBoxSizerContent3 = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1,_("Smoothing Thresholds")),wxVERTICAL);
			{
				wxBoxSizer * hSizerThreshold1 = new wxBoxSizer(wxHORIZONTAL);
				{
					m_pTextIntralineThresholdLabel = new wxStaticText(pPanel,-1,_("Intra-line Smoothing &Threshold"));

					hSizerThreshold1->Add( m_pTextIntralineThresholdLabel,
										   FIX, wxALIGN_CENTER_VERTICAL, M);
				
					// spin ctrl does not use generic validator so m_iMisc... field won't be updated automatically.

					m_pSpinIntralineThreshold = new wxSpinCtrl(pPanel,ID_MISC__INTRALINE_THRESHOLD,wxEmptyString,
															   wxDefaultPosition,wxDefaultSize,wxSP_ARROW_KEYS,
															   0,99,
															   m_iMisc_IntralineThreshold);

					hSizerThreshold1->Add( m_pSpinIntralineThreshold, FIX, wxALIGN_CENTER_VERTICAL | wxLEFT, M);

					m_pSpinIntralineThreshold->SetValue(m_iMisc_IntralineThreshold);
				}
				staticBoxSizerContent3->Add(hSizerThreshold1, FIX, wxGROW|wxALL, M);

				wxBoxSizer * hSizerThreshold2 = new wxBoxSizer(wxHORIZONTAL);
				{
					hSizerThreshold2->Add( new wxStaticText(pPanel,-1,_("Inter-line Smoothing &Threshold")),
										   FIX, wxALIGN_CENTER_VERTICAL, M);
				
					// spin ctrl does not use generic validator so m_iMisc... field won't be updated automatically.

					m_pSpinInterlineThreshold = new wxSpinCtrl(pPanel,ID_MISC__INTERLINE_THRESHOLD,wxEmptyString,
															   wxDefaultPosition,wxDefaultSize,wxSP_ARROW_KEYS,
															   0,99,
															   m_iMisc_InterlineThreshold);

					hSizerThreshold2->Add( m_pSpinInterlineThreshold, FIX, wxALIGN_CENTER_VERTICAL | wxLEFT, M);

					m_pSpinInterlineThreshold->SetValue(m_iMisc_InterlineThreshold);
				}
				staticBoxSizerContent3->Add(hSizerThreshold2, FIX, wxGROW|wxALL, M);
			}
			vSizerBody->Add(staticBoxSizerContent3, FIX, wxGROW | wxALL, M);
		}
		vSizerTop->Add(vSizerBody, VAR, wxGROW | wxALIGN_CENTRE | wxALL, M);

		vSizerTop->Add( new wxButton(pPanel,ID_MISC__RESTORE_DEFAULTS,_("Restore Defaults")), FIX, wxALIGN_RIGHT|wxALL, M);
	}
	pPanel->SetSizer(vSizerTop);
	vSizerTop->Fit(pPanel);

	m_pPanelMisc = pPanel;

	return pPanel;
}

wxPanel * OptionsDlg::createPanel_messages(void)
{
	preload_fields_messages(false);

	wxPanel * pPanel = new wxPanel(m_pBookCtrl, ID_MESG__PANEL);
	
	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		wxBoxSizer * vSizerBody = new wxBoxSizer(wxVERTICAL);
		{
			wxStaticBoxSizer * staticBoxSizerContent1 = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1,_("Show Message Dialogs")),wxVERTICAL);
			{
				staticBoxSizerContent1->Add( new wxCheckBox(pPanel,ID_MESG__SHOW_FILES_EQUIVALENT_MSGBOX,
															_("&Show 'Files are Identical' Messages"),
															wxDefaultPosition,wxDefaultSize,0,
															wxGenericValidator(&m_bMessages_ShowFilesEquivalentMsgBox)),
											 FIX, wxALL, M);

				staticBoxSizerContent1->Add( new wxCheckBox(pPanel,ID_MESG__SHOW_AUTOMERGE_CONFLICTS_MSGBOX,
															_("&Show 'Auto-Merge Conflicts' Messages"),
															wxDefaultPosition,wxDefaultSize,0,
															wxGenericValidator(&m_bMessages_ShowAutoMergeConflictsMsgBox)),
											 FIX, wxALL, M);

				//staticBoxSizerContent1->Add( new wxCheckBox(pPanel,ID_MESG__SHOW_AUTOMERGE_RESULT_MSGBOX,
				//											_("&Show 'Auto-Merge Result' Messages"),
				//											wxDefaultPosition,wxDefaultSize,0,
				//											wxGenericValidator(&m_bMessages_ShowAutoMergeResultMsgBox)),
				//							 FIX, wxALL, M);

				staticBoxSizerContent1->Add( new wxCheckBox(pPanel,ID_MESG__SHOW_FIND_DIALOG_EOF_MSGBOX,
															_("&Show 'Reached EOF in Find Dialog' Messages"),
															wxDefaultPosition,wxDefaultSize,0,
															wxGenericValidator(&m_bMessages_ShowFindDialogEofMsgBox)),
											 FIX, wxALL, M);

				staticBoxSizerContent1->Add( new wxCheckBox(pPanel,ID_MESG__SHOW_SAME_FOLDER_MSGBOX,
															_("&Show 'Same Folder Warning' Messages"),
															wxDefaultPosition,wxDefaultSize,0,
															wxGenericValidator(&m_bMessages_ShowSameFolderMsgBox)),
											 FIX, wxALL, M);

				staticBoxSizerContent1->Add( new wxCheckBox(pPanel,ID_MESG__SHOW_SAME_FILES_MSGBOX,
															_("&Show 'Same Files Warning' Messages"),
															wxDefaultPosition,wxDefaultSize,0,
															wxGenericValidator(&m_bMessages_ShowSameFilesMsgBox)),
											 FIX, wxALL, M);

				staticBoxSizerContent1->Add( new wxCheckBox(pPanel,ID_MESG__SHOW_EXIT_MULTIPLE_WINDOWS_MSGBOX,
															_("&Show 'Multiple Windows on Exit Warning' Messages"),
															wxDefaultPosition,wxDefaultSize,0,
															wxGenericValidator(&m_bMessages_ShowExitMultipleWindowsMsgBox)),
											 FIX, wxALL, M);

				staticBoxSizerContent1->Add( new wxCheckBox(pPanel,ID_MESG__SHOW_EXTERNAL_TOOLS_MSGBOX,
															_("&Show 'Using External Tool' Messages"),
															wxDefaultPosition,wxDefaultSize,0,
															wxGenericValidator(&m_bMessages_ShowExternalToolsMsgBox)),
											 FIX, wxALL, M);
			}
			vSizerBody->Add(staticBoxSizerContent1, FIX, wxGROW | wxALL, M);
		}
		vSizerTop->Add(vSizerBody, VAR, wxGROW | wxALIGN_CENTRE | wxALL, M);

		vSizerTop->Add( new wxButton(pPanel,ID_MESG__RESTORE_DEFAULTS,_("Restore Defaults")), FIX, wxALIGN_RIGHT|wxALL, M);
	}
	pPanel->SetSizer(vSizerTop);
	vSizerTop->Fit(pPanel);

	m_pPanelMessages = pPanel;

	return pPanel;
}

#ifdef FEATURE_SHEX
wxPanel * OptionsDlg::createPanel_shex(void)
{
	preload_fields_shex(false);

	wxPanel * pPanel = new wxPanel(m_pBookCtrl, ID_SHEX__PANEL);

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		wxBoxSizer * vSizerBody = new wxBoxSizer(wxVERTICAL);
		{
			wxStaticBoxSizer * staticBoxSizerContent2 = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1,_("Personal Settings for Windows Explorer Integration")),wxVERTICAL);
			{
				m_pCheckShexEnabled = new wxCheckBox(pPanel,ID_SHEX__ENABLE_CONTEXT_MENU,
													 _("&Enable the DiffMerge Context Menu in Windows Explorer"),
													 wxDefaultPosition,wxDefaultSize,0,
													 wxGenericValidator(&m_bShexEnabled));
				staticBoxSizerContent2->Add( m_pCheckShexEnabled, FIX, wxALL, M);
				m_pCheckShexEnabled->Enable(m_bShexInstalled);
			}
			vSizerBody->Add(staticBoxSizerContent2, FIX, wxGROW | wxALL, M);
		}
		vSizerTop->Add(vSizerBody, VAR, wxGROW | wxALIGN_CENTRE | wxALL, M);

		// put [help]    [restore] buttons at bottom

		wxBoxSizer * hSizerPageButtons = new wxBoxSizer(wxHORIZONTAL);
		{
			hSizerPageButtons->Add( new wxButton(pPanel,ID_SHEX__HELP,_("&Help...")), FIX, 0, 0);
			hSizerPageButtons->AddStretchSpacer(VAR);
			hSizerPageButtons->Add( new wxButton(pPanel,ID_SHEX__RESTORE_DEFAULTS,_("Restore Personal Defaults")), FIX, 0, 0);
		}
		vSizerTop->Add(hSizerPageButtons,FIX,wxGROW|wxALL,M);
	}
	pPanel->SetSizer(vSizerTop);
	vSizerTop->Fit(pPanel);

	m_pPanelShEx = pPanel;

	return pPanel;
}
#endif

wxPanel * OptionsDlg::createPanel_external_tools(void)
{
	preload_fields_external_tools(false);

	wxPanel * pPanel = new wxPanel(m_pBookCtrl, ID_XT__PANEL);

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		wxBoxSizer * vSizerBody = new wxBoxSizer(wxVERTICAL);
		{
			wxStaticBoxSizer * staticBoxExternalToolsSizer = new wxStaticBoxSizer( new wxStaticBox(pPanel,-1,_("External Tools")),wxVERTICAL);
			{
				staticBoxExternalToolsSizer->Add( new wxCheckBox(pPanel,ID_XT__ENABLE_EXTERNAL_TOOLS,_("Enable E&xternal Tools"),
																 wxDefaultPosition,wxDefaultSize,0,
																 wxGenericValidator(&m_bExternalTools_EnableExternalTools)),
												  FIX, wxTOP|wxLEFT|wxRIGHT, M);

				wxBoxSizer * vSizer = new wxBoxSizer(wxVERTICAL);
				{
					vSizer->Add( new wxCheckBox(pPanel,ID_XT__IGNORE_SUFFIX_CASE,_("&Ignore Case When Matching File Suffixes"),
												wxDefaultPosition,wxDefaultSize,0,
												wxGenericValidator(&m_bExternalTools_IgnoreSuffixCase)),
								 FIX, wxTOP, M);

					vSizer->Add( new wxCheckBox(pPanel,ID_XT__REQUIRE_COMPLETE_MATCH,_("&Require Complete Match (Must Match All Files)"),
												wxDefaultPosition,wxDefaultSize,0,
												wxGenericValidator(&m_bExternalTools_RequireCompleteMatch)),
								 FIX, wxTOP, M);
				}
				staticBoxExternalToolsSizer->Add(vSizer, FIX, wxLEFT|wxRIGHT, M*4);

				wxBoxSizer * hSizer = new wxBoxSizer(wxHORIZONTAL);
				{
					wxString * array = NULL;
					int cTools = m_pXTTWorkingCopy->allocateArrayOfNames(&array);	// this returns (0,NULL) if no tools

					m_pListBoxExternalTools = new wxListBox(pPanel,ID_XT__LISTBOX,
															wxDefaultPosition,wxDefaultSize,
															cTools,array,wxLB_SINGLE);
					hSizer->Add(m_pListBoxExternalTools, VAR, wxGROW, 0);

					m_pXTTWorkingCopy->freeArrayOfNames(array);

					wxBoxSizer * vSizerButtons = new wxBoxSizer(wxVERTICAL);
					{
						vSizerButtons->Add( new wxButton(pPanel, ID_XT__ADD,      _("&Add...")),    FIX, 0, 0);
						vSizerButtons->Add( new wxButton(pPanel, ID_XT__EDIT,     _("&Edit...")),   FIX, wxTOP, M);
						vSizerButtons->Add( new wxButton(pPanel, ID_XT__DELETE,   _("&Delete")),    FIX, wxTOP, M);
						vSizerButtons->Add( new wxButton(pPanel, ID_XT__CLONE,    _("C&lone")),     FIX, wxTOP, M);
						vSizerButtons->Add( new wxButton(pPanel, ID_XT__MOVEUP,   _("Move &Up")),   FIX, wxTOP, M);
						vSizerButtons->Add( new wxButton(pPanel, ID_XT__MOVEDOWN, _("Move Do&wn")), FIX, wxTOP, M);
					}
					hSizer->Add(vSizerButtons, FIX, wxLEFT, M*2);
				}
				staticBoxExternalToolsSizer->Add(hSizer, VAR, wxGROW|wxALL, M);
			}
			vSizerBody->Add(staticBoxExternalToolsSizer, VAR, wxGROW|wxALL, M);
		}
		vSizerTop->Add(vSizerBody, VAR, wxGROW | wxALIGN_CENTRE | wxALL, M);

		vSizerTop->Add( new wxButton(pPanel,ID_XT__RESTORE_DEFAULTS,_("Restore Defaults")), FIX, wxALIGN_RIGHT|wxALL, M);
	}
	pPanel->SetSizer(vSizerTop);
	vSizerTop->Fit(pPanel);

	m_pPanelExternalTools = pPanel;

	return pPanel;
}

//////////////////////////////////////////////////////////////////
// these are called when the "restore defaults" button is pressed on
// the corresponding panel.
//
// we set the member variables to their builtin default values
// and tickle the panel to re-read them and update all the controls
// on the panel.

void OptionsDlg::onButtonEvent_RestoreDefaults_folder_windows(wxCommandEvent & /*e*/)
{
	preload_fields_folder_windows(true);
	m_pPanelFolderWindows->TransferDataToWindow();

#if VIEW_FOLDER_LIST_CTRL_USE_FONTS
	m_bFontFolderDirty = true;
#endif
	m_bFontPrinterFolderDirty = true;

	_set_static_font_fields();
}

void OptionsDlg::onButtonEvent_RestoreDefaults_folder_filters(wxCommandEvent & /*e*/)
{
	preload_fields_folder_filters(true);
	m_pPanelFolderFilters->TransferDataToWindow();
}

void OptionsDlg::onButtonEvent_RestoreDefaults_folder_colors(wxCommandEvent & /*e*/)
{
	preload_fields_folder_colors(true);
	m_pPanelFolderColors->TransferDataToWindow();
}

void OptionsDlg::onButtonEvent_RestoreDefaults_folder_softmatch(wxCommandEvent & /*e*/)
{
	preload_fields_folder_softmatch(true);
	m_pPanelFolderSoftMatch->TransferDataToWindow();

	m_pSpinSoftMatchRulesetFileLimitMb->SetValue(m_iSoftMatchRulesetFileLimitMb);

	_enable_softmatch_fields();
}

void OptionsDlg::onButtonEvent_RestoreDefaults_file_windows(wxCommandEvent & /*e*/)
{
	preload_fields_file_windows(true);
	m_pPanelFileWindows->TransferDataToWindow();

	m_bFontFileDirty = true;
	m_bFontPrinterFileDirty = true;

	_set_static_font_fields();

	m_pSpinAutoSaveInterval->SetValue(m_iMisc_AutoSaveInterval);
	
	m_pTextAutoSaveIntervalLabel->Enable(m_bMisc_EnableAutoSave);
	m_pSpinAutoSaveInterval->Enable(m_bMisc_EnableAutoSave);

	refreshFilePreviews();
}

void OptionsDlg::onButtonEvent_RestoreDefaults_file_line_colors(wxCommandEvent & /*e*/)
{
	preload_fields_file_line_colors(true);
	m_pPanelFileLineColors->TransferDataToWindow();

	refreshFilePreviews();
	refreshFileColorBitmaps();
}

void OptionsDlg::onButtonEvent_RestoreDefaults_file_il_colors(wxCommandEvent & /*e*/)
{
	preload_fields_file_il_colors(true);
	m_pPanelFileILColors->TransferDataToWindow();

	refreshFilePreviews();
}

void OptionsDlg::onButtonEvent_RestoreDefaults_file_other_colors(wxCommandEvent & /*e*/)
{
	preload_fields_file_other_colors(true);
	m_pPanelFileOtherColors->TransferDataToWindow();

	refreshFilePreviews();
}

void OptionsDlg::onButtonEvent_RestoreDefaults_file_rulesets(wxCommandEvent & /*e*/)
{
	// the "Restore Defaults" button will delete all of their rulesets
	// (both the ones we initially created and any that they may have
	// created) and restore the original builtin list.
	//
	// make sure they understand this.

	wxMessageDialog dlg(this,
						wxGetTranslation(
							L"This will delete ALL of the rulesets that you have created and\n"
							L"any changes that you have made to the original, predefined rulesets.\n"
							L"\n"
							L"Are you sure that you want to do this?"),
						_("Warning"),
						wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);
	int answer = dlg.ShowModal();
	if (answer != wxID_YES)
		return;

	preload_fields_file_rulesets(true);
	m_pPanelFileRulesets->TransferDataToWindow();

	// we're responsible for rebuilding the listbox

	wxString * array = NULL;
	int cRulesets = m_pRSTWorkingCopy->allocateArrayOfNames(&array);	// this returns (0,NULL) if no rulesets
	m_pListBoxRulesets->Set(cRulesets,array);
	m_pRSTWorkingCopy->freeArrayOfNames(array);

	_enable_file_ruleset_fields();
}

void OptionsDlg::onButtonEvent_RestoreDefaults_misc(wxCommandEvent & /*e*/)
{
	preload_fields_misc(true);
	m_pPanelMisc->TransferDataToWindow();

	m_pSpinIntralineThreshold->SetValue(m_iMisc_IntralineThreshold);
	m_pSpinInterlineThreshold->SetValue(m_iMisc_InterlineThreshold);
}

void OptionsDlg::onButtonEvent_RestoreDefaults_messages(wxCommandEvent & /*e*/)
{
	preload_fields_messages(true);
	m_pPanelMessages->TransferDataToWindow();
}

#ifdef FEATURE_SHEX
void OptionsDlg::onButtonEvent_RestoreDefaults_shex(wxCommandEvent & /*e*/)
{
	_shex_refresh_fields(true);
}

void OptionsDlg::_shex_refresh_fields(bool bDefault)
{
	preload_fields_shex(bDefault);

	m_pPanelShEx->TransferDataToWindow();

	m_pCheckShexEnabled->Enable(m_bShexInstalled);
}
#endif

void OptionsDlg::onButtonEvent_RestoreDefaults_ExternalTools(wxCommandEvent & /*e*/)
{
	preload_fields_external_tools(true);
	m_pPanelExternalTools->TransferDataToWindow();

	// we're responsible for rebuilding the listbox

	wxString * array = NULL;
	int cTools = m_pXTTWorkingCopy->allocateArrayOfNames(&array);	// this returns (0,NULL) if no tools
	m_pListBoxExternalTools->Set(cTools,array);
	m_pXTTWorkingCopy->freeArrayOfNames(array);

	_enable_external_tools_fields();
}

//////////////////////////////////////////////////////////////////
// when a color button is pressed, we run the color chooser common
// dialog and then reload the color into button if the user presses
// ok.
//
// WARNING: we must use TransferDataToWindow() on the Panel (rather
// WARNING: than on the individual button).  the other way doesn't
// WARNING: do anything.
//
// Note: SetChooseFull() detemines whether the "advanced>>" thing is
// Note: open (and the color wheel visible) on windows.  the array of
// Note: custom colors is always shown regardless of whether the advanced
// Note: portion is shown or not.  so, since we can't hide the custom
// Note: colors, we should support it.


void OptionsDlg::_setColor(wxColor * pField, wxPanel * pPanel, _my_static_bitmap * pBmpAlsoUpdate)
{
    wxColourData data;
    data.SetChooseFull(true);
    if (pField->Ok())
        data.SetColour(*pField);

	gpGlobalProps->loadCustomColorData(&data);

    wxColour colRet;
    wxColourDialog dlg(this, &data);

	if (dlg.ShowModal() == wxID_OK)
	{
		// save chosen color into m_clr{...} variable (doesn't take effect
		// until they hit OK on the Options Dialog).
		//
		// but go ahead an preserve the custom color vector now.

		*pField = dlg.GetColourData().GetColour();
		pPanel->TransferDataToWindow();

		gpGlobalProps->saveCustomColorData(&dlg.GetColourData());

		// if this color is being used by a preview, we need to
		// invalidate it and let it redraw.

		refreshFilePreviews();

		if (pBmpAlsoUpdate)
			pBmpAlsoUpdate->set_bitmap();
	}
}

//////////////////////////////////////////////////////////////////

void OptionsDlg::onButtonEvent_folder_color_different_fg  (wxCommandEvent & /*e*/)	{ _setColor(&m_clrFolderColorDifferentFG, m_pPanelFolderColors); }
void OptionsDlg::onButtonEvent_folder_color_different_bg  (wxCommandEvent & /*e*/)	{ _setColor(&m_clrFolderColorDifferentBG, m_pPanelFolderColors); }
void OptionsDlg::onButtonEvent_folder_color_equal_fg      (wxCommandEvent & /*e*/)	{ _setColor(&m_clrFolderColorEqualFG,     m_pPanelFolderColors); }
void OptionsDlg::onButtonEvent_folder_color_equal_bg      (wxCommandEvent & /*e*/)	{ _setColor(&m_clrFolderColorEqualBG,     m_pPanelFolderColors); }
void OptionsDlg::onButtonEvent_folder_color_equivalent_fg (wxCommandEvent & /*e*/)	{ _setColor(&m_clrFolderColorEquivalentFG,m_pPanelFolderColors); }
void OptionsDlg::onButtonEvent_folder_color_equivalent_bg (wxCommandEvent & /*e*/)	{ _setColor(&m_clrFolderColorEquivalentBG,m_pPanelFolderColors); }
void OptionsDlg::onButtonEvent_folder_color_folders_fg    (wxCommandEvent & /*e*/)	{ _setColor(&m_clrFolderColorFoldersFG,   m_pPanelFolderColors); }
void OptionsDlg::onButtonEvent_folder_color_folders_bg    (wxCommandEvent & /*e*/)	{ _setColor(&m_clrFolderColorFoldersBG,   m_pPanelFolderColors); }
void OptionsDlg::onButtonEvent_folder_color_peerless_fg   (wxCommandEvent & /*e*/)	{ _setColor(&m_clrFolderColorPeerlessFG,  m_pPanelFolderColors); }
void OptionsDlg::onButtonEvent_folder_color_peerless_bg   (wxCommandEvent & /*e*/)	{ _setColor(&m_clrFolderColorPeerlessBG,  m_pPanelFolderColors); }
void OptionsDlg::onButtonEvent_folder_color_error_fg      (wxCommandEvent & /*e*/)	{ _setColor(&m_clrFolderColorErrorFG,     m_pPanelFolderColors); }
void OptionsDlg::onButtonEvent_folder_color_error_bg      (wxCommandEvent & /*e*/)	{ _setColor(&m_clrFolderColorErrorBG,     m_pPanelFolderColors); }

//////////////////////////////////////////////////////////////////
// colors on "line colors" tab

void OptionsDlg::onButtonEvent_file_color_all_eq_fg			(wxCommandEvent & /*e*/) { _setColor(&m_clrFileColorAllEqFG,        m_pPanelFileLineColors,
																								 m_pBitmapFileColorAllEqFG); }
void OptionsDlg::onButtonEvent_file_color_all_eq_bg			(wxCommandEvent & /*e*/) { _setColor(&m_clrFileColorAllEqBG,        m_pPanelFileLineColors); }
void OptionsDlg::onButtonEvent_file_color_none_eq_fg		(wxCommandEvent & /*e*/) { _setColor(&m_clrFileColorNoneEqFG,       m_pPanelFileLineColors,
																								 m_pBitmapFileColorNoneEqFG); }
void OptionsDlg::onButtonEvent_file_color_none_eq_bg		(wxCommandEvent & /*e*/) { _setColor(&m_clrFileColorNoneEqBG,       m_pPanelFileLineColors,
																								 m_pBitmapFileColorNoneEqBG); }
void OptionsDlg::onButtonEvent_file_color_sub_equal_fg		(wxCommandEvent & /*e*/) { _setColor(&m_clrFileColorSubEqualFG,     m_pPanelFileLineColors,
																								 m_pBitmapFileColorSubEqualFG); }
void OptionsDlg::onButtonEvent_file_color_sub_equal_bg		(wxCommandEvent & /*e*/) { _setColor(&m_clrFileColorSubEqualBG,     m_pPanelFileLineColors,
																								 m_pBitmapFileColorSubEqualBG); }
void OptionsDlg::onButtonEvent_file_color_sub_notequal_fg	(wxCommandEvent & /*e*/) { _setColor(&m_clrFileColorSubNotEqualFG,  m_pPanelFileLineColors,
																								 m_pBitmapFileColorSubNotEqualFG); }
void OptionsDlg::onButtonEvent_file_color_sub_notequal_bg	(wxCommandEvent & /*e*/) { _setColor(&m_clrFileColorSubNotEqualBG,  m_pPanelFileLineColors,
																								 m_pBitmapFileColorSubNotEqualBG); }
void OptionsDlg::onButtonEvent_file_color_conflict_fg		(wxCommandEvent & /*e*/) { _setColor(&m_clrFileColorConflictFG,     m_pPanelFileLineColors,
																								 m_pBitmapFileColorConflictFG); }
void OptionsDlg::onButtonEvent_file_color_conflict_bg		(wxCommandEvent & /*e*/) { _setColor(&m_clrFileColorConflictBG,     m_pPanelFileLineColors,
																								 m_pBitmapFileColorConflictBG); }

// colors on "intra-line colors" tab

void OptionsDlg::onButtonEvent_file_color_all_eq_unimp_fg	(wxCommandEvent & /*e*/) { _setColor(&m_clrFileColorAllEqUnimpFG,   m_pPanelFileILColors); }
void OptionsDlg::onButtonEvent_file_color_none_eq_unimp_fg	(wxCommandEvent & /*e*/) { _setColor(&m_clrFileColorNoneEqUnimpFG,  m_pPanelFileILColors); }
void OptionsDlg::onButtonEvent_file_color_sub_unimp_fg		(wxCommandEvent & /*e*/) { _setColor(&m_clrFileColorSubUnimpFG,     m_pPanelFileILColors); }
void OptionsDlg::onButtonEvent_file_color_conflict_unimp_fg	(wxCommandEvent & /*e*/) { _setColor(&m_clrFileColorConflictUnimpFG,m_pPanelFileILColors); }
void OptionsDlg::onButtonEvent_file_color_none_eq_il_bg		(wxCommandEvent & /*e*/) { _setColor(&m_clrFileColorNoneEqIlBG,     m_pPanelFileILColors); }
void OptionsDlg::onButtonEvent_file_color_conflict_il_bg	(wxCommandEvent & /*e*/) { _setColor(&m_clrFileColorConflictIlBG,   m_pPanelFileILColors); }
void OptionsDlg::onButtonEvent_file_color_sub_equal_il_bg	(wxCommandEvent & /*e*/) { _setColor(&m_clrFileColorSubEqualIlBG,   m_pPanelFileILColors); }
void OptionsDlg::onButtonEvent_file_color_sub_notequal_il_bg(wxCommandEvent & /*e*/) { _setColor(&m_clrFileColorSubNotEqualIlBG,m_pPanelFileILColors); }

// colors on "other colors" tab

void OptionsDlg::onButtonEvent_file_color_window_bg			(wxCommandEvent & /*e*/) { _setColor(&m_clrFileColorWindowBG,       m_pPanelFileOtherColors); }
void OptionsDlg::onButtonEvent_file_color_omit_fg			(wxCommandEvent & /*e*/) { _setColor(&m_clrFileColorOmitFG,         m_pPanelFileOtherColors); }
void OptionsDlg::onButtonEvent_file_color_omit_bg			(wxCommandEvent & /*e*/) { _setColor(&m_clrFileColorOmitBG,         m_pPanelFileOtherColors); }
void OptionsDlg::onButtonEvent_file_color_eol_unknown_fg	(wxCommandEvent & /*e*/) { _setColor(&m_clrFileColorEOLUnknownFG,   m_pPanelFileOtherColors); }
void OptionsDlg::onButtonEvent_file_color_void_fg			(wxCommandEvent & /*e*/) { _setColor(&m_clrFileColorVoidFG,         m_pPanelFileOtherColors); }
void OptionsDlg::onButtonEvent_file_color_void_bg			(wxCommandEvent & /*e*/) { _setColor(&m_clrFileColorVoidBG,         m_pPanelFileOtherColors); }
void OptionsDlg::onButtonEvent_file_color_linenr_fg			(wxCommandEvent & /*e*/) { _setColor(&m_clrFileColorLineNrFG,       m_pPanelFileOtherColors); }
void OptionsDlg::onButtonEvent_file_color_linenr_bg			(wxCommandEvent & /*e*/) { _setColor(&m_clrFileColorLineNrBG,       m_pPanelFileOtherColors); }
void OptionsDlg::onButtonEvent_file_color_caret_fg			(wxCommandEvent & /*e*/) { _setColor(&m_clrFileColorCaretFG,        m_pPanelFileOtherColors); }
void OptionsDlg::onButtonEvent_file_color_selection_fg		(wxCommandEvent & /*e*/) { _setColor(&m_clrFileColorSelectionFG,    m_pPanelFileOtherColors); }
void OptionsDlg::onButtonEvent_file_color_selection_bg		(wxCommandEvent & /*e*/) { _setColor(&m_clrFileColorSelectionBG,    m_pPanelFileOtherColors); }

//////////////////////////////////////////////////////////////////
// when the upper-most checkboxes are turned off, we need to disable
// the fields that no long apply.

void OptionsDlg::_enable_file_ruleset_fields(void)
{
	int index = m_pListBoxRulesets->GetSelection();
	int limit = m_pRSTWorkingCopy->getCountRuleSets();

	bool bHaveSelection = (index != wxNOT_FOUND);

	FindWindow(ID_FILE_RULESETS__ENABLE_AUTOMATIC_MATCH)->Enable(m_bFileRulesets_EnableCustomRulesets);
	FindWindow(ID_FILE_RULESETS__IGNORE_SUFFIX_CASE    )->Enable(m_bFileRulesets_EnableCustomRulesets  &&  m_bFileRulesets_EnableAutomaticMatch);
	FindWindow(ID_FILE_RULESETS__REQUIRE_COMPLETE_MATCH)->Enable(m_bFileRulesets_EnableCustomRulesets  &&  m_bFileRulesets_EnableAutomaticMatch);
	FindWindow(ID_FILE_RULESETS__ASK_IF_NO_MATCH       )->Enable(m_bFileRulesets_EnableCustomRulesets  &&  m_bFileRulesets_EnableAutomaticMatch);
	FindWindow(ID_FILE_RULESETS__LISTBOX               )->Enable(m_bFileRulesets_EnableCustomRulesets);
	FindWindow(ID_FILE_RULESETS__RULESET_ADD           )->Enable(m_bFileRulesets_EnableCustomRulesets);
	FindWindow(ID_FILE_RULESETS__RULESET_EDIT          )->Enable(m_bFileRulesets_EnableCustomRulesets  && bHaveSelection);
	FindWindow(ID_FILE_RULESETS__RULESET_DELETE        )->Enable(m_bFileRulesets_EnableCustomRulesets  && bHaveSelection);
	FindWindow(ID_FILE_RULESETS__RULESET_CLONE         )->Enable(m_bFileRulesets_EnableCustomRulesets  && bHaveSelection);
	FindWindow(ID_FILE_RULESETS__RULESET_MOVEUP        )->Enable(m_bFileRulesets_EnableCustomRulesets  && bHaveSelection  &&  (index > 0));
	FindWindow(ID_FILE_RULESETS__RULESET_MOVEDOWN      )->Enable(m_bFileRulesets_EnableCustomRulesets  && bHaveSelection  &&  (index+1 < limit));
}

void OptionsDlg::_enable_softmatch_fields(void)
{
	FindWindow(ID_FOLDER_SOFTMATCH__SIMPLE_SUFFIX           )->Enable(m_iSoftMatch_Mode==FD_SOFTMATCH_MODE_SIMPLE);
	FindWindow(ID_FOLDER_SOFTMATCH__SIMPLE_IGNORE_EOL       )->Enable(m_iSoftMatch_Mode==FD_SOFTMATCH_MODE_SIMPLE);
	FindWindow(ID_FOLDER_SOFTMATCH__SIMPLE_IGNORE_WHITESPACE)->Enable(m_iSoftMatch_Mode==FD_SOFTMATCH_MODE_SIMPLE);
	FindWindow(ID_FOLDER_SOFTMATCH__SIMPLE_IGNORE_TAB       )->Enable((m_iSoftMatch_Mode==FD_SOFTMATCH_MODE_SIMPLE) && m_bSoftMatchSimpleIgnoreWhitespace);

	m_pTextSoftMatchRulesetFileLimitMb->Enable(m_iSoftMatch_Mode==FD_SOFTMATCH_MODE_RULESET);
	m_pSpinSoftMatchRulesetFileLimitMb->Enable(m_iSoftMatch_Mode==FD_SOFTMATCH_MODE_RULESET);
	FindWindow(ID_FOLDER_SOFTMATCH__RULESET_ALLOW_DEFAULT   )->Enable(m_iSoftMatch_Mode==FD_SOFTMATCH_MODE_RULESET);

	// Quick-match fields are not controlled by soft-match radio
}

void OptionsDlg::_enable_external_tools_fields(void)
{
	int index = m_pListBoxExternalTools->GetSelection();
	int limit = m_pXTTWorkingCopy->getCountTools();

	bool bHaveSelection = (index != wxNOT_FOUND);
	
	FindWindow(ID_XT__IGNORE_SUFFIX_CASE    )->Enable(m_bExternalTools_EnableExternalTools);
	FindWindow(ID_XT__REQUIRE_COMPLETE_MATCH)->Enable(m_bExternalTools_EnableExternalTools);

	FindWindow(ID_XT__LISTBOX               )->Enable(m_bExternalTools_EnableExternalTools);
	FindWindow(ID_XT__ADD                   )->Enable(m_bExternalTools_EnableExternalTools);
	FindWindow(ID_XT__EDIT                  )->Enable(m_bExternalTools_EnableExternalTools && bHaveSelection);
	FindWindow(ID_XT__DELETE                )->Enable(m_bExternalTools_EnableExternalTools && bHaveSelection);
	FindWindow(ID_XT__CLONE                 )->Enable(m_bExternalTools_EnableExternalTools && bHaveSelection);
	FindWindow(ID_XT__MOVEUP                )->Enable(m_bExternalTools_EnableExternalTools && bHaveSelection && (index > 0));
	FindWindow(ID_XT__MOVEDOWN              )->Enable(m_bExternalTools_EnableExternalTools && bHaveSelection && (index+1 < limit));
}

//////////////////////////////////////////////////////////////////

void OptionsDlg::_set_static_font_fields(void)
{
	wxString strFaceFile(m_pFont_File->GetFaceName());
	if (strFaceFile.Length() == 0)
		strFaceFile = _("_Default_");
	FindWindow(ID_FONTS__STATIC_FILE  )->SetLabel( wxString::Format(_("%s, %d Point"), strFaceFile.wc_str(), m_pFont_File->GetPointSize()) );

#if VIEW_FOLDER_LIST_CTRL_USE_FONTS
	wxString strFaceFolder(m_pFont_Folder->GetFaceName());
	if (strFaceFolder.Length() == 0)
		strFaceFolder = _("_Default_");
	FindWindow(ID_FONTS__STATIC_FOLDER)->SetLabel( wxString::Format(_("%s, %d Point"), strFaceFolder.wc_str(), m_pFont_Folder->GetPointSize()) );
#endif

	wxString strFacePrinterFile(m_pFont_PrinterFile->GetFaceName());
	if (strFacePrinterFile.Length() == 0)
		strFacePrinterFile = _("_Default_");
	FindWindow(ID_FONTS__STATIC_PRINTER_FILE)->SetLabel( wxString::Format(_("%s, %d Point"),
																		  strFacePrinterFile.wc_str(),
																		  m_pFont_PrinterFile->GetPointSize()) );

	wxString strFacePrinterFolder(m_pFont_PrinterFolder->GetFaceName());
	if (strFacePrinterFolder.Length() == 0)
		strFacePrinterFolder = _("_Default_");
	FindWindow(ID_FONTS__STATIC_PRINTER_FOLDER)->SetLabel( wxString::Format(_("%s, %d Point"),
																			strFacePrinterFolder.wc_str(),
																			m_pFont_PrinterFolder->GetPointSize()) );

	// after we change the font, recalibrate vertical scrollbar on previews

	if (m_pPreviewFileLineColor2)
		m_pPreviewFileLineColor2->setNrLines();
	if (m_pPreviewFileLineColor3)
		m_pPreviewFileLineColor3->setNrLines();
	if (m_pPreviewFileILColor2)
		m_pPreviewFileILColor2->setNrLines();
	if (m_pPreviewFileILColor3)
		m_pPreviewFileILColor3->setNrLines();
}

//////////////////////////////////////////////////////////////////

void OptionsDlg::onCheckEvent_enable_custom_rulesets(wxCommandEvent & e)
{
	m_bFileRulesets_EnableCustomRulesets = e.IsChecked();

	_enable_file_ruleset_fields();
}

void OptionsDlg::onCheckEvent_enable_automatic_match(wxCommandEvent & e)
{
	m_bFileRulesets_EnableAutomaticMatch = e.IsChecked();

	_enable_file_ruleset_fields();
}

void OptionsDlg::onListBoxEvent_listbox(wxCommandEvent & /*e*/)
{
	_enable_file_ruleset_fields();
}

void OptionsDlg::onListBoxDClickEvent_listbox(wxCommandEvent & e)
{
	onButtonEvent_ruleset_edit(e);
}

void OptionsDlg::onButtonEvent_ruleset_add(wxCommandEvent & /*e*/)
{
	rs_ruleset_dlg__add dlg(this);

	rs_ruleset * pNew = NULL;
	int result = dlg.run(&pNew);

	if (result != wxID_OK)
		return;

	m_pRSTWorkingCopy->addRuleSet(pNew);	// we assume that this is an append()
	int index = m_pListBoxRulesets->Append(pNew->getName());
	m_pListBoxRulesets->SetSelection(index);

	_enable_file_ruleset_fields();
}

void OptionsDlg::onButtonEvent_ruleset_edit(wxCommandEvent & /*e*/)
{
	int index = m_pListBoxRulesets->GetSelection();
	if (index == wxNOT_FOUND)
	{
		// if nothing was selected in this listbox, we shouldn't
		// do anything.  (ideally, we should have disabled the
		// EDIT button, but there are times when we get this --
		// like when nothing is selected when we first come up.)

		return;
	}

	int limit = m_pRSTWorkingCopy->getCountRuleSets();
	if (index >= limit)		// should not happen
		return;
	
	const rs_ruleset * pRS_Current = m_pRSTWorkingCopy->getNthRuleSet(index);
	
	rs_ruleset_dlg__edit dlg(this,pRS_Current);

	rs_ruleset * pRS_New = NULL;
	int result = dlg.run(&pRS_New);

	if (result != wxID_OK)
		return;

	if (pRS_Current->isEqual(pRS_New))				// if they just clicked OK without changing anything,
	{												// we don't really need to do anything.
		delete pRS_New;
		return;
	}
	
	m_pRSTWorkingCopy->replaceRuleSet(index,pRS_New,true);
	//pRS_Current=NULL;
	m_pListBoxRulesets->SetString(index,pRS_New->getName());
	m_pListBoxRulesets->SetSelection(index);

	_enable_file_ruleset_fields();
}

void OptionsDlg::onButtonEvent_ruleset_delete(wxCommandEvent & /*e*/)
{
	int index = m_pListBoxRulesets->GetSelection();
	if (index == wxNOT_FOUND)
		return;

	int limit = m_pRSTWorkingCopy->getCountRuleSets();
	if (index >= limit)		// should not happen
		return;

	m_pRSTWorkingCopy->deleteRuleSet(index);
	m_pListBoxRulesets->Delete(index);

	_enable_file_ruleset_fields();
}

void OptionsDlg::onButtonEvent_ruleset_clone(wxCommandEvent & /*e*/)
{
	int index = m_pListBoxRulesets->GetSelection();
	if (index == wxNOT_FOUND)
		return;

	int limit = m_pRSTWorkingCopy->getCountRuleSets();
	if (index >= limit)		// should not happen
		return;

	const rs_ruleset * pRS_Current = m_pRSTWorkingCopy->getNthRuleSet(index);
	rs_ruleset * pRS_New = pRS_Current->clone();

	m_pRSTWorkingCopy->addRuleSet(pRS_New);
	int index_new = m_pListBoxRulesets->Append(pRS_New->getName());
	m_pListBoxRulesets->SetSelection(index_new);

	_enable_file_ruleset_fields();
}

void OptionsDlg::onButtonEvent_ruleset_moveup(wxCommandEvent & /*e*/)
{
	int index = m_pListBoxRulesets->GetSelection();
	if (index == wxNOT_FOUND)
		return;

	if (index == 0)			// first item in list can't be moved up
		return;

	int limit = m_pRSTWorkingCopy->getCountRuleSets();
	if (index >= limit)		// should not happen
		return;

	const rs_ruleset * pRS_Current = m_pRSTWorkingCopy->getNthRuleSet(index);

	int index_new = index - 1;

	m_pRSTWorkingCopy->moveRuleSetUpOne(index);
	m_pListBoxRulesets->Delete(index);
	m_pListBoxRulesets->Insert(pRS_Current->getName(),index_new);
	m_pListBoxRulesets->SetSelection(index_new);
	
	_enable_file_ruleset_fields();
}

void OptionsDlg::onButtonEvent_ruleset_movedown(wxCommandEvent & /*e*/)
{
	int index = m_pListBoxRulesets->GetSelection();
	if (index == wxNOT_FOUND)
		return;

	int limit = m_pRSTWorkingCopy->getCountRuleSets();
	if (index >= limit)		// should not happen
		return;

	if (index+1 == limit)	// last item in list can't be moved down
		return;

	const rs_ruleset * pRS_Current = m_pRSTWorkingCopy->getNthRuleSet(index);

	int index_new = index + 1;

	m_pRSTWorkingCopy->moveRuleSetDownOne(index);
	m_pListBoxRulesets->Delete(index);
	m_pListBoxRulesets->Insert(pRS_Current->getName(),index_new);
	m_pListBoxRulesets->SetSelection(index_new);
	
	_enable_file_ruleset_fields();
}

void OptionsDlg::onButtonEvent_ruleset_edit_default_ruleset(wxCommandEvent & /*e*/)
{
	const rs_ruleset * pOldRSDefault = m_pRSTWorkingCopy->getDefaultRuleSet();
	
	rs_ruleset_dlg__edit_default dlg(this,pOldRSDefault);

	rs_ruleset * pNewRSDefault = NULL;
	int result = dlg.run(&pNewRSDefault);

	if (result != wxID_OK)
		return;

	if (pOldRSDefault->isEqual(pNewRSDefault))		// if they just clicked OK withing changing anything,
	{												// we don't really need to do anything.
		delete pNewRSDefault;
		return;
	}
	
	m_pRSTWorkingCopy->replaceDefaultRuleSet(pNewRSDefault);
	//pOldRSDefault=NULL;

	_enable_file_ruleset_fields();
}

//////////////////////////////////////////////////////////////////

void OptionsDlg::onButtonEvent_fonts_file(wxCommandEvent & /*e*/)
{
	// raise the font chooser dialog and let the user choose a font.
	
	wxFontData fd;

	fd.SetInitialFont(*m_pFont_File);

	fd.EnableEffects(false);
	fd.SetAllowSymbols(false);
	fd.SetShowHelp(false);

	wxFontDialog dlg(this,fd);

	int result = dlg.ShowModal();

	if (result != wxID_OK)
		return;

	// extract the chosen font from the dialog.

	delete m_pFont_File;

	m_pFont_File     = new wxFont( dlg.GetFontData().GetChosenFont() );
	m_bFontFileDirty = true;

	_set_static_font_fields();
}

#if VIEW_FOLDER_LIST_CTRL_USE_FONTS
void OptionsDlg::onButtonEvent_fonts_folder(wxCommandEvent & /*e*/)
{
	// raise the font chooser dialog and let the user choose a font.
	
	wxFontData fd;

	fd.SetInitialFont(*m_pFont_Folder);

	fd.EnableEffects(false);
	fd.SetAllowSymbols(false);
	fd.SetShowHelp(false);

	wxFontDialog dlg(this,fd);

	int result = dlg.ShowModal();

	if (result != wxID_OK)
		return;

	// extract the chosen font from the dialog.

	delete m_pFont_Folder;

	m_pFont_Folder     = new wxFont( dlg.GetFontData().GetChosenFont() );
	m_bFontFolderDirty = true;

	_set_static_font_fields();
}
#endif

void OptionsDlg::onButtonEvent_fonts_printer_file(wxCommandEvent & /*e*/)
{
	// raise the font chooser dialog and let the user choose a font.
	
	wxFontData fd;

	fd.SetInitialFont(*m_pFont_PrinterFile);

	fd.EnableEffects(false);
	fd.SetAllowSymbols(false);
	fd.SetShowHelp(false);

	wxFontDialog dlg(this,fd);

	int result = dlg.ShowModal();

	if (result != wxID_OK)
		return;

	// extract the chosen font from the dialog.

	delete m_pFont_PrinterFile;

	m_pFont_PrinterFile = new wxFont( dlg.GetFontData().GetChosenFont() );
	m_bFontPrinterFileDirty = true;

	_set_static_font_fields();
}

void OptionsDlg::onButtonEvent_fonts_printer_folder(wxCommandEvent & /*e*/)
{
	// raise the font chooser dialog and let the user choose a font.
	
	wxFontData fd;

	fd.SetInitialFont(*m_pFont_PrinterFolder);

	fd.EnableEffects(false);
	fd.SetAllowSymbols(false);
	fd.SetShowHelp(false);

	wxFontDialog dlg(this,fd);

	int result = dlg.ShowModal();

	if (result != wxID_OK)
		return;

	// extract the chosen font from the dialog.

	delete m_pFont_PrinterFolder;

	m_pFont_PrinterFolder = new wxFont( dlg.GetFontData().GetChosenFont() );
	m_bFontPrinterFolderDirty = true;

	_set_static_font_fields();
}

//////////////////////////////////////////////////////////////////

void OptionsDlg::onCheckEvent_enable_autosave(wxCommandEvent & e)
{
	m_bMisc_EnableAutoSave = e.IsChecked();

	m_pTextAutoSaveIntervalLabel->Enable(m_bMisc_EnableAutoSave);
	m_pSpinAutoSaveInterval->Enable(m_bMisc_EnableAutoSave);
}

//////////////////////////////////////////////////////////////////

void OptionsDlg::onRadioEvent_DetailLevel(wxCommandEvent & e)
{
	m_iMisc_DetailLevel = e.GetInt();

	_enable_threshold_fields();
}

void OptionsDlg::_enable_threshold_fields(void)
{
	bool bEnableIntralineThreshold = (m_iMisc_DetailLevel == DE_DETAIL_LEVEL__CHAR);

	m_pTextIntralineThresholdLabel->Enable(bEnableIntralineThreshold);
	m_pSpinIntralineThreshold->Enable(bEnableIntralineThreshold);

	m_pRadioMultiLineDetailLevel->Enable(bEnableIntralineThreshold);
}

//////////////////////////////////////////////////////////////////

void OptionsDlg::onCheckEvent_use_file_filters(wxCommandEvent & e)
{
	m_bFolderFilters_UseFileFilter = e.IsChecked();

	m_pTextFileFilter->Enable(m_bFolderFilters_UseFileFilter);
}

void OptionsDlg::onCheckEvent_use_folder_filters(wxCommandEvent & e)
{
	m_bFolderFilters_UseFolderFilter = e.IsChecked();

	m_pTextFolderFilter->Enable(m_bFolderFilters_UseFolderFilter);
}

void OptionsDlg::onCheckEvent_use_full_filename_filters(wxCommandEvent & e)
{
	m_bFolderFilters_UseFullFilenameFilter = e.IsChecked();

	m_pTextFullFilenameFilter->Enable(m_bFolderFilters_UseFullFilenameFilter);
}

//////////////////////////////////////////////////////////////////

void OptionsDlg::onCheckEvent_enable_external_tools(wxCommandEvent & e)
{
	m_bExternalTools_EnableExternalTools = e.IsChecked();

	_enable_external_tools_fields();
}

void OptionsDlg::onListBoxEvent_listbox_external_tools(wxCommandEvent & /*e*/)
{
	_enable_external_tools_fields();
}

void OptionsDlg::onListBoxDClickEvent_listbox_external_tools(wxCommandEvent & e)
{
	onButtonEvent_external_tools_edit(e);
}

void OptionsDlg::onButtonEvent_external_tools_add(wxCommandEvent & /*e*/)
{
	xt_tool_dlg__add dlg(this);

	xt_tool * pNew = NULL;
	int result = dlg.run(&pNew);

	if (result != wxID_OK)
		return;

	m_pXTTWorkingCopy->addTool(pNew);		// we assume that this is an append()
	int index = m_pListBoxExternalTools->Append(pNew->getName());
	m_pListBoxExternalTools->SetSelection(index);

	_enable_external_tools_fields();
}

void OptionsDlg::onButtonEvent_external_tools_edit(wxCommandEvent & /*e*/)
{
	int index = m_pListBoxExternalTools->GetSelection();
	if (index == wxNOT_FOUND)
	{
		// if nothing was selected in this listbox, we shouldn't
		// do anything.  (ideally, we should have disabled the
		// EDIT button, but there are times when we get this --
		// like when nothing is selected when we first come up.)

		return;
	}

	int limit = m_pXTTWorkingCopy->getCountTools();
	if (index >= limit)		// should not happen
		return;
	
	const xt_tool * pXT_Current = m_pXTTWorkingCopy->getNthTool(index);
	
	xt_tool_dlg__edit dlg(this,pXT_Current);

	xt_tool * pXT_New = NULL;
	int result = dlg.run(&pXT_New);

	if (result != wxID_OK)
		return;

	if (pXT_Current->isEqual(pXT_New))				// if they just clicked OK without changing anything,
	{												// we don't really need to do anything.
		delete pXT_New;
		return;
	}
	
	m_pXTTWorkingCopy->replaceTool(index,pXT_New,true);
	//pRS_Current=NULL;
	m_pListBoxExternalTools->SetString(index,pXT_New->getName());
	m_pListBoxExternalTools->SetSelection(index);

	_enable_external_tools_fields();
}

void OptionsDlg::onButtonEvent_external_tools_delete(wxCommandEvent & /*e*/)
{
	int index = m_pListBoxExternalTools->GetSelection();
	if (index == wxNOT_FOUND)
		return;

	int limit = m_pXTTWorkingCopy->getCountTools();
	if (index >= limit)		// should not happen
		return;

	m_pXTTWorkingCopy->deleteTool(index);
	m_pListBoxExternalTools->Delete(index);

	_enable_external_tools_fields();
}

void OptionsDlg::onButtonEvent_external_tools_clone(wxCommandEvent & /*e*/)
{
	int index = m_pListBoxExternalTools->GetSelection();
	if (index == wxNOT_FOUND)
		return;

	int limit = m_pXTTWorkingCopy->getCountTools();
	if (index >= limit)		// should not happen
		return;

	const xt_tool * pXT_Current = m_pXTTWorkingCopy->getNthTool(index);
	xt_tool * pXT_New = pXT_Current->clone();

	m_pXTTWorkingCopy->addTool(pXT_New);
	int index_new = m_pListBoxExternalTools->Append(pXT_New->getName());
	m_pListBoxExternalTools->SetSelection(index_new);

	_enable_external_tools_fields();
}

void OptionsDlg::onButtonEvent_external_tools_moveup(wxCommandEvent & /*e*/)
{
	int index = m_pListBoxExternalTools->GetSelection();
	if (index == wxNOT_FOUND)
		return;

	if (index == 0)			// first item in list can't be moved up
		return;

	int limit = m_pXTTWorkingCopy->getCountTools();
	if (index >= limit)		// should not happen
		return;

	const xt_tool * pXT_Current = m_pXTTWorkingCopy->getNthTool(index);

	int index_new = index - 1;

	m_pXTTWorkingCopy->moveToolUpOne(index);
	m_pListBoxExternalTools->Delete(index);
	m_pListBoxExternalTools->Insert(pXT_Current->getName(),index_new);
	m_pListBoxExternalTools->SetSelection(index_new);
	
	_enable_external_tools_fields();
}

void OptionsDlg::onButtonEvent_external_tools_movedown(wxCommandEvent & /*e*/)
{
	int index = m_pListBoxExternalTools->GetSelection();
	if (index == wxNOT_FOUND)
		return;

	int limit = m_pXTTWorkingCopy->getCountTools();
	if (index >= limit)		// should not happen
		return;

	if (index+1 == limit)	// last item in list can't be moved down
		return;

	const xt_tool * pXT_Current = m_pXTTWorkingCopy->getNthTool(index);

	int index_new = index + 1;

	m_pXTTWorkingCopy->moveToolDownOne(index);
	m_pListBoxExternalTools->Delete(index);
	m_pListBoxExternalTools->Insert(pXT_Current->getName(),index_new);
	m_pListBoxExternalTools->SetSelection(index_new);
	
	_enable_external_tools_fields();
}

//////////////////////////////////////////////////////////////////

void OptionsDlg::onCheckEvent_SoftMatch_SimpleIgnoreWhitespace(wxCommandEvent & e)
{
	m_bSoftMatchSimpleIgnoreWhitespace = e.IsChecked();

	_enable_softmatch_fields();
}

void OptionsDlg::onRadioEvent_SoftMatch_Mode(wxCommandEvent & e)
{
	m_iSoftMatch_Mode = e.GetInt();

	_enable_softmatch_fields();
}

void OptionsDlg::onButtonEvent_SoftMatch_Help(wxCommandEvent & /*e*/)
{
	util_help_dlg dlg(this,
					  wxGetTranslation(
						  L"When a Folder Diff Window scans the file system, it tries to QUICKLY compare each pair of files and\n"
						  L"determine if they are byte-for-byte IDENTICAL.\n"
						  L"\n"
						  L"For files that are different, DiffMerge can optionally look and see if they are EQUIVALENT by ignoring\n"
						  L"such things as line terminataion and whitespace.  If equivalence testing fails or is disabled, the\n"
						  L"files are marked DIFFERENT.\n"
						  L"\n"
						  L"The goal of equivalence testing in the Folder Diff Window is to reduce the number of files marked\n"
						  L"DIFFERENT that when opened in a File Diff Window appear to be equivalent due to Ruleset settings.\n"
						  L"\n"
						  L"Equivalence testing is optional because it can only INCREASE the time needed to scan the file system\n"
						  L"and file system scanning is already quite time consuming.  For this reason, there are 2 levels of\n"
						  L"equivalence testing, each with different levels of complexity:\n"
						  L"\n"
						  L"SIMPLE EQUIVALENCE - Simple equivalence attempts to be a quick approximation.  It ignores simple\n"
						  L"differences in line termination and whitespace in TEXT files.  It does not look at letter case.  It\n"
						  L"does not handle character encoding issues (nor import to Unicode).  It assumes that files are in an\n"
						  L"8-bit encoding compatible with US-ASCII.  It is only attempted for files with a suffix in the given list.\n"
						  L"\n"
						  L"RULESET EQUIVALENCE - Ruleset equivalence is a more thorough attempt.  It uses MOST of the\n"
						  L"settings in the corresponding Ruleset for eair pair of files.  This includes ignoring differences in\n"
						  L"character encoding (by importing the files into Unicode), line termination, whitespace, and case.  It\n"
						  L"also strips out lines matched by the Lines to Omit settings (which may help when Vault or RCS Keyword\n"
						  L"Expansion is in use).  It DOES NOT use any of the Content Handling 'Context' settings.\n"
						  L"\n"
						  L"For Ruleset Equivalence to work, Rulesets and automatic suffix matching must be enabled.  For an\n"
						  L"individual Ruleset, character encoding selection must be automatic.  If a Ruleset and character\n"
						  L"encoding cannot be automatically chosen, equivalence testing will either be skipped or the Default\n"
						  L"Ruleset chosen instead.  As a performance consideration, you may want to set an upper file size\n"
						  L"limit for ruleset equivalence testing.\n"
						  L"\n"
						  L"DO NOT enable the Default Ruleset if you have binary files in your folders (since the attempt to\n"
						  L"import the files into Unicode will generally fail and just waste time)."
						  ));
	dlg.ShowModal();
}

//////////////////////////////////////////////////////////////////

wxPanel * OptionsDlg::createPanel_Preview(wxNotebook * pNotebook,
										  TNdxPreview ndxPreview,
										  _preview ** ppPreview)
{
	wxPanel * pPanel = new wxPanel(pNotebook);
	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		_preview * pPreview = new _preview(pPanel,this,ndxPreview);

		*ppPreview = pPreview;
		
		vSizerTop->Add(pPreview, VAR, wxGROW, 0);
	}
	pPanel->SetSizer(vSizerTop);
	vSizerTop->Fit(pPanel);

	return pPanel;
}

//////////////////////////////////////////////////////////////////

#if defined(__WXMAC__)
void OptionsDlg::_preview::onPaintEvent(wxPaintEvent & /*e*/)
{
	// WXBUG: __WXMAC__:
	// wxScrolledWindow has a default onPaint event handler that is
	// supposed to adjust the origin of the DC and call OnDraw().
	// it doesn't appear to work on the MAC -- at least as of 2.6.3.
	//
	// TODO revisit this with 2.8.0 and see if we can use the builtin
	// TODO code and remove this event handler completely.

	wxPaintDC dc(this);
	DoPrepareDC(dc);

	OnDraw(dc);
}
#endif

void OptionsDlg::drawPreview(_preview * pPreview, wxDC & dc, TNdxPreview ndxPreview)
{
	int xPixelsClientSize, yPixelsClientSize;
	int x, y, panelWidth;
	wxCoord charWidth, charHeight;

	pPreview->GetClientSize(&xPixelsClientSize,&yPixelsClientSize);
	dc.SetFont(*m_pFont_File);
	dc.GetTextExtent(_T("X"),&charWidth,&charHeight);

	// use overall window-bg to fill window

	wxBrush brushWindowBG(m_clrFileColorWindowBG);
	dc.SetBackground(brushWindowBG);
	dc.Clear();

	wxPen penWindowBG(m_clrFileColorWindowBG);

	// background will be specified when text is drawn

	dc.SetBackgroundMode(wxSOLID);

	// draw example text depending on the ndx given.

	switch (ndxPreview)
	{
	default:
		wxASSERT_MSG((0),_T("Coding Error"));
		return;
		
	case NDX_PREVIEW_FILE_LINE_COLOR_2:		// detail-level line-only
		{
			panelWidth = xPixelsClientSize / 2;

			y = 0;

			x = 0;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorAllEqBG);
			_drawPreviewString(dc,x,y,_T("identical line"),m_clrFileColorAllEqFG,m_clrFileColorAllEqBG);
			y += charHeight;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorNoneEqBG);
			_drawPreviewString(dc,x,y,_T("added line"),m_clrFileColorNoneEqFG,m_clrFileColorNoneEqBG);
			y += charHeight;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorAllEqBG);
			_drawPreviewString(dc,x,y,_T("identical line"),m_clrFileColorAllEqFG,m_clrFileColorAllEqBG);
			y += charHeight;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorNoneEqBG);
			_drawPreviewString(dc,x,y,_T("changed line"),m_clrFileColorNoneEqFG,m_clrFileColorNoneEqBG);
			y += charHeight;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorAllEqBG);
			_drawPreviewString(dc,x,y,_T("identical line"),m_clrFileColorAllEqFG,m_clrFileColorAllEqBG);
			y += charHeight;
			
			y = 0;

			x += panelWidth;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorAllEqBG);
			_drawPreviewString(dc,x,y,_T("identical line"),m_clrFileColorAllEqFG,m_clrFileColorAllEqBG);
			y += charHeight;
			_drawPreviewVoid(dc,x,y,panelWidth,charHeight);
			y += charHeight;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorAllEqBG);
			_drawPreviewString(dc,x,y,_T("identical line"),m_clrFileColorAllEqFG,m_clrFileColorAllEqBG);
			y += charHeight;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorNoneEqBG);
			_drawPreviewString(dc,x,y,_T("line changed"),m_clrFileColorNoneEqFG,m_clrFileColorNoneEqBG);
			y += charHeight;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorAllEqBG);
			_drawPreviewString(dc,x,y,_T("identical line"),m_clrFileColorAllEqFG,m_clrFileColorAllEqBG);
			y += charHeight;

			dc.SetPen(penWindowBG);
			dc.DrawLine(x-1,0,x-1,y);
			dc.DrawLine(x-2,0,x-2,y);
		}
		return;

	case NDX_PREVIEW_FILE_LINE_COLOR_3:
		{
			panelWidth = xPixelsClientSize / 3;

			y = 0;

			x = 0;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorAllEqBG);
			_drawPreviewString(dc,x,y,_T("identical line"),m_clrFileColorAllEqFG,m_clrFileColorAllEqBG);
			y += charHeight;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorSubNotEqualBG);
			_drawPreviewString(dc,x,y,_T("non-matching line"),m_clrFileColorSubNotEqualFG,m_clrFileColorSubNotEqualBG);
			y += charHeight;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorAllEqBG);
			_drawPreviewString(dc,x,y,_T("identical line"),m_clrFileColorAllEqFG,m_clrFileColorAllEqBG);
			y += charHeight;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorSubEqualBG);
			_drawPreviewString(dc,x,y,_T("matching line"),m_clrFileColorSubEqualFG,m_clrFileColorSubEqualBG);
			y += charHeight;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorAllEqBG);
			_drawPreviewString(dc,x,y,_T("identical line"),m_clrFileColorAllEqFG,m_clrFileColorAllEqBG);
			y += charHeight;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorSubNotEqualBG);
			_drawPreviewString(dc,x,y,_T("non-matching line"),m_clrFileColorSubNotEqualFG,m_clrFileColorSubNotEqualBG);
			y += charHeight;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorAllEqBG);
			_drawPreviewString(dc,x,y,_T("identical line"),m_clrFileColorAllEqFG,m_clrFileColorAllEqBG);
			y += charHeight;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorConflictBG);
			_drawPreviewString(dc,x,y,_T("conflict line"),m_clrFileColorConflictFG,m_clrFileColorConflictBG);
			y += charHeight;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorAllEqBG);
			_drawPreviewString(dc,x,y,_T("identical line"),m_clrFileColorAllEqFG,m_clrFileColorAllEqBG);
			y += charHeight;
			
			y = 0;

			x += panelWidth;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorAllEqBG);
			_drawPreviewString(dc,x,y,_T("identical line"),m_clrFileColorAllEqFG,m_clrFileColorAllEqBG);
			y += charHeight;
			_drawPreviewVoid(dc,x,y,panelWidth,charHeight);
			y += charHeight;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorAllEqBG);
			_drawPreviewString(dc,x,y,_T("identical line"),m_clrFileColorAllEqFG,m_clrFileColorAllEqBG);
			y += charHeight;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorSubEqualBG);
			_drawPreviewString(dc,x,y,_T("matching line"),m_clrFileColorSubEqualFG,m_clrFileColorSubEqualBG);
			y += charHeight;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorAllEqBG);
			_drawPreviewString(dc,x,y,_T("identical line"),m_clrFileColorAllEqFG,m_clrFileColorAllEqBG);
			y += charHeight;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorSubEqualBG);
			_drawPreviewString(dc,x,y,_T("matching line"),m_clrFileColorSubEqualFG,m_clrFileColorSubEqualBG);
			y += charHeight;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorAllEqBG);
			_drawPreviewString(dc,x,y,_T("identical line"),m_clrFileColorAllEqFG,m_clrFileColorAllEqBG);
			y += charHeight;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorConflictBG);
			_drawPreviewString(dc,x,y,_T("CONFLICT LINE"),m_clrFileColorConflictFG,m_clrFileColorConflictBG);
			y += charHeight;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorAllEqBG);
			_drawPreviewString(dc,x,y,_T("identical line"),m_clrFileColorAllEqFG,m_clrFileColorAllEqBG);
			y += charHeight;

			dc.SetPen(penWindowBG);
			dc.DrawLine(x-1,0,x-1,y);
			dc.DrawLine(x-2,0,x-2,y);

			y = 0;

			x += panelWidth;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorAllEqBG);
			_drawPreviewString(dc,x,y,_T("identical line"),m_clrFileColorAllEqFG,m_clrFileColorAllEqBG);
			y += charHeight;
			_drawPreviewVoid(dc,x,y,panelWidth,charHeight);
			y += charHeight;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorAllEqBG);
			_drawPreviewString(dc,x,y,_T("identical line"),m_clrFileColorAllEqFG,m_clrFileColorAllEqBG);
			y += charHeight;
			_drawPreviewVoid(dc,x,y,panelWidth,charHeight);
			y += charHeight;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorAllEqBG);
			_drawPreviewString(dc,x,y,_T("identical line"),m_clrFileColorAllEqFG,m_clrFileColorAllEqBG);
			y += charHeight;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorSubEqualBG);
			_drawPreviewString(dc,x,y,_T("matching line"),m_clrFileColorSubEqualFG,m_clrFileColorSubEqualBG);
			y += charHeight;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorAllEqBG);
			_drawPreviewString(dc,x,y,_T("identical line"),m_clrFileColorAllEqFG,m_clrFileColorAllEqBG);
			y += charHeight;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorConflictBG);
			_drawPreviewString(dc,x,y,_T("Conflict Line"),m_clrFileColorConflictFG,m_clrFileColorConflictBG);
			y += charHeight;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorAllEqBG);
			_drawPreviewString(dc,x,y,_T("identical line"),m_clrFileColorAllEqFG,m_clrFileColorAllEqBG);
			y += charHeight;

			dc.SetPen(penWindowBG);
			dc.DrawLine(x-1,0,x-1,y);
			dc.DrawLine(x-2,0,x-2,y);
		}
		return;

	case NDX_PREVIEW_FILE_IL_COLOR_2:		// intra-line (lines-and-chars) detail-level
		{
			panelWidth = xPixelsClientSize / 2;

			// left panel

			y = 0;

			x = 0;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorNoneEqBG);
			x += _drawPreviewString(dc,x,y,_T("A"),m_clrFileColorNoneEqFG,m_clrFileColorNoneEqIlBG);
			x += _drawPreviewString(dc,x,y,_T(" important identical"),m_clrFileColorAllEqFG,m_clrFileColorNoneEqBG);
			y += charHeight;

			x = 0;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorNoneEqBG);
			x += _drawPreviewString(dc,x,y,_T("B"),m_clrFileColorNoneEqFG,m_clrFileColorNoneEqIlBG);
			x += _drawPreviewString(dc,x,y,_T(" //"),m_clrFileColorAllEqFG,m_clrFileColorNoneEqBG);
			x += _drawPreviewString(dc,x,y,_T(" unimportant identical"),m_clrFileColorAllEqUnimpFG,m_clrFileColorNoneEqBG);
			y += charHeight;

			x = 0;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorNoneEqBG);
			x += _drawPreviewString(dc,x,y,_T("important"),m_clrFileColorNoneEqFG,m_clrFileColorNoneEqIlBG);
			x += _drawPreviewString(dc,x,y,_T(" change"),m_clrFileColorAllEqFG,m_clrFileColorNoneEqBG);
			y += charHeight;

			x = 0;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorNoneEqBG);
			x += _drawPreviewString(dc,x,y,_T("// "),m_clrFileColorAllEqFG,m_clrFileColorNoneEqBG);
			x += _drawPreviewString(dc,x,y,_T("unimportant"),m_clrFileColorNoneEqUnimpFG,m_clrFileColorNoneEqIlBG);
			x += _drawPreviewString(dc,x,y,_T(" change"),m_clrFileColorAllEqUnimpFG,m_clrFileColorNoneEqBG);
			y += charHeight;

			// right panel

			y = 0;

			x = panelWidth;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorNoneEqBG);
			x += _drawPreviewString(dc,x,y,_T("a"),m_clrFileColorNoneEqFG,m_clrFileColorNoneEqIlBG);
			x += _drawPreviewString(dc,x,y,_T(" important identical"),m_clrFileColorAllEqFG,m_clrFileColorNoneEqBG);
			y += charHeight;

			x = panelWidth;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorNoneEqBG);
			x += _drawPreviewString(dc,x,y,_T("b"),m_clrFileColorNoneEqFG,m_clrFileColorNoneEqIlBG);
			x += _drawPreviewString(dc,x,y,_T(" //"),m_clrFileColorAllEqFG,m_clrFileColorNoneEqBG);
			x += _drawPreviewString(dc,x,y,_T(" unimportant identical"),m_clrFileColorAllEqUnimpFG,m_clrFileColorNoneEqBG);
			y += charHeight;

			x = panelWidth;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorNoneEqBG);
			x += _drawPreviewString(dc,x,y,_T("IMPORTANT"),m_clrFileColorNoneEqFG,m_clrFileColorNoneEqIlBG);
			x += _drawPreviewString(dc,x,y,_T(" change"),m_clrFileColorAllEqFG,m_clrFileColorNoneEqBG);
			y += charHeight;

			x = panelWidth;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorNoneEqBG);
			x += _drawPreviewString(dc,x,y,_T("// "),m_clrFileColorAllEqFG,m_clrFileColorNoneEqBG);
			x += _drawPreviewString(dc,x,y,_T("UNIMPORTANT"),m_clrFileColorNoneEqUnimpFG,m_clrFileColorNoneEqIlBG);
			x += _drawPreviewString(dc,x,y,_T(" change"),m_clrFileColorAllEqUnimpFG,m_clrFileColorNoneEqBG);
			y += charHeight;

			dc.SetPen(penWindowBG);
			dc.DrawLine(panelWidth-1,0,panelWidth-1,y);
			dc.DrawLine(panelWidth-2,0,panelWidth-2,y);
		}
		return;

	case NDX_PREVIEW_FILE_IL_COLOR_3:		// intra-line (lines-and-chars) detail-level
		{
			panelWidth = xPixelsClientSize / 3;

			// left panel

			y = 0;

			x = 0;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorSubNotEqualBG);
			x += _drawPreviewString(dc,x,y,_T("a"),m_clrFileColorSubNotEqualFG,m_clrFileColorSubNotEqualIlBG);
			x += _drawPreviewString(dc,x,y,_T(" important identical"),m_clrFileColorAllEqFG,m_clrFileColorSubNotEqualBG);
			y += charHeight;

			x = 0;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorSubNotEqualBG);
			x += _drawPreviewString(dc,x,y,_T("b"),m_clrFileColorSubNotEqualFG,m_clrFileColorSubNotEqualIlBG);
			x += _drawPreviewString(dc,x,y,_T(" //"),m_clrFileColorAllEqFG,m_clrFileColorSubNotEqualBG);
			x += _drawPreviewString(dc,x,y,_T(" unimportant identical"),m_clrFileColorAllEqUnimpFG,m_clrFileColorSubNotEqualBG);
			y += charHeight;

			x = 0;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorSubNotEqualBG);
			x += _drawPreviewString(dc,x,y,_T("IMPORTANT Non-"),m_clrFileColorSubNotEqualFG,m_clrFileColorSubNotEqualIlBG);
			x += _drawPreviewString(dc,x,y,_T("Matching change"),m_clrFileColorAllEqFG,m_clrFileColorSubNotEqualBG);
			y += charHeight;

			x = 0;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorSubNotEqualBG);
			x += _drawPreviewString(dc,x,y,_T("// "),m_clrFileColorAllEqFG,m_clrFileColorSubNotEqualBG);
			x += _drawPreviewString(dc,x,y,_T("UNIMPORTANT Non-"),m_clrFileColorSubUnimpFG,m_clrFileColorSubNotEqualIlBG);
			x += _drawPreviewString(dc,x,y,_T("Matching change"),m_clrFileColorAllEqUnimpFG,m_clrFileColorSubNotEqualBG);
			y += charHeight;

			x = 0;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorAllEqBG);
			_drawPreviewString(dc,x,y,_T("identical line"),m_clrFileColorAllEqFG,m_clrFileColorAllEqBG);
			y += charHeight;

			x = 0;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorConflictBG);
			x += _drawPreviewString(dc,x,y,_T("conflict "),m_clrFileColorAllEqFG,m_clrFileColorConflictBG);
			x += _drawPreviewString(dc,x,y,_T("AAAA"),m_clrFileColorConflictFG,m_clrFileColorConflictIlBG);
			x += _drawPreviewString(dc,x,y,_T(" // "),m_clrFileColorAllEqFG,m_clrFileColorConflictBG);
			x += _drawPreviewString(dc,x,y,_T("aaaa"),m_clrFileColorConflictUnimpFG,m_clrFileColorConflictIlBG);
			y += charHeight;

			x = 0;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorConflictBG);
			x += _drawPreviewString(dc,x,y,_T("IMPORTANT Non-"),m_clrFileColorSubNotEqualFG,m_clrFileColorSubNotEqualIlBG);
			x += _drawPreviewString(dc,x,y,_T("Matching change"),m_clrFileColorAllEqFG,m_clrFileColorConflictBG);
			y += charHeight;

			x = 0;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorConflictBG);
			x += _drawPreviewString(dc,x,y,_T("// "),m_clrFileColorAllEqFG,m_clrFileColorConflictBG);
			x += _drawPreviewString(dc,x,y,_T("UNIMPORTANT Non-"),m_clrFileColorSubUnimpFG,m_clrFileColorSubNotEqualIlBG);
			x += _drawPreviewString(dc,x,y,_T("Matching change"),m_clrFileColorAllEqUnimpFG,m_clrFileColorConflictBG);
			y += charHeight;

			// center panel

			y = 0;

			x = panelWidth;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorSubEqualBG);
			x += _drawPreviewString(dc,x,y,_T("A"),m_clrFileColorSubEqualFG,m_clrFileColorSubEqualIlBG);
			x += _drawPreviewString(dc,x,y,_T(" important identical"),m_clrFileColorAllEqFG,m_clrFileColorSubEqualBG);
			y += charHeight;

			x = panelWidth;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorSubEqualBG);
			x += _drawPreviewString(dc,x,y,_T("B"),m_clrFileColorSubEqualFG,m_clrFileColorSubEqualIlBG);
			x += _drawPreviewString(dc,x,y,_T(" //"),m_clrFileColorAllEqFG,m_clrFileColorSubEqualBG);
			x += _drawPreviewString(dc,x,y,_T(" unimportant identical"),m_clrFileColorAllEqUnimpFG,m_clrFileColorSubEqualBG);
			y += charHeight;

			x = panelWidth;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorSubEqualBG);
			x += _drawPreviewString(dc,x,y,_T("important "),m_clrFileColorSubEqualFG,m_clrFileColorSubEqualIlBG);
			x += _drawPreviewString(dc,x,y,_T("Matching change"),m_clrFileColorAllEqFG,m_clrFileColorSubEqualBG);
			y += charHeight;

			x = panelWidth;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorSubEqualBG);
			x += _drawPreviewString(dc,x,y,_T("// "),m_clrFileColorAllEqFG,m_clrFileColorSubEqualBG);
			x += _drawPreviewString(dc,x,y,_T("unimportant "),m_clrFileColorSubUnimpFG,m_clrFileColorSubEqualIlBG);
			x += _drawPreviewString(dc,x,y,_T("Matching change"),m_clrFileColorAllEqUnimpFG,m_clrFileColorSubEqualBG);
			y += charHeight;

			x = panelWidth;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorAllEqBG);
			_drawPreviewString(dc,x,y,_T("identical line"),m_clrFileColorAllEqFG,m_clrFileColorAllEqBG);
			y += charHeight;

			x = panelWidth;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorConflictBG);
			x += _drawPreviewString(dc,x,y,_T("conflict "),m_clrFileColorAllEqFG,m_clrFileColorConflictBG);
			x += _drawPreviewString(dc,x,y,_T("BBBB"),m_clrFileColorConflictFG,m_clrFileColorConflictIlBG);
			x += _drawPreviewString(dc,x,y,_T(" // "),m_clrFileColorAllEqFG,m_clrFileColorConflictBG);
			x += _drawPreviewString(dc,x,y,_T("bbbb"),m_clrFileColorConflictUnimpFG,m_clrFileColorConflictIlBG);
			y += charHeight;

			x = panelWidth;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorConflictBG);
			x += _drawPreviewString(dc,x,y,_T("important "),m_clrFileColorSubEqualFG,m_clrFileColorSubEqualIlBG);
			x += _drawPreviewString(dc,x,y,_T("Matching change"),m_clrFileColorAllEqFG,m_clrFileColorConflictBG);
			y += charHeight;

			x = panelWidth;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorConflictBG);
			x += _drawPreviewString(dc,x,y,_T("// "),m_clrFileColorAllEqFG,m_clrFileColorConflictBG);
			x += _drawPreviewString(dc,x,y,_T("unimportant "),m_clrFileColorSubUnimpFG,m_clrFileColorSubEqualIlBG);
			x += _drawPreviewString(dc,x,y,_T("Matching change"),m_clrFileColorAllEqUnimpFG,m_clrFileColorConflictBG);
			y += charHeight;

			dc.SetPen(penWindowBG);
			dc.DrawLine(panelWidth-1,0,panelWidth-1,y);
			dc.DrawLine(panelWidth-2,0,panelWidth-2,y);

			// right panel

			y = 0;

			x = 2*panelWidth;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorSubEqualBG);
			x += _drawPreviewString(dc,x,y,_T("A"),m_clrFileColorSubEqualFG,m_clrFileColorSubEqualIlBG);
			x += _drawPreviewString(dc,x,y,_T(" important identical"),m_clrFileColorAllEqFG,m_clrFileColorSubEqualBG);
			y += charHeight;

			x = 2*panelWidth;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorSubEqualBG);
			x += _drawPreviewString(dc,x,y,_T("B"),m_clrFileColorSubEqualFG,m_clrFileColorSubEqualIlBG);
			x += _drawPreviewString(dc,x,y,_T(" //"),m_clrFileColorAllEqFG,m_clrFileColorSubEqualBG);
			x += _drawPreviewString(dc,x,y,_T(" unimportant identical"),m_clrFileColorAllEqUnimpFG,m_clrFileColorSubEqualBG);
			y += charHeight;

			x = 2*panelWidth;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorSubEqualBG);
			x += _drawPreviewString(dc,x,y,_T("important "),m_clrFileColorSubEqualFG,m_clrFileColorSubEqualIlBG);
			x += _drawPreviewString(dc,x,y,_T("Matching change"),m_clrFileColorAllEqFG,m_clrFileColorSubEqualBG);
			y += charHeight;

			x = 2*panelWidth;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorSubEqualBG);
			x += _drawPreviewString(dc,x,y,_T("// "),m_clrFileColorAllEqFG,m_clrFileColorSubEqualBG);
			x += _drawPreviewString(dc,x,y,_T("unimportant "),m_clrFileColorSubUnimpFG,m_clrFileColorSubEqualIlBG);
			x += _drawPreviewString(dc,x,y,_T("Matching change"),m_clrFileColorAllEqUnimpFG,m_clrFileColorSubEqualBG);
			y += charHeight;

			x = 2*panelWidth;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorAllEqBG);
			_drawPreviewString(dc,x,y,_T("identical line"),m_clrFileColorAllEqFG,m_clrFileColorAllEqBG);
			y += charHeight;

			x = 2*panelWidth;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorConflictBG);
			x += _drawPreviewString(dc,x,y,_T("conflict "),m_clrFileColorAllEqFG,m_clrFileColorConflictBG);
			x += _drawPreviewString(dc,x,y,_T("CCCC"),m_clrFileColorConflictFG,m_clrFileColorConflictIlBG);
			x += _drawPreviewString(dc,x,y,_T(" // "),m_clrFileColorAllEqFG,m_clrFileColorConflictBG);
			x += _drawPreviewString(dc,x,y,_T("cccc"),m_clrFileColorConflictUnimpFG,m_clrFileColorConflictIlBG);
			y += charHeight;

			x = 2*panelWidth;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorConflictBG);
			x += _drawPreviewString(dc,x,y,_T("important "),m_clrFileColorSubEqualFG,m_clrFileColorSubEqualIlBG);
			x += _drawPreviewString(dc,x,y,_T("Matching change"),m_clrFileColorAllEqFG,m_clrFileColorConflictBG);
			y += charHeight;

			x = 2*panelWidth;
			_drawPreviewRectangle(dc,x,y,panelWidth,charHeight,m_clrFileColorConflictBG);
			x += _drawPreviewString(dc,x,y,_T("// "),m_clrFileColorAllEqFG,m_clrFileColorConflictBG);
			x += _drawPreviewString(dc,x,y,_T("unimportant "),m_clrFileColorSubUnimpFG,m_clrFileColorSubEqualIlBG);
			x += _drawPreviewString(dc,x,y,_T("Matching change"),m_clrFileColorAllEqUnimpFG,m_clrFileColorConflictBG);
			y += charHeight;

			dc.SetPen(penWindowBG);
			dc.DrawLine(2*panelWidth-1,0,2*panelWidth-1,y);
			dc.DrawLine(2*panelWidth-2,0,2*panelWidth-2,y);
		}
		return;

	}
}

//////////////////////////////////////////////////////////////////

void OptionsDlg::_drawPreviewVoid(wxDC & dc,
								  int x, int y, int width, int height)
{
	wxPen penNone(*wxBLACK,1,wxTRANSPARENT);		// a "no pen" pen -- needed for DrawRectangle() when we don't want borders
	wxBrush brushHatch(m_clrFileColorVoidFG, wxBDIAGONAL_HATCH);
	wxBrush brushVoidBG(m_clrFileColorVoidBG);

	dc.SetPen(penNone);
	dc.SetBrush(brushHatch);
	dc.SetBackground(brushVoidBG);

	dc.DrawRectangle(x,y,width,height);
}

void OptionsDlg::_drawPreviewRectangle(wxDC & dc,
									   int x, int y, int width, int height,
									   const wxColor & clr)
{
	wxPen penNone(*wxBLACK,1,wxTRANSPARENT);		// a "no pen" pen -- needed for DrawRectangle() when we don't want borders
	wxBrush brush(clr);

	dc.SetPen(penNone);
	dc.SetBrush(brush);

	dc.DrawRectangle(x,y,width,height);
}

int OptionsDlg::_drawPreviewString(wxDC & dc,
								   int x, int y, const wxString & str,
								   const wxColor & clrFG, const wxColor & clrBG)
{
	dc.SetTextForeground(clrFG);
	dc.SetTextBackground(clrBG);

	dc.DrawText(str,x,y);

	int wStr, hStr;
	dc.GetTextExtent(str,&wStr,&hStr);

	return wStr;
}
	
//////////////////////////////////////////////////////////////////

void OptionsDlg::setPreviewNrLines(_preview * pPreview, TNdxPreview ndxPreview)
{
	static int nrLines[/*TNdxPreview*/] = {	5,	// NDX_PREVIEW_FILE_LINE_COLOR_2
											9,	// NDX_PREVIEW_FILE_LINE_COLOR_3
											4,	// NDX_PREVIEW_FILE_IL_COLOR_2
											8,	// NDX_PREVIEW_FILE_IL_COLOR_3
	};
	wxASSERT_MSG( (ndxPreview < (TNdxPreview)NrElements(nrLines)), _T("Coding Error") );

	int nr = nrLines[ndxPreview];

	int xPixelsClientSize, yPixelsClientSize;
	pPreview->GetClientSize(&xPixelsClientSize,&yPixelsClientSize);

	int charWidth,charHeight;
	wxClientDC dc(pPreview);
	dc.SetFont(*m_pFont_File);
	dc.GetTextExtent(_T("X"),&charWidth,&charHeight);

	pPreview->SetScrollRate(1,charHeight);
	pPreview->SetVirtualSize(xPixelsClientSize, nr*charHeight);
}

void OptionsDlg::refreshFilePreviews(void)
{
	if (m_pPreviewFileLineColor2)
		m_pPreviewFileLineColor2->Refresh();
	if (m_pPreviewFileLineColor3)
		m_pPreviewFileLineColor3->Refresh();
	if (m_pPreviewFileILColor2)
		m_pPreviewFileILColor2->Refresh();
	if (m_pPreviewFileILColor3)
		m_pPreviewFileILColor3->Refresh();
}

//////////////////////////////////////////////////////////////////

OptionsDlg::_my_static_bitmap::_my_static_bitmap(wxWindow * pParent,
												 OptionsDlg * pDlg,
												 wxColor * pClr)
	: wxStaticBitmap(),
	  m_pParent(pParent),
	  m_pDlg(pDlg),
	  m_pClr(pClr)
{
	set_bitmap(true);
}

void OptionsDlg::_my_static_bitmap::set_bitmap(bool bCreate)
{
	int w = 60;
	int h = 24;
	
	wxBitmap bm(w,h);
	wxMemoryDC dc;
	dc.SelectObject(bm);

	wxBrush brush(*m_pClr);

	dc.SetPen(*wxBLACK_PEN);
	dc.SetBrush(brush);
	
	dc.DrawRectangle(0,0,w,h);

	if (bCreate)
		wxStaticBitmap::Create(m_pParent,wxID_ANY,bm);
	else
		wxStaticBitmap::SetBitmap(bm);
}

void OptionsDlg::refreshFileColorBitmaps(void)
{
	if (m_pBitmapFileColorAllEqFG)
		m_pBitmapFileColorAllEqFG->set_bitmap();
	if (m_pBitmapFileColorNoneEqFG)
		m_pBitmapFileColorNoneEqFG->set_bitmap();
	if (m_pBitmapFileColorNoneEqBG)
		m_pBitmapFileColorNoneEqBG->set_bitmap();
	if (m_pBitmapFileColorSubEqualFG)
		m_pBitmapFileColorSubEqualFG->set_bitmap();
	if (m_pBitmapFileColorSubEqualBG)
		m_pBitmapFileColorSubEqualBG->set_bitmap();
	if (m_pBitmapFileColorSubNotEqualFG)
		m_pBitmapFileColorSubNotEqualFG->set_bitmap();
	if (m_pBitmapFileColorSubNotEqualBG)
		m_pBitmapFileColorSubNotEqualBG->set_bitmap();
	if (m_pBitmapFileColorConflictFG)
		m_pBitmapFileColorConflictFG->set_bitmap();
	if (m_pBitmapFileColorConflictBG)
		m_pBitmapFileColorConflictBG->set_bitmap();
}

//////////////////////////////////////////////////////////////////

#ifdef FEATURE_SHEX
bool OptionsDlg::_is_shex_installed(void)
{
	bool bResult = false;

	HKEY hKey = NULL;
	LONG lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
								_T("SOFTWARE\\SourceGear\\Common\\DiffMerge\\Installer"),
								0,
								KEY_QUERY_VALUE,
								&hKey);
	if (lResult == ERROR_SUCCESS)
	{
		lResult = RegQueryValueEx(hKey,_T("Extension"),NULL,NULL,NULL,NULL);
		bResult = (lResult == ERROR_SUCCESS);
		RegCloseKey(hKey);
	}

	return bResult;
}
	
void OptionsDlg::onButtonEvent_Shex_Help(wxCommandEvent & /*e*/)
{
	util_help_dlg dlg(this,
					  wxGetTranslation(
						  L"Windows Explorer Integration is a feature that allows DiffMerge to add commands to the\n"
						  L"context menu of Windows Explorer.  These commands allow you to launch DiffMerge on the\n"
						  L"selected file(s) or folder(s) directly from Windows Explorer.\n"
						  L"\n"
						  L"Explorer Integration is a feature that is optionally installed during an 'All Users'\n"
						  L"installation.  (It is not installed during a per-user installation.)\n"
						  L"\n"
						  L"Once installed it may be turned on or off for your login.\n"));
	dlg.ShowModal();
}

#endif

//////////////////////////////////////////////////////////////////

static void _scrub_textctrl(wxTextCtrl * pTextCtrl)
{
	wxString strSrc = pTextCtrl->GetLineText(0);
	wxString strMod = strSrc;

	// The user can't directly enter these special chars
	// unless they do a PASTE.

	strMod.Replace(_T("\r\n"),_T(" "),true);
	strMod.Replace(_T("\r"),_T(" "),true);
	strMod.Replace(_T("\n"),_T(" "),true);
	strMod.Replace(_T("\t"),_T(" "),true);

	if (strMod != strSrc)
	{
//		wxLogTrace(wxTRACE_Messages, _T("_scrub_textctrl: [%s] ==> [%s]"), strSrc.wc_str(), strMod.wc_str());

		// if our scrubbing of the input changed the
		// string, re-populate it and move to the end.

		pTextCtrl->SetValue(strMod);
		pTextCtrl->SetInsertionPointEnd();
	}
}

void OptionsDlg::onTextEventChanged__FolderFilters__FileFilter(wxCommandEvent & /*e*/)
{
	_scrub_textctrl( m_pTextFileFilter );
}

void OptionsDlg::onTextEventChanged__FolderFilters__FullFilenameFilter(wxCommandEvent & /*e*/)
{
	_scrub_textctrl( m_pTextFullFilenameFilter );
}

void OptionsDlg::onTextEventChanged__FolderFilters__FolderFilter(wxCommandEvent & /*e*/)
{
	_scrub_textctrl( m_pTextFolderFilter );
}


