// OptionsDlg.h
//////////////////////////////////////////////////////////////////

#ifndef H_OPTIONSDLG_H
#define H_OPTIONSDLG_H

//////////////////////////////////////////////////////////////////

class OptionsDlg : public wxDialog
{
	class _preview;
	class _my_static_bitmap;

public:
	OptionsDlg(wxFrame * pFrame);
	~OptionsDlg(void);

	wxPanel *			createPanel_folder_windows(void);
	wxPanel *			createPanel_folder_filters(void);
	wxPanel *			createPanel_folder_colors(void);
	wxPanel *			createPanel_folder_softmatch(void);
	wxPanel *			createPanel_file_windows(void);
	wxPanel *			createPanel_file_line_colors(void);
	wxPanel *			createPanel_file_il_colors(void);
	wxPanel *			createPanel_file_other_colors(void);
	wxPanel *			createPanel_file_rulesets(void);
	wxPanel *			createPanel_misc(void);
	wxPanel *			createPanel_messages(void);
#ifdef FEATURE_SHEX
	wxPanel *			createPanel_shex(void);
#endif
	wxPanel *			createPanel_external_tools(void);

private:
	wxFrame *			m_pFrame;

	// TODO we can remove this #ifdef once we get the build system updated to 2.8.
#if wxCHECK_VERSION(2,8,0)
	wxTreebook *		m_pBookCtrl;
#else
	wxBookCtrlBase *	m_pBookCtrl;
#endif

	wxPanel *			m_pPanelFolderWindows;
	wxPanel *			m_pPanelFolderFilters;
	wxPanel *			m_pPanelFolderColors;
	wxPanel *			m_pPanelFolderSoftMatch;
	wxPanel *			m_pPanelFileWindows;
	wxPanel *			m_pPanelFileColors;
	wxPanel *			m_pPanelFileLineColors;
	wxPanel *			m_pPanelFileILColors;
	wxPanel *			m_pPanelFileOtherColors;
	wxPanel *			m_pPanelFileRulesets;
	wxPanel *			m_pPanelMisc;
	wxPanel *			m_pPanelMessages;
#ifdef FEATURE_SHEX
	wxPanel *			m_pPanelShEx;
#endif
	wxPanel *			m_pPanelExternalTools;

	wxBitmapButton *	m_pButtonFolderColorDifferentFG,	* m_pButtonFolderColorDifferentBG;
	wxBitmapButton *	m_pButtonFolderColorEqualFG,		* m_pButtonFolderColorEqualBG;
	wxBitmapButton *	m_pButtonFolderColorEquivalentFG,	* m_pButtonFolderColorEquivalentBG;
	wxBitmapButton *	m_pButtonFolderColorFoldersFG,		* m_pButtonFolderColorFoldersBG;
	wxBitmapButton *	m_pButtonFolderColorPeerlessFG,		* m_pButtonFolderColorPeerlessBG;
	wxBitmapButton *	m_pButtonFolderColorErrorFG,		* m_pButtonFolderColorErrorBG;

	wxBitmapButton *	m_pButtonFileColorAllEqFG,          * m_pButtonFileColorAllEqUnimpFG,      * m_pButtonFileColorAllEqBG;
	wxBitmapButton *	m_pButtonFileColorNoneEqFG,         * m_pButtonFileColorNoneEqUnimpFG,     * m_pButtonFileColorNoneEqBG;
	wxBitmapButton *	m_pButtonFileColorSubEqualFG,                                              * m_pButtonFileColorSubEqualBG;
	wxBitmapButton                                          * m_pButtonFileColorSubUnimpFG;
	wxBitmapButton *	m_pButtonFileColorSubNotEqualFG,                                           * m_pButtonFileColorSubNotEqualBG;
	wxBitmapButton *	m_pButtonFileColorConflictFG,       * m_pButtonFileColorConflictUnimpFG,   * m_pButtonFileColorConflictBG;
	wxBitmapButton *	m_pButtonFileColorNoneEqIlBG;
	wxBitmapButton *	m_pButtonFileColorConflictIlBG;
	wxBitmapButton *	m_pButtonFileColorSubEqualIlBG;
	wxBitmapButton *	m_pButtonFileColorSubNotEqualIlBG;

	wxBitmapButton *	m_pButtonFileColorWindowBG;
	wxBitmapButton *	m_pButtonFileColorOmitFG,       * m_pButtonFileColorOmitBG;
	wxBitmapButton *	m_pButtonFileColorEOLUnknownFG;
	wxBitmapButton *	m_pButtonFileColorVoidFG,       * m_pButtonFileColorVoidBG;
	wxBitmapButton *	m_pButtonFileColorLineNrFG,     * m_pButtonFileColorLineNrBG;
	wxBitmapButton *	m_pButtonFileColorCaretFG;
	wxBitmapButton *	m_pButtonFileColorSelectionFG,	* m_pButtonFileColorSelectionBG;

	wxListBox *			m_pListBoxRulesets;
	wxListBox *			m_pListBoxExternalTools;

	wxStaticText *		m_pTextAutoSaveIntervalLabel;
	wxSpinCtrl *		m_pSpinAutoSaveInterval;

	wxStaticText *		m_pTextIntralineThresholdLabel;
	wxSpinCtrl *		m_pSpinIntralineThreshold;
	wxSpinCtrl *		m_pSpinInterlineThreshold;
	wxStaticText *		m_pTextSoftMatchRulesetFileLimitMb;
	wxSpinCtrl *		m_pSpinSoftMatchRulesetFileLimitMb;

	wxTextCtrl *		m_pTextFileFilter;
	wxTextCtrl *		m_pTextFolderFilter;
	wxTextCtrl *		m_pTextFullFilenameFilter;
	wxTextCtrl *		m_pTextSoftMatchSimpleSuffix;
	wxTextCtrl *		m_pTextQuickMatchSuffix;

	wxRadioBox *		m_pRadioMultiLineDetailLevel;

#ifdef FEATURE_SHEX
	wxCheckBox *		m_pCheckShexEnabled;
#endif

	typedef enum {		ID_FOLDER_WINDOWS__PANEL = 100,
						ID_FOLDER_WINDOWS__RESTORE_DEFAULTS,

						ID_FOLDER_FILTERS__PANEL,
						ID_FOLDER_FILTERS__USE_FILE_FILTER,		ID_FOLDER_FILTERS__FILE_FILTER,
						ID_FOLDER_FILTERS__USE_FOLDER_FILTER,	ID_FOLDER_FILTERS__FOLDER_FILTER,
						ID_FOLDER_FILTERS__USE_FULL_FILENAME_FILTER, ID_FOLDER_FILTERS__FULL_FILENAME_FILTER,
						ID_FOLDER_FILTERS__IGNORE_PATTERN_CASE,
						ID_FOLDER_FILTERS__IGNORE_MATCHUP_CASE,
						ID_FOLDER_FILTERS__RESTORE_DEFAULTS,

						ID_FOLDER_COLORS__PANEL,
						ID_FOLDER_COLORS__DIFFERENT_FG,		ID_FOLDER_COLORS__DIFFERENT_BG,
						ID_FOLDER_COLORS__EQUAL_FG,			ID_FOLDER_COLORS__EQUAL_BG,
						ID_FOLDER_COLORS__EQUIVALENT_FG,	ID_FOLDER_COLORS__EQUIVALENT_BG,
						ID_FOLDER_COLORS__FOLDERS_FG,		ID_FOLDER_COLORS__FOLDERS_BG,
						ID_FOLDER_COLORS__PEERLESS_FG,		ID_FOLDER_COLORS__PEERLESS_BG,
						ID_FOLDER_COLORS__ERROR_FG,			ID_FOLDER_COLORS__ERROR_BG,
						ID_FOLDER_COLORS__RESTORE_DEFAULTS,

						ID_FOLDER_SOFTMATCH__PANEL,
						ID_FOLDER_SOFTMATCH__RADIO_MODE,
						ID_FOLDER_SOFTMATCH__SIMPLE_SUFFIX,
						ID_FOLDER_SOFTMATCH__SIMPLE_IGNORE_EOL,
						ID_FOLDER_SOFTMATCH__SIMPLE_IGNORE_WHITESPACE,
						ID_FOLDER_SOFTMATCH__SIMPLE_IGNORE_TAB,
						ID_FOLDER_SOFTMATCH__RULESET_FILE_LIMIT_MB,
						ID_FOLDER_SOFTMATCH__RULESET_ALLOW_DEFAULT,
						ID_FOLDER_SOFTMATCH__HELP,
						ID_FOLDER_SOFTMATCH__RESTORE_DEFAULTS,

						ID_FOLDER_QUICKMATCH__ENABLE,
						ID_FOLDER_QUICKMATCH__SUFFIX,

						ID_FILE_WINDOWS__PANEL,
						ID_FILE_WINDOWS__RESTORE_DEFAULTS,

						ID_FILE_LINE_COLORS__PANEL,
						ID_FILE_COLORS__ALL_EQ_FG,			ID_FILE_COLORS__ALL_EQ_BG,
						ID_FILE_COLORS__NONE_EQ_FG,			ID_FILE_COLORS__NONE_EQ_BG,
						ID_FILE_COLORS__SUB_EQUAL_FG,		ID_FILE_COLORS__SUB_EQUAL_BG,
						ID_FILE_COLORS__SUB_NOTEQUAL_FG,	ID_FILE_COLORS__SUB_NOTEQUAL_BG,
						ID_FILE_COLORS__CONFLICT_FG,		ID_FILE_COLORS__CONFLICT_BG,
						ID_FILE_LINE_COLORS__RESTORE_DEFAULTS,

						ID_FILE_IL_COLORS__PANEL,
						ID_FILE_COLORS__ALL_EQ_UNIMP_FG,
						ID_FILE_COLORS__NONE_EQ_UNIMP_FG,
						ID_FILE_COLORS__SUB_UNIMP_FG,
						ID_FILE_COLORS__CONFLICT_UNIMP_FG,
						ID_FILE_COLORS__NONE_EQ_IL_BG,
						ID_FILE_COLORS__CONFLICT_IL_BG,
						ID_FILE_COLORS__SUB_EQUAL_IL_BG,
						ID_FILE_COLORS__SUB_NOTEQUAL_IL_BG,
						ID_FILE_IL_COLORS__RESTORE_DEFAULTS,

						ID_FILE_OTHER_COLORS__PANEL,
						ID_FILE_COLORS__WINDOW_BG,
						ID_FILE_COLORS__OMIT_FG,			ID_FILE_COLORS__OMIT_BG,
						ID_FILE_COLORS__EOL_UNKNOWN_FG,
						ID_FILE_COLORS__VOID_FG,			ID_FILE_COLORS__VOID_BG,
						ID_FILE_COLORS__LINENR_FG,			ID_FILE_COLORS__LINENR_BG,
						ID_FILE_COLORS__CARET_FG,
						ID_FILE_COLORS__SELECTION_FG,		ID_FILE_COLORS__SELECTION_BG,
						ID_FILE_OTHER_COLORS__RESTORE_DEFAULTS,

						ID_FILE_RULESETS__PANEL,
						ID_FILE_RULESETS__ENABLE_CUSTOM_RULESETS,
						ID_FILE_RULESETS__ENABLE_AUTOMATIC_MATCH,
						ID_FILE_RULESETS__IGNORE_SUFFIX_CASE,
						ID_FILE_RULESETS__REQUIRE_COMPLETE_MATCH,
						ID_FILE_RULESETS__ASK_IF_NO_MATCH,
						ID_FILE_RULESETS__LISTBOX,
						ID_FILE_RULESETS__RULESET_ADD, ID_FILE_RULESETS__RULESET_EDIT, ID_FILE_RULESETS__RULESET_DELETE,
						ID_FILE_RULESETS__RULESET_CLONE,
						ID_FILE_RULESETS__RULESET_MOVEUP, ID_FILE_RULESETS__RULESET_MOVEDOWN, 
						ID_FILE_RULESETS__EDIT_DEFAULT_RULESET,
						ID_FILE_RULESETS__RESTORE_DEFAULTS,

						ID_FONTS__STATIC_FILE,
						ID_FONTS__EDIT_FILE_FONT,
#if VIEW_FOLDER_LIST_CTRL_USE_FONTS
						ID_FONTS__STATIC_FOLDER,
						ID_FONTS__EDIT_FOLDER_FONT,
#endif
						ID_FONTS__STATIC_PRINTER_FILE,
						ID_FONTS__EDIT_PRINTER_FILE_FONT,

						ID_FONTS__STATIC_PRINTER_FOLDER,
						ID_FONTS__EDIT_PRINTER_FOLDER_FONT,

						ID_MESG__PANEL,
						ID_MESG__SHOW_FILES_EQUIVALENT_MSGBOX,
						ID_MESG__SHOW_AUTOMERGE_CONFLICTS_MSGBOX,
						//ID_MESG__SHOW_AUTOMERGE_RESULT_MSGBOX,
						ID_MESG__SHOW_FIND_DIALOG_EOF_MSGBOX,
						ID_MESG__SHOW_SAME_FOLDER_MSGBOX,
						ID_MESG__SHOW_SAME_FILES_MSGBOX,
						ID_MESG__SHOW_EXIT_MULTIPLE_WINDOWS_MSGBOX,
						ID_MESG__SHOW_EXTERNAL_TOOLS_MSGBOX,
						ID_MESG__CHECK_FILES_ON_ACTIVATE,
						ID_MESG__CHECK_FOLDERS_ON_ACTIVATE,
						ID_MESG__RESTORE_DEFAULTS,

						ID_MISC__PANEL,
						ID_MISC__PRINT_ACROSS,
						ID_MISC__REQUIRE_FINAL_EOL,
						ID_MISC__ENABLE_AUTOSAVE,
						ID_MISC__AUTOSAVE_INTERVAL,
						ID_MISC__AUTO_ADVANCE_AFTER_APPLY,
						ID_MISC__INTRALINE_THRESHOLD,
						ID_MISC__INTERLINE_THRESHOLD,
						ID_MISC__RADIO_DETAIL_LEVEL,
						ID_MISC__RADIO_MULTILINE_DETAIL_LEVEL,
						ID_MISC__RESTORE_DEFAULTS,

#ifdef FEATURE_SHEX
						ID_SHEX__PANEL,
						ID_SHEX__ENABLE_CONTEXT_MENU,
						ID_SHEX__HELP,
						ID_SHEX__RESTORE_DEFAULTS,
#endif

						ID_XT__PANEL,
						ID_XT__ENABLE_EXTERNAL_TOOLS,
						ID_XT__IGNORE_SUFFIX_CASE,
						ID_XT__REQUIRE_COMPLETE_MATCH,
						ID_XT__LISTBOX,
						ID_XT__ADD,
						ID_XT__EDIT,
						ID_XT__DELETE,
						ID_XT__CLONE,
						ID_XT__MOVEUP,
						ID_XT__MOVEDOWN,
						ID_XT__RESTORE_DEFAULTS,

	} ID;

	DECLARE_EVENT_TABLE();

public:
	void				OnOK(wxCommandEvent & e);
	void				updateGlobalProps(void);

	void				preload_fields_folder_windows(bool bDefault);
	void				preload_fields_folder_filters(bool bDefault);
	void				preload_fields_folder_colors(bool bDefault);
	void				preload_fields_folder_softmatch(bool bDefault);

	void				preload_fields_file_windows(bool bDefault);
	void				preload_fields_file_line_colors(bool bDefault);
	void				preload_fields_file_il_colors(bool bDefault);
	void				preload_fields_file_other_colors(bool bDefault);
	void				preload_fields_file_rulesets(bool bDefault);
	void				preload_fields_misc(bool bDefault);
	void				preload_fields_messages(bool bDefault);
#ifdef FEATURE_SHEX
	void				preload_fields_shex(bool bDefault);
#endif
	void				preload_fields_external_tools(bool bDefault);

	void				save_fields_folder_windows(void);
	void				save_fields_folder_filters(void);
	void				save_fields_folder_colors(void);
	void				save_fields_folder_softmatch(void);

	void				save_fields_file_windows(void);
	void				save_fields_file_line_colors(void);
	void				save_fields_file_il_colors(void);
	void				save_fields_file_other_colors(void);
	void				save_fields_file_rulesets(void);
	void				save_fields_misc(void);
	void				save_fields_messages(void);
#ifdef FEATURE_SHEX
	void				save_fields_shex(void);
#endif
	void				save_fields_external_tools(void);

	void				onButtonEvent_RestoreDefaults_folder_windows(wxCommandEvent & e);
	void				onButtonEvent_RestoreDefaults_folder_filters(wxCommandEvent & e);
	void				onButtonEvent_RestoreDefaults_folder_colors(wxCommandEvent & e);
	void				onButtonEvent_RestoreDefaults_folder_softmatch(wxCommandEvent & e);

	void				onButtonEvent_RestoreDefaults_file_windows(wxCommandEvent & e);
	void				onButtonEvent_RestoreDefaults_file_line_colors(wxCommandEvent & e);
	void				onButtonEvent_RestoreDefaults_file_il_colors(wxCommandEvent & e);
	void				onButtonEvent_RestoreDefaults_file_other_colors(wxCommandEvent & e);
	void				onButtonEvent_RestoreDefaults_file_rulesets(wxCommandEvent & e);
	void				onButtonEvent_RestoreDefaults_misc(wxCommandEvent & e);
	void				onButtonEvent_RestoreDefaults_messages(wxCommandEvent & e);
#ifdef FEATURE_SHEX
	void				onButtonEvent_Shex_Help(wxCommandEvent & e);
	void				onButtonEvent_RestoreDefaults_shex(wxCommandEvent & e);
#endif
	void				onButtonEvent_RestoreDefaults_ExternalTools(wxCommandEvent & e);

	void onButtonEvent_folder_color_different_fg  (wxCommandEvent & e);		void onButtonEvent_folder_color_different_bg  (wxCommandEvent & e);
	void onButtonEvent_folder_color_equal_fg      (wxCommandEvent & e);		void onButtonEvent_folder_color_equal_bg      (wxCommandEvent & e);
	void onButtonEvent_folder_color_equivalent_fg (wxCommandEvent & e);		void onButtonEvent_folder_color_equivalent_bg (wxCommandEvent & e);
	void onButtonEvent_folder_color_folders_fg    (wxCommandEvent & e);		void onButtonEvent_folder_color_folders_bg    (wxCommandEvent & e);
	void onButtonEvent_folder_color_peerless_fg   (wxCommandEvent & e);		void onButtonEvent_folder_color_peerless_bg   (wxCommandEvent & e);
	void onButtonEvent_folder_color_error_fg      (wxCommandEvent & e);		void onButtonEvent_folder_color_error_bg      (wxCommandEvent & e);

	void				onButtonEvent_file_color_all_eq_fg			(wxCommandEvent & e);
	void				onButtonEvent_file_color_all_eq_unimp_fg	(wxCommandEvent & e);
	void				onButtonEvent_file_color_all_eq_bg			(wxCommandEvent & e);
	void				onButtonEvent_file_color_none_eq_fg			(wxCommandEvent & e);
	void				onButtonEvent_file_color_none_eq_unimp_fg	(wxCommandEvent & e);
	void				onButtonEvent_file_color_none_eq_bg			(wxCommandEvent & e);
	void				onButtonEvent_file_color_sub_equal_fg		(wxCommandEvent & e);
	void				onButtonEvent_file_color_sub_equal_bg		(wxCommandEvent & e);
	void				onButtonEvent_file_color_sub_unimp_fg		(wxCommandEvent & e);
	void				onButtonEvent_file_color_sub_notequal_fg	(wxCommandEvent & e);
	void				onButtonEvent_file_color_sub_notequal_bg	(wxCommandEvent & e);
	void				onButtonEvent_file_color_conflict_fg		(wxCommandEvent & e);
	void				onButtonEvent_file_color_conflict_unimp_fg	(wxCommandEvent & e);
	void				onButtonEvent_file_color_conflict_bg		(wxCommandEvent & e);
	void				onButtonEvent_file_color_none_eq_il_bg		(wxCommandEvent & e);
	void				onButtonEvent_file_color_conflict_il_bg		(wxCommandEvent & e);
	void				onButtonEvent_file_color_sub_equal_il_bg	(wxCommandEvent & e);
	void				onButtonEvent_file_color_sub_notequal_il_bg	(wxCommandEvent & e);

	void				onButtonEvent_file_color_window_bg			(wxCommandEvent & e);
	void				onButtonEvent_file_color_omit_fg			(wxCommandEvent & e);
	void				onButtonEvent_file_color_omit_bg			(wxCommandEvent & e);
	void				onButtonEvent_file_color_eol_unknown_fg		(wxCommandEvent & e);
	void				onButtonEvent_file_color_void_fg			(wxCommandEvent & e);
	void				onButtonEvent_file_color_void_bg			(wxCommandEvent & e);
	void				onButtonEvent_file_color_linenr_fg			(wxCommandEvent & e);
	void				onButtonEvent_file_color_linenr_bg			(wxCommandEvent & e);
	void				onButtonEvent_file_color_caret_fg			(wxCommandEvent & e);
	void				onButtonEvent_file_color_selection_fg		(wxCommandEvent & e);
	void				onButtonEvent_file_color_selection_bg		(wxCommandEvent & e);

	void				onCheckEvent_enable_custom_rulesets(wxCommandEvent & e);
	void				onCheckEvent_enable_automatic_match(wxCommandEvent & e);
	void				onListBoxEvent_listbox(wxCommandEvent & e);
	void				onListBoxDClickEvent_listbox(wxCommandEvent & e);
	void				onButtonEvent_ruleset_add(wxCommandEvent & e);
	void				onButtonEvent_ruleset_edit(wxCommandEvent & e);
	void				onButtonEvent_ruleset_delete(wxCommandEvent & e);
	void				onButtonEvent_ruleset_clone(wxCommandEvent & e);
	void				onButtonEvent_ruleset_moveup(wxCommandEvent & e);
	void				onButtonEvent_ruleset_movedown(wxCommandEvent & e);
	void				onButtonEvent_ruleset_edit_default_ruleset(wxCommandEvent & e);

	void				onButtonEvent_fonts_file(wxCommandEvent & e);
#if VIEW_FOLDER_LIST_CTRL_USE_FONTS
	void				onButtonEvent_fonts_folder(wxCommandEvent & e);
#endif
	void				onButtonEvent_fonts_printer_file(wxCommandEvent & e);
	void				onButtonEvent_fonts_printer_folder(wxCommandEvent & e);

	void				onCheckEvent_enable_autosave(wxCommandEvent & e);

	void				onRadioEvent_DetailLevel(wxCommandEvent & e);

	void				onCheckEvent_use_file_filters(wxCommandEvent & e);
	void				onCheckEvent_use_folder_filters(wxCommandEvent & e);
	void				onCheckEvent_use_full_filename_filters(wxCommandEvent & e);

	void				onCheckEvent_enable_external_tools(wxCommandEvent & e);
	void				onListBoxEvent_listbox_external_tools(wxCommandEvent & e);
	void				onListBoxDClickEvent_listbox_external_tools(wxCommandEvent & e);
	void				onButtonEvent_external_tools_add(wxCommandEvent & e);
	void				onButtonEvent_external_tools_edit(wxCommandEvent & e);
	void				onButtonEvent_external_tools_delete(wxCommandEvent & e);
	void				onButtonEvent_external_tools_clone(wxCommandEvent & e);
	void				onButtonEvent_external_tools_moveup(wxCommandEvent & e);
	void				onButtonEvent_external_tools_movedown(wxCommandEvent & e);

	void				onCheckEvent_SoftMatch_SimpleIgnoreWhitespace(wxCommandEvent & e);
	void				onRadioEvent_SoftMatch_Mode(wxCommandEvent & e);
	void				onButtonEvent_SoftMatch_Help(wxCommandEvent & e);

	void				onTextEventChanged__FolderFilters__FileFilter(wxCommandEvent & e);
	void				onTextEventChanged__FolderFilters__FullFilenameFilter(wxCommandEvent & e);
	void				onTextEventChanged__FolderFilters__FolderFilter(wxCommandEvent & e);


private:
	void				_setColor(wxColor * pField, wxPanel * pPanel, _my_static_bitmap * pBmpAlsoUpdate=NULL);
	void				_enable_file_ruleset_fields(void);
	void				_set_static_font_fields(void);
	void				_enable_threshold_fields(void);
	void				_enable_external_tools_fields(void);
	void				_enable_softmatch_fields(void);

public:
	//////////////////////////////////////////////////////////////////
	// these fields are used by the validators to automatically load
	// and fetch values into/from dialog controls.
	//////////////////////////////////////////////////////////////////

	bool				m_bFolderFilters_UseFileFilter;
	bool				m_bFolderFilters_UseFolderFilter;
	bool				m_bFolderFilters_UseFullFilenameFilter;
	bool				m_bFolderFilters_IgnorePatternCase;
	bool				m_bFolderFilters_IgnoreMatchupCase;

	wxString			m_strFolderFilters_FileFilter;
	wxString			m_strFolderFilters_FolderFilter;
	wxString			m_strFolderFilters_FullFilenameFilter;

	wxColor				m_clrFolderColorDifferentFG,  m_clrFolderColorDifferentBG;
	wxColor				m_clrFolderColorEqualFG,      m_clrFolderColorEqualBG;
	wxColor				m_clrFolderColorEquivalentFG, m_clrFolderColorEquivalentBG;
	wxColor				m_clrFolderColorFoldersFG,    m_clrFolderColorFoldersBG;
	wxColor				m_clrFolderColorPeerlessFG,   m_clrFolderColorPeerlessBG;
	wxColor				m_clrFolderColorErrorFG,      m_clrFolderColorErrorBG;

	wxColor				m_clrFileColorAllEqFG,			m_clrFileColorAllEqUnimpFG,			m_clrFileColorAllEqBG;
	wxColor				m_clrFileColorNoneEqFG,			m_clrFileColorNoneEqUnimpFG,		m_clrFileColorNoneEqBG;
	wxColor				m_clrFileColorSubEqualFG,		/**/								m_clrFileColorSubEqualBG;
	wxColor				/**/							m_clrFileColorSubUnimpFG;			/**/
	wxColor				m_clrFileColorSubNotEqualFG,	/**/								m_clrFileColorSubNotEqualBG;
	wxColor				m_clrFileColorConflictFG,		m_clrFileColorConflictUnimpFG,		m_clrFileColorConflictBG;
	wxColor				m_clrFileColorNoneEqIlBG;
	wxColor				m_clrFileColorConflictIlBG;
	wxColor				m_clrFileColorSubEqualIlBG;
	wxColor				m_clrFileColorSubNotEqualIlBG;

	// other file colors

	wxColor				m_clrFileColorWindowBG;
	wxColor				m_clrFileColorOmitFG,		m_clrFileColorOmitBG;
	wxColor				m_clrFileColorEOLUnknownFG;
	wxColor				m_clrFileColorVoidFG,		m_clrFileColorVoidBG;
	wxColor				m_clrFileColorLineNrFG,		m_clrFileColorLineNrBG;
	wxColor				m_clrFileColorCaretFG;
	wxColor				m_clrFileColorSelectionFG,	m_clrFileColorSelectionBG;

	bool				m_bFileRulesets_EnableCustomRulesets;
	bool				m_bFileRulesets_EnableAutomaticMatch;
	bool				m_bFileRulesets_IgnoreSuffixCase;
	bool				m_bFileRulesets_RequireCompleteMatch;
	bool				m_bFileRulesets_AskIfNoMatch;

	bool				m_bMessages_ShowFilesEquivalentMsgBox;
	bool				m_bMessages_ShowAutoMergeConflictsMsgBox;
	//bool				m_bMessages_ShowAutoMergeResultMsgBox;
	bool				m_bMessages_ShowFindDialogEofMsgBox;
	bool				m_bMessages_ShowSameFolderMsgBox;
	bool				m_bMessages_ShowSameFilesMsgBox;
	bool				m_bMessages_ShowExitMultipleWindowsMsgBox;
	bool				m_bMessages_ShowExternalToolsMsgBox;
	bool				m_bMessages_CheckFilesOnActivate;
	bool				m_bMessages_CheckFoldersOnActivate;

	bool				m_bMisc_PrintAcross;
	bool				m_bMisc_RequireFinalEOL;
	bool				m_bMisc_EnableAutoSave;
	bool				m_bMisc_AutoAdvanceAfterApply;

	int					m_iMisc_AutoSaveInterval;
	int					m_iMisc_IntralineThreshold;
	int					m_iMisc_InterlineThreshold;
	int					m_iMisc_DetailLevel;		// must be "int" for generic validator
	int					m_iMisc_MultiLineDetailLevel;

	int					m_iSoftMatch_Mode;
	wxString			m_strSoftMatchSimpleSuffix;
	bool				m_bSoftMatchSimpleIgnoreEOL;
	bool				m_bSoftMatchSimpleIgnoreWhitespace;
	bool				m_bSoftMatchSimpleIgnoreTAB;
	bool				m_bSoftMatchRulesetAllowDefault;
	int					m_iSoftMatchRulesetFileLimitMb;

	bool				m_bEnableQuickMatch;
	wxString			m_strQuickMatchSuffix;

#ifdef FEATURE_SHEX
	bool				m_bShexInstalled;
	bool				m_bShexEnabled;				// field for auto validation
	long				m_lShexEnabled;				// value from global props (hopefully from registry)

	bool				_is_shex_installed(void);

	void				_shex_refresh_fields(bool bDefault);
#endif

	bool				m_bExternalTools_EnableExternalTools;
	bool				m_bExternalTools_IgnoreSuffixCase;
	bool				m_bExternalTools_RequireCompleteMatch;

	bool				m_bFontFileDirty;
	wxFont *			m_pFont_File;

#if VIEW_FOLDER_LIST_CTRL_USE_FONTS
	bool				m_bFontFolderDirty;
	wxFont *			m_pFont_Folder;
#endif

	bool				m_bFontPrinterFileDirty;
	wxFont *			m_pFont_PrinterFile;

	bool				m_bFontPrinterFolderDirty;
	wxFont *			m_pFont_PrinterFolder;

	rs_ruleset_table *	m_pRSTWorkingCopy;
	xt_tool_table *		m_pXTTWorkingCopy;

	//////////////////////////////////////////////////////////////////
	// Preview pane widget
	//////////////////////////////////////////////////////////////////

	typedef enum _TNdxPreview { NDX_PREVIEW_FILE_LINE_COLOR_2,
								NDX_PREVIEW_FILE_LINE_COLOR_3,
								NDX_PREVIEW_FILE_IL_COLOR_2,
								NDX_PREVIEW_FILE_IL_COLOR_3,
	} TNdxPreview;

private:
	class _preview : public wxScrolledWindow
	{
	public:
		_preview(wxWindow * pParent, OptionsDlg * pDlg, TNdxPreview ndxPreview)
			: wxScrolledWindow(pParent,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxFULL_REPAINT_ON_RESIZE|wxVSCROLL),
			  m_pParent(pParent),
			  m_pDlg(pDlg),
			  m_ndxPreview(ndxPreview)
			{
				setNrLines();
			};

		void onSizeEvent(wxSizeEvent & /*e*/)	{ Layout(); };
		virtual void	OnDraw(wxDC & dc)		{ m_pDlg->drawPreview(this,dc,m_ndxPreview); };
		void			setNrLines(void)		{ m_pDlg->setPreviewNrLines(this,m_ndxPreview); };

#if defined(__WXMAC__)
		void			onPaintEvent(wxPaintEvent & e);
#endif

	protected:
		wxWindow *		m_pParent;
		OptionsDlg *	m_pDlg;
		TNdxPreview		m_ndxPreview;

		DECLARE_EVENT_TABLE();
	};
	
	wxNotebook *		m_pNotebookPreviewFileLineColor;
	wxPanel *			m_pPanelPreviewFileLineColor2;
	wxPanel *			m_pPanelPreviewFileLineColor3;
	_preview *			m_pPreviewFileLineColor2;
	_preview *			m_pPreviewFileLineColor3;

	wxNotebook *		m_pNotebookPreviewFileILColor;
	wxPanel *			m_pPanelPreviewFileILColor2;
	wxPanel *			m_pPanelPreviewFileILColor3;
	_preview *			m_pPreviewFileILColor2;
	_preview *			m_pPreviewFileILColor3;

	wxPanel *			createPanel_Preview(wxNotebook * pNotebook,
											TNdxPreview ndxPreview,
											_preview ** ppPreview);

	void				drawPreview(_preview * pPreview, wxDC & dc, TNdxPreview ndxPreview);
	void				setPreviewNrLines(_preview * pPreview, TNdxPreview ndxPreview);
	void				refreshFilePreviews(void);

	void				_drawPreviewVoid(wxDC & dc,
										 int x, int y, int width, int height);
	void				_drawPreviewRectangle(wxDC & dc,
											  int x, int y, int width, int height,
											  const wxColor & clr);
	int					_drawPreviewString(wxDC & dc,
										   int x, int y, const wxString & str,
										   const wxColor & clrFG, const wxColor & clrBG);

	//////////////////////////////////////////////////////////////////
	// a static bitmap to display an item color without being a push button.
	//////////////////////////////////////////////////////////////////

	class _my_static_bitmap : public wxStaticBitmap
	{
	public:
		_my_static_bitmap(wxWindow * pParent, OptionsDlg * pDlg, wxColor * pClr);

		void			set_bitmap(bool bCreate=false);

	protected:
		wxWindow *		m_pParent;
		OptionsDlg *	m_pDlg;
		wxColor *		m_pClr;
	};

	_my_static_bitmap *	m_pBitmapFileColorAllEqFG;
	_my_static_bitmap *	m_pBitmapFileColorNoneEqFG,			* m_pBitmapFileColorNoneEqBG;
	_my_static_bitmap *	m_pBitmapFileColorSubEqualFG,		* m_pBitmapFileColorSubEqualBG;
	_my_static_bitmap *	m_pBitmapFileColorSubNotEqualFG,	* m_pBitmapFileColorSubNotEqualBG;
	_my_static_bitmap *	m_pBitmapFileColorConflictFG,		* m_pBitmapFileColorConflictBG;

	void				refreshFileColorBitmaps(void);
};


//////////////////////////////////////////////////////////////////

#endif//H_OPTIONSDLG_H
