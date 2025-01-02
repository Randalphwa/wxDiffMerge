// ViewFolder_ListCtrl.h
// the list control at the center of the folder diff view.
//////////////////////////////////////////////////////////////////

#ifndef H_VIEWFOLDER_LISTCTRL_H
#define H_VIEWFOLDER_LISTCTRL_H

//////////////////////////////////////////////////////////////////

class ViewFolder_ListCtrl : public wxListCtrl
{
public:
	ViewFolder_ListCtrl(ViewFolder * pViewFolder, const wxSize & size);
	virtual ~ViewFolder_ListCtrl(void);

	inline bool						isEnabled_FolderOpenFiles(void)		const { return m_bEnableFolderOpenFiles; };
	void							onEvent_FolderOpenFiles(void)		const;

	inline bool						isEnabled_FolderExportDiffFiles(void)	const { return m_bEnableFolderExportDiffFiles; };
	void							onEvent_FolderExportDiffFiles(int /*gui_frame::_iface_id*/ id) const;

	inline bool						isEnabled_FolderOpenFolders(void)	const { return m_bEnableFolderOpenFolders; };
	void							onEvent_FolderOpenFolders(void)		const;

#if defined(__WXMSW__)
	inline bool						isEnabled_FolderOpenShortcuts(void)	const { return m_bEnableFolderOpenShortcuts; };
	void							onEvent_FolderOpenShortcuts(void);
#endif

#if defined(__WXMAC__) || defined(__WXGTK__)
	inline bool						isEnabled_FolderOpenSymlinks(void)	const { return m_bEnableFolderOpenSymlinks; };
	void							onEvent_FolderOpenSymlinks(void);
#endif

	bool							getSelectedRowIndex(long * pRow)	const;
	fd_item::Status					getItemStatus(long item)			const;

	void							populateControl(long itemToSelectAfterwards);
	void							onLIAChange(void);

private:
	void							_setColumnWidths(void);
	void							_update_tb_button_flags(long item);
	void							_show_item_error(long item);
#if defined(__WXMSW__)
	void							_show_shortcut_info(fd_item * pFdItem);
#endif
#if defined(__WXMAC__) || defined(__WXGTK__)
	void							_show_symlink_info(fd_item * pFdItem);
#endif

private:
	virtual wxString				OnGetItemText(long item, long column) const;
	virtual int						OnGetItemImage(long item) const;
	virtual wxListItemAttr *		OnGetItemAttr(long item) const;

private:
	void	onSizeEvent				(wxSizeEvent & e);

	void	onListItemActivated		(wxListEvent & event);
	void	onListItemFocused		(wxListEvent & event);
	void	onListItemSelected		(wxListEvent & event);
	void	onListItemDeselected	(wxListEvent & event);
	void	onListItemRightClick	(wxListEvent & event);

	void	onListColBeginDrag		(wxListEvent & event);
	void	onListKeyDown			(wxListEvent & event);

	void	onRightClick			(wxMouseEvent & event);

	void	onMenuEvent_CTX_OPEN_WINDOW(wxCommandEvent & event);
	void	onMenuEvent_CTX_COPY_LEFT_PATH(wxCommandEvent & event);
	void	onMenuEvent_CTX_COPY_RIGHT_PATH(wxCommandEvent & event);

#if defined(__WXMSW__)
	void	onMenuEvent_CTX_SHOW_SHORTCUT_INFO(wxCommandEvent & event);
#endif
#if defined(__WXMAC__) || defined(__WXGTK__)
	void	onMenuEvent_CTX_SHOW_SYMLINK_INFO(wxCommandEvent & event);
#endif

	void	onMenuEvent_CTX_CLONE_ITEM(wxCommandEvent & event);
	void	onMenuEvent_CTX_CLONE_ITEM_RECURSIVE(wxCommandEvent & event);
	void	onMenuEvent_CTX_OVERWRITE_LEFT_ITEM(wxCommandEvent & event);
	void	onMenuEvent_CTX_OVERWRITE_RIGHT_ITEM(wxCommandEvent & event);

	typedef enum _ctx_id
	{
		CTX_OPEN_WINDOW				= wxID_HIGHEST + 100,
		CTX_COPY_LEFT_PATH			= wxID_HIGHEST + 101,
		CTX_COPY_RIGHT_PATH			= wxID_HIGHEST + 102,
#if defined(__WXMSW__)
		CTX_SHOW_SHORTCUT_INFO		= wxID_HIGHEST + 103,
#endif
		CTX_CLONE_ITEM				= wxID_HIGHEST + 104,
		CTX_CLONE_ITEM_RECURSIVE	= wxID_HIGHEST + 105,
		CTX_OVERWRITE_LEFT_ITEM		= wxID_HIGHEST + 106,
		CTX_OVERWRITE_RIGHT_ITEM	= wxID_HIGHEST + 107,
#if defined(__WXMAC__) || defined(__WXGTK__)
		CTX_SHOW_SYMLINK_INFO		= wxID_HIGHEST + 108,
#endif
	} _ctx_id;
	
	DECLARE_EVENT_TABLE();

private:
	ViewFolder *					m_pViewFolder;
	bool							m_bEnableFolderOpenFiles;	// enable if focus on a pair of files
	bool							m_bEnableFolderOpenFolders;	// enable if focus on a pair of folders
	bool							m_bEnableFolderOpenShortcuts;	// enable if focus on a shortcut (single or pair)
	bool							m_bEnableFolderOpenSymlinks;	// enable if focus on a symlink (single or pair)
	bool							m_bEnableFolderExportDiffFiles;	// enable if focus on a file pair with changes

	bool							m_bUserResizedColumns;
	bool							m_bInvalid;
	long							m_itemRightMouse;			// only valid while right-mouse context menu up
};

//////////////////////////////////////////////////////////////////

#endif//H_VIEWFOLDER_LISTCTRL_H
