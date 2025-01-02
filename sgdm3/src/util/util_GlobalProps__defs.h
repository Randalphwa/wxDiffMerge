// util_GlobalProps__defs.h
// define global props.  this file is included multiple times.
//////////////////////////////////////////////////////////////////
//
// we have several classes of global properties.
// 
// [1] dynamic initial default value:
//     example: initial window size and position, tabstops.
//     these are stored between sessions and updated when the corresponding
//     item in the program is changed (such as when the user resizes a
//     window).  new windows will be seeded with the "new" initial value.
//     these items ARE NOT in the preferences dialog.  windows DO NOT WATCH
//     them (for changes).  these items are either on the toolbar/menu or
//     are a window property (like size/position).
//
//     you can think of these as being used only to provide an initial value
//     and then not referenced; that is, a window has a private variable that
//     it uses for this after creation.  the window usually updates the global
//     as the private variable changes.
//
// [1a] printer and page setup properties:
//      all the various values that the print-dialog and page-setup-dialog
//      present that should persist between printings.
//
// [2] shared property:
//     example: colors and fonts.
//     these are stored between sessions and are either always directly
//     referenced when a window needs the value or are "cached and watched".
//     these items ARE in the preferences dialog and NOT on the toolbar/menu.
//
// [3] shared value:
//     example: show-files-without-peers
//     these are like [2], but are on the toolbar/menu rather than in the
//     preferences dialog.  they are always directly referenced or "cached
//     and watched".
//
// [4] insignificant shared property:
//     like [2] but not in the preferences dialog -- things like "don't show
//     this message again".
//
// so, if you toggle show-files-without-peers (a type [3] variable) on a
// folder window toolbar, it will change on all open folder windows.  but
// if you change the tabstop (a type [1] variable) on a file window, only
// that window will change.
//
// TODO review type [3]'s and see if we *REALLY* want this behavior.  i did
// TODO it this way because of user confusion in the non wxWidgets version.
// TODO i think the folder window show-/hide- options are too important to
// TODO be buried in a tab on the preferences dialog (like a type [2]) -- because
// TODO otherwise we're hiding information with no indication that we've done so.
// TODO on the other hand, i don't think they'll toggle them very often, so i
// TODO don't think they'll turn it on in one window and off in another. so we're
// TODO either showing-equals or we're not.  i contrast that with something like
// TODO show-/hide-line-numbers (a type [1] variable) on file windows which is
// TODO pretty harmless and can be on/off on a per-window basis since the user 
// TODO may be looking at different types of source files.
//
//////////////////////////////////////////////////////////////////
// bools (actually just longs)

GPL(FOLDER_IGNORE_SUFFIX_FILTER,_T("Folder/IgnoreSuffixFilter"),	0)	// type 2
GPL(FOLDER_IGNORE_SUBDIR_FILTER,_T("Folder/IgnoreSubdirFilter"),	0)	// type 2
GPL(FOLDER_IGNORE_FULL_FILENAME_FILTER,_T("Folder/IgnoreFullFilenameFilter"),	0)	// type 2
GPL(FOLDER_IGNORE_PATTERN_CASE,_T("Folder/IgnorePatternCase"), 1)
#if defined(__WXMSW__) || defined(__WXMAC__)
// ignore case when matching rows in folder window on mac/windows by default
// since they have case-insenstive filesystems.
GPL(FOLDER_IGNORE_MATCHUP_CASE,_T("Folder/IgnoreMatchupCase"), 1)
#else
// allow, but don't default to it on linux.
GPL(FOLDER_IGNORE_MATCHUP_CASE,_T("Folder/IgnoreMatchupCase"), 0)
#endif

GPL(FILE_RULESET_ENABLE_CUSTOM_RULESETS,	_T("File/Ruleset/EnableCustomRulesets"),	1)		// type 2
GPL(FILE_RULESET_ENABLE_AUTOMATIC_MATCH,	_T("File/Ruleset/EnableAutomaticMatch"),	1)		// type 2
GPL(FILE_RULESET_IGNORE_SUFFIX_CASE,		_T("File/Ruleset/IgnoreSuffixCase"),		1)		// type 2
GPL(FILE_RULESET_REQUIRE_COMPLETE_MATCH,	_T("File/Ruleset/RequireCompleteMatch"),	0)		// type 2
GPL(FILE_RULESET_ASK_IF_NO_MATCH,			_T("File/Ruleset/AskIfNoMatch"),			0)		// type 2

GPL(VIEW_FILE_DONT_SHOW_FILES_EQUIVALENT_MSGBOX,		_T("File/View/DontShowEquivalentMsg"),			0)		// type 4
GPL(VIEW_FILE_DONT_SHOW_AUTOMERGE_CONFLICTS_MSGBOX,		_T("File/View/DontShowAutoMergeConflictsMsg"),	0)
GPL(VIEW_FOLDER_DONT_SHOW_SAME_FOLDER_MSGBOX,			_T("Folder/View/DontShowSameFolderMsg"),		0)
GPL(VIEW_FILE_DONT_SHOW_SAME_FILES_MSGBOX,				_T("File/View/DontShowSameFilesMsg"),			0)
GPL(VIEW_FILE_DONT_SHOW_EXIT_MULTIPLE_WINDOWS_MSGBOX,	_T("File/View/DontShowExitMultipleWindowsMsg"),	1)

//////////////////////////////////////////////////////////////////
// longs

// I changed how we store the show/hide flags between 4.0 and 4.1
// from these 5 deprecated fields to a single show/hide flag.
// See fd_fd::fd_fd().
GPL(FOLDER_SHOW_HIDE_FLAGS,					_T("Folder/ShowFlags"),			0xffffffff)	// default to show all rows
/*deprecated*/GPL(FOLDER_SHOW_EQUAL,		_T("Folder/ShowEqual"),			0xffffffff)	// was 0
/*deprecated*/GPL(FOLDER_SHOW_EQUIVALENT,	_T("Folder/ShowEquivalent"),	0xffffffff)	// was 0
/*deprecated*/GPL(FOLDER_SHOW_SINGLES,		_T("Folder/ShowSingles"),		0xffffffff)	// was 0
/*deprecated*/GPL(FOLDER_SHOW_FOLDERS,		_T("Folder/ShowFolders"),		0xffffffff)	// was 0
/*deprecated*/GPL(FOLDER_SHOW_ERRORS,		_T("Folder/ShowErrors"),		0xffffffff)	// was 1

	// all colors are type 2

GPL(FILE_COLOR_WINDOW_BG,		_T("File/Color/Window/bg"),				0xffffff)		// white

GPL(FILE_COLOR_OMIT_BG,			_T("File/Color/Omit/bg"),				0xf8f8f8)
GPL(FILE_COLOR_OMIT_FG,			_T("File/Color/Omit/fg"),				0x808080)

GPL(FILE_COLOR_ALL_EQ_BG,		_T("File/Color/AllEqual/bg"),			0xffffff)		// white
GPL(FILE_COLOR_ALL_EQ_FG,		_T("File/Color/AllEqual/fg"),			0x000000)		// black
GPL(FILE_COLOR_ALL_EQ_UNIMP_FG,	_T("File/Color/AllEqual/Unimp/fg"),		0x000000)


GPL(FILE_COLOR_NONE_EQ_BG,		_T("File/Color/NoneEqual/bg"),			0xf8f0f0)
GPL(FILE_COLOR_NONE_EQ_IL_BG,	_T("File/Color/NoneEqual/IL/bg"),		0xffd2c8)
GPL(FILE_COLOR_NONE_EQ_FG,		_T("File/Color/NoneEqual/fg"),			0xd20000)
GPL(FILE_COLOR_NONE_EQ_UNIMP_FG,_T("File/Color/NoneEqual/Unimp/fg"),	0x87493d)


GPL(FILE_COLOR_CONFLICT_BG,			_T("File/Color/Conflict/bg"),		0xffffd0)
GPL(FILE_COLOR_CONFLICT_IL_BG,		_T("File/Color/Conflict/IL/bg"),	0xffd2c8)
GPL(FILE_COLOR_CONFLICT_FG,			_T("File/Color/Conflict/fg"),		0xd20000)
GPL(FILE_COLOR_CONFLICT_UNIMP_FG,	_T("File/Color/Conflict/Unimp/fg"),	0x87493d)

GPL(FILE_COLOR_SUB_EQUAL_BG,		_T("File/Color/SubEqual/bg"),			0xf0f8f0)
GPL(FILE_COLOR_SUB_EQUAL_IL_BG,		_T("File/Color/SubEqual/IL/bg"),		0xc0fac0)
GPL(FILE_COLOR_SUB_EQUAL_FG,		_T("File/Color/SubEqual/fg"),			0x008000)

GPL(FILE_COLOR_SUB_NOTEQUAL_BG,		_T("File/Color/SubNotEqual/bg"),		0xf2f2ff)
GPL(FILE_COLOR_SUB_NOTEQUAL_IL_BG,	_T("File/Color/SubNotEqual/IL/bg"),		0xdbdbff)
GPL(FILE_COLOR_SUB_NOTEQUAL_FG,		_T("File/Color/SubNotEqual/fg"),		0x000080)

GPL(FILE_COLOR_SUB_UNIMP_FG,	_T("File/Color/Sum/Unimp/fg"),			0x629588)

GPL(FILE_COLOR_EOL_UNKNOWN_FG,	_T("File/Color/EolUnknown/fg"),			0xc0c0c0)


GPL(FILE_COLOR_LINENR_FG,		_T("File/Color/LineNr/fg"),				0x629588)
GPL(FILE_COLOR_LINENR_BG,		_T("File/Color/LineNr/bg"),				0xffffff)

GPL(FILE_COLOR_CARET_FG,		_T("File/Color/Caret/fg"),				0x000000)		// black

GPL(FILE_COLOR_VOID_FG,			_T("File/Color/Void/fg"),				0xdddddd)
GPL(FILE_COLOR_VOID_BG,			_T("File/Color/Void/bg"),				0xffffff)

GPL(FILE_COLOR_SELECTION_FG,		_T("File/Color/Selection/fg"),			0xffffff)
GPL(FILE_COLOR_SELECTION_BG,		_T("File/Color/Selection/bg"),			0x0000ff)


GPL(FOLDER_COLOR_DIFFERENT_FG,	_T("Folder/Color/Different/fg"),	0xd20000)
GPL(FOLDER_COLOR_DIFFERENT_BG,	_T("Folder/Color/Different/bg"),	0xf8f0f0)
GPL(FOLDER_COLOR_EQUAL_FG,		_T("Folder/Color/Equal/fg"),		0x000000)
GPL(FOLDER_COLOR_EQUAL_BG,		_T("Folder/Color/Equal/bg"),		0xffffff)
GPL(FOLDER_COLOR_EQUIVALENT_FG,	_T("Folder/Color/Equivalent/fg"),	0x808080)
GPL(FOLDER_COLOR_EQUIVALENT_BG,	_T("Folder/Color/Equivalent/bg"),	0xf8f8f8)
GPL(FOLDER_COLOR_FOLDERS_FG,	_T("Folder/Color/Folders/fg"),		0x000000)
GPL(FOLDER_COLOR_FOLDERS_BG,	_T("Folder/Color/Folders/bg"),		0xffffff)
GPL(FOLDER_COLOR_PEERLESS_FG,	_T("Folder/Color/Peerless/fg"),		0x0000c8)
GPL(FOLDER_COLOR_PEERLESS_BG,	_T("Folder/Color/Peerless/bg"),		0xffffff)
GPL(FOLDER_COLOR_ERROR_FG,		_T("Folder/Color/Error/fg"),		0xff0000)
GPL(FOLDER_COLOR_ERROR_BG,		_T("Folder/Color/Error/bg"),		0xffffe0)

	// all window sizes/positions are type 1

GPL(WINDOW_SIZE_BLANK_W,		_T("Window/Size/Blank/w"),			-1)	// assumes wxDefaultSize is (-1,-1)
GPL(WINDOW_SIZE_BLANK_H,		_T("Window/Size/Blank/h"),			-1)
GPL(WINDOW_SIZE_FOLDER_W,		_T("Window/Size/Folder/w"),			-1)
GPL(WINDOW_SIZE_FOLDER_H,		_T("Window/Size/Folder/h"),			-1)
GPL(WINDOW_SIZE_FILE_DIFF_W,	_T("Window/Size/Diff/w"),			-1)
GPL(WINDOW_SIZE_FILE_DIFF_H,	_T("Window/Size/Diff/h"),			-1)
GPL(WINDOW_SIZE_FILE_MERGE_W,	_T("Window/Size/Merge/w"),			-1)
GPL(WINDOW_SIZE_FILE_MERGE_H,	_T("Window/Size/Merge/h"),			-1)
GPL(WINDOW_SIZE_PRINT_PREVIEW_W,_T("Window/Size/PrintPreview/w"),	-1)
GPL(WINDOW_SIZE_PRINT_PREVIEW_H,_T("Window/Size/PrintPreview/h"),	-1)

GPL(WINDOW_SIZE_BLANK_X,		_T("Window/Size/Blank/x"),			-1)	// assumes wxDefaultPosition is (-1,-1)
GPL(WINDOW_SIZE_BLANK_Y,		_T("Window/Size/Blank/y"),			-1)
GPL(WINDOW_SIZE_FOLDER_X,		_T("Window/Size/Folder/x"),			-1)
GPL(WINDOW_SIZE_FOLDER_Y,		_T("Window/Size/Folder/y"),			-1)
GPL(WINDOW_SIZE_FILE_DIFF_X,	_T("Window/Size/Diff/x"),			-1)
GPL(WINDOW_SIZE_FILE_DIFF_Y,	_T("Window/Size/Diff/y"),			-1)
GPL(WINDOW_SIZE_FILE_MERGE_X,	_T("Window/Size/Merge/x"),			-1)
GPL(WINDOW_SIZE_FILE_MERGE_Y,	_T("Window/Size/Merge/y"),			-1)
GPL(WINDOW_SIZE_PRINT_PREVIEW_X,_T("Window/Size/PrintPreview/x"),	-1)
GPL(WINDOW_SIZE_PRINT_PREVIEW_Y,_T("Window/Size/PrintPreview/y"),	-1)

GPL(WINDOW_SIZE_BLANK_MAXIMIZED,		_T("Window/Size/Blank/maximized"),	0)
GPL(WINDOW_SIZE_FOLDER_MAXIMIZED,		_T("Window/Size/Folder/maximized"),	0)
GPL(WINDOW_SIZE_FILE_DIFF_MAXIMIZED,	_T("Window/Size/Diff/maximized"), 	0)
GPL(WINDOW_SIZE_FILE_MERGE_MAXIMIZED,	_T("Window/Size/Merge/maximized"),	0)


GPL(FILE_CONTEXT_GOAL,			_T("File/Context/Goal"),			3)	// nr lines of context in diff-with-context	// TODO should this be in ruleset??
GPL(FILE_INTRALINE_SMOOTHING_THRESHOLD,	_T("File/Context/IntraLineSmoothingThreshold"), 3)	// threshold for smoothing small EQs between adjacent NEQs // TODO should this be in ruleset??
GPL(FILE_LINE_SMOOTHING_THRESHOLD,		_T("File/Context/LineSmoothingThreshold"),      0)	// threshold for smoothing small EQs between adjacent NEQs // TODO should this be in ruleset??
GPL(FILE_DETAIL_LEVEL,					_T("File/Context/DetailLevel"),					1)	// see de_detail_level in de_dcl.h // TODO should this be in ruleset??
GPL(FILE_MULTILINE_DETAIL_LEVEL,		_T("File/Context/MultiLineDetailLevel"),		1)	// aggressiveness of multi-line intra-line calculation -- see de_multiline_detail_level
GPL(FILE_MULTILINE_DETAIL_LIMIT,		_T("File/Context/MultiLineDetailLimit"),		40)	// line limit for multi-line-intra-line when level is _NEQS

	// all these are type 1

GPL(VIEW_FILE_DISPLAY_OP,		_T("File/View/DisplayOp"),			0)	// see DE_DOP_ in de/de_dcl.h
GPL(VIEW_FILE_LINE_NUMBERS,		_T("File/View/LineNumbers"),		1)
GPL(VIEW_FILE_PILCROW,			_T("File/View/Pilcrow"),			0)
GPL(VIEW_FILE_TABSTOP,			_T("File/View/Tabstop"),			8)

GPL(VIEW_FILE_DIFF_VIEW_SPLITTER_ORIENTATION,	_T("File/View/Diff/ViewSplitterOrientation"),	1)	// 1==vertical, 0==horizontal
GPL(VIEW_FILE_DIFF_EDIT_SPLITTER_ORIENTATION,	_T("File/View/Diff/EditSplitterOrientation"),	1)	// 1==vertical, 0==horizontal

GPL(VIEW_FILE_MERGE_VIEW_SPLITTER_ORIENTATION,	_T("File/View/Merge/ViewSplitterOrientation"),	1)	// 1==vertical, 0==horizontal
GPL(VIEW_FILE_MERGE_EDIT_SPLITTER_ORIENTATION,	_T("File/View/Merge/EditSplitterOrientation"),	1)	// 1==vertical, 0==horizontal

GPL(OPTIONS_DLG_INITIAL_PAGE,	_T("Options/Dialog/InitialPage"),	0)	// which tab on options dialog was last used.
GPL(RULESET_DLG_INITIAL_PAGE,	_T("Ruleset/Dialog/InitialPage"),	0)	// which tab on options dialog was last used.

//////////////////////////////////////////////////////////////////
// strings

	// filters are type 2

GPS(FOLDER_SUFFIX_FILTER,		_T("Folder/SuffixFilter"),		_T("a aps bsc bz2 cab chm dll dmg exe exp id idb ilk iso lib map mdp msi ncb nvram o obj ocx out pch pdb res sbr so tar tgz vcp vmdk vmem vmsd vmss zip"))
GPS(FOLDER_SUBDIR_FILTER,		_T("Folder/SubdirFilter"),		_T("Debug Release _sgbak _sgvault objs .svn .git .bzr .hg .sgdrawer .sgcloset"))

// emacs backup files are *~
// veracity backup files are g[0-9]*filename~sg[0-9]*~ (which is also matched by *~)
// emacs buffer dirty buffer links are .#*
GPS(FOLDER_FULL_FILENAME_FILTER, _T("Folder/FullFilenameFilter"), _T(".DS_Store core *~ .#* "))

	// dialog seeds are type 1 -- relative to an invocation of a common dialog rather than a frame window

GPS(DIALOG_CHOOSE_FOLDER_SEED_L,		_T("Dialog/Choose/Folder/Seed/Left"),	_T(""))
GPS(DIALOG_CHOOSE_FOLDER_SEED_R,		_T("Dialog/Choose/Folder/Seed/Right"),	_T(""))
GPS(DIALOG_CHOOSE_FILE_DIFF_SEED_0,		_T("Dialog/Choose/File/Diff/Seed/0"),	_T(""))
GPS(DIALOG_CHOOSE_FILE_DIFF_SEED_1,		_T("Dialog/Choose/File/Diff/Seed/1"),	_T(""))
GPS(DIALOG_CHOOSE_FILE_MERGE_SEED_0,	_T("Dialog/Choose/File/Merge/Seed/0"),	_T(""))
GPS(DIALOG_CHOOSE_FILE_MERGE_SEED_1,	_T("Dialog/Choose/File/Merge/Seed/1"),	_T(""))
GPS(DIALOG_CHOOSE_FILE_MERGE_SEED_2,	_T("Dialog/Choose/File/Merge/Seed/2"),	_T(""))

GPS(DIALOG_CHOOSE_FOLDER_SAVEAS_SEED,	_T("Dialog/Choose/Folder/Seed/SaveAs"),	_T(""))

GPS(DIALOG_CHOOSE_FILEDIFF_EXPORT_SEED, _T("Dialog/Choose/File/Diff/Seed/Export"), _T(""))

//////////////////////////////////////////////////////////////////
	
GPL(DIALOG_FIND_ICASE,			_T("Dialog/Find/ICase"),		true)
GPL(DIALOG_FIND_WRAP,			_T("Dialog/Find/Wrap"),			false)
GPL(DIALOG_FIND_REGEX,			_T("Dialog/Find/RegEx"),		false)
GPL(DIALOG_FIND_DONT_SHOW_EOF_MSGBOX,	_T("Dialog/Find/DontShowEofMsg"),	0)

	// fonts are type 2

GPS(VIEW_FILE_FONT,				_T("File/Font"),				_T("10::"))	// "pointsize_d:family_d:facename_s" descriptor for the font we use for all file-diff/-merge views
GPS(VIEW_FOLDER_FONT,			_T("Folder/Font"),				_T("10::"))	// "pointsize_d:family_d:facename_s" descriptor for the font we use for all ViewFolders
GPS(PRINTER_FILE_FONT,			_T("File/Printer/Font"),		_T("10::")) // descriptor that we use for all printing
GPS(PRINTER_FOLDER_FONT,		_T("Folder/Printer/Font"),		_T("10::")) // descriptor that we use for all printing

	// builtin rulesets are type 2 [see rs_ruleset_table for default initial value]

GPS(FILE_RULESET_SERIALIZED,	_T("File/Ruleset/Serialized"),	_T(""))

	// custom color table for the color selector dialog (the array of 16 custom colors on the Win32 dialog)

GPS(DIALOG_COLOR_CUSTOM_COLORS,	_T("Dialog/Color/CustomColors"),	_T(""))

//////////////////////////////////////////////////////////////////
// printer and page-setup properties (type 1a)

GPL(PAGE_SETUP_MARGIN_TOP,		_T("PageSetup/MarginTop"),		10)			// margins are in MM
GPL(PAGE_SETUP_MARGIN_LEFT,		_T("PageSetup/MarginLeft"),		10)			
GPL(PAGE_SETUP_MARGIN_BOTTOM,	_T("PageSetup/MarginBottom"),	10)			
GPL(PAGE_SETUP_MARGIN_RIGHT,	_T("PageSetup/MarginRight"),	10)			

GPL(PRINT_DATA_BIN,				_T("PrintData/Bin"),			wxPRINTBIN_DEFAULT)
GPL(PRINT_DATA_COLLATE,			_T("PrintData/Collate"),		false)
GPL(PRINT_DATA_COLOR,			_T("PrintData/Color"),			true)
GPL(PRINT_DATA_DUPLEX,			_T("PrintData/Duplex"),			wxDUPLEX_SIMPLEX)
GPL(PRINT_DATA_COPIES,			_T("PrintData/Copies"),			1)
GPL(PRINT_DATA_ORIENTATION,		_T("PrintData/Orientation"),	wxPORTRAIT)
GPL(PRINT_DATA_PAPER_ID,		_T("PrintData/PaperId"),		wxPAPER_LETTER)			// paper-id is in both print-data and page-setup-data
GPS(PRINT_DATA_PRINTER_NAME,	_T("PrintData/PrinterName"),	_T(""))
GPL(PRINT_DATA_QUALITY,			_T("PrintData/Quality"),		wxPRINT_QUALITY_HIGH)

GPL(MISC_PRINT_ACROSS,                  _T("Misc/Print/Across"),		true)		// print 1a,1b,1c,2a,2b,... or 1a,2a,3a,...1b,2b,... (a type 2)
GPL(MISC_REQUIRE_FINAL_EOL,             _T("Misc/RequireFinalEOL"),		true)		// append final EOL chars if necessary when saving
GPL(MISC_AUTOSAVE_INTERVAL,             _T("Misc/AutoSaveInterval"),	100)		// every n edits
GPL(MISC_CHECK_FILES_ON_ACTIVATE,       _T("Misc/CheckFilesOnActivate"),	true)	// check for modified files when file window activated
GPL(MISC_CHECK_FOLDERS_ON_ACTIVATE,     _T("Misc/CheckFoldersOnActivate"),	false)	// reload folders when window activated vs reload-only-when-requested
GPL(MISC_AUTO_ADVANCE_AFTER_APPLY,      _T("Misc/AutoAdvanceAfterApply"),	true)	// automatically advance to next change after using green-arrow on toolbar

GPL(MISC_BANNER_CHECK_TIMESTAMP,        _T("Misc/BCTS"),                    0)      // the timestamp of the last banner check
GPS(MISC_BANNER_CHECK_KEY,              _T("Misc/BCK"),                     _T("")) // the banner check key
                                                                                    // the banner contents... note, this will need to be expanded to include "numbers"
GPS(MISC_BANNER_CONTENTS_01,            _T("Misc/BC01"),                    _T("")) // for Misc/BC01, Misc/BC02, Misc/BC03, ... Misc/B10.  No more than BANNER_PODS_MAX
GPS(MISC_BANNER_CONTENTS_02,            _T("Misc/BC02"),                    _T(""))
GPS(MISC_BANNER_CONTENTS_03,            _T("Misc/BC03"),                    _T(""))
GPS(MISC_BANNER_CONTENTS_04,            _T("Misc/BC04"),                    _T(""))
GPS(MISC_BANNER_CONTENTS_05,            _T("Misc/BC05"),                    _T(""))
GPS(MISC_BANNER_CONTENTS_06,            _T("Misc/BC06"),                    _T(""))
GPS(MISC_BANNER_CONTENTS_07,            _T("Misc/BC07"),                    _T(""))
GPS(MISC_BANNER_CONTENTS_08,            _T("Misc/BC08"),                    _T(""))
GPS(MISC_BANNER_CONTENTS_09,            _T("Misc/BC09"),                    _T(""))
GPS(MISC_BANNER_CONTENTS_10,            _T("Misc/BC10"),                    _T(""))


//////////////////////////////////////////////////////////////////

#ifdef FEATURE_SHEX
// the windows explorer shell integration module defines 2 sets of registry keys.
// 
// the first set is controlled by the various .rgs files in that module and uses
// the DllRegisterServer() stuff to create/delete the register values.  these are
// per-system settings (and may/maynot require admin priviledges).  since these are
// stored in HKCR (rather than HKCU), we do not reference them thru the global props.
//
// the second set controls the per-user settings - if enabled and various command
// line args, and etc.  these are stored under HKCU along with all the other global
// props, so we can use the existing global props mechanism to store them.  since
// these variables affect how the extension module .DLL that is hooked into Windows
// Explorer operates, we need to always load/store them to the registry and not use
// any of the global props caching.  WARNING: The values of these key pathnames must
// match the values in the .DLL.

GPL(SHEX_ENABLED,	_T("ShellExtension/Enabled"),	1)
GPS(SHEX_CLARGS,	_T("ShellExtension/CLARGS"),	_T("/nosplash /shex"))

#endif

// config stuff for XT (external tools)

GPS(EXTERNAL_TOOLS_SERIALIZED,				_T("ExternalTools/Serialized"),				_T(""))	// see xt_tools_table for default initial value
GPL(EXTERNAL_TOOLS_IGNORE_SUFFIX_CASE,		_T("ExternalTools/IgnoreSuffixCase"),		1)		// ignore case in filename when matching suffixes?
GPL(EXTERNAL_TOOLS_ENABLE_TOOLS,			_T("ExternalTools/EnableTools"),			0)		// turn on/off whole external tool mechanism?
GPL(EXTERNAL_TOOLS_REQUIRE_COMPLETE_MATCH,	_T("ExternalTools/RequireCompleteMatch"),	0)		// do all suffixes on a set of files have to match?
GPL(EXTERNAL_TOOLS_DONT_SHOW_EXEC_MSG,		_T("ExternalTools/DontShowExecMsg"),		0)		// don't show 'do you want to exec external tool' dialog.
GPL(EXTERNAL_TOOLS_DLG_INITIAL_PAGE,		_T("ExternalTools/DlgInitialPage"),			0)		// initial tab for xt_tool_dlg

//////////////////////////////////////////////////////////////////
// Stuff for new license key.

GPS(LICENSE_KEY, _T("License/Key"), _T(""))
GPL(LICENSE_CHECK, _T("License/Check"), 0)

GPL(NEW_REVISION_CHECK, _T("Revision/Check"), 0)

GPS(DPF_STATS, _T("DPF"), _T(""))				// See dlg_paid.cpp

//////////////////////////////////////////////////////////////////
// "soft" matching for folder windows

GPL(FOLDER_SOFTMATCH_MODE,						_T("Folder/SoftMatch/Mode"),					2)		// 0==EXACT-MATCH-ONLY, 1=EXACT-OR-SIMPLE, 2=EXACT-OR-RULESET

GPS(FOLDER_SOFTMATCH_SIMPLE_SUFFIX,				_T("Folder/SoftMatch/SimpleSuffixes"),			_T("c cpp cs h bas frm cls vbp ctl vbs java jav txt"))	// simple-mode suffixes 
GPL(FOLDER_SOFTMATCH_SIMPLE_IGNORE_EOL,			_T("Folder/SoftMatch/SimpleIgnoreEOL"),			1)		// ignore differences in EOL chars
GPL(FOLDER_SOFTMATCH_SIMPLE_IGNORE_WHITESPACE,	_T("Folder/SoftMatch/SimpleIgnoreWhitespace"),	1)		// ignore differences in whitespace
GPL(FOLDER_SOFTMATCH_SIMPLE_IGNORE_TAB,			_T("Folder/SoftMatch/SimpleIgnoreTAB"),			1)		// consider tabs as whitespace

GPL(FOLDER_SOFTMATCH_RULESET_FILE_LIMIT_MB,		_T("Folder/SoftMatch/RulesetFileLimitMb"),		1)		// file size limit in Mb
GPL(FOLDER_SOFTMATCH_RULESET_ALLOW_DEFAULT,		_T("Folder/SoftMatch/RulesetAllowDefault"),		0)		// allow the default ruleset to be used

//////////////////////////////////////////////////////////////////
// "quick" matching for folder windows

GPL(FOLDER_QUICKMATCH_ENABLED, _T("Folder/QuickMatch/Enabled"), 0)						// make them turn this on, since it is an approximation
GPS(FOLDER_QUICKMATCH_SUFFIX,  _T("Folder/QuickMatch/Suffixes"), _T("m4a m4p mov mp3 wav"))

