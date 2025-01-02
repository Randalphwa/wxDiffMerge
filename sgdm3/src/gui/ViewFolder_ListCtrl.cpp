// ViewFolder_ListCtrl.cpp
// wraps a wxListCtrl to display folderdiff window content.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fs.h>
#include <de.h>
#include <fd.h>
#include <gui.h>

//////////////////////////////////////////////////////////////////

#if defined(__WXMAC__)
// WXBUG on Win32 and GTK, the selection is shown in the highlight color (say
// WXBUG blue) when the top-level window is the active window and a dull color (say
// WXBUG gray) when the top-level window is not the active window [this is pretty
// WXBUG normal.]
// WXBUG
// WXBUG but the MAC does not -- it always shows the selection in the 
// WXBUG active highlight (blue) color.
#endif

//////////////////////////////////////////////////////////////////

static void _cb_lia_change(void * pThis, const util_cbl_arg & /*arg*/)
{
	// ViewFolder_ListItemAttr has changed (probably means the font
	// and/or some of the folder colors have changed).

	ViewFolder_ListCtrl * pLC = (ViewFolder_ListCtrl *)pThis;
	pLC->onLIAChange();
}

//////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(ViewFolder_ListCtrl, wxListCtrl)

	EVT_SIZE					(ViewFolder_ListCtrl::onSizeEvent)

	EVT_LIST_ITEM_ACTIVATED		(ID_VIEWFOLDER_LIST_CTRL, ViewFolder_ListCtrl::onListItemActivated)
	EVT_LIST_ITEM_FOCUSED		(ID_VIEWFOLDER_LIST_CTRL, ViewFolder_ListCtrl::onListItemFocused)
	EVT_LIST_ITEM_SELECTED		(ID_VIEWFOLDER_LIST_CTRL, ViewFolder_ListCtrl::onListItemSelected)
	EVT_LIST_ITEM_DESELECTED	(ID_VIEWFOLDER_LIST_CTRL, ViewFolder_ListCtrl::onListItemDeselected)
	EVT_LIST_ITEM_RIGHT_CLICK	(ID_VIEWFOLDER_LIST_CTRL, ViewFolder_ListCtrl::onListItemRightClick)

    EVT_LIST_COL_BEGIN_DRAG		(ID_VIEWFOLDER_LIST_CTRL, ViewFolder_ListCtrl::onListColBeginDrag)

	EVT_LIST_KEY_DOWN			(ID_VIEWFOLDER_LIST_CTRL, ViewFolder_ListCtrl::onListKeyDown)

	EVT_RIGHT_DOWN				(ViewFolder_ListCtrl::onRightClick)
	EVT_MENU					(CTX_OPEN_WINDOW,		ViewFolder_ListCtrl::onMenuEvent_CTX_OPEN_WINDOW)
	EVT_MENU					(CTX_COPY_LEFT_PATH,	ViewFolder_ListCtrl::onMenuEvent_CTX_COPY_LEFT_PATH)
	EVT_MENU					(CTX_COPY_RIGHT_PATH,	ViewFolder_ListCtrl::onMenuEvent_CTX_COPY_RIGHT_PATH)

#if defined(__WXMSW__)
	EVT_MENU					(CTX_SHOW_SHORTCUT_INFO,ViewFolder_ListCtrl::onMenuEvent_CTX_SHOW_SHORTCUT_INFO)
#endif
#if defined(__WXMAC__) || defined(__WXGTK__)
	EVT_MENU					(CTX_SHOW_SYMLINK_INFO,		ViewFolder_ListCtrl::onMenuEvent_CTX_SHOW_SYMLINK_INFO)
#endif

	EVT_MENU					(CTX_CLONE_ITEM,			ViewFolder_ListCtrl::onMenuEvent_CTX_CLONE_ITEM)
	EVT_MENU					(CTX_CLONE_ITEM_RECURSIVE,	ViewFolder_ListCtrl::onMenuEvent_CTX_CLONE_ITEM_RECURSIVE)
	EVT_MENU					(CTX_OVERWRITE_LEFT_ITEM,	ViewFolder_ListCtrl::onMenuEvent_CTX_OVERWRITE_LEFT_ITEM)
	EVT_MENU					(CTX_OVERWRITE_RIGHT_ITEM,	ViewFolder_ListCtrl::onMenuEvent_CTX_OVERWRITE_RIGHT_ITEM)

END_EVENT_TABLE();

//////////////////////////////////////////////////////////////////
// we tweak the style bits on the list control depending on platform.
// this is personal preferences thing.

#if defined(__WXMSW__)
#	define MY_BORDER_STYLE		0
#elif defined(__WXGTK__)
#	define MY_BORDER_STYLE		wxSUNKEN_BORDER
#elif defined(__WXMAC__)
#	define MY_BORDER_STYLE		0
#endif

// WXBUG i'd like to turn on wxLC_HRULES and wxLC_VRULES, but they
// WXBUG cause display dirt above the first row, when scrolling, and
// WXBUG just don't work on the mac.

#if defined(__WXMSW__)
// vrules cause display dirt above line 0.
#	define MY_RULES_STYLE		0
#elif defined(__WXGTK__)
// hrules don't always get drawn when vscrolling.
// vrules look stupid without hrules.
#	define MY_RULES_STYLE		0
#elif defined(__WXMAC__)
// rules just don't work on mac.
#	define MY_RULES_STYLE		0
#endif

//////////////////////////////////////////////////////////////////

ViewFolder_ListCtrl::ViewFolder_ListCtrl(ViewFolder * pViewFolder, const wxSize & size)
	: wxListCtrl(pViewFolder,ID_VIEWFOLDER_LIST_CTRL,
				 wxDefaultPosition,size,
				 wxLC_VIRTUAL|wxLC_REPORT|wxLC_SINGLE_SEL|MY_BORDER_STYLE|MY_RULES_STYLE),
	  m_pViewFolder(pViewFolder),
	  m_bEnableFolderOpenFiles(false),
	  m_bEnableFolderOpenFolders(false),
	  m_bEnableFolderOpenShortcuts(false),
	  m_bEnableFolderOpenSymlinks(false),
	  m_bEnableFolderExportDiffFiles(false),
	  m_bUserResizedColumns(false),
	  m_bInvalid(true)
{
	fd_fd * pFdFd = m_pViewFolder->getDoc()->getFdFd();
	wxASSERT_MSG( pFdFd, _T("Coding Error: ViewFolder::ViewFolder: NULL fd_fd") );
	
	// install our global list-item-attributes to give us
	// fonts and colors for each type of row in our list.

	if (!gpViewFolderListItemAttr)
		gpViewFolderListItemAttr = new ViewFolder_ListItemAttr();
	gpViewFolderListItemAttr->addChangeCB(_cb_lia_change,this);

#if VIEW_FOLDER_LIST_CTRL_WIDGET_FONTS
	// See note in ViewFolder_ListItemAttr.h
	SetFont( *gpViewFolderListItemAttr->getFont() );
#endif

	// install our global image list to give us little icons
	// for each type of row in our list.

	if (!gpViewFolderImageList)
		gpViewFolderImageList = new ViewFolder_ImageList();
	SetImageList(gpViewFolderImageList,wxIMAGE_LIST_SMALL);

	poi_item * pPoiRoot0 = pFdFd->getRootPoi(0);
	poi_item * pPoiRoot1 = pFdFd->getRootPoi(1);

	// TODO decide if we need to elide pathnames or try to lop off the common prefix.
	// TODO that is, should we show '/home/.../f1' and '/home/.../f2'
	// TODO or '.../f1' and '.../f2'.

	wxListItem col;
	int c = 0;

	col.SetText( pPoiRoot0->getFullPath() );
	col.SetAlign(wxLIST_FORMAT_LEFT);
	col.SetMask(wxLIST_MASK_TEXT);
	InsertColumn(c++,col);

	col.SetText( pPoiRoot1->getFullPath() );
	col.SetAlign(wxLIST_FORMAT_LEFT);
	col.SetMask(wxLIST_MASK_TEXT);
	InsertColumn(c++,col);

	// NOTE setting column widths to 'autosize' doesn't really work
	// NOTE when using wxLC_VIRTUAL (at least on GTK). the 2 pathname
	// NOTE columns both come up at about 75 pixels -- regardless of
	// NOTE how long the initial pathnames are.
	// NOTE so we have have code in onSizeEvent() to automatically
	// NOTE adjust the column widths.
	// NOTE
	// NOTE SetColumnWidth(0, wxLIST_AUTOSIZE);
	// NOTE SetColumnWidth(1, wxLIST_AUTOSIZE);
	//
	// force a call to automatically set the column widths (in case
	// our initial size already matches the frame's client area).

	_setColumnWidths();

	// now trick ourselves to properly populate the list.

	populateControl(0);
}

ViewFolder_ListCtrl::~ViewFolder_ListCtrl(void)
{
	gpViewFolderListItemAttr->delChangeCB(_cb_lia_change,this);
}

//////////////////////////////////////////////////////////////////

void ViewFolder_ListCtrl::populateControl(long itemToSelectAfterwards)
{
	// the list of files/folders has changed or been rebuilt (this
	// happens periodically when the underlying folders on disk
	// change and when the user hits the various toggle- buttons
	// on the toolbar.
	//
	// the arg contains the index of the fd_item that we should try
	// to re-select after we re-populate the list.  if -1, don't.
	
	long item;
	
	// wxLogTrace(wxTRACE_Messages, _T("ViewFolder::populateControl"));

	// there are lots of bugs/quirks in the wxListCtrl widget.
	// the Win32 version is a thin wrapper around the ListView_
	// widget in comctl32.dll.  the GTK and MAC versions are
	// currently built using a "wxGenericListCtrl" -- a generic
	// list.
	//
	// some of the quirks include:
	// [] there is a notion of a "focus" row and "selection" row.
	//    most of the time these are the same, but they don't have
	//    to be.
	//
	//    [focus is drawn as a solid(Win32) or dotted(GTK) line
	//    around a row; focus is NOT drawn on the MAC.]
	//    
	//    [the selection is drawn in reverse video.  on Win32 and
	//    GTK, the highlight color (blue) is used when the top-level
	//    window is active and a dim color (gray) is used when the
	//    top-level window is not active;  the hightlight color is
	//    always used on the mac -- it doesn't seem to do the gray
	//    thing.]
	//    
	// [] changing the number of rows *DOES NOT* invalidate the
	//    selection or focus.
	// [] changing the number of rows *DOES NOT* cause the widget
	//    to scroll.
	// [] changing the number of rows *DOES NOT* cause the widget
	//    repaint.
	//    
	// [] on GTK and MAC, if you clear the selection/focus, it
	//    internally sets the focus to row 0.  on Win32, it leaves
	//    it unset.
	//
	// [] on Win32, when the focus is unset the keyboard arrow keys
	//    do not work -- the user has to explicitly use the mouse to
	//    pick something in the list.
	//
	// [] on Win32, SetItemCount() uses LVSICF_NOINVALIDATEALL and
	//    LVSICF_NOSCROLL, so we don't get any kind of update.  so,
	//    if there was a long list and it was scrolled down and our
	//    list is now very short, the widget display is goofy.  it
	//    sometimes displays row 0 with several inches of whitespace
	//    above it [this can only be fixed by the user playing with
	//    the vertical scrollbar.]
	//
	// [] on Win32, setting the focus/selection *DOES NOT* cause the
	//    widget to send us an EVT_LIST_ITEM_FOCUSED event.
	//
	// some problems that this causes, include:
	// [] on GTK, if we are making the number of rows smaller and
	//    the index of the previously selected row less than the new
	//    number of row and the previously selected row is on screen,
	//    we get a dirty row (blank selection row) where the old
	//    selection was.
	//
	// [] if the list radically changes, then row k in the old list
	//    and row k in the new list are completly unrelated.  having
	//    row k be selected before and after just looks dumb.  the
	//    selection should be cleared.  it *MIGHT* make sense for
	//    focus to remain.
	//
	// [] we enable/disable the toolbar buttons "...OpenFolders..."
	//    and "...OpenFiles..." depending the fd_item status of the
	//    currently selected row [so they can open a sub-folder
	//    ViewFolder or ViewFile-diff window].
	//
	//    so on the MAC, having an (invisible) focus without a selection
	//    cause the toolbar buttons to look like they're wrong.
	//
	// note:
	// 
	// [] we need to be careful here because turning off the selection
	//    (and/or focus) causes a repaint of the affected row and since
	//    we're virtual, our OnGetItem...() routines get called.  if the
	//    highlighted row is past the new end, we crash on Win32 & GTK.
	//
	// The following seems to minimize the quirks/problems across all 3
	// platforms:

	// [] set m_bInvalid to prevent our OnGetItem...() from doing anything
	//    (while responding to events generated from what we're doing here).

	m_bInvalid = true;

	// [] if the list in the widget is not empty, warp scroll the widget
	//    to row 0.  (so that we get the top of the list to the top of the
	//    window before the scroll bar disappears.)

	if (GetItemCount() > 0)
		EnsureVisible(0);

	// [] if we had a selection and/or focus, clear them.  (while we can
	//    still reference them (incase the list gets shorter))

	item = GetNextItem(-1,wxLIST_NEXT_ALL,wxLIST_STATE_SELECTED);
	if (item != -1)
		SetItemState(item,0,wxLIST_STATE_SELECTED);	// clear selected

	item = GetNextItem(-1,wxLIST_NEXT_ALL,wxLIST_STATE_FOCUSED);
	if (item != -1)
		SetItemState(item,0,wxLIST_STATE_FOCUSED);	// clear focused

	// [] set the new number of rows

	fd_fd * pFdFd = m_pViewFolder->getDoc()->getFdFd();
	long c = pFdFd->getItemCount();
	SetItemCount(c);

	// [] if the new list is not empty, set focus and selection to row 0 or the requested row.

	if (c > 0)
	{
		long itemNew = ((itemToSelectAfterwards != -1) ? itemToSelectAfterwards : 0);
		SetItemState(itemNew,
					 wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED,
					 wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
		EnsureVisible(itemNew);
	}

	// [] clear m_bInvalid (so that our OnGetItem...() CB's will work.

	m_bInvalid = false;

	// [] invalidate the window and force a full repaint

	Refresh(true,NULL);

	// [] force update the toolbar buttons/menu items (since we don't always
	//    get this event).  row -1 will disable both buttons when the new list
	//    is empty.  [m_bInvalid must be false for this to have an effect.]
	
	_update_tb_button_flags( ((c>0) ? 0 : -1) );

	// what a pain....

//	wxLogTrace(wxTRACE_Messages, _T("ViewFolder::populateControl: bottom [count %d][focus %d][selected %d]"),
//			   c, GetNextItem(-1,wxLIST_NEXT_ALL,wxLIST_STATE_SELECTED), GetNextItem(-1,wxLIST_NEXT_ALL,wxLIST_STATE_FOCUSED));
}

//////////////////////////////////////////////////////////////////

void ViewFolder_ListCtrl::onLIAChange(void)
{
	// ViewFolder_ListItemAttr has changed (probably means the font
	// and/or some of the folder colors have changed).

	// wxLogTrace(wxTRACE_Messages, _T("ViewFolder::OnLIAChange") );
	
	// Force a repaint of entire list.

#if VIEW_FOLDER_LIST_CTRL_WIDGET_FONTS
	// See note in ViewFolder_ListItemAttr.h
	SetFont( *gpViewFolderListItemAttr->getFont() );
#endif

	Refresh(true,NULL);
}

//////////////////////////////////////////////////////////////////

fd_item::Status ViewFolder_ListCtrl::getItemStatus(long item) const
{
	if ((item == -1) || (m_bInvalid))
		return fd_item::FD_ITEM_STATUS_UNKNOWN;

	fd_fd * pFdFd = m_pViewFolder->getDoc()->getFdFd();

	wxASSERT_MSG( pFdFd, _T("Coding Error: ViewFolder::getItemStatus: NULL fd_fd") );

	if (item >= pFdFd->getItemCount())				// should not happen, but see notes 
		return fd_item::FD_ITEM_STATUS_UNKNOWN;		// above before call to clearSelection()

	return pFdFd->getItem(item)->getStatus();
}

//////////////////////////////////////////////////////////////////

wxString ViewFolder_ListCtrl::OnGetItemText(long item, long column) const
{
//	wxLogTrace(wxTRACE_Messages, _T("ViewFolder::OnGetItemText: [%d,%d]"),item,column);

	if ((item == -1) || (m_bInvalid))
		return _T("");

	fd_fd * pFdFd = m_pViewFolder->getDoc()->getFdFd();

	wxASSERT_MSG( pFdFd, _T("Coding Error: ViewFolder::OnGetItemText: NULL fd_fd") );

	if (item >= pFdFd->getItemCount())		// should not happen, but see notes 
		return _T("");						// above before call to clearSelection()

	fd_item * pFdItem = pFdFd->getItem(item);
	poi_item * pPoiItem;

	switch (column)
	{
	case 0:
		pPoiItem = pFdItem->getPoiItem(0);
		return ( (pPoiItem) ? pFdItem->getRelativePathname(0) : _T("") ); // use rel path from fd_item rather than full path from poi_item

	case 1:
		pPoiItem = pFdItem->getPoiItem(1);
		return ( (pPoiItem) ? pFdItem->getRelativePathname(1) : _T("") ); // use rel path from fd_item rather than full path from poi_item

	default:
		wxASSERT_MSG( 0, _T("Coding Error: unknown column value") );
		return _T("");
	}
}

int ViewFolder_ListCtrl::OnGetItemImage(long item) const
{
//	wxLogTrace(wxTRACE_Messages, _T("ViewFolder::OnGetItemImage: [%d][focus %d][selected %d]"),
//			   item, GetNextItem(-1,wxLIST_NEXT_ALL,wxLIST_STATE_SELECTED), GetNextItem(-1,wxLIST_NEXT_ALL,wxLIST_STATE_FOCUSED));

	return getItemStatus(item);
}

wxListItemAttr * ViewFolder_ListCtrl::OnGetItemAttr(long item) const
{
//	wxLogTrace(wxTRACE_Messages, _T("ViewFolder::OnGetItemAttr: [%d]"),item);

	return gpViewFolderListItemAttr->getAttr( getItemStatus(item) );
}

//////////////////////////////////////////////////////////////////

void ViewFolder_ListCtrl::onListItemActivated(wxListEvent & event)
{
	// ENTER key or DoubleClick on (selected or focused) row.

#if defined(__WXMAC__)
	// WXBUG there's a problem on the MAC when the user double-clicks
	// WXBUG on a row and we open a new window with the 2 items in
	// WXBUG the list.  the new window appears under our window rather
	// WXBUG than on top (as a new window would be expected to be).
	// WXBUG See FrameFactory::raiseFrame().  this happens with the
	// WXBUG GENERIC version (see wxMAC_ALWAYS_USE_GENERIC_LISTCTRL
	// WXBUG in gui_app.cpp).
	//
	// WXBUG but when using the NATIVE version, the window doesn't
	// WXBUG raise when you click on items in the list -- but new
	// WXBUG file windows do appear in front of us.  if i have to
	// WXBUG pick a bug, i'll take this one.
#endif

	long kItem = event.GetIndex();

	// wxLogTrace(wxTRACE_Messages, _T("ViewFolder::onListItemActivated [kItem %d]"), kItem);

	// if a pair of regular files, open/raise a filediff window.
	// if a pair of folders, open/raise folderdiff window.
	//
	// see also our onEvent_FoldersOpenFolders()

	fd_item * pFdItem;

	switch ( getItemStatus(kItem) )
	{
	case fd_item::FD_ITEM_STATUS_SAME_FILE:
		// TODO decide if we want to handle these differently -- for example,
		// TODO if they both refer to the same (physical) file, there's no
		// TODO need for a 2-way diff window....
		//
		// TODO for same-files, the fd_ code currently does not fstat() or
		// TODO check access or anything -- they might be unreadable or
		// TODO character devices for all we know.  add code deal with this.
	case fd_item::FD_ITEM_STATUS_IDENTICAL:
	case fd_item::FD_ITEM_STATUS_EQUIVALENT:
	case fd_item::FD_ITEM_STATUS_QUICKMATCH:
	case fd_item::FD_ITEM_STATUS_DIFFERENT:
		pFdItem = m_pViewFolder->getDoc()->getFdFd()->getItem(kItem);
		gpFrameFactory->openFileDiffFrameOrSpawnAsyncXT(pFdItem->getPoiItem(0)->getFullPath(),
														pFdItem->getPoiItem(1)->getFullPath(),
														NULL);
		break;

	case fd_item::FD_ITEM_STATUS_FOLDERS:
		pFdItem = m_pViewFolder->getDoc()->getFdFd()->getItem(kItem);
		gpFrameFactory->openFolderFrame(pFdItem->getPoiItem(0)->getFullPath(),
										pFdItem->getPoiItem(1)->getFullPath(),
										NULL);
		break;

#if defined(__WXMSW__)
	case fd_item::FD_ITEM_STATUS_SHORTCUT_NULL:
	case fd_item::FD_ITEM_STATUS_NULL_SHORTCUT:
	case fd_item::FD_ITEM_STATUS_SHORTCUTS_SAME:
	case fd_item::FD_ITEM_STATUS_SHORTCUTS_EQ:
	case fd_item::FD_ITEM_STATUS_SHORTCUTS_NEQ:
		pFdItem = m_pViewFolder->getDoc()->getFdFd()->getItem(kItem);
		_show_shortcut_info(pFdItem);
		break;
#endif

#if defined(__WXMAC__) || defined(__WXGTK__)
	case fd_item::FD_ITEM_STATUS_SYMLINK_NULL:
	case fd_item::FD_ITEM_STATUS_NULL_SYMLINK:
	case fd_item::FD_ITEM_STATUS_SYMLINKS_SAME:
	case fd_item::FD_ITEM_STATUS_SYMLINKS_EQ:
	case fd_item::FD_ITEM_STATUS_SYMLINKS_NEQ:
		pFdItem = m_pViewFolder->getDoc()->getFdFd()->getItem(kItem);
		_show_symlink_info(pFdItem);
		break;
#endif

	case fd_item::FD_ITEM_STATUS_MISMATCH:
	case fd_item::FD_ITEM_STATUS_ERROR:
		_show_item_error(kItem);
		break;

	default:
		break;
	}
}

//////////////////////////////////////////////////////////////////

void ViewFolder_ListCtrl::_update_tb_button_flags(long item)
{
	// if selected item is a pair of non-error files,
	// allow FOLDER_OPEN_FILES to be enabled -- the next
	// time that it asks.
	//
	// if selected item is a pair of folders, allow
	// FOLDER_OPEN_FOLDERS to be enabled.

	fd_item::Status s = getItemStatus(item);
	
	m_bEnableFolderOpenFiles	   = false;
	m_bEnableFolderOpenFolders	   = false;
	m_bEnableFolderOpenShortcuts   = false;
	m_bEnableFolderOpenSymlinks    = false;
	m_bEnableFolderExportDiffFiles = false;

	switch (s)
	{
	case fd_item::FD_ITEM_STATUS_SAME_FILE:
	case fd_item::FD_ITEM_STATUS_IDENTICAL:
	case fd_item::FD_ITEM_STATUS_EQUIVALENT:
	case fd_item::FD_ITEM_STATUS_QUICKMATCH:
		m_bEnableFolderOpenFiles       = true;
		break;

	case fd_item::FD_ITEM_STATUS_DIFFERENT:
		m_bEnableFolderOpenFiles       = true;
		m_bEnableFolderExportDiffFiles = true;
		break;

	case fd_item::FD_ITEM_STATUS_FOLDERS:
		m_bEnableFolderOpenFolders     = true;
		break;

	case fd_item::FD_ITEM_STATUS_SHORTCUT_NULL:
	case fd_item::FD_ITEM_STATUS_NULL_SHORTCUT:
	case fd_item::FD_ITEM_STATUS_SHORTCUTS_SAME:
	case fd_item::FD_ITEM_STATUS_SHORTCUTS_EQ:
	case fd_item::FD_ITEM_STATUS_SHORTCUTS_NEQ:
		m_bEnableFolderOpenShortcuts   = true;
		break;

	case fd_item::FD_ITEM_STATUS_SYMLINK_NULL:
	case fd_item::FD_ITEM_STATUS_NULL_SYMLINK:
	case fd_item::FD_ITEM_STATUS_SYMLINKS_SAME:
	case fd_item::FD_ITEM_STATUS_SYMLINKS_EQ:
	case fd_item::FD_ITEM_STATUS_SYMLINKS_NEQ:
		m_bEnableFolderOpenSymlinks    = true;
		break;

	default:
		break;
	}

//	wxLogTrace(wxTRACE_Messages, _T("ViewFolder::_update_tb: [item %d][status %d][invalid %d]"), item, s, m_bInvalid);
}

void ViewFolder_ListCtrl::onListItemFocused(wxListEvent & event)
{
	long kItem = event.GetIndex();

//	wxLogTrace(wxTRACE_Messages, _T("ViewFolder::onListItemFocused [kItem %d]"), kItem);

	_update_tb_button_flags(kItem);

	// forward the event to the actual list control so that
	// normal focus stuff happens -- this is a very poorly
	// named function.

	event.Skip();
}

void ViewFolder_ListCtrl::onListItemSelected(wxListEvent & event)
{
	long kItem = event.GetIndex();

#if 0 && defined(DEBUG)
	wxString strMsg;
	fd_item * pFdItem = m_pViewFolder->getDoc()->getFdFd()->getItem(kItem);
	pFdItem->getSoftQuickMatchSummaryMessage(strMsg);

	fd_item::Status s = pFdItem->getStatus();
	wxString strRelativePath0( pFdItem->getRelativePathname(0) );
	wxString strRelativePath1( pFdItem->getRelativePathname(1) );
	wxString strFullPath0;
	poi_item * pPoi0 = pFdItem->getPoiItem(0);
	if (pPoi0)
		strFullPath0 = pPoi0->getFullName();
	poi_item * pPoi1 = pFdItem->getPoiItem(1);
	wxString strFullPath1;
	if (pPoi1)
		strFullPath1 = pPoi1->getFullName();

	wxLogTrace(wxTRACE_Messages,
			   _T("ViewFolder::onListItemSelected [kItem %ld][%s][%s][%s][%s][%s][%s]"),
			   kItem,
			   strRelativePath0.wc_str(), strRelativePath1.wc_str(),
			   strFullPath0.wc_str(), strFullPath1.wc_str(),
			   fd_item::getStatusText(s),
			   strMsg.wc_str());
#endif

	_update_tb_button_flags(kItem);

	// forward the event to the actual list control so that
	// normal selection stuff happens -- this is a very poorly
	// named function.

	event.Skip();
}

void ViewFolder_ListCtrl::onListItemDeselected(wxListEvent & event)
{
//	long kItem = event.GetIndex();

//	wxLogTrace(wxTRACE_Messages, _T("ViewFolder::onListItemDeselected [kItem %d]"), kItem);

	// forward the event to the actual list control so that
	// normal selection stuff happens -- this is a very poorly
	// named function.

	event.Skip();
}

void ViewFolder_ListCtrl::onListItemRightClick(wxListEvent & /*event*/)
{
	// we don't get these anymore since we added onRightClick()

//	wxLogTrace(wxTRACE_Messages, _T("ViewFolder::onListItemRightClick"));
}

void ViewFolder_ListCtrl::onRightClick(wxMouseEvent & event)
{
	event.Skip(false);

	int flags = 0;
	wxPoint pointClick = event.GetPosition();
	long item = HitTest(pointClick,flags,NULL);
	//wxLogTrace(wxTRACE_Messages, _T("ViewFolder::onRightClick [item %lx][flags %x]"),item,flags);

	if (item == wxNOT_FOUND)
		return;
	if ((flags & wxLIST_HITTEST_ONITEM) == 0)
		return;

	fd_item::Status status = getItemStatus(item);
	//wxLogTrace(wxTRACE_Messages, _T("ViewFolder::onRightClick [item %lx][flags %x][status %x]"),item,flags,status);
	if (status == fd_item::FD_ITEM_STATUS_UNKNOWN)
		return;

	long itemOld;

	itemOld = GetNextItem(-1,wxLIST_NEXT_ALL,wxLIST_STATE_SELECTED);
	if ((itemOld != -1) && (itemOld != item))
		SetItemState(itemOld,0,wxLIST_STATE_SELECTED);	// clear selected

	itemOld = GetNextItem(-1,wxLIST_NEXT_ALL,wxLIST_STATE_FOCUSED);
	if ((itemOld != -1) && (itemOld != item))
		SetItemState(itemOld,0,wxLIST_STATE_FOCUSED);	// clear focused

	SetItemState(item,
				 wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED,
				 wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);

#if defined(__WXOSX__)
	bool bControlDown = event.RawControlDown();	// see note in __kb_mac.cpp
#else
	bool bControlDown = event.ControlDown();
#endif

	if (bControlDown)
	{
		// control right mouse -- raise a little info/debug dialog -- IFF -- we have something to say.

		fd_item * pFdItem = m_pViewFolder->getDoc()->getFdFd()->getItem(item);
		wxString strMsg;

		if (pFdItem->getSoftQuickMatchSummaryMessage(strMsg))
		{
			// TODO consider using the no-activation dialog so that the window doesn't reload when
			// TODO this dialog goes down.

			wxMessageDialog dlg(m_pViewFolder->getFrame(),strMsg,_("File Comparison Summary"),wxOK|wxICON_INFORMATION);
			dlg.ShowModal();
		}
		
		return;
	}

	// normal right mouse.  raise context menu.

	wxMenu menu;

	switch (status)
	{
	case fd_item::FD_ITEM_STATUS_SAME_FILE:
	case fd_item::FD_ITEM_STATUS_IDENTICAL:
	case fd_item::FD_ITEM_STATUS_FOLDERS:
		menu.Append(CTX_OPEN_WINDOW,_("Open Pair in New DiffMerge Window"));
		menu.AppendSeparator();
		menu.Append(CTX_COPY_LEFT_PATH,_("Copy Left Pathname to Clipboard"));
		menu.Append(CTX_COPY_RIGHT_PATH,_("Copy Right Pathname to Clipboard"));
		break;

	case fd_item::FD_ITEM_STATUS_EQUIVALENT:
	case fd_item::FD_ITEM_STATUS_QUICKMATCH:
	case fd_item::FD_ITEM_STATUS_DIFFERENT:
		menu.Append(CTX_OPEN_WINDOW,_("Open Pair in New DiffMerge Window"));
		menu.AppendSeparator();
		menu.Append(CTX_COPY_LEFT_PATH,_("Copy Left Pathname to Clipboard"));
		menu.Append(CTX_COPY_RIGHT_PATH,_("Copy Right Pathname to Clipboard"));
		menu.AppendSeparator();
		menu.Append(CTX_OVERWRITE_RIGHT_ITEM,_("Copy File Left to Right"));
		menu.Append(CTX_OVERWRITE_LEFT_ITEM,_("Copy File Right to Left"));
		break;

#if defined(__WXMSW__)
	case fd_item::FD_ITEM_STATUS_SHORTCUTS_SAME:
	case fd_item::FD_ITEM_STATUS_SHORTCUTS_EQ:
		menu.Append(CTX_SHOW_SHORTCUT_INFO,_("Shortcut Details..."));
		menu.AppendSeparator();
		menu.Append(CTX_COPY_LEFT_PATH,_("Copy Left Pathname to Clipboard"));
		menu.Append(CTX_COPY_RIGHT_PATH,_("Copy Right Pathname to Clipboard"));
		break;

	case fd_item::FD_ITEM_STATUS_SHORTCUTS_NEQ:
		menu.Append(CTX_SHOW_SHORTCUT_INFO,_("Shortcut Details..."));
		menu.AppendSeparator();
		menu.Append(CTX_COPY_LEFT_PATH,_("Copy Left Pathname to Clipboard"));
		menu.Append(CTX_COPY_RIGHT_PATH,_("Copy Right Pathname to Clipboard"));
		menu.AppendSeparator();
		menu.Append(CTX_OVERWRITE_RIGHT_ITEM,_("Copy Shortcut Left to Right"));
		menu.Append(CTX_OVERWRITE_LEFT_ITEM,_("Copy Shortcut Right to Left"));
		break;
#endif

#if defined(__WXMAC__) || defined(__WXGTK__)
	case fd_item::FD_ITEM_STATUS_SYMLINKS_SAME:
	case fd_item::FD_ITEM_STATUS_SYMLINKS_EQ:
		menu.Append(CTX_SHOW_SYMLINK_INFO,_("Symlink Details..."));
		menu.AppendSeparator();
		menu.Append(CTX_COPY_LEFT_PATH,_("Copy Left Pathname to Clipboard"));
		menu.Append(CTX_COPY_RIGHT_PATH,_("Copy Right Pathname to Clipboard"));
		break;

	case fd_item::FD_ITEM_STATUS_SYMLINKS_NEQ:
		menu.Append(CTX_SHOW_SYMLINK_INFO,_("Symlink Details..."));
		menu.AppendSeparator();
		menu.Append(CTX_COPY_LEFT_PATH,_("Copy Left Pathname to Clipboard"));
		menu.Append(CTX_COPY_RIGHT_PATH,_("Copy Right Pathname to Clipboard"));
		menu.AppendSeparator();
		menu.Append(CTX_OVERWRITE_RIGHT_ITEM,_("Copy Symlink Left to Right"));
		menu.Append(CTX_OVERWRITE_LEFT_ITEM,_("Copy Symlink Right to Left"));
		break;
#endif

	default:			// quiet compiler
	case fd_item::FD_ITEM_STATUS_UNKNOWN:
	case fd_item::FD_ITEM_STATUS_ERROR:
	case fd_item::FD_ITEM_STATUS_BOTH_NULL:
		return;			// can't guarantee anything

	case fd_item::FD_ITEM_STATUS_MISMATCH:
		menu.Append(CTX_COPY_LEFT_PATH,_("Copy Left Pathname to Clipboard"));
		menu.Append(CTX_COPY_RIGHT_PATH,_("Copy Right Pathname to Clipboard"));
		break;

	case fd_item::FD_ITEM_STATUS_FILE_NULL:
		menu.Append(CTX_COPY_LEFT_PATH,_("Copy Left Pathname to Clipboard"));
		menu.AppendSeparator();
		menu.Append(CTX_CLONE_ITEM, _("Copy File Left to Right"));
		break;

	case fd_item::FD_ITEM_STATUS_FOLDER_NULL:
		menu.Append(CTX_COPY_LEFT_PATH,_("Copy Left Pathname to Clipboard"));
		menu.AppendSeparator();
		menu.Append(CTX_CLONE_ITEM, _("Copy Folder Left to Right"));
		menu.AppendSeparator();
		menu.Append(CTX_CLONE_ITEM_RECURSIVE, _("Copy Folder Left to Right Recursively"));
		break;

	case fd_item::FD_ITEM_STATUS_NULL_FILE:
		menu.Append(CTX_COPY_RIGHT_PATH,_("Copy Right Pathname to Clipboard"));
		menu.AppendSeparator();
		menu.Append(CTX_CLONE_ITEM, _("Copy File Right to Left"));
		break;

	case fd_item::FD_ITEM_STATUS_NULL_FOLDER:
		menu.Append(CTX_COPY_RIGHT_PATH,_("Copy Right Pathname to Clipboard"));
		menu.AppendSeparator();
		menu.Append(CTX_CLONE_ITEM, _("Copy Folder Right to Left"));
		menu.AppendSeparator();
		menu.Append(CTX_CLONE_ITEM_RECURSIVE, _("Copy Folder Right to Left Recursively"));
		break;

#if defined(__WXMSW__)
	case fd_item::FD_ITEM_STATUS_SHORTCUT_NULL:
		menu.Append(CTX_SHOW_SHORTCUT_INFO,_("Shortcut Details..."));
		menu.AppendSeparator();
		menu.Append(CTX_COPY_LEFT_PATH,_("Copy Left Pathname to Clipboard"));
		menu.AppendSeparator();
		menu.Append(CTX_CLONE_ITEM, _("Copy Shortcut Left to Right"));
		break;

	case fd_item::FD_ITEM_STATUS_NULL_SHORTCUT:
		menu.Append(CTX_SHOW_SHORTCUT_INFO,_("Shortcut Details..."));
		menu.AppendSeparator();
		menu.Append(CTX_COPY_RIGHT_PATH,_("Copy Right Pathname to Clipboard"));
		menu.AppendSeparator();
		menu.Append(CTX_CLONE_ITEM, _("Copy Shortcut Right to Left"));
		break;
#endif

#if defined(__WXMAC__) || defined(__WXGTK__)
	case fd_item::FD_ITEM_STATUS_SYMLINK_NULL:
		menu.Append(CTX_SHOW_SYMLINK_INFO,_("Symlink Details..."));
		menu.AppendSeparator();
		menu.Append(CTX_COPY_LEFT_PATH,_("Copy Left Pathname to Clipboard"));
		menu.AppendSeparator();
		menu.Append(CTX_CLONE_ITEM, _("Copy Symlink Left to Right"));
		break;

	case fd_item::FD_ITEM_STATUS_NULL_SYMLINK:
		menu.Append(CTX_SHOW_SYMLINK_INFO,_("Symlink Details..."));
		menu.AppendSeparator();
		menu.Append(CTX_COPY_RIGHT_PATH,_("Copy Right Pathname to Clipboard"));
		menu.AppendSeparator();
		menu.Append(CTX_CLONE_ITEM, _("Copy Symlink Right to Left"));
		break;
#endif

	}

	m_itemRightMouse = item;
	PopupMenu(&menu);
}

void ViewFolder_ListCtrl::onMenuEvent_CTX_OPEN_WINDOW(wxCommandEvent & /*event*/)
{
//	wxLogTrace(wxTRACE_Messages,_T("CTX_OPEN_WINDOW: %lx"),m_itemRightMouse);

	fd_item * pFdItem;
	fd_item::Status status = getItemStatus(m_itemRightMouse);

	switch (status)
	{
	case fd_item::FD_ITEM_STATUS_SAME_FILE:
		// TODO decide if we want to handle these differently -- for example,
		// TODO if they both refer to the same (physical) file, there's no
		// TODO need for a 2-way diff window....
		//
		// TODO for same-files, the fd_ code currently does not fstat() or
		// TODO check access or anything -- they might be unreadable or
		// TODO character devices for all we know.  add code deal with this.
	case fd_item::FD_ITEM_STATUS_IDENTICAL:
	case fd_item::FD_ITEM_STATUS_EQUIVALENT:
	case fd_item::FD_ITEM_STATUS_QUICKMATCH:
	case fd_item::FD_ITEM_STATUS_DIFFERENT:
		pFdItem = m_pViewFolder->getDoc()->getFdFd()->getItem(m_itemRightMouse);
		gpFrameFactory->openFileDiffFrameOrSpawnAsyncXT(pFdItem->getPoiItem(0)->getFullPath(),
														pFdItem->getPoiItem(1)->getFullPath(),
														NULL);
		break;

	case fd_item::FD_ITEM_STATUS_FOLDERS:
		pFdItem = m_pViewFolder->getDoc()->getFdFd()->getItem(m_itemRightMouse);
		gpFrameFactory->openFolderFrame(pFdItem->getPoiItem(0)->getFullPath(),
										pFdItem->getPoiItem(1)->getFullPath(),
										NULL);
		break;

	default:	// should not happen
		break;
	}
}

void ViewFolder_ListCtrl::onMenuEvent_CTX_COPY_LEFT_PATH(wxCommandEvent & /*event*/)
{
	fd_item * pFdItem = m_pViewFolder->getDoc()->getFdFd()->getItem(m_itemRightMouse);
	if (!pFdItem)	// should not happen
		return;

	poi_item * pPoiItem = pFdItem->getPoiItem(0);
	wxASSERT_MSG( (pPoiItem), _T("Coding Error") );
	if (!pPoiItem)	// should not happen
		return;

	wxString strPath = pPoiItem->getFullPath();
	
//	wxLogTrace(wxTRACE_Messages,_T("CTX_COPY_LEFT_PATH: %lx: [%s]"),m_itemRightMouse,util_printable_s(strPath).wc_str());

	if (wxTheClipboard->Open())
	{
		wxTextDataObject * pTDO = new wxTextDataObject(strPath);

		wxTheClipboard->SetData(pTDO);
		wxTheClipboard->Close();
	}
}

void ViewFolder_ListCtrl::onMenuEvent_CTX_COPY_RIGHT_PATH(wxCommandEvent & /*event*/)
{
	fd_item * pFdItem = m_pViewFolder->getDoc()->getFdFd()->getItem(m_itemRightMouse);
	if (!pFdItem)	// should not happen
		return;

	poi_item * pPoiItem = pFdItem->getPoiItem(1);
	wxASSERT_MSG( (pPoiItem), _T("Coding Error") );
	if (!pPoiItem)	// should not happen
		return;

	wxString strPath = pPoiItem->getFullPath();
	
//	wxLogTrace(wxTRACE_Messages,_T("CTX_COPY_RIGHT_PATH: %lx: [%s]"),m_itemRightMouse,util_printable_s(strPath).wc_str());

	if (wxTheClipboard->Open())
	{
		wxTextDataObject * pTDO = new wxTextDataObject(strPath);

		wxTheClipboard->SetData(pTDO);
		wxTheClipboard->Close();
	}
}

//////////////////////////////////////////////////////////////////

#if defined(__WXMSW__)
void ViewFolder_ListCtrl::onMenuEvent_CTX_SHOW_SHORTCUT_INFO(wxCommandEvent & /*event*/)
{
	fd_item * pFdItem = m_pViewFolder->getDoc()->getFdFd()->getItem(m_itemRightMouse);
	if (!pFdItem)	// should not happen
		return;

	_show_shortcut_info(pFdItem);
}
#endif//__WXMSW__


#if defined(__WXMAC__) || defined(__WXGTK__)
void ViewFolder_ListCtrl::onMenuEvent_CTX_SHOW_SYMLINK_INFO(wxCommandEvent & /*event*/)
{
	fd_item * pFdItem = m_pViewFolder->getDoc()->getFdFd()->getItem(m_itemRightMouse);
	if (!pFdItem)	// should not happen
		return;

	_show_symlink_info(pFdItem);
}
#endif

//////////////////////////////////////////////////////////////////

void ViewFolder_ListCtrl::_setColumnWidths(void)
{
	// automatically set columns widths -- if the user has not
	// manually adjusted one of the columns widths.

	if (m_bUserResizedColumns)
		return;
	
	// we want the default behavior to be automatically balanced
	// columns (50%/50%), but wxWidgets wxLIST_AUTOSIZE means to
	// adapt the columns to the width of the widest item in each
	// column.  so we catch onSizeEvent and do it ourselves.
	//
	// since we can't tell if there is a vertical scroll bar visible,
	// we set the columns to (client_area - vscroll_width).  this
	// keeps the horizontal scrollbar from appearing at the bottom.
	// 
	// on GTK and MAC, it also has the nice property that you can
	// make the window taller/shorter and the vscroll bar appears/
	// disappears and the columns don't shift.
	//
	// the Win32 version does not behave like this -- it looks like
	// the client area does not include the area used by the scrollbar.
	// so we're always too narrow.
	//
	// WXBUG there's still some dirt in the Win32 version.  making
	// WXBUG the window narrower will sometimes trick the horizontal
	// WXBUG scrollbar to appear with goofy parameters -- and if you
	// WXBUG click on it, it will disappear.

	wxSize s = GetClientSize();

//	wxLogTrace(wxTRACE_Messages, _T("ViewFolder_ListCtrl::_setColumnWidths: [%d,%d]"),s.x,s.y);

#if defined(__WXMSW__)
	int xAvail = s.x - 1;
#elif defined(__WXGTK__) || defined(__WXMAC__)
	int xScroll = wxSystemSettingsNative::GetMetric(wxSYS_VSCROLL_X);
	int xAvail = s.x - xScroll - 1;
#endif

	int c0, c1;

	c0 = xAvail / 2;
	c1 = xAvail - c0;
	SetColumnWidth(0, c0);
	SetColumnWidth(1, c1);
}

void ViewFolder_ListCtrl::onSizeEvent(wxSizeEvent & e)
{
	// our list control is changing size.  if the user has not
	// adjusted the column widths, we will automatically do it.

	_setColumnWidths();
	e.Skip();
}

//////////////////////////////////////////////////////////////////

void ViewFolder_ListCtrl::onListColBeginDrag(wxListEvent & /*e*/)
{
	// user is starting to resize the columns.
	// turn off our auto-resize.

	m_bUserResizedColumns = true;
}

//////////////////////////////////////////////////////////////////

bool ViewFolder_ListCtrl::getSelectedRowIndex(long * pRow) const
{
	// get the currently selected row.
	//
	// there's something goofy in the widget that it allows the selected
	// and focused rows to be different -- see the Win32 ListView_ docs.
	// 99+% of the time they will be the same.  but there are times when
	// there will be a row with focus and no selection -- and our toolbar/
	// menu toggles will reflect that.  so, if there's not a selected row,
	// see if there is a focused one.

	// wxBUG: on the mac, when the list is empty, selected returns -1 and
	// wxBUG: focused returns 0.  this causes us to think that item 0 is
	// wxBUG: selected.  so make sure that we have something.

	if (GetItemCount() == 0)
		return false;

	long item = GetNextItem(-1,wxLIST_NEXT_ALL,wxLIST_STATE_SELECTED);
	if (item == -1)
		item = GetNextItem(-1,wxLIST_NEXT_ALL,wxLIST_STATE_FOCUSED);
	
	if (item == -1)
		return false;
	
	if (pRow)
		*pRow = item;

	return true;
}

//////////////////////////////////////////////////////////////////

void ViewFolder_ListCtrl::onEvent_FolderOpenFolders(void) const
{
	// frame received menu/toolbar event and is asking us to do the actual work.
	// that is, open a folder-diff window on the currently selected subdirs.
	// 
	// if none selected or if not a pair of subdirs, silently fail (since the
	// menu item/toolbar button should not have been enabled anyway).
	//
	// note: the behavior here should match our onListItemActivated().

	long kItem;
	if (!getSelectedRowIndex(&kItem))
		return;

	// we only need to handle the status cases in onListItemFocused() that
	// enabled this event.

	if (getItemStatus(kItem) != fd_item::FD_ITEM_STATUS_FOLDERS)
		return;
	
	fd_item * pFdItem = m_pViewFolder->getDoc()->getFdFd()->getItem(kItem);

	// now that we have identified the subdir pair, ask the frame factory
	// to either raise an existing frame, create a new frame, or populate
	// an empty frame -- as appropriate.

	gpFrameFactory->openFolderFrame(pFdItem->getPoiItem(0)->getFullPath(),
									pFdItem->getPoiItem(1)->getFullPath(),
									NULL);
}

//////////////////////////////////////////////////////////////////

void ViewFolder_ListCtrl::onEvent_FolderOpenFiles(void) const
{
	// frame received menu/toolbar event and is asking us to do the actual work.
	// that is, open a file-diff window on the files in the currently selected row.
	// 
	// if none selected or if not a pair of files, silently fail (since the
	// menu item/toolbar button should not have been enabled anyway).
	//
	// note: the behavior here should match our onListItemActivated().

	long kItem;
	if (!getSelectedRowIndex(&kItem))
		return;

	// we only need to handle the status cases in onListItemFocused() that
	// enabled this event.

	switch ( getItemStatus(kItem) )
	{
	default:
		return;

	case fd_item::FD_ITEM_STATUS_SAME_FILE:
		// TODO decide if we want to handle these differently -- for example,
		// TODO if they both refer to the same (physical) file, there's no
		// TODO need for a 2-way diff window....
	case fd_item::FD_ITEM_STATUS_IDENTICAL:
	case fd_item::FD_ITEM_STATUS_EQUIVALENT:
	case fd_item::FD_ITEM_STATUS_QUICKMATCH:
	case fd_item::FD_ITEM_STATUS_DIFFERENT:
		break;
	}
	
	fd_item * pFdItem = m_pViewFolder->getDoc()->getFdFd()->getItem(kItem);

	// now that we have identified the file pair, ask the frame factory
	// to either raise an existing frame, create a new frame, or populate
	// an empty frame -- as appropriate.

	gpFrameFactory->openFileDiffFrameOrSpawnAsyncXT(pFdItem->getPoiItem(0)->getFullPath(),
													pFdItem->getPoiItem(1)->getFullPath(),
													NULL);
}

static util_error _MyExportFileDiffsWithoutView(wxWindow * pParent, fd_item * pFdItem, int id, long kSync)
{
	util_error ue;
	de_de * pDeDe = NULL;

	// We do not have this pair open in a window, so
	// create a new fs_fs and de_de without a View.
	// 
	// This is modeled on the gui_app__batchoutput code,
	// but differs because:
	// [] instead of using cl_args, we use current global
	//    settings for things like tabs and show/hide stuff.
	// [] we can use dialogs to ask for stuff.

	Doc doc;
	doc.initFileDiff(pFdItem->getPoiItem(0)->getFullPath(),
					 pFdItem->getPoiItem(1)->getFullPath(),
					 NULL);

	ue = doc.getFsFs()->loadFiles(pParent);
	if (ue.isOK())
	{
		poi_item * pPoiDestination = NULL;
		ue = ViewFile::s_getExportPathnameFromUser(pParent, NULL, 0, id, &pPoiDestination);
		if (ue.isOK())
		{
			de_detail_level detailLevel = (de_detail_level)gpGlobalProps->getLong(GlobalProps::GPL_FILE_DETAIL_LEVEL);

			// create a DE_DE for this file pair.
			// the _DOP_ we init this with doesn't matter since
			// we aren't going to force a pDeDe->run() until we
			// get into ViewFile::s_doExportToString() and it
			// will set it to the correct value for this "id".
			pDeDe = new de_de(doc.getFsFs(), DE_DOP_ALL, DE_DOP_ALL, detailLevel);

			wxString strOutput;
			bool bHadChanges = false;

			ue = ViewFile::s_doExportToString(pDeDe, kSync, id,
											  strOutput, &bHadChanges,
											  (int) gpGlobalProps->getLong(GlobalProps::GPL_VIEW_FILE_TABSTOP));
			if (ue.isOK())
			{
				ue = ViewFile::s_doExportStringToDestination(pDeDe, kSync, id,
															 strOutput,
															 pPoiDestination);
			}
		}
	}

	DELETEP(pDeDe);
	return ue;
}
	
void ViewFolder_ListCtrl::onEvent_FolderExportDiffFiles(int /*gui_frame::_iface_id*/ id) const
{
	// folder frame received menu/toolbar event and it is the folder
	// view to export file diffs for the pair of files currently
	// selected.
	// 
	// if none selected or if not a pair of files, silently fail (since the
	// menu item/toolbar button should not have been enabled anyway).
	//
	// note: the behavior here should match our onListItemActivated().

	util_error ue;
	fd_item * pFdItem = NULL;				// we do not own this
	fs_fs * pFsFs_ExistingDiff = NULL;		// we do not own this

	long kItem;
	if (!getSelectedRowIndex(&kItem))
		return;

	switch ( getItemStatus(kItem) )
	{
	default:
		// should not get here since we only enabled the menu item
		// for file pairs with changes.
		ue.set(util_error::UE_UNSUPPORTED, _T("file pair not different"));
		goto Finished;

	case fd_item::FD_ITEM_STATUS_DIFFERENT:
		break;
	}
	
	pFdItem = m_pViewFolder->getDoc()->getFdFd()->getItem(kItem);

	// We may or may not have this file pair open in
	// another window.  if we do, use it -- as if they
	// had hit the Export Menu from (the reference-view of)
	// that window rather than from this folder window.

	pFsFs_ExistingDiff = gpFsFsTable->find(pFdItem->getPoiItem(0),
										   pFdItem->getPoiItem(1));
	if (pFsFs_ExistingDiff)
	{
		gui_frame * pFrame_ExistingDiff = gpFrameFactory->findFileDiffFrame(pFsFs_ExistingDiff);
		if (pFrame_ExistingDiff)
		{
			ViewFile * pViewFile = static_cast<ViewFile *>(pFrame_ExistingDiff->getView());
			if (pViewFile)
			{
				pViewFile->doExport( id, SYNC_VIEW );
				// we assume that the doExport() will print an error dialog if necessary.
				goto Finished;
			}
		}

		// We have a fs_fs, but no owning frame/view.
		// We should not get here.  I could assert this
		// but it's not worth crashing.
		ue.set(util_error::UE_UNSUPPORTED, _T("fs_fs without ViewFile"));
		goto Finished;
	}

	ue = _MyExportFileDiffsWithoutView(m_pViewFolder->getFrame(), pFdItem, id, SYNC_VIEW);

Finished:
	if (!ue.isOK() && (ue.getErr() != util_error::UE_CANCELED))
	{
		wxMessageDialog dlg(m_pViewFolder->getFrame(),
							ue.getMBMessage(),_("Export Cancelled!"),wxOK|wxICON_ERROR);
		dlg.ShowModal();
	}

}

#if defined(__WXMSW__)
void ViewFolder_ListCtrl::onEvent_FolderOpenShortcuts(void)
{
	// frame received menu/toolbar event and is asking us to do the actual work.
	// that is, open the show-shortcut-info dialog on the currently selected row
	// (which contains 1 or 2 shortcuts).

	long kItem;
	if (!getSelectedRowIndex(&kItem))
		return;

	switch (getItemStatus(kItem))
	{
	default:
		// quietly return if not looking at a shortcut.
		// this shouldn't happen because the menu item
		// should not have been enabled.
		return;
		
	case fd_item::FD_ITEM_STATUS_SHORTCUT_NULL:
	case fd_item::FD_ITEM_STATUS_NULL_SHORTCUT:
	case fd_item::FD_ITEM_STATUS_SHORTCUTS_SAME:
	case fd_item::FD_ITEM_STATUS_SHORTCUTS_EQ:
	case fd_item::FD_ITEM_STATUS_SHORTCUTS_NEQ:
		_show_shortcut_info(m_pViewFolder->getDoc()->getFdFd()->getItem(kItem));
		return;
	}

}
#endif

#if defined(__WXMAC__) || defined(__WXGTK__)
void ViewFolder_ListCtrl::onEvent_FolderOpenSymlinks(void)
{
	// frame received menu/toolbar event and is asking us to do the actual work.
	// that is, open the show-symlink-info dialog on the currently selected row
	// (which contains 1 or 2 symlinks).

	long kItem;
	if (!getSelectedRowIndex(&kItem))
		return;

	switch (getItemStatus(kItem))
	{
	default:
		// quietly return if not looking at a symlink.
		// this shouldn't happen because the menu item
		// should not have been enabled.
		return;
		
	case fd_item::FD_ITEM_STATUS_SYMLINK_NULL:
	case fd_item::FD_ITEM_STATUS_NULL_SYMLINK:
	case fd_item::FD_ITEM_STATUS_SYMLINKS_SAME:
	case fd_item::FD_ITEM_STATUS_SYMLINKS_EQ:
	case fd_item::FD_ITEM_STATUS_SYMLINKS_NEQ:
		_show_symlink_info(m_pViewFolder->getDoc()->getFdFd()->getItem(kItem));
		return;
	}

}
#endif

//////////////////////////////////////////////////////////////////

void ViewFolder_ListCtrl::_show_item_error(long item)
{
	util_error err = m_pViewFolder->getDoc()->getFdFd()->getItem(item)->getError();

//	wxLogTrace(wxTRACE_Messages, _T("ViewFolder::Error: [%d][%s][%s]"),
//			   err.getErr(),
//			   util_printable_s(err.getMessage()).wc_str(),
//			   util_printable_s(err.getExtraInfo()).wc_str());

	// raise MessageBox-like dialog explaining the error that
	// we encountered while comparing/reading/etc. the pair of
	// files or folders in this line item.

	wxMessageDialog dlg(m_pViewFolder->getFrame(),err.getMBMessage(),_("Error!"),wxOK|wxICON_ERROR);
	dlg.ShowModal();
}

//////////////////////////////////////////////////////////////////

void ViewFolder_ListCtrl::onListKeyDown(wxListEvent & event)
{
//	wxLogTrace(wxTRACE_Messages, _T("ViewFolder_ListCtrl::KeyDown: [%p][keycode %d]"),
//			   this, event.GetKeyCode());

	switch (event.GetKeyCode())
	{
	default:			{ event.Skip();	return; }

	case WXK_ESCAPE:	{ m_pViewFolder->getFrame()->postEscapeCloseCommand(); return; }
	}
	
}

//////////////////////////////////////////////////////////////////

#if defined(__WXMSW__)
void ViewFolder_ListCtrl::_show_shortcut_info(fd_item * pFdItem)
{
	poi_item * pPoiItem_0 = pFdItem->getPoiItem(0);
	poi_item * pPoiItem_1 = pFdItem->getPoiItem(1);
	
	dlg_show_shortcut_details::run_modal(this, pPoiItem_0, pPoiItem_1);
}
#endif//__WXMSW__

#if defined(__WXMAC__) || defined(__WXGTK__)
void ViewFolder_ListCtrl::_show_symlink_info(fd_item * pFdItem)
{
	poi_item * pPoiItem_0 = pFdItem->getPoiItem(0);
	poi_item * pPoiItem_1 = pFdItem->getPoiItem(1);
	
	dlg_show_symlink_details::run_modal(this, pPoiItem_0, pPoiItem_1);
}
#endif

//////////////////////////////////////////////////////////////////

void ViewFolder_ListCtrl::onMenuEvent_CTX_CLONE_ITEM(wxCommandEvent & /*event*/)
{
	fd_item * pFdItem = m_pViewFolder->getDoc()->getFdFd()->getItem(m_itemRightMouse);
	if (!pFdItem)	// should not happen
		return;

	util_error err = pFdItem->clone_item_on_disk_in_other_folder(false);
	if (err.isErr())
	{
		wxMessageDialog dlg(this, err.getMBMessage().wc_str(), VER_APP_TITLE, wxOK|wxICON_ERROR);
		dlg.ShowModal();
	}

	// clone updates the various fd_items, but we need to
	// force a rebuild of the display list.
	m_pViewFolder->queueEvent(VIEWFOLDER_QUEUE_EVENT_BUILD);
	
}

void ViewFolder_ListCtrl::onMenuEvent_CTX_CLONE_ITEM_RECURSIVE(wxCommandEvent & /*event*/)
{
	fd_item * pFdItem = m_pViewFolder->getDoc()->getFdFd()->getItem(m_itemRightMouse);
	if (!pFdItem)	// should not happen
		return;

	util_error err = pFdItem->clone_item_on_disk_in_other_folder(true);
	if (err.isErr())
	{
		wxMessageDialog dlg(this, err.getMBMessage().wc_str(), VER_APP_TITLE, wxOK|wxICON_ERROR);
		dlg.ShowModal();
	}

	// clone updates the various fd_items, but we need to
	// force a rebuild of the display list.
	m_pViewFolder->queueEvent(VIEWFOLDER_QUEUE_EVENT_BUILD);
}

void ViewFolder_ListCtrl::onMenuEvent_CTX_OVERWRITE_LEFT_ITEM(wxCommandEvent & /*event*/)
{
	fd_item * pFdItem = m_pViewFolder->getDoc()->getFdFd()->getItem(m_itemRightMouse);
	if (!pFdItem)	// should not happen
		return;

	util_error err = pFdItem->overwrite_item_on_disk(true);
	if (err.isErr())
	{
		wxMessageDialog dlg(this, err.getMBMessage().wc_str(), VER_APP_TITLE, wxOK|wxICON_ERROR);
		dlg.ShowModal();
	}

	// clone updates the various fd_items, but we need to
	// force a rebuild of the display list.
	m_pViewFolder->queueEvent(VIEWFOLDER_QUEUE_EVENT_BUILD);
}

void ViewFolder_ListCtrl::onMenuEvent_CTX_OVERWRITE_RIGHT_ITEM(wxCommandEvent & /*event*/)
{
	fd_item * pFdItem = m_pViewFolder->getDoc()->getFdFd()->getItem(m_itemRightMouse);
	if (!pFdItem)	// should not happen
		return;

	util_error err = pFdItem->overwrite_item_on_disk(false);
	if (err.isErr())
	{
		wxMessageDialog dlg(this, err.getMBMessage().wc_str(), VER_APP_TITLE, wxOK|wxICON_ERROR);
		dlg.ShowModal();
	}

	// clone updates the various fd_items, but we need to
	// force a rebuild of the display list.
	m_pViewFolder->queueEvent(VIEWFOLDER_QUEUE_EVENT_BUILD);
}
