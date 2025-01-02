// gui_frame__iface_defs.h
// a portion of class gui_frame.
//////////////////////////////////////////////////////////////////

	// menu IDs are used by both the menubar and toolbar.
	// the frame cannot tell how the event was triggered.

public:
	typedef enum _iface_id
	{
		__MENU__START__				= wxID_HIGHEST + 1,

		MENU_FILE_FOLDER_DIFF		= wxID_HIGHEST + 1,
		MENU_FILE_FILE_DIFF			= wxID_HIGHEST + 2,
		MENU_FILE_FILE_MERGE		= wxID_HIGHEST + 3,
		MENU_FILE_RELOAD			= wxID_HIGHEST + 4,
		MENU_FILE_SAVE				= wxID_HIGHEST + 5,
		MENU_FILE_SAVEAS			= wxID_HIGHEST + 6,
		MENU_FILE_SAVEALL			= wxID_HIGHEST + 7,
		MENU_FILE_CLOSE_WINDOW		= wxID_HIGHEST + 8,
		MENU_FILE_EXIT				= wxID_HIGHEST + 9,

		MENU_EDIT_UNDO				= wxID_HIGHEST + 10,
		MENU_EDIT_REDO				= wxID_HIGHEST + 11,
		MENU_EDIT_CUT				= wxID_HIGHEST + 12,
		MENU_EDIT_COPY				= wxID_HIGHEST + 13,
		MENU_EDIT_PASTE				= wxID_HIGHEST + 14,
		MENU_EDIT_SELECTALL			= wxID_HIGHEST + 15,
		MENU_EDIT_NEXT_DELTA		= wxID_HIGHEST + 16,
		MENU_EDIT_PREV_DELTA		= wxID_HIGHEST + 17,
		MENU_EDIT_NEXT_CONFLICT		= wxID_HIGHEST + 18,
		MENU_EDIT_PREV_CONFLICT		= wxID_HIGHEST + 19,
		MENU_EDIT_AUTOMERGE			= wxID_HIGHEST + 20,

		MENU_SETTINGS_PREFERENCES	= wxID_HIGHEST + 21,

		MENU_HELP_CONTENTS			= wxID_HIGHEST + 22,
		MENU_HELP_ABOUT				= wxID_HIGHEST + 23,

		MENU_FOLDER_OPEN_FILES		= wxID_HIGHEST + 24,
		MENU_FOLDER_OPEN_FOLDERS	= wxID_HIGHEST + 25,
		MENU_FOLDER_SHOW_EQUAL		= wxID_HIGHEST + 26,
		MENU_FOLDER_SHOW_SINGLES	= wxID_HIGHEST + 27,
		MENU_FOLDER_SHOW_FOLDERS	= wxID_HIGHEST + 28,
		MENU_FOLDER_SHOW_ERRORS		= wxID_HIGHEST + 29,

		MENU_VIEWFILE_SHOW_ALL		= wxID_HIGHEST + 30,
		MENU_VIEWFILE_SHOW_DIF		= wxID_HIGHEST + 31,
		MENU_VIEWFILE_SHOW_CTX		= wxID_HIGHEST + 32,

		MENU_VISIT_SG			= wxID_HIGHEST + 33,

		MENU_VIEWFILE_IGN_UNIMPORTANT	= wxID_HIGHEST + 34,	// treat unimportant changes as equal text
		MENU_VIEWFILE_HIDE_OMITTED		= wxID_HIGHEST + 35,

		MENU_VIEWFILE_PILCROW		= wxID_HIGHEST + 36,
		MENU_VIEWFILE_TAB2			= wxID_HIGHEST + 37,
		MENU_VIEWFILE_TAB4			= wxID_HIGHEST + 38,
		MENU_VIEWFILE_TAB8			= wxID_HIGHEST + 39,
		MENU_VIEWFILE_LINENUMBERS	= wxID_HIGHEST + 40,

		MENU_FILE_CHANGE_RULESET	= wxID_HIGHEST + 41,

		MENU_PRINT					= wxID_HIGHEST + 42,
		MENU_PRINT_PREVIEW			= wxID_HIGHEST + 43,
		MENU_PAGE_SETUP				= wxID_HIGHEST + 44,

		MENU_FOLDER_SHOW_EQUIVALENT	= wxID_HIGHEST + 45,
		MENU_VIEWFILE_INSERT_MARK	= wxID_HIGHEST + 46,
		MENU_VIEWFILE_DELETE_ALL_MARK	= wxID_HIGHEST + 47,
		MENU_EDIT_FIND				= wxID_HIGHEST + 48,
		MENU_EDIT_APPLY_DEFAULT_L	= wxID_HIGHEST + 49,
		MENU_EDIT_APPLY_DEFAULT_R	= wxID_HIGHEST + 50,
		MENU_EDIT_FIND_PREV			= wxID_HIGHEST + 51,
		MENU_EDIT_FIND_NEXT			= wxID_HIGHEST + 52,
		MENU_EDIT_GOTO				= wxID_HIGHEST + 53,

		MENU_VIEWFILE_SPLIT_VERTICALLY		= wxID_HIGHEST + 54,
		MENU_VIEWFILE_SPLIT_HORIZONTALLY	= wxID_HIGHEST + 55,

		MENU_LICENSE_KEY = wxID_HIGHEST + 56,
		MENU_CHECK_NEW_REVISION = wxID_HIGHEST + 57,

		MENU_WEBHELP = wxID_HIGHEST + 58,

		MENU_EXPORT__FOLDER_SUMMARY__CSV__FILE     = wxID_HIGHEST + 59,			// folder window export
		MENU_EXPORT__FOLDER_SUMMARY__RQ__FILE      = wxID_HIGHEST + 60,			// folder window export
		MENU_EXPORT__FOLDER_SUMMARY__RQ__CLIPBOARD = wxID_HIGHEST + 61,			// folder window export

		MENU_FOLDER_OPEN_SHORTCUT_INFO = wxID_HIGHEST + 62,
		MENU_FOLDER_OPEN_SYMLINK_INFO = wxID_HIGHEST + 63,

		MENU_EDIT_USE_SELECTION_FOR_FIND = wxID_HIGHEST + 64,	// inspired by Command-E on mac

		MENU_EXPORT__FILE_DIFFS__UNIFIED__TEXT__FILE		= wxID_HIGHEST + 65,	// file diff in unified to file
		MENU_EXPORT__FILE_DIFFS__TRADITIONAL__TEXT__FILE		= wxID_HIGHEST + 66,	// file diff in normal/traditional to file
		MENU_EXPORT__FILE_DIFFS__UNIFIED__TEXT__CLIPBOARD		= wxID_HIGHEST + 67,	// file diff in unified to clipboard
		MENU_EXPORT__FILE_DIFFS__TRADITIONAL__TEXT__CLIPBOARD		= wxID_HIGHEST + 68,	// file diff in normal/traditional to clipboard

		MENU_FOLDER_SHOW_QUICKMATCH = wxID_HIGHEST + 69,

		MENU_EXPORT__FOLDER_SUMMARY__HTML__FILE			= wxID_HIGHEST + 70,	// folder window export html to file
		MENU_EXPORT__FILE_DIFFS__UNIFIED__HTML__FILE		= wxID_HIGHEST + 71,	// file diff in unified to file in html
		MENU_EXPORT__FILE_DIFFS__TRADITIONAL__HTML__FILE	= wxID_HIGHEST + 72,	// file diff in normal/tradational to file in html
		MENU_EXPORT__FILE_DIFFS__SXS__HTML__FILE			= wxID_HIGHEST + 73,	// file diff in side-by-side to file in html

		__MENU__COUNT__						// must be last

	} _iface_id;

public:
	static wxSize const GetToolBarImageSize() { return _get_toolbar_bitmap_size(); }

private:
	typedef enum _t_icon { _T_NONE=0, _T_XPM_VARIABLE, _T_PNG2SRC_VARIABLE } _t_icon;

	// _iface_def defines all the user-visible strings and icons
	// associated with a menu or tool command.

	typedef struct _iface_def
	{
		_iface_id		id;
		int				wx_id;	// there are times when we want the wxID_ value

		const wxChar *	szMenuLabel;
		const wxChar *	szToolBarLabel;
		const wxChar *	szStatusBar;
		const wxChar *	szToolBarToolTip;
		
		_t_icon			tbType;			// discriminant for szToolBarIcon
		const void *	szToolBarIcon;	// union { const char * xpm[], const char * png_h_base16 ... }
		const void *	szToolBarIconDisabled;	// the grayed version of the icon
		
	} _iface_def;

	static const _iface_def *	_get_iface_def(_iface_id id);
	static void					_iface_add_menu_item(wxMenu * pMenu, _iface_id id, wxItemKind kind=wxITEM_NORMAL);

	void				_iface_define_menu(ToolBarType tbt);
	void				_iface_define_toolbar(ToolBarType tbt);

	void				_iface_add_toolbar_item(wxToolBar * pToolBar, _iface_id id, wxItemKind kind=wxITEM_NORMAL);

	void				_iface_create_file_menu(void);
	void				_iface_create_edit_menu(int pos);
	void				_iface_create_view_menu(int pos, ToolBarType tbt);
	void				_iface_create_export_menu(int pos, ToolBarType tbt);
	void				_iface_create_settings_menu(void);
	void				_iface_create_help_menu(void);

	static wxSize		_get_toolbar_bitmap_size(void);
