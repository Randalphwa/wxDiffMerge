// gui_frame__iface_menu.cpp
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

/*static*/ void gui_frame::_iface_add_menu_item(wxMenu * pMenu, gui_frame::_iface_id id, wxItemKind kind)
{
	const _iface_def * pDef = gui_frame::_get_iface_def(id);

	if (!pDef)
		return;

	int local_id = ((pDef->wx_id != wxID_NONE) ? pDef->wx_id : pDef->id);

	pMenu->Append(local_id,pDef->szMenuLabel,pDef->szStatusBar,kind);
}

//////////////////////////////////////////////////////////////////

void gui_frame::_iface_create_file_menu(void)
{
	wxASSERT_MSG( (m_pMenu_File==NULL), _T("Coding Error") );

	// File Menu

	m_pMenu_File = new wxMenu();

#if defined(__WXMAC__)
	// On Mac, wxWidgets will move the Preferences/Options
	// item to the Apple menu.  But with 2.9.4 we must add
	// the item to the menu *BEFORE* we append the menu to
	// the menubar (because wxMenu::DoRearrange() gets called by
	// wxMenuBar::Append().
	_iface_add_menu_item(m_pMenu_File,MENU_SETTINGS_PREFERENCES);
#endif

	_iface_add_menu_item(m_pMenu_File,MENU_FILE_FOLDER_DIFF);
	m_pMenu_File->AppendSeparator();
	_iface_add_menu_item(m_pMenu_File,MENU_FILE_FILE_DIFF);
	_iface_add_menu_item(m_pMenu_File,MENU_FILE_FILE_MERGE);
	m_pMenu_File->AppendSeparator();
	_iface_add_menu_item(m_pMenu_File,MENU_FILE_RELOAD);
	_iface_add_menu_item(m_pMenu_File,MENU_FILE_CHANGE_RULESET);
	m_pMenu_File->AppendSeparator();
	_iface_add_menu_item(m_pMenu_File,MENU_FILE_SAVE);
	_iface_add_menu_item(m_pMenu_File,MENU_FILE_SAVEAS);
	_iface_add_menu_item(m_pMenu_File,MENU_FILE_SAVEALL);
	m_pMenu_File->AppendSeparator();
	_iface_add_menu_item(m_pMenu_File,MENU_PAGE_SETUP);
	_iface_add_menu_item(m_pMenu_File,MENU_PRINT_PREVIEW);
	_iface_add_menu_item(m_pMenu_File,MENU_PRINT);
	m_pMenu_File->AppendSeparator();
	_iface_add_menu_item(m_pMenu_File,MENU_FILE_CLOSE_WINDOW);
	_iface_add_menu_item(m_pMenu_File,MENU_FILE_EXIT);

	m_pMenuBar->Append(m_pMenu_File,		_("&File"));
}

void gui_frame::_iface_create_edit_menu(int pos)
{
	wxASSERT_MSG( (m_pMenu_Edit==NULL), _T("Coding Error") );

	// Edit Menu

	m_pMenu_Edit = new wxMenu();
	_iface_add_menu_item(m_pMenu_Edit,MENU_EDIT_UNDO);
	_iface_add_menu_item(m_pMenu_Edit,MENU_EDIT_REDO);
	m_pMenu_Edit->AppendSeparator();
	_iface_add_menu_item(m_pMenu_Edit,MENU_EDIT_CUT);
	_iface_add_menu_item(m_pMenu_Edit,MENU_EDIT_COPY);
	_iface_add_menu_item(m_pMenu_Edit,MENU_EDIT_PASTE);
	_iface_add_menu_item(m_pMenu_Edit,MENU_EDIT_SELECTALL);
	m_pMenu_Edit->AppendSeparator();
	_iface_add_menu_item(m_pMenu_Edit,MENU_EDIT_FIND);
	_iface_add_menu_item(m_pMenu_Edit,MENU_EDIT_FIND_NEXT);
	_iface_add_menu_item(m_pMenu_Edit,MENU_EDIT_FIND_PREV);
	_iface_add_menu_item(m_pMenu_Edit,MENU_EDIT_USE_SELECTION_FOR_FIND);
	_iface_add_menu_item(m_pMenu_Edit,MENU_EDIT_GOTO);
	m_pMenu_Edit->AppendSeparator();
	_iface_add_menu_item(m_pMenu_Edit,MENU_EDIT_NEXT_DELTA);
	_iface_add_menu_item(m_pMenu_Edit,MENU_EDIT_PREV_DELTA);
	m_pMenu_Edit->AppendSeparator();
	_iface_add_menu_item(m_pMenu_Edit,MENU_EDIT_NEXT_CONFLICT);
	_iface_add_menu_item(m_pMenu_Edit,MENU_EDIT_PREV_CONFLICT);
	m_pMenu_Edit->AppendSeparator();
	_iface_add_menu_item(m_pMenu_Edit,MENU_EDIT_AUTOMERGE);

	m_pMenuBar->Insert(pos,m_pMenu_Edit,_("&Edit"));
}

void gui_frame::_iface_create_view_menu(int pos, ToolBarType tbt)
{
	// View Menu

	m_pMenu_View = new wxMenu();

	if (tbt == TBT_FOLDER)
	{
		_iface_add_menu_item(m_pMenu_View,MENU_FOLDER_OPEN_FILES);
		_iface_add_menu_item(m_pMenu_View,MENU_FOLDER_OPEN_FOLDERS);
#if defined(__WXMSW__)
		_iface_add_menu_item(m_pMenu_View,MENU_FOLDER_OPEN_SHORTCUT_INFO);
#endif
#if defined(__WXMAC__) || defined(__WXGTK__)
		_iface_add_menu_item(m_pMenu_View,MENU_FOLDER_OPEN_SYMLINK_INFO);
#endif
		m_pMenu_View->AppendSeparator();
		_iface_add_menu_item(m_pMenu_View,MENU_FOLDER_SHOW_EQUAL,wxITEM_CHECK);
		_iface_add_menu_item(m_pMenu_View,MENU_FOLDER_SHOW_EQUIVALENT,wxITEM_CHECK);
		_iface_add_menu_item(m_pMenu_View,MENU_FOLDER_SHOW_QUICKMATCH,wxITEM_CHECK);
		_iface_add_menu_item(m_pMenu_View,MENU_FOLDER_SHOW_SINGLES,wxITEM_CHECK);
		_iface_add_menu_item(m_pMenu_View,MENU_FOLDER_SHOW_FOLDERS,wxITEM_CHECK);
		_iface_add_menu_item(m_pMenu_View,MENU_FOLDER_SHOW_ERRORS,wxITEM_CHECK);
	}
	else if ((tbt == TBT_DIFF) || (tbt==TBT_MERGE))
	{
		_iface_add_menu_item(m_pMenu_View,MENU_VIEWFILE_SHOW_ALL,wxITEM_RADIO);
		_iface_add_menu_item(m_pMenu_View,MENU_VIEWFILE_SHOW_DIF,wxITEM_RADIO);
		_iface_add_menu_item(m_pMenu_View,MENU_VIEWFILE_SHOW_CTX,wxITEM_RADIO);
		m_pMenu_View->AppendSeparator();
		_iface_add_menu_item(m_pMenu_View,MENU_VIEWFILE_IGN_UNIMPORTANT,wxITEM_CHECK);
		_iface_add_menu_item(m_pMenu_View,MENU_VIEWFILE_HIDE_OMITTED,wxITEM_CHECK);
		_iface_add_menu_item(m_pMenu_View,MENU_VIEWFILE_LINENUMBERS,wxITEM_CHECK);
		_iface_add_menu_item(m_pMenu_View,MENU_VIEWFILE_PILCROW,wxITEM_CHECK);
		m_pMenu_View->AppendSeparator();
		_iface_add_menu_item(m_pMenu_View,MENU_VIEWFILE_TAB2,wxITEM_RADIO);
		_iface_add_menu_item(m_pMenu_View,MENU_VIEWFILE_TAB4,wxITEM_RADIO);
		_iface_add_menu_item(m_pMenu_View,MENU_VIEWFILE_TAB8,wxITEM_RADIO);
		m_pMenu_View->AppendSeparator();
		_iface_add_menu_item(m_pMenu_View,MENU_VIEWFILE_INSERT_MARK);
		_iface_add_menu_item(m_pMenu_View,MENU_VIEWFILE_DELETE_ALL_MARK);
		m_pMenu_View->AppendSeparator();
		_iface_add_menu_item(m_pMenu_View,MENU_VIEWFILE_SPLIT_VERTICALLY,wxITEM_RADIO);
		_iface_add_menu_item(m_pMenu_View,MENU_VIEWFILE_SPLIT_HORIZONTALLY,wxITEM_RADIO);
	}

	m_pMenuBar->Insert(pos,m_pMenu_View,_("&View"));
}

void gui_frame::_iface_create_export_menu(int pos, ToolBarType tbt)
{
	bool bHaveFileDiffSection = false;

	// Export Menu

	m_pMenu_Export = new wxMenu();

	if ((tbt == TBT_DIFF) || (tbt == TBT_FOLDER))
	{
		wxMenu * pMenu_file_diff = new wxMenu();
		{
			wxMenu * pMenu_file_diff_unified = new wxMenu();
			{
				wxMenu * pMenu_file_diff_unified_html = new wxMenu();
				_iface_add_menu_item(pMenu_file_diff_unified_html, MENU_EXPORT__FILE_DIFFS__UNIFIED__HTML__FILE);
				// TODO add to-brower
				pMenu_file_diff_unified->AppendSubMenu(pMenu_file_diff_unified_html, _T("In HTML"));

				wxMenu * pMenu_file_diff_unified_text = new wxMenu();
				_iface_add_menu_item(pMenu_file_diff_unified_text, MENU_EXPORT__FILE_DIFFS__UNIFIED__TEXT__FILE);
				_iface_add_menu_item(pMenu_file_diff_unified_text, MENU_EXPORT__FILE_DIFFS__UNIFIED__TEXT__CLIPBOARD);
				pMenu_file_diff_unified->AppendSubMenu(pMenu_file_diff_unified_text, _T("In Text"));
			}
			pMenu_file_diff->AppendSubMenu(pMenu_file_diff_unified, _T("Unified"));

			wxMenu * pMenu_file_diff_traditional = new wxMenu();
			{
				wxMenu * pMenu_file_diff_traditional_html = new wxMenu();
				_iface_add_menu_item(pMenu_file_diff_traditional_html, MENU_EXPORT__FILE_DIFFS__TRADITIONAL__HTML__FILE);
				// TODO add to-brower
				pMenu_file_diff_traditional->AppendSubMenu(pMenu_file_diff_traditional_html, _T("In HTML"));

				wxMenu * pMenu_file_diff_traditional_text = new wxMenu();
				_iface_add_menu_item(pMenu_file_diff_traditional_text, MENU_EXPORT__FILE_DIFFS__TRADITIONAL__TEXT__FILE);
				_iface_add_menu_item(pMenu_file_diff_traditional_text, MENU_EXPORT__FILE_DIFFS__TRADITIONAL__TEXT__CLIPBOARD);
				pMenu_file_diff_traditional->AppendSubMenu(pMenu_file_diff_traditional_text, _T("In Text"));
			}
			pMenu_file_diff->AppendSubMenu(pMenu_file_diff_traditional, _T("Traditional"));

			wxMenu * pMenu_file_diff_sxs = new wxMenu();
			{
				wxMenu * pMenu_file_diff_sxs_html = new wxMenu();
				_iface_add_menu_item(pMenu_file_diff_sxs_html, MENU_EXPORT__FILE_DIFFS__SXS__HTML__FILE);
				// TODO add to-brower
				pMenu_file_diff_sxs->AppendSubMenu(pMenu_file_diff_sxs_html, _T("In HTML"));
			}
			pMenu_file_diff->AppendSubMenu(pMenu_file_diff_sxs, _T("Side-by-Side"));
		}

		m_pMenu_Export->AppendSubMenu(pMenu_file_diff, _T("File Diffs"),
									  _T("Export diffs of this file pair"));

		bHaveFileDiffSection = true;
	}


	if (tbt == TBT_FOLDER)
	{
		if (bHaveFileDiffSection)
			m_pMenu_Export->AppendSeparator();
			
		wxMenu * pMenu_folder_summary = new wxMenu();
		{
			wxMenu * pMenu_folder_summary_html = new wxMenu();
			_iface_add_menu_item(pMenu_folder_summary_html, MENU_EXPORT__FOLDER_SUMMARY__HTML__FILE);
			// TODO add to-brower
			pMenu_folder_summary->AppendSubMenu(pMenu_folder_summary_html, _T("In HTML"));
		}
		{
			wxMenu * pMenu_folder_summary_rq = new wxMenu();
			_iface_add_menu_item(pMenu_folder_summary_rq, MENU_EXPORT__FOLDER_SUMMARY__RQ__FILE);
			_iface_add_menu_item(pMenu_folder_summary_rq, MENU_EXPORT__FOLDER_SUMMARY__RQ__CLIPBOARD);
			pMenu_folder_summary->AppendSubMenu(pMenu_folder_summary_rq, _T("In Text"));
		}
		{
			wxMenu * pMenu_folder_summary_csv = new wxMenu();
			_iface_add_menu_item(pMenu_folder_summary_csv, MENU_EXPORT__FOLDER_SUMMARY__CSV__FILE);
			pMenu_folder_summary->AppendSubMenu(pMenu_folder_summary_csv, _T("In CSV"));
		}

		m_pMenu_Export->AppendSubMenu(pMenu_folder_summary, _T("Folder Summary"),
									_T("Export a summary of the current folder window"));
	}

	m_pMenuBar->Insert(pos,m_pMenu_Export,_("Export"));
}

void gui_frame::_iface_create_settings_menu(void)
{
	// Settings Menu

	wxASSERT_MSG( (m_pMenu_Settings==NULL), _T("Coding Error") );

	m_pMenu_Settings = new wxMenu();

#if defined(__WXMAC__)
	// on the mac, PREFERENCES appears under the system's application menu.
	// under wxWidgets 2.8, we cheated here and appended it to the file menu
	// rather than creating a new tool menu and would have to then be deleted
	// or appear as empty.  we can't do that with 2.9/cocoa because DoRearrange()
	// has already been called.
#else
	_iface_add_menu_item(m_pMenu_Settings,MENU_SETTINGS_PREFERENCES);
	m_pMenu_Settings->AppendSeparator();
#endif

	m_pMenuBar->Append(m_pMenu_Settings,	_("&Tools")); // TODO on Mac this menu is empty
}

void gui_frame::_iface_create_help_menu(void)
{
	wxASSERT_MSG( (m_pMenu_Help==NULL), _T("Coding Error") );
	
	// Help Menu

	m_pMenu_Help = new wxMenu();
	_iface_add_menu_item(m_pMenu_Help,MENU_HELP_CONTENTS);
	_iface_add_menu_item(m_pMenu_Help,MENU_WEBHELP);
	_iface_add_menu_item(m_pMenu_Help,MENU_HELP_ABOUT);

	wxString helpTitle( _("&Help") );

	m_pMenuBar->Append(m_pMenu_Help,		helpTitle);
}

//////////////////////////////////////////////////////////////////

void gui_frame::_iface_define_menu(ToolBarType tbt)
{
	// we dynamically change the contents of the menubar based
	// upon the type of window that we are.  the first window
	// we open (when nothing given on the command line) is called
	// a BASIC window.  it does not have EDIT or VIEW menus.
	// once the window is given a type (either FOLDERS or a set
	// of FILES), we INSERT the appropriate menus into the middle
	// of the menubar.  if a typed window has an error and cannot
	// load, we revert the window back to a BASIC one and remove
	// the EDIT and/or VIEW menus that were added.
	//
	// when we are creating a new window and already know the
	// type, we create the fully populated menu in one step
	// here; that is, we call __initial() and then _create_{edit,view}_
	// before returning.

	if (!m_pMenuBar)
	{
#if defined(__WXMAC__)
		// on the mac, ABOUT, PREFERENCES, and EXIT appear under the system's
		// application menu (the one with the app name and next to the 'apple' menu).
		// also, 'exit' is relabeled 'quit'.
		//
		// tell wxWidgets what id's they have and/or where they are and wxWidgets
		// will secretly steal them and move them to the application menu.
		//
		// NOTE: With wxWidgets 2.9.4, we need to set the wxApp::s_mac{...}
		//       fields *before* wxMenu::DoRearrange() gets called by wxMenuBar::Append().
		//       Otherwise, the ABOUT, PREFERENCES, and EXIT don't get moved
		//       to the Apple menu on the menu for the first frame window.

		wxApp::s_macExitMenuItemId = wxID_EXIT;
		wxApp::s_macPreferencesMenuItemId = wxID_PREFERENCES;
		wxString helpTitle( _("&Help") );
		wxApp::s_macHelpMenuTitleName = helpTitle;
		wxApp::s_macAboutMenuItemId = wxID_ABOUT;
#endif

		// if we do not have a menu, create the initial BASIC one.
		// this is good for empty windows and serves as a base for
		// all other windows types.
	
		m_pMenuBar = new wxMenuBar(0 /*not wxMB_DOCKABLE*/);

		_iface_create_file_menu();
		_iface_create_settings_menu();
		_iface_create_help_menu();

		SetMenuBar(m_pMenuBar);

		m_tbtMenu = TBT_BASIC;
	}

	if (m_tbtMenu == tbt)		// we already have the proper menu
		return;					// or we just created a basic one.

	if (m_tbtMenu == TBT_BASIC)
	{
		// an empty window finally received some content
		// add Edit and/or View Menus as appropriate.

		if ((tbt==TBT_DIFF) || (tbt==TBT_MERGE))
		{
			_iface_create_edit_menu(1);
			_iface_create_view_menu(2,tbt);
			if (tbt==TBT_DIFF) 
				_iface_create_export_menu(3,tbt);
			m_tbtMenu = tbt;	// remember the type of menu we created.
			return;
		}
		if (tbt==TBT_FOLDER)
		{
			_iface_create_view_menu(1,tbt);
			_iface_create_export_menu(2,tbt);
			m_tbtMenu = tbt;	// remember the type of menu we created.
			return;
		}

		wxASSERT_MSG( (0), _T("Coding Error") );
		return;
	}

	if (tbt == TBT_BASIC)
	{
		// a purposed window is being reverted back to
		// an empty window -- happens, for example, when
		// there are file errors loading the files given
		// on the command line.

		if (m_tbtMenu==TBT_DIFF)
		{
			wxASSERT_MSG( (m_pMenuBar->GetMenu(3) == m_pMenu_Export), _T("Coding Error") );
			m_pMenuBar->Remove(3);
			DELETEP(m_pMenu_Export);
		}

		if ((m_tbtMenu==TBT_DIFF) || (m_tbtMenu==TBT_MERGE))
		{
			wxASSERT_MSG( (m_pMenuBar->GetMenu(2) == m_pMenu_View), _T("Coding Error") );
			m_pMenuBar->Remove(2);
			DELETEP(m_pMenu_View);

			wxASSERT_MSG( (m_pMenuBar->GetMenu(1) == m_pMenu_Edit), _T("Coding Error") );
			m_pMenuBar->Remove(1);
			DELETEP(m_pMenu_Edit);

			m_tbtMenu = tbt;
			return;
		}

		if (m_tbtMenu==TBT_FOLDER)
		{
			wxASSERT_MSG( (m_pMenuBar->GetMenu(2) == m_pMenu_Export), _T("Coding Error") );
			m_pMenuBar->Remove(2);
			DELETEP(m_pMenu_Export);

			wxASSERT_MSG( (m_pMenuBar->GetMenu(1) == m_pMenu_View), _T("Coding Error") );
			m_pMenuBar->Remove(1);
			DELETEP(m_pMenu_View);
			
			m_tbtMenu = tbt;
			return;
		}

		wxASSERT_MSG( (0), _T("Coding Error") );
		return;
	}

	wxASSERT_MSG( (0), _T("Coding Error") );
}

