// gui_frame__iface_defs.cpp
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
// we use two different size XPM's for the toolbar artwork to make
// the toolbar look more native.
//
// we adapt some of the menu item strings/accelerators to match
// platform conventions.

#if   defined(__WXMSW__)
#   define K(w,g,m)						w
#elif defined(__WXGTK__)
#   define K(w,g,m)						g
#elif defined(__WXMAC__)
#   define K(w,g,m)						m
#else
#error Add platform
#endif

/*static*/ wxSize gui_frame::_get_toolbar_bitmap_size(void)
{
	return wxSize(32,32);
}

//////////////////////////////////////////////////////////////////

/*static*/ const gui_frame::_iface_def * gui_frame::_get_iface_def(gui_frame::_iface_id id)
{
	static _iface_def saDefs[] = 
		{
			{	MENU_FILE_FOLDER_DIFF, wxID_NONE,
				_("&Open Folder Diff..."),
				_("Open Folders"),
				_("Open new folder diff window"),
				_("Open Folder Window"),
				_T_PNG2SRC_VARIABLE, TB__FOLDER_DIFF, TB__FOLDER_DIFF__DIS,
			},
			
			{	MENU_FILE_FILE_DIFF, wxID_NONE,
				_("Open File Diff...\tCtrl+O"),
				_("Open Diff"),
				_("Open new 2-way file diff window"),
				_("Open Diff Window"),
				_T_PNG2SRC_VARIABLE, TB__FILE_DIFF, TB__FILE_DIFF__DIS,
			},
			
			{	MENU_FILE_FILE_MERGE, wxID_NONE,
				_("Open File Merge...\tCtrl+Shift+O"),
				_("Open Merge"),
				_("Open new 3-way file merge window"),
				_("Open Merge Window"),
				_T_PNG2SRC_VARIABLE, TB__FILE_MERGE, TB__FILE_MERGE,
			},
			
			{	MENU_FILE_RELOAD, wxID_NONE,
				_("&Reload\tCtrl+R"),
				NULL,
				_("Reload the files/folders in this window from disk"),
				NULL,
				_T_NONE, NULL,
			},
			
			{	MENU_FILE_SAVE, wxID_SAVE,
				_("&Save\tCtrl+S"),
				_("Save"),
				_("Save the file edit panel or re-export the folder window to a file"),
				_("Save File"),
				_T_PNG2SRC_VARIABLE, TB__SAVE, TB__SAVE__DIS
			},
			
			{	MENU_FILE_SAVEAS, wxID_SAVEAS,
				_("Save &As..."),
				NULL,
				_("Save the file edit panel or export the folder window to a new file"),
				NULL,
				_T_NONE, NULL,
			},
			
			{	MENU_FILE_SAVEALL, wxID_NONE,
				K( _("Save A&ll\tCtrl+Shift+S"), _("Save A&ll\tCtrl+Shift+S"), _("Save A&ll") ),		// Ctrl+Shift+S is SaveAs on MAC
				NULL,
				_("Save the edit panels in all file windows."),
				NULL,
				_T_NONE, NULL,
			},
			
			{	MENU_FILE_CLOSE_WINDOW, wxID_CLOSE,
				K( _("&Close Window\tAlt+F4"), _("&Close Window\tCtrl+W"), _("&Close Window\tCtrl+W") ),
				NULL,
				_("Close this window"),
				NULL,
				_T_NONE, NULL,
			},
			
			{	MENU_FILE_EXIT, wxID_EXIT,
				K( _("E&xit\tAlt+X"), _("E&xit\tAlt+X"), _("Quit") ), // this gets moved to application menu on mac -- system owns accelerator.
				NULL,
				_("Close all windows and exit"),
				NULL,
				_T_NONE, NULL,
			},
			
			{	MENU_EDIT_UNDO, wxID_UNDO,
				_("&Undo\tCtrl+Z"),
				_("Undo"),
				_("Undo the last edit"),
				_("Undo the last edit"),
				_T_PNG2SRC_VARIABLE, TB__EDIT_UNDO, TB__EDIT_UNDO__DIS,
			},
			
			{	MENU_EDIT_REDO, wxID_REDO,
				K( _("&Redo\tCtrl+Y"), _("&Redo\tCtrl+Y"), _("&Redo\tCtrl+Shift+Z") ),
				_("Redo"),
				_("Redo the last edit"),
				_("Redo the last edit"),
				_T_PNG2SRC_VARIABLE, TB__EDIT_REDO, TB__EDIT_REDO__DIS,
			},
			
			{	MENU_EDIT_CUT, wxID_CUT,
				_("Cu&t\tCtrl+X"),
				_("Cut"),
				_("Cut the selection to the clipboard"),
				_("Cut"),
				_T_PNG2SRC_VARIABLE, TB__EDIT_CUT, TB__EDIT_CUT__DIS,
			},
			
			{	MENU_EDIT_COPY, wxID_COPY,
				_("&Copy\tCtrl+C"),
				_("Copy"),
				_("Copy the selection to the clipboard"),
				_("Copy"),
				_T_PNG2SRC_VARIABLE, TB__EDIT_COPY, TB__EDIT_COPY__DIS,
			},
			
			{	MENU_EDIT_PASTE, wxID_PASTE,
				_("&Paste\tCtrl+V"),
				_("Paste"),
				_("Paste from the clipboard"),
				_("Paste"),
				_T_PNG2SRC_VARIABLE, TB__EDIT_PASTE, TB__EDIT_PASTE__DIS,
			},
			
			{	MENU_EDIT_SELECTALL, wxID_SELECTALL,
				_("Select A&ll\tCtrl+A"),
				NULL,
				_("Select the entire file"),
				NULL,
				_T_NONE, NULL,
			},
			
			// NOTE 2013/06/10 Using F7 and Shift+F7 in the menu on a MAC
			// NOTE            causes it to appear as "fn F7" and "fn F7".
			// NOTE            (we lose the Shift+ on the second one.)
			// NOTE            They work as accelerators, but the menu item
			// NOTE            displayed looks wrong, so I'm going to remove
			// NOTE            the F7 key on the MAC.  This is the original
			// NOTE            binding before I added the +Down/+Up arrow
			// NOTE            keys.
			// NOTE
			// NOTE            I tried making the accelerator "Ctrl+Option+Down"
			// NOTE            but that shows up as "fn <downarrow>" and without
			// NOTE            the Ctrl+Option+ part.  And that looks like the
			// NOTE            binding for PageDown on mini keyboards.  So I'm
			// NOTE            going to just not put an accelerator on the menu
			// NOTE            item.
			// 
			// See: .../wxWidgets-2.9.4/src/osx/cocoa/menuitem.mm:wxMacCocoaMenuItemSetAccelerator()
			// See: http://developer.apple.com/library/mac/#documentation/Cocoa/Conceptual/MenuList/Articles/SettingMenuKeyEquiv.html

			{	MENU_EDIT_NEXT_DELTA, wxID_NONE,
				K( _("&Next Change\tF7"), _("&Next Change\tF7"), _("&Next Change") ),
				_("Next Change"),
				_("Jump to the next change"),
				K( _("Jump to the next change (Alt+Down)"), _("Jump to the next change (Alt+Down)"), _("Jump to the next change (Command+Option+Down)") ),
				_T_PNG2SRC_VARIABLE, TB__CHANGE_NEXT, TB__CHANGE_NEXT__DIS,
			},
			
			{	MENU_EDIT_PREV_DELTA, wxID_NONE,
				K( _("P&revious Change\tShift+F7"), _("P&revious Change\tShift+F7"), _("P&revious Change") ),
				_("Previous Change"),
				_("Jump to the previous change"),
				K( _("Jump to the previous change (Alt+Up)"), _("Jump to the previous change (Alt+Up)"), _("Jump to the previous change (Command+Option+Up)") ),
				_T_PNG2SRC_VARIABLE, TB__CHANGE_PREV, TB__CHANGE_PREV__DIS,
			},
			
			{	MENU_EDIT_NEXT_CONFLICT, wxID_NONE,
				K( _("Ne&xt Conflict\tF8"), _("Ne&xt Conflict\tF8"), _("Ne&xt Conflict") ),
				_("Next Conflict"),
				_("Jump to the next conflict"),
				_("Jump to the next conflict"),
				_T_PNG2SRC_VARIABLE, TB__CONFLICT_NEXT, TB__CONFLICT_NEXT__DIS,
			},
			
			{	MENU_EDIT_PREV_CONFLICT, wxID_NONE,
				K( _("Pre&vious Conflict\tShift+F8"), _("Pre&vious Conflict\tShift+F8"), _("Pre&vious Conflict") ),
				_("Previous Conflict"),
				_("Jump to the previous conflict"),
				_("Jump to the previous conflict"),
				_T_PNG2SRC_VARIABLE, TB__CONFLICT_PREV, TB__CONFLICT_PREV__DIS,
			},
			
			{	MENU_EDIT_AUTOMERGE, wxID_NONE,
				_("&Merge to Center Panel..."),
				_("Auto Merge"),
				_("Automatically merge changes to the center panel"),
				_("Merge to center"),
				_T_PNG2SRC_VARIABLE, TB__AUTO_MERGE, TB__AUTO_MERGE__DIS,
			},

			{	MENU_SETTINGS_PREFERENCES, wxID_PREFERENCES,
				K( _("&Options..."), _("&Options..."), _("Preferences...\tCtrl+,") ),
				NULL,
				_("Open the Options Dialog"),
				NULL,
				_T_NONE, NULL,
			},

			{	MENU_HELP_CONTENTS, wxID_HELP_CONTENTS,
				K( _("&Contents...\tF1"), _("&Contents...\tF1"), _("DiffMerge Help...\tCtrl+?") ),
				NULL,
				K( _("Open local DiffMerge Help"), _("Open local DiffMerge PDF Manual"), _("Open local DiffMerge PDF Manual") ),
				NULL,
				_T_NONE, NULL,
			},
			
			{	MENU_HELP_ABOUT, wxID_ABOUT,
				K( _("&About DiffMerge..."), _("&About DiffMerge..."), _("About DiffMerge") ), // mac doesn't use ... on about
				NULL,
				_("About this application"),
				NULL,
				_T_NONE, NULL,
			},
			
			{	MENU_FOLDER_OPEN_FILES, wxID_NONE,
				_("&Compare Selected Files"),
				_("Compare Files"),
				_("Compare selected file pair in new window"),
				_("Compare Selected Files"),
				_T_PNG2SRC_VARIABLE, TB__FILE_DIFF, TB__FILE_DIFF__DIS,
			},

			{	MENU_FOLDER_OPEN_FOLDERS, wxID_NONE,
				_("Compare &Selected Folders"),
				_("Compare Folders"),
				_("Compare selected folder pair in new window"),
				_("Compare Selected Folders"),
				_T_PNG2SRC_VARIABLE, TB__FOLDER_DIFF, TB__FOLDER_DIFF__DIS,
			},
			
			{	MENU_FOLDER_SHOW_EQUAL, wxID_NONE,
				_("Show E&qual"),
				_("Show Equal"),
				_("Show equal"),
				_("Show Equal"),
				_T_PNG2SRC_VARIABLE, TB__FOLDER_EQUAL, TB__FOLDER_EQUAL,
			},

			{	MENU_FOLDER_SHOW_SINGLES, wxID_NONE,
				_("Show &Peerless"),
				_("Show Peerless"),
				_("Show peerless"),
				_("Show Peerless"),
				_T_PNG2SRC_VARIABLE, TB__FOLDER_NOPEER, TB__FOLDER_NOPEER,
			},

			{	MENU_FOLDER_SHOW_FOLDERS, wxID_NONE,
				_("Show &Folders"),
				_("Show Folders"),
				_("Show Folders"),
				_("Show Folders"),
				_T_PNG2SRC_VARIABLE, TB__FOLDER_FOLDERS, TB__FOLDER_FOLDERS__DIS,
			},

			{	MENU_FOLDER_SHOW_ERRORS, wxID_NONE,
				_("Show &Errors"),
				_("Errors"),
				_("Show Errors"),
				_("Show Errors"),
				_T_NONE, NULL,
			},
			
			{	MENU_VIEWFILE_SHOW_ALL, wxID_NONE,			// id
				_T("Show A&ll"),					// menu label
				_T("Show All"),						// toolbar label
				_T("Show All"),					// status bar
				_T("Show All"),					// tool tip
				_T_PNG2SRC_VARIABLE, TB__DOP_ALL, TB__DOP_ALL__DIS		// toolbar icon
			},	
			{	MENU_VIEWFILE_SHOW_DIF, wxID_NONE,			// id
				_T("&Differences Only"),			// menu label
				_T("Show Diffs"),					// toolbar label
				_T("Show Differences Only"),	// status bar
				_T("Show Differences Only"),	// tool tip
				_T_PNG2SRC_VARIABLE, TB__DOP_DIF, TB__DOP_DIF__DIS		// toolbar icon
			},	
			{	MENU_VIEWFILE_SHOW_CTX, wxID_NONE,			// id
				_T("Differences with Conte&xt"),	// menu label
				_T("Show Context"),							// toolbar label
				_T("Show Differences with Context"),	// status bar
				_T("Show Differences with Context"),	// tool tip
				_T_PNG2SRC_VARIABLE, TB__DOP_CTX, TB__DOP_CTX__DIS		// toolbar icon
			},	

			{	MENU_VISIT_SG, wxID_NONE,			// id -- link to SG website to see other products/offers
				_T("Visit SourceGear..."),	// menu label
				_T(""),						// toolbar label
				_T("Visit SourceGear"),		// status bar
				_T("Visit SourceGear"),		// tool tip
				_T_NONE, NULL,
			},	

			{	MENU_VIEWFILE_IGN_UNIMPORTANT, wxID_NONE,	// id
				_T("Hide &Unimportant Differences\tCtrl+U"),	// menu label
				_T(""),							// toolbar label
				_T("Do Not Highlight Unimportant Differences"),	// status bar
				_T(""),	// tool tip
				_T_NONE, NULL,
			},	
			{	MENU_VIEWFILE_HIDE_OMITTED,wxID_NONE,	// id
				_T("Hide &Omitted Lines"),		// menu label
				_T(""),					// toolbar label
				_T("Hide Content Omitted from Comparison"),	// status bar
				_T(""),	// tool tip
				_T_NONE, NULL,
			},	
			{	MENU_VIEWFILE_PILCROW, wxID_NONE,			// id
				_T("&Show Invisibles"),							// menu label
				_T("Show Invisibles"),							// toolbar label
				_T("Show Invisibles"),							// status bar
				_T("Show Invisibles"),							// tool tip
				_T_PNG2SRC_VARIABLE, TB__PILCROW, TB__PILCROW,	// toolbar icon
			},	
			{	MENU_VIEWFILE_TAB2, wxID_NONE,			// id
				_T("&2 Space Tabs"),							// menu label
				_T(""),							// toolbar label
				_T("2 Space Tabs"),							// status bar
				_T(""),							// tool tip
				_T_NONE, NULL,
			},	
			{	MENU_VIEWFILE_TAB4, wxID_NONE,			// id
				_T("&4 Space Tabs"),							// menu label
				_T(""),							// toolbar label
				_T("4 Space Tabs"),							// status bar
				_T(""),							// tool tip
				_T_NONE, NULL,
			},	
			{	MENU_VIEWFILE_TAB8, wxID_NONE,			// id
				_T("&8 Space Tabs"),							// menu label
				_T(""),							// toolbar label
				_T("8 Space Tabs"),							// status bar
				_T(""),							// tool tip
				_T_NONE, NULL,
			},	
			{	MENU_VIEWFILE_LINENUMBERS, wxID_NONE,			// id
				_T("Show Line &Numbers"),							// menu label
				_T("Show Line Numbers"),							// toolbar label
				_T("Show Line Numbers"),							// status bar
				_T("Show Line Numbers"),							// tool tip
				_T_PNG2SRC_VARIABLE, TB__LINE_NUMBERS, TB__LINE_NUMBERS,	// toolbar icon
			},	

			{	MENU_FILE_CHANGE_RULESET, wxID_NONE,
				_("C&hange Ruleset..."),
				NULL,
				_("Change Ruleset for This Set of Files"),
				NULL,
				_T_NONE, NULL,
			},

			{	MENU_PRINT, wxID_PRINT,
				_("&Print...\tCtrl+P"),
				NULL,
				_("Print the Contents of This Window"),
				NULL,
				_T_NONE, NULL,
			},

			{	MENU_PRINT_PREVIEW, wxID_PREVIEW,
				_("Print Pre&view..."),
				NULL,
				_("Preview Printing"),
				NULL,
				_T_NONE, NULL,
			},

			{	MENU_PAGE_SETUP, wxID_PAGE_SETUP,
				_("Pa&ge Setup...\tCtrl+Shift+P"),
				NULL,
				_("Page Setup"),
				NULL,
				_T_NONE, NULL,
			},

			{	MENU_FOLDER_SHOW_EQUIVALENT, wxID_NONE,
				_("Show Equi&valent Files"),
				_("Show Equivalent"),
				_("Show equivalent files"),
				_("Show Equivalent Files"),
				_T_PNG2SRC_VARIABLE, TB__FOLDER_EQUIVALENT, TB__FOLDER_EQUIVALENT,
			},
			
			{	MENU_VIEWFILE_INSERT_MARK, wxID_NONE,
				_("&Manual Alignment Marker..."),
				NULL,
				_("Insert/Delete Manual Alignment Markers"),
				NULL,
				_T_NONE, NULL,
			},

			{	MENU_VIEWFILE_DELETE_ALL_MARK, wxID_NONE,
				_("Delete All Manual Alignment Markers"),
				NULL,
				_("Delete All Manual Alignments Markers on this Page"),
				NULL,
				_T_NONE, NULL,
			},

			{	MENU_EDIT_FIND, wxID_FIND,
				_("&Find...\tCtrl+F"),
				NULL,
				_("Find Text Within Documents"),
				NULL,
				_T_NONE, NULL,
			},

			{	MENU_EDIT_APPLY_DEFAULT_L, wxID_NONE,
				_("Apply Change from Left"),
				_("Apply Change from Left"),
				_("Apply Change from Left"),
				K( _("Apply Change from Left (Alt+Right)"), _("Apply Change from Left (Alt+Right)"), _("Apply Change from Left (Command+Option+Right)") ),
				_T_PNG2SRC_VARIABLE, TB__APPLY_LEFT, TB__APPLY_LEFT__DIS,
			},

			{	MENU_EDIT_APPLY_DEFAULT_R, wxID_NONE,
				_("Apply Change from Right"),
				_("Apply Change from Right"),
				_("Apply Change from Right"),
				K( _("Apply Change from Right (Alt+Left)"), _("Apply Change from Right (Alt+Left)"), _("Apply Change from Right (Command+Option+Left)") ),
				_T_PNG2SRC_VARIABLE, TB__APPLY_RIGHT, TB__APPLY_RIGHT__DIS,
			},

			{	MENU_EDIT_FIND_PREV, wxID_NONE,		// id
				K( _("&Find Previous\tShift+F3"), _("&Find Previous\tShift+F3"), _("&Find Previous\tShift+Ctrl+G") ),
				NULL,
				_T("Find Again Backward"),				// status bar
				NULL,
				_T_NONE, NULL,					// toolbar icon
			},	

			{	MENU_EDIT_FIND_NEXT, wxID_NONE,
				K( _("&Find Next\tF3"), _("&Find Next\tF3"), _("&Find Next\tCtrl+G") ),
				NULL,
				_("Find Again Forward"),
				NULL,
				_T_NONE, NULL,
			},

			{	MENU_EDIT_GOTO, wxID_NONE,
				K( _("&Go To Line...\tCtrl+G"), _("&Go To Line...\tCtrl+G"), _("&Go To Line...") ),
				NULL,
				_("Go to Line in Document"),
				NULL,
				_T_NONE, NULL,
			},

			{	MENU_VIEWFILE_SPLIT_VERTICALLY, wxID_NONE,
				_("Split Window &Vertically"),
				_("Split Window Vertically"),
				_("Split Window Vertically"),
				_("Split Window Vertically"),
				_T_PNG2SRC_VARIABLE, TB__SPLIT_VERTICALLY, TB__SPLIT_VERTICALLY,
			},

			{	MENU_VIEWFILE_SPLIT_HORIZONTALLY, wxID_NONE,
				_("Split Window &Horizontally"),
				_("Split Window Horizontally"),
				_("Split Window Horizontally"),
				_("Split Window Horizontally"),
				_T_PNG2SRC_VARIABLE, TB__SPLIT_HORIZONTALLY, TB__SPLIT_HORIZONTALLY,
			},

			{	MENU_LICENSE_KEY, wxID_NONE,
				_("Registration..."),
				_(""),
				_("Open the Registration Dialog"),
				_(""),
				_T_NONE, NULL,
			},
			{	MENU_CHECK_NEW_REVISION, wxID_NONE,
				_("Check for Updates..."),
				_(""),
				_("Check for DiffMerge Updates"),
				_(""),
				_T_NONE, NULL,
			},
			
			{	MENU_WEBHELP, wxID_NONE,
				_("Online Help..."),
				NULL,
				_("Open web browser to online version of the manual"),
				NULL,
				_T_NONE, NULL,
			},

			{	MENU_EXPORT__FOLDER_SUMMARY__CSV__FILE, wxID_NONE,
				_("To File..."),
				NULL,
				_("Write CSV summary of this folder window to a file"),
				NULL,
				_T_NONE, NULL,
			},

			{	MENU_EXPORT__FOLDER_SUMMARY__RQ__FILE, wxID_NONE,
				_("To File..."),
				NULL,
				_("Write plain text summary of this folder window to a file"),
				NULL,
				_T_NONE, NULL,
			},

			{	MENU_EXPORT__FOLDER_SUMMARY__RQ__CLIPBOARD, wxID_NONE,
				_("To Clipboard"),
				NULL,
				_("Write plain text summary of this folder window to the clipboard"),
				NULL,
				_T_NONE, NULL,
			},

			{	MENU_FOLDER_OPEN_SHORTCUT_INFO, wxID_NONE,
				_("Shortcut Details..."),
				NULL,
				_("Opens the Shortcut Details dialog the selected row"),
				NULL,
				_T_NONE, NULL,
			},

			{	MENU_FOLDER_OPEN_SYMLINK_INFO, wxID_NONE,
				_("Symlink Details..."),
				NULL,
				_("Opens the Symlink Details dialog the selected row"),
				NULL,
				_T_NONE, NULL,
			},

			{	MENU_EDIT_USE_SELECTION_FOR_FIND, wxID_NONE,
				_("Use Selection for Find\tCtrl+E"),
				NULL,
				_("Use current selection as basis for subsequent Find Next or Find Prev commands"),
				NULL,
				_T_NONE, NULL,
			},

			{	MENU_EXPORT__FILE_DIFFS__UNIFIED__TEXT__FILE, wxID_NONE,
				_("To File..."),
				NULL,
				_("Write unified text diffs to a file"),
				NULL,
				_T_NONE, NULL,
			},

			{	MENU_EXPORT__FILE_DIFFS__TRADITIONAL__TEXT__FILE, wxID_NONE,
				_("To File..."),
				NULL,
				_("Write traditional text diffs to a file"),
				NULL,
				_T_NONE, NULL,
			},

			{	MENU_EXPORT__FILE_DIFFS__UNIFIED__TEXT__CLIPBOARD, wxID_NONE,
				_("To Clipboard"),
				NULL,
				_("Write unified text diffs to the clipboard"),
				NULL,
				_T_NONE, NULL,
			},

			{	MENU_EXPORT__FILE_DIFFS__TRADITIONAL__TEXT__CLIPBOARD, wxID_NONE,
				_("To Clipboard"),
				NULL,
				_("Write traditional text diffs to the clipboard"),
				NULL,
				_T_NONE, NULL,
			},

			{	MENU_FOLDER_SHOW_QUICKMATCH, wxID_NONE,
				_("Show Quick-Match Files"),
				_("Show Quick-Match"),
				_("Show quick-match files"),
				_("Show Quick-Match Files"),
				_T_PNG2SRC_VARIABLE, TB__FOLDER_QUICKMATCH, TB__FOLDER_QUICKMATCH,
			},
			
			{	MENU_EXPORT__FOLDER_SUMMARY__HTML__FILE, wxID_NONE,
				_("To File..."),
				NULL,
				_("Write HTML summary of this folder window to a file"),
				NULL,
				_T_NONE, NULL,
			},

			{	MENU_EXPORT__FILE_DIFFS__UNIFIED__HTML__FILE, wxID_NONE,
				_("To File..."),
				NULL,
				_("Write unified html diffs to a file"),
				NULL,
				_T_NONE, NULL,
			},

			{	MENU_EXPORT__FILE_DIFFS__TRADITIONAL__HTML__FILE, wxID_NONE,
				_("To File..."),
				NULL,
				_("Write normal html diffs to a file"),
				NULL,
				_T_NONE, NULL,
			},

			{	MENU_EXPORT__FILE_DIFFS__SXS__HTML__FILE, wxID_NONE,
				_("To File..."),
				NULL,
				_("Write side-by-side html diffs to a file"),
				NULL,
				_T_NONE, NULL,
			},

//			{	MENU_VIEWFILE_SHOW_, wxID_NONE,	// id
//				_T(""),							// menu label
//				_T(""),							// toolbar label
//				_T(""),							// status bar
//				_T(""),							// tool tip
//				_T_XPM_VARIABLE, TB_XPM__BLANK,	// toolbar icon
//			},	
			
		};

	wxASSERT_MSG( ((__MENU__COUNT__ - __MENU__START__)== NrElements(saDefs)), _T("Coding Error") );
	wxASSERT_MSG( ((id >= __MENU__START__) && (id < __MENU__COUNT__)), _T("Coding Error") );
	wxASSERT_MSG( (saDefs[id - __MENU__START__].id == id), _T("Coding Error") );

	return &(saDefs[id - __MENU__START__]);
}
