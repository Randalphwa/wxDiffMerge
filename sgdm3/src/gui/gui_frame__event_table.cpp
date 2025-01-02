// gui_frame__event_table.cpp
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fs.h>
#include <fd.h>
#include <gui.h>

//////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(gui_frame,wxFrame)

	EVT_SIZE(gui_frame::onSizeEvent)
	EVT_MOVE(gui_frame::onMoveEvent)

	EVT_MENU(MENU_WEBHELP, gui_frame::onWebhelp)

	// close event gets sent by window manager or system menu
	// when user clicks on 'X' button on the title bar (or when
	// we call this->Close() from our menu (close_window or exit)

	EVT_CLOSE(gui_frame::onCloseEvent)

	// _activate gets fired when the gains/loses activation (top-
	// most window with highlighted title bar and focus).
	// see gui_frame__events_frame.cpp

	EVT_ACTIVATE(gui_frame::onActivateEvent)

	EVT_SET_FOCUS(gui_frame::onSetFocusEvent)

	// menu events get fired when user selects a menu item.
	//
	// update_ui events get fired before popups and during idle
	// time to see if menu item should be enabled/disabled or
	// checked/unchecked, etc.  we define one for each that
	// has state.
	//
	// since we also use the same id's for toolbar buttons, these
	// update_ui events will get fired as the mouse wanders over
	// the screen.

	EVT_MENU(MENU_FILE_FOLDER_DIFF,			gui_frame::onFileFolderDiff)
	EVT_MENU(MENU_FILE_FILE_DIFF,			gui_frame::onFileFileDiff)
	EVT_MENU(MENU_FILE_FILE_MERGE,			gui_frame::onFileFileMerge)
	EVT_MENU(MENU_FILE_RELOAD,				gui_frame::onFileReload)		EVT_UPDATE_UI(MENU_FILE_RELOAD,			gui_frame::onUpdateFileReload)
	EVT_MENU(MENU_FILE_CHANGE_RULESET,		gui_frame::onFileChangeRuleset)	EVT_UPDATE_UI(MENU_FILE_CHANGE_RULESET,	gui_frame::onUpdateFileChangeRuleset)
//	EVT_MENU(MENU_FILE_SAVE,				gui_frame::onFileSave)			EVT_UPDATE_UI(MENU_FILE_SAVE,			gui_frame::onUpdateFileSave)
	EVT_MENU(wxID_SAVE,						gui_frame::onFileSave)			EVT_UPDATE_UI(wxID_SAVE,				gui_frame::onUpdateFileSave)
//	EVT_MENU(MENU_FILE_SAVEAS,				gui_frame::onFileSaveAs)		EVT_UPDATE_UI(MENU_FILE_SAVEAS,			gui_frame::onUpdateFileSaveAs)
	EVT_MENU(wxID_SAVEAS,					gui_frame::onFileSaveAs)		EVT_UPDATE_UI(wxID_SAVEAS,				gui_frame::onUpdateFileSaveAs)
	EVT_MENU(MENU_FILE_SAVEALL,				gui_frame::onFileSaveAll)		EVT_UPDATE_UI(MENU_FILE_SAVEALL,		gui_frame::onUpdateFileSaveAll)
//	EVT_MENU(MENU_FILE_CLOSE_WINDOW,		gui_frame::onFileCloseWindow)
	EVT_MENU(wxID_CLOSE,					gui_frame::onFileCloseWindow)
//	EVT_MENU(MENU_FILE_EXIT,				gui_frame::onFileExit)
	EVT_MENU(wxID_EXIT,						gui_frame::onFileExit)

//	EVT_MENU(MENU_EDIT_UNDO,				gui_frame::onEditUndo)			EVT_UPDATE_UI(MENU_EDIT_UNDO,			gui_frame::onUpdateEditUndo)
	EVT_MENU(wxID_UNDO,						gui_frame::onEditUndo)			EVT_UPDATE_UI(wxID_UNDO,				gui_frame::onUpdateEditUndo)
//	EVT_MENU(MENU_EDIT_REDO,				gui_frame::onEditRedo)			EVT_UPDATE_UI(MENU_EDIT_REDO,			gui_frame::onUpdateEditRedo)
	EVT_MENU(wxID_REDO,						gui_frame::onEditRedo)			EVT_UPDATE_UI(wxID_REDO,				gui_frame::onUpdateEditRedo)
//	EVT_MENU(MENU_EDIT_CUT,					gui_frame::onEditCut)			EVT_UPDATE_UI(MENU_EDIT_CUT,			gui_frame::onUpdateEditCut)
	EVT_MENU(wxID_CUT,						gui_frame::onEditCut)			EVT_UPDATE_UI(wxID_CUT,					gui_frame::onUpdateEditCut)
//	EVT_MENU(MENU_EDIT_COPY,				gui_frame::onEditCopy)			EVT_UPDATE_UI(MENU_EDIT_COPY,			gui_frame::onUpdateEditCopy)
	EVT_MENU(wxID_COPY,						gui_frame::onEditCopy)			EVT_UPDATE_UI(wxID_COPY,				gui_frame::onUpdateEditCopy)
//	EVT_MENU(MENU_EDIT_PASTE,				gui_frame::onEditPaste)			EVT_UPDATE_UI(MENU_EDIT_PASTE,			gui_frame::onUpdateEditPaste)
	EVT_MENU(wxID_PASTE,					gui_frame::onEditPaste)			EVT_UPDATE_UI(wxID_PASTE,				gui_frame::onUpdateEditPaste)
//	EVT_MENU(MENU_EDIT_SELECTALL,			gui_frame::onEditSelectAll)		EVT_UPDATE_UI(MENU_EDIT_SELECTALL,		gui_frame::onUpdateEditSelectAll)
	EVT_MENU(wxID_SELECTALL,				gui_frame::onEditSelectAll)		EVT_UPDATE_UI(wxID_SELECTALL,			gui_frame::onUpdateEditSelectAll)
	EVT_MENU(MENU_EDIT_NEXT_DELTA,			gui_frame::onEditNextDelta)		EVT_UPDATE_UI(MENU_EDIT_NEXT_DELTA,		gui_frame::onUpdateEditNextDelta)
	EVT_MENU(MENU_EDIT_PREV_DELTA,			gui_frame::onEditPrevDelta)		EVT_UPDATE_UI(MENU_EDIT_PREV_DELTA,		gui_frame::onUpdateEditPrevDelta)
	EVT_MENU(MENU_EDIT_NEXT_CONFLICT,		gui_frame::onEditNextConflict)	EVT_UPDATE_UI(MENU_EDIT_NEXT_CONFLICT,	gui_frame::onUpdateEditNextConflict)
	EVT_MENU(MENU_EDIT_PREV_CONFLICT,		gui_frame::onEditPrevConflict)	EVT_UPDATE_UI(MENU_EDIT_PREV_CONFLICT,	gui_frame::onUpdateEditPrevConflict)
	EVT_MENU(MENU_EDIT_AUTOMERGE,			gui_frame::onEditAutoMerge)		EVT_UPDATE_UI(MENU_EDIT_AUTOMERGE,		gui_frame::onUpdateEditAutoMerge)

//	EVT_MENU(MENU_SETTINGS_PREFERENCES,		gui_frame::onSettingsPreferences)
	EVT_MENU(wxID_PREFERENCES,				gui_frame::onSettingsPreferences)

//	EVT_MENU(MENU_HELP_CONTENTS,			gui_frame::onHelpContents)
	EVT_MENU(wxID_HELP_CONTENTS,			gui_frame::onHelpContents)
//	EVT_MENU(MENU_HELP_ABOUT,				gui_frame::onHelpAbout)
	EVT_MENU(wxID_ABOUT,					gui_frame::onHelpAbout)

	EVT_MENU(MENU_FOLDER_OPEN_FILES,		gui_frame::onFolderOpenFiles)		EVT_UPDATE_UI(MENU_FOLDER_OPEN_FILES,		gui_frame::onUpdateFolderOpenFiles)
	EVT_MENU(MENU_FOLDER_OPEN_FOLDERS,		gui_frame::onFolderOpenFolders)		EVT_UPDATE_UI(MENU_FOLDER_OPEN_FOLDERS,		gui_frame::onUpdateFolderOpenFolders)
#if defined(__WXMSW__)
	EVT_MENU(MENU_FOLDER_OPEN_SHORTCUT_INFO,gui_frame::onFolderOpenShortcuts)	EVT_UPDATE_UI(MENU_FOLDER_OPEN_SHORTCUT_INFO,gui_frame::onUpdateFolderOpenShortcuts)
#endif
#if defined(__WXMAC__) || defined(__WXGTK__)
	EVT_MENU(MENU_FOLDER_OPEN_SYMLINK_INFO,gui_frame::onFolderOpenSymlinks)		EVT_UPDATE_UI(MENU_FOLDER_OPEN_SYMLINK_INFO,gui_frame::onUpdateFolderOpenSymlinks)
#endif

	EVT_MENU(MENU_FOLDER_SHOW_EQUAL,		gui_frame::onFolderShowEqual)	EVT_UPDATE_UI(MENU_FOLDER_SHOW_EQUAL,	gui_frame::onUpdateFolderShowEqual)
	EVT_MENU(MENU_FOLDER_SHOW_EQUIVALENT,	gui_frame::onFolderShowEquivalent)	EVT_UPDATE_UI(MENU_FOLDER_SHOW_EQUIVALENT,	gui_frame::onUpdateFolderShowEquivalent)
	EVT_MENU(MENU_FOLDER_SHOW_QUICKMATCH,	gui_frame::onFolderShowQuickMatch)	EVT_UPDATE_UI(MENU_FOLDER_SHOW_QUICKMATCH,	gui_frame::onUpdateFolderShowQuickMatch)
	EVT_MENU(MENU_FOLDER_SHOW_SINGLES,		gui_frame::onFolderShowSingles)	EVT_UPDATE_UI(MENU_FOLDER_SHOW_SINGLES,	gui_frame::onUpdateFolderShowSingles)
	EVT_MENU(MENU_FOLDER_SHOW_FOLDERS,		gui_frame::onFolderShowFolders)	EVT_UPDATE_UI(MENU_FOLDER_SHOW_FOLDERS,	gui_frame::onUpdateFolderShowFolders)
	EVT_MENU(MENU_FOLDER_SHOW_ERRORS,		gui_frame::onFolderShowErrors)	EVT_UPDATE_UI(MENU_FOLDER_SHOW_ERRORS,	gui_frame::onUpdateFolderShowErrors)

	EVT_MENU(MENU_VIEWFILE_SHOW_ALL,		gui_frame::onViewFileShowAll)	EVT_UPDATE_UI(MENU_VIEWFILE_SHOW_ALL,	gui_frame::onUpdateViewFileShowAll)
	EVT_MENU(MENU_VIEWFILE_SHOW_DIF,		gui_frame::onViewFileShowDif)	EVT_UPDATE_UI(MENU_VIEWFILE_SHOW_DIF,	gui_frame::onUpdateViewFileShowDif)
	EVT_MENU(MENU_VIEWFILE_SHOW_CTX,		gui_frame::onViewFileShowCtx)	EVT_UPDATE_UI(MENU_VIEWFILE_SHOW_CTX,	gui_frame::onUpdateViewFileShowCtx)
//	EVT_MENU(MENU_VIEWFILE_SHOW_EQL,		gui_frame::onViewFileShowEql)	EVT_UPDATE_UI(MENU_VIEWFILE_SHOW_EQL,	gui_frame::onUpdateViewFileShowEql)

	EVT_MENU(MENU_VIEWFILE_IGN_UNIMPORTANT,	gui_frame::onViewFileIgnUnimportant)	EVT_UPDATE_UI(MENU_VIEWFILE_IGN_UNIMPORTANT,gui_frame::onUpdateViewFileIgnUnimportant)
	EVT_MENU(MENU_VIEWFILE_HIDE_OMITTED,	gui_frame::onViewFileHideOmitted)		EVT_UPDATE_UI(MENU_VIEWFILE_HIDE_OMITTED,	gui_frame::onUpdateViewFileHideOmitted)
	EVT_MENU(MENU_VIEWFILE_LINENUMBERS,		gui_frame::onViewFileLineNumbers)		EVT_UPDATE_UI(MENU_VIEWFILE_LINENUMBERS,	gui_frame::onUpdateViewFileLineNumbers)
	EVT_MENU(MENU_VIEWFILE_PILCROW,			gui_frame::onViewFilePilcrow)	EVT_UPDATE_UI(MENU_VIEWFILE_PILCROW,	gui_frame::onUpdateViewFilePilcrow)

	EVT_MENU(MENU_VIEWFILE_TAB2,			gui_frame::onViewFileTab2)		EVT_UPDATE_UI(MENU_VIEWFILE_TAB2,		gui_frame::onUpdateViewFileTab2)
	EVT_MENU(MENU_VIEWFILE_TAB4,			gui_frame::onViewFileTab4)		EVT_UPDATE_UI(MENU_VIEWFILE_TAB4,		gui_frame::onUpdateViewFileTab4)
	EVT_MENU(MENU_VIEWFILE_TAB8,			gui_frame::onViewFileTab8)		EVT_UPDATE_UI(MENU_VIEWFILE_TAB8,		gui_frame::onUpdateViewFileTab8)

//	EVT_MENU(MENU_PRINT,					gui_frame::onPrint)				EVT_UPDATE_UI(MENU_PRINT,				gui_frame::onUpdatePrint)
	EVT_MENU(wxID_PRINT,					gui_frame::onPrint)				EVT_UPDATE_UI(wxID_PRINT,				gui_frame::onUpdatePrint)
//	EVT_MENU(MENU_PRINT_PREVIEW,			gui_frame::onPrintPreview)		EVT_UPDATE_UI(MENU_PRINT_PREVIEW,		gui_frame::onUpdatePrintPreview)
	EVT_MENU(wxID_PREVIEW,					gui_frame::onPrintPreview)		EVT_UPDATE_UI(wxID_PREVIEW,				gui_frame::onUpdatePrintPreview)
//	EVT_MENU(MENU_PAGE_SETUP,				gui_frame::onPageSetup)
	EVT_MENU(wxID_PAGE_SETUP,				gui_frame::onPageSetup)

	EVT_MENU(MENU_VIEWFILE_INSERT_MARK,		gui_frame::onViewFileInsertMark)	EVT_UPDATE_UI(MENU_VIEWFILE_INSERT_MARK,	gui_frame::onUpdateViewFileInsertMark)
	EVT_MENU(MENU_VIEWFILE_DELETE_ALL_MARK,	gui_frame::onViewFileDeleteAllMark)	EVT_UPDATE_UI(MENU_VIEWFILE_DELETE_ALL_MARK,	gui_frame::onUpdateViewFileDeleteAllMark)

	EVT_MENU(wxID_FIND,       gui_frame::onViewFileEdit_Find_GoTo)   EVT_UPDATE_UI(wxID_FIND,      gui_frame::onUpdateViewFileEdit_Find_GoTo)
	EVT_MENU(MENU_EDIT_GOTO,  gui_frame::onViewFileEdit_Find_GoTo)   EVT_UPDATE_UI(MENU_EDIT_GOTO, gui_frame::onUpdateViewFileEdit_Find_GoTo)


	EVT_MENU(MENU_EDIT_FIND_NEXT,              gui_frame::onViewFileEditFind__Next__Prev)    EVT_UPDATE_UI(MENU_EDIT_FIND_NEXT,              gui_frame::onUpdateViewFileEditFind__Next__Prev)
	EVT_MENU(MENU_EDIT_FIND_PREV,              gui_frame::onViewFileEditFind__Next__Prev)    EVT_UPDATE_UI(MENU_EDIT_FIND_PREV,              gui_frame::onUpdateViewFileEditFind__Next__Prev)

	EVT_MENU(MENU_EDIT_USE_SELECTION_FOR_FIND, gui_frame::onViewFileEditUseSelectionForFind) EVT_UPDATE_UI(MENU_EDIT_USE_SELECTION_FOR_FIND, gui_frame::onUpdateViewFileEditUseSelectionForFind)

	EVT_MENU(MENU_EDIT_APPLY_DEFAULT_L, gui_frame::onViewFileEditApplyDefaultActionL)	EVT_UPDATE_UI(MENU_EDIT_APPLY_DEFAULT_L, gui_frame::onUpdateViewFileEditApplyDefaultActionL)
	EVT_MENU(MENU_EDIT_APPLY_DEFAULT_R, gui_frame::onViewFileEditApplyDefaultActionR)	EVT_UPDATE_UI(MENU_EDIT_APPLY_DEFAULT_R, gui_frame::onUpdateViewFileEditApplyDefaultActionR)


	EVT_MENU(MENU_VIEWFILE_SPLIT_VERTICALLY,   gui_frame::onViewFileSplitVertically)	EVT_UPDATE_UI(MENU_VIEWFILE_SPLIT_VERTICALLY,   gui_frame::onUpdateViewFileSplitVertically)
	EVT_MENU(MENU_VIEWFILE_SPLIT_HORIZONTALLY, gui_frame::onViewFileSplitHorizontally)	EVT_UPDATE_UI(MENU_VIEWFILE_SPLIT_HORIZONTALLY, gui_frame::onUpdateViewFileSplitHorizontally)

	EVT_MENU(MENU_EXPORT__FOLDER_SUMMARY__HTML__FILE,    gui_frame::onExport__FolderSummary__id)   EVT_UPDATE_UI(MENU_EXPORT__FOLDER_SUMMARY__HTML__FILE,    gui_frame::onUpdate__Export__FolderSummary__id)
	EVT_MENU(MENU_EXPORT__FOLDER_SUMMARY__CSV__FILE,     gui_frame::onExport__FolderSummary__id)   EVT_UPDATE_UI(MENU_EXPORT__FOLDER_SUMMARY__CSV__FILE,     gui_frame::onUpdate__Export__FolderSummary__id)
	EVT_MENU(MENU_EXPORT__FOLDER_SUMMARY__RQ__FILE,      gui_frame::onExport__FolderSummary__id)   EVT_UPDATE_UI(MENU_EXPORT__FOLDER_SUMMARY__RQ__FILE,      gui_frame::onUpdate__Export__FolderSummary__id)
	EVT_MENU(MENU_EXPORT__FOLDER_SUMMARY__RQ__CLIPBOARD, gui_frame::onExport__FolderSummary__id)   EVT_UPDATE_UI(MENU_EXPORT__FOLDER_SUMMARY__RQ__CLIPBOARD, gui_frame::onUpdate__Export__FolderSummary__id)

	EVT_MENU(MENU_EXPORT__FILE_DIFFS__TRADITIONAL__HTML__FILE,       gui_frame::onFileDiffExport__id)	EVT_UPDATE_UI(MENU_EXPORT__FILE_DIFFS__TRADITIONAL__HTML__FILE,      gui_frame::onUpdate__FileDiffExport__id)
	EVT_MENU(MENU_EXPORT__FILE_DIFFS__TRADITIONAL__TEXT__FILE,       gui_frame::onFileDiffExport__id)	EVT_UPDATE_UI(MENU_EXPORT__FILE_DIFFS__TRADITIONAL__TEXT__FILE,      gui_frame::onUpdate__FileDiffExport__id)
	EVT_MENU(MENU_EXPORT__FILE_DIFFS__TRADITIONAL__TEXT__CLIPBOARD,  gui_frame::onFileDiffExport__id)	EVT_UPDATE_UI(MENU_EXPORT__FILE_DIFFS__TRADITIONAL__TEXT__CLIPBOARD, gui_frame::onUpdate__FileDiffExport__id)

	EVT_MENU(MENU_EXPORT__FILE_DIFFS__UNIFIED__HTML__FILE,           gui_frame::onFileDiffExport__id)	EVT_UPDATE_UI(MENU_EXPORT__FILE_DIFFS__UNIFIED__HTML__FILE,          gui_frame::onUpdate__FileDiffExport__id)
	EVT_MENU(MENU_EXPORT__FILE_DIFFS__UNIFIED__TEXT__FILE,           gui_frame::onFileDiffExport__id)	EVT_UPDATE_UI(MENU_EXPORT__FILE_DIFFS__UNIFIED__TEXT__FILE,          gui_frame::onUpdate__FileDiffExport__id)
	EVT_MENU(MENU_EXPORT__FILE_DIFFS__UNIFIED__TEXT__CLIPBOARD,      gui_frame::onFileDiffExport__id)	EVT_UPDATE_UI(MENU_EXPORT__FILE_DIFFS__UNIFIED__TEXT__CLIPBOARD,     gui_frame::onUpdate__FileDiffExport__id)

	EVT_MENU(MENU_EXPORT__FILE_DIFFS__SXS__HTML__FILE,               gui_frame::onFileDiffExport__id)	EVT_UPDATE_UI(MENU_EXPORT__FILE_DIFFS__SXS__HTML__FILE,              gui_frame::onUpdate__FileDiffExport__id)

	EVT_TIMER(HACK__ID_DROP_TARGET, gui_frame::HACK_on_drop_target_timer)

END_EVENT_TABLE();
