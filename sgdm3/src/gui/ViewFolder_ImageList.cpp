// ViewFolder_ImageList.cpp
// maintain a global wxImageList for use by the wxListCtrl in the
// ViewFolder (since we can share the bitmaps, we only create one).
//
// See gpViewFolder_ImageList
// 
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fd.h>
#include <fs.h>
#include <fl.h>
#include <de.h>
#include <gui.h>

//////////////////////////////////////////////////////////////////
// Icons for line items in ViewFolder ListCtrl

#include <Resources/FolderWindow/blank.xpm>
#include <Resources/FolderWindow/error.xpm>

#include <Resources/FolderWindow/flat_file_equal.xpm>
#include <Resources/FolderWindow/flat_file_equivalent.xpm>
#include <Resources/FolderWindow/flat_file_quickmatch.xpm>
#include <Resources/FolderWindow/flat_file_notequal.xpm>
#include <Resources/FolderWindow/flat_file_plusminus.xpm>

#include <Resources/FolderWindow/flat_folder.xpm>
#include <Resources/FolderWindow/flat_folder_plusminus.xpm>

#include <Resources/FolderWindow/flat_link_equal.xpm>
#include <Resources/FolderWindow/flat_link_notequal.xpm>
#include <Resources/FolderWindow/flat_link_plusminus.xpm>


//////////////////////////////////////////////////////////////////

ViewFolder_ImageList::ViewFolder_ImageList(void)
	: wxImageList(16,15,true,fd_item::__FD_ITEM_STATUS__COUNT__)
{
#define AddXPM(_item_,_xpm_) { int k=Add(wxBitmap((const char **)(_xpm_))); MY_ASSERT( (k==(fd_item::_item_)) ); }

	// WARNING: do not reorder these -- they must be in FD_ITEM_STATUS_ order

	AddXPM(FD_ITEM_STATUS_UNKNOWN,		blank_xpm);			// should probably never be seen
	AddXPM(FD_ITEM_STATUS_ERROR,		error_xpm);
	AddXPM(FD_ITEM_STATUS_MISMATCH,		error_xpm);			// should probably never be seen
	AddXPM(FD_ITEM_STATUS_BOTH_NULL,	blank_xpm);			// should probably never be seen

	AddXPM(FD_ITEM_STATUS_SAME_FILE,	flat_file_equal_xpm);
	AddXPM(FD_ITEM_STATUS_IDENTICAL,	flat_file_equal_xpm);
	AddXPM(FD_ITEM_STATUS_EQUIVALENT,	flat_file_equivalent_xpm);
	AddXPM(FD_ITEM_STATUS_QUICKMATCH,	flat_file_quickmatch_xpm);
	AddXPM(FD_ITEM_STATUS_DIFFERENT,	flat_file_notequal_xpm);

	AddXPM(FD_ITEM_STATUS_FOLDERS,		flat_folder_xpm);

	AddXPM(FD_ITEM_STATUS_FILE_NULL,	flat_file_plusminus_xpm);
	AddXPM(FD_ITEM_STATUS_NULL_FILE,	flat_file_plusminus_xpm);

	AddXPM(FD_ITEM_STATUS_FOLDER_NULL,	flat_folder_plusminus_xpm);
	AddXPM(FD_ITEM_STATUS_NULL_FOLDER,	flat_folder_plusminus_xpm);

	AddXPM(FD_ITEM_STATUS_SHORTCUT_NULL,	flat_link_plusminus_xpm);
	AddXPM(FD_ITEM_STATUS_NULL_SHORTCUT,	flat_link_plusminus_xpm);
	AddXPM(FD_ITEM_STATUS_SHORTCUTS_SAME,	flat_link_equal_xpm);
	AddXPM(FD_ITEM_STATUS_SHORTCUTS_EQ,		flat_link_equal_xpm);
	AddXPM(FD_ITEM_STATUS_SHORTCUTS_NEQ,	flat_link_notequal_xpm);

	AddXPM(FD_ITEM_STATUS_SYMLINK_NULL,		flat_link_plusminus_xpm);
	AddXPM(FD_ITEM_STATUS_NULL_SYMLINK,		flat_link_plusminus_xpm);
	AddXPM(FD_ITEM_STATUS_SYMLINKS_SAME,	flat_link_equal_xpm);
	AddXPM(FD_ITEM_STATUS_SYMLINKS_EQ,		flat_link_equal_xpm);
	AddXPM(FD_ITEM_STATUS_SYMLINKS_NEQ,		flat_link_notequal_xpm)

}

ViewFolder_ImageList::~ViewFolder_ImageList(void)
{
}

//////////////////////////////////////////////////////////////////

