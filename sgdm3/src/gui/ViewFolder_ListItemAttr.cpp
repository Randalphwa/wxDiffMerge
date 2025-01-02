// ViewFolder_ListItemAttr.cpp
// maintain a global set of wxListItemAttr's for use by the wxListCtrl
// in the ViewFolder.  (since all folder views will use the same colors
// we can create only one share it.)
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
// static callbacks to let us know when global props that we are
// interested in change.

static void _cb_long(void * pThis, const util_cbl_arg & arg)
{
	GlobalProps::EnumGPL id = (GlobalProps::EnumGPL)arg.m_l;

	ViewFolder_ListItemAttr * pVFLIA = (ViewFolder_ListItemAttr *)pThis;
	pVFLIA->gp_cb_long(id);
}

#if VIEW_FOLDER_LIST_CTRL_USE_FONTS
static void _cb_string(void * pThis, const util_cbl_arg & arg)
{
	GlobalProps::EnumGPS id = (GlobalProps::EnumGPS)arg.m_l;

	ViewFolder_ListItemAttr * pVFLIA = (ViewFolder_ListItemAttr *)pThis;
	pVFLIA->gp_cb_string(id);
}
#endif

//////////////////////////////////////////////////////////////////

ViewFolder_ListItemAttr::ViewFolder_ListItemAttr(void)
	: m_bStale(true)
{
	// register for notification when any of the folder window colors change

	gpGlobalProps->addLongCB(GlobalProps::GPL_FOLDER_COLOR_DIFFERENT_FG,	_cb_long, this);
	gpGlobalProps->addLongCB(GlobalProps::GPL_FOLDER_COLOR_DIFFERENT_BG,	_cb_long, this);
	gpGlobalProps->addLongCB(GlobalProps::GPL_FOLDER_COLOR_EQUAL_FG,		_cb_long, this);
	gpGlobalProps->addLongCB(GlobalProps::GPL_FOLDER_COLOR_EQUAL_BG,		_cb_long, this);
	gpGlobalProps->addLongCB(GlobalProps::GPL_FOLDER_COLOR_EQUIVALENT_FG,	_cb_long, this);
	gpGlobalProps->addLongCB(GlobalProps::GPL_FOLDER_COLOR_EQUIVALENT_BG,	_cb_long, this);
	gpGlobalProps->addLongCB(GlobalProps::GPL_FOLDER_COLOR_FOLDERS_FG,		_cb_long, this);
	gpGlobalProps->addLongCB(GlobalProps::GPL_FOLDER_COLOR_FOLDERS_BG,		_cb_long, this);
	gpGlobalProps->addLongCB(GlobalProps::GPL_FOLDER_COLOR_PEERLESS_FG,		_cb_long, this);
	gpGlobalProps->addLongCB(GlobalProps::GPL_FOLDER_COLOR_PEERLESS_BG,		_cb_long, this);
	gpGlobalProps->addLongCB(GlobalProps::GPL_FOLDER_COLOR_ERROR_FG,		_cb_long, this);
	gpGlobalProps->addLongCB(GlobalProps::GPL_FOLDER_COLOR_ERROR_BG,		_cb_long, this);

#if VIEW_FOLDER_LIST_CTRL_USE_FONTS
	m_pFont = NULL;
	gpGlobalProps->addStringCB(GlobalProps::GPS_VIEW_FOLDER_FONT,			_cb_string, this);
#endif

	// defer _populate() until actually needed.
}

ViewFolder_ListItemAttr::~ViewFolder_ListItemAttr(void)
{
	gpGlobalProps->delLongCB(GlobalProps::GPL_FOLDER_COLOR_DIFFERENT_FG,	_cb_long, this);
	gpGlobalProps->delLongCB(GlobalProps::GPL_FOLDER_COLOR_DIFFERENT_BG,	_cb_long, this);
	gpGlobalProps->delLongCB(GlobalProps::GPL_FOLDER_COLOR_EQUAL_FG,		_cb_long, this);
	gpGlobalProps->delLongCB(GlobalProps::GPL_FOLDER_COLOR_EQUAL_BG,		_cb_long, this);
	gpGlobalProps->delLongCB(GlobalProps::GPL_FOLDER_COLOR_EQUIVALENT_FG,	_cb_long, this);
	gpGlobalProps->delLongCB(GlobalProps::GPL_FOLDER_COLOR_EQUIVALENT_BG,	_cb_long, this);
	gpGlobalProps->delLongCB(GlobalProps::GPL_FOLDER_COLOR_FOLDERS_FG,		_cb_long, this);
	gpGlobalProps->delLongCB(GlobalProps::GPL_FOLDER_COLOR_FOLDERS_BG,		_cb_long, this);
	gpGlobalProps->delLongCB(GlobalProps::GPL_FOLDER_COLOR_PEERLESS_FG,		_cb_long, this);
	gpGlobalProps->delLongCB(GlobalProps::GPL_FOLDER_COLOR_PEERLESS_BG,		_cb_long, this);
	gpGlobalProps->delLongCB(GlobalProps::GPL_FOLDER_COLOR_ERROR_FG,		_cb_long, this);
	gpGlobalProps->delLongCB(GlobalProps::GPL_FOLDER_COLOR_ERROR_BG,		_cb_long, this);

#if VIEW_FOLDER_LIST_CTRL_USE_FONTS
	gpGlobalProps->delStringCB(GlobalProps::GPS_VIEW_FOLDER_FONT,			_cb_string, this);
	DELETEP(m_pFont);
#endif
}

//////////////////////////////////////////////////////////////////

void ViewFolder_ListItemAttr::_populate(void)
{
	// populate attribute structures for each type of line item.

#if VIEW_FOLDER_LIST_CTRL_USE_FONTS
	if (!m_pFont)
	{
		m_pFont = gpGlobalProps->createNormalFont(GlobalProps::GPS_VIEW_FOLDER_FONT);

#if VIEW_FOLDER_LIST_CTRL_PERITEM_FONTS
		// See note in ViewFolder_ListItemAttr.h
		for (int kItem=0; (kItem<fd_item::__FD_ITEM_STATUS__COUNT__); kItem++)
			m_attr[kItem].SetFont(*m_pFont);
#endif
	}
#endif

	m_attr[fd_item::FD_ITEM_STATUS_SAME_FILE].SetTextColour(      gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_EQUAL_FG));
	m_attr[fd_item::FD_ITEM_STATUS_SAME_FILE].SetBackgroundColour(gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_EQUAL_BG));
	
	m_attr[fd_item::FD_ITEM_STATUS_IDENTICAL].SetTextColour(      gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_EQUAL_FG));
	m_attr[fd_item::FD_ITEM_STATUS_IDENTICAL].SetBackgroundColour(gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_EQUAL_BG));

	m_attr[fd_item::FD_ITEM_STATUS_EQUIVALENT].SetTextColour(      gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_EQUIVALENT_FG));
	m_attr[fd_item::FD_ITEM_STATUS_EQUIVALENT].SetBackgroundColour(gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_EQUIVALENT_BG));

	m_attr[fd_item::FD_ITEM_STATUS_QUICKMATCH].SetTextColour(      gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_EQUIVALENT_FG));
	m_attr[fd_item::FD_ITEM_STATUS_QUICKMATCH].SetBackgroundColour(gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_EQUIVALENT_BG));

	m_attr[fd_item::FD_ITEM_STATUS_DIFFERENT].SetTextColour(      gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_DIFFERENT_FG));
	m_attr[fd_item::FD_ITEM_STATUS_DIFFERENT].SetBackgroundColour(gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_DIFFERENT_BG));

	m_attr[fd_item::FD_ITEM_STATUS_FOLDERS    ].SetTextColour(      gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_FOLDERS_FG));
	m_attr[fd_item::FD_ITEM_STATUS_FOLDERS    ].SetBackgroundColour(gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_FOLDERS_BG));

	m_attr[fd_item::FD_ITEM_STATUS_FOLDER_NULL].SetTextColour(      gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_PEERLESS_FG));
	m_attr[fd_item::FD_ITEM_STATUS_FOLDER_NULL].SetBackgroundColour(gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_PEERLESS_BG));
	m_attr[fd_item::FD_ITEM_STATUS_NULL_FOLDER].SetTextColour(      gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_PEERLESS_FG));
	m_attr[fd_item::FD_ITEM_STATUS_NULL_FOLDER].SetBackgroundColour(gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_PEERLESS_BG));

	m_attr[fd_item::FD_ITEM_STATUS_FILE_NULL].SetTextColour(      gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_PEERLESS_FG));
	m_attr[fd_item::FD_ITEM_STATUS_FILE_NULL].SetBackgroundColour(gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_PEERLESS_BG));
	m_attr[fd_item::FD_ITEM_STATUS_NULL_FILE].SetTextColour(      gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_PEERLESS_FG));
	m_attr[fd_item::FD_ITEM_STATUS_NULL_FILE].SetBackgroundColour(gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_PEERLESS_BG));

	m_attr[fd_item::FD_ITEM_STATUS_UNKNOWN  ].SetTextColour(      gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_ERROR_FG));	// shouldn't be needed
	m_attr[fd_item::FD_ITEM_STATUS_UNKNOWN  ].SetBackgroundColour(gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_ERROR_BG));	// shouldn't be needed
	m_attr[fd_item::FD_ITEM_STATUS_BOTH_NULL].SetTextColour(      gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_ERROR_FG));	// shouldn't be needed
	m_attr[fd_item::FD_ITEM_STATUS_BOTH_NULL].SetBackgroundColour(gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_ERROR_BG));	// shouldn't be needed
	m_attr[fd_item::FD_ITEM_STATUS_ERROR    ].SetTextColour(      gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_ERROR_FG));
	m_attr[fd_item::FD_ITEM_STATUS_ERROR    ].SetBackgroundColour(gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_ERROR_BG));
	m_attr[fd_item::FD_ITEM_STATUS_MISMATCH ].SetTextColour(      gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_ERROR_FG));	// shouldn't be needed
	m_attr[fd_item::FD_ITEM_STATUS_MISMATCH ].SetBackgroundColour(gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_ERROR_BG));	// shouldn't be needed

	m_attr[fd_item::FD_ITEM_STATUS_SHORTCUTS_SAME].SetTextColour(      gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_EQUAL_FG));
	m_attr[fd_item::FD_ITEM_STATUS_SHORTCUTS_SAME].SetBackgroundColour(gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_EQUAL_BG));
	m_attr[fd_item::FD_ITEM_STATUS_SHORTCUTS_EQ  ].SetTextColour(      gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_EQUAL_FG));
	m_attr[fd_item::FD_ITEM_STATUS_SHORTCUTS_EQ  ].SetBackgroundColour(gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_EQUAL_BG));
	m_attr[fd_item::FD_ITEM_STATUS_SHORTCUTS_NEQ ].SetTextColour(      gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_DIFFERENT_FG));
	m_attr[fd_item::FD_ITEM_STATUS_SHORTCUTS_NEQ ].SetBackgroundColour(gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_DIFFERENT_BG));

	m_attr[fd_item::FD_ITEM_STATUS_SHORTCUT_NULL].SetTextColour(      gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_PEERLESS_FG));
	m_attr[fd_item::FD_ITEM_STATUS_SHORTCUT_NULL].SetBackgroundColour(gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_PEERLESS_BG));
	m_attr[fd_item::FD_ITEM_STATUS_NULL_SHORTCUT].SetTextColour(      gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_PEERLESS_FG));
	m_attr[fd_item::FD_ITEM_STATUS_NULL_SHORTCUT].SetBackgroundColour(gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_PEERLESS_BG));

	m_attr[fd_item::FD_ITEM_STATUS_SYMLINKS_SAME].SetTextColour(      gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_EQUAL_FG));
	m_attr[fd_item::FD_ITEM_STATUS_SYMLINKS_SAME].SetBackgroundColour(gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_EQUAL_BG));
	m_attr[fd_item::FD_ITEM_STATUS_SYMLINKS_EQ  ].SetTextColour(      gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_EQUAL_FG));
	m_attr[fd_item::FD_ITEM_STATUS_SYMLINKS_EQ  ].SetBackgroundColour(gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_EQUAL_BG));
	m_attr[fd_item::FD_ITEM_STATUS_SYMLINKS_NEQ ].SetTextColour(      gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_DIFFERENT_FG));
	m_attr[fd_item::FD_ITEM_STATUS_SYMLINKS_NEQ ].SetBackgroundColour(gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_DIFFERENT_BG));

	m_attr[fd_item::FD_ITEM_STATUS_SYMLINK_NULL].SetTextColour(      gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_PEERLESS_FG));
	m_attr[fd_item::FD_ITEM_STATUS_SYMLINK_NULL].SetBackgroundColour(gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_PEERLESS_BG));
	m_attr[fd_item::FD_ITEM_STATUS_NULL_SYMLINK].SetTextColour(      gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_PEERLESS_FG));
	m_attr[fd_item::FD_ITEM_STATUS_NULL_SYMLINK].SetBackgroundColour(gpGlobalProps->getColor(GlobalProps::GPL_FOLDER_COLOR_PEERLESS_BG));

	m_bStale = false;
}

wxListItemAttr * ViewFolder_ListItemAttr::getAttr(fd_item::Status s)
{
	if (m_bStale)
		_populate();
	
	return &m_attr[s];
}

#if VIEW_FOLDER_LIST_CTRL_USE_FONTS
wxFont * ViewFolder_ListItemAttr::getFont(void)
{
	if (m_bStale)
		_populate();
	
	return m_pFont;
}
#endif

//////////////////////////////////////////////////////////////////

void ViewFolder_ListItemAttr::gp_cb_long(GlobalProps::EnumGPL id)
{
	// one of the "long" global variables that we care about has changed.

	switch (id)
	{
	case GlobalProps::GPL_FOLDER_COLOR_DIFFERENT_FG:
	case GlobalProps::GPL_FOLDER_COLOR_DIFFERENT_BG:
	case GlobalProps::GPL_FOLDER_COLOR_EQUAL_FG:
	case GlobalProps::GPL_FOLDER_COLOR_EQUAL_BG:
	case GlobalProps::GPL_FOLDER_COLOR_EQUIVALENT_FG:
	case GlobalProps::GPL_FOLDER_COLOR_EQUIVALENT_BG:
	case GlobalProps::GPL_FOLDER_COLOR_FOLDERS_FG:
	case GlobalProps::GPL_FOLDER_COLOR_FOLDERS_BG:
	case GlobalProps::GPL_FOLDER_COLOR_PEERLESS_FG:
	case GlobalProps::GPL_FOLDER_COLOR_PEERLESS_BG:
	case GlobalProps::GPL_FOLDER_COLOR_ERROR_FG:
	case GlobalProps::GPL_FOLDER_COLOR_ERROR_BG:
		// we need to reload the attr's with the now current colors.
		// we could be try to be efficient and use "id" and only load
		// the ones that reference this property, but i don't
		// want to recreate all the stuff in _populate(), so we just
		// mark everything stale and let _populate() reload all of them.
		// but we do do it in a lazy fashion.
		m_bStale = true;
		break;

	default:
		wxASSERT_MSG( (0), _T("Coding Error") );
		return;
	}

	// tell anyone who cares (all ViewFolder windows) that we have
	// changed the colors and let them do whatever they need to.

	if (m_bStale)
		m_cbl.callAll( util_cbl_arg(this,0) );
}

void ViewFolder_ListItemAttr::gp_cb_string(GlobalProps::EnumGPS id)
{
	// one of the "string" global variables that we care about has changed.

	switch (id)
	{
#if VIEW_FOLDER_LIST_CTRL_USE_FONTS
	case GlobalProps::GPS_VIEW_FOLDER_FONT:
		DELETEP(m_pFont);
		m_bStale = true;
		break;
#endif
		
	default:
		wxASSERT_MSG( (0), _T("Coding Error") );
		return;
	}

	// tell anyone who cares (all ViewFolder windows) that we have
	// changed something and let them do whatever they need to.

	if (m_bStale)
		m_cbl.callAll( util_cbl_arg(this,0) );
}

