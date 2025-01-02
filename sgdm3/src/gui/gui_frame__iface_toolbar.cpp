// gui_frame__iface_toolbar.cpp
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

wxBitmap _src2png2bmp(const unsigned char * szSrc)
{
	// variable contain PNG data encoded in base16.  convert to normal
	// binary data and then into an wxImage and then into a wxBitmap.

	wxMemoryOutputStream mos;

	// convert base16 to normal using output-stream

	for (const unsigned char * sz=szSrc; *sz; /**/)
	{
		unsigned char ch = *sz++;
		unsigned char cl = *sz++;

		unsigned char c;
		
		if      ((ch >= 'a') && (ch <= 'f'))	c = ((ch - 'a' + 10) << 4);
		else if ((ch >= '0') && (ch <= '9'))	c = ((ch - '0'     ) << 4);
		else wxASSERT_MSG( (0), _T("Coding Error") );

		if      ((cl >= 'a') && (cl <= 'f'))	c |= (cl - 'a' + 10);
		else if ((cl >= '0') && (cl <= '9'))	c |= (cl - '0'     );
		else wxASSERT_MSG( (0), _T("Coding Error") );

		mos.PutC(c);
	}

	// Create input-stream with our data to supply data to wxImage
	// and then return a bitmap from the given image.
	// With wx 2.9.5 we can get warings of the form:
	//    "iCCP: known incorrect sRGB profile"
	// for some of our PNGs, such as TB__EDIT_CUT.
	// Divert the error/warning log so we can ignore them.

	wxMemoryInputStream mis(mos);
	util_error err;
	{
		util_logToString uLog(&err.refExtraInfo(), true);

		wxImage im(mis,wxBITMAP_TYPE_PNG);
		return wxBitmap(im);
	}

}

//////////////////////////////////////////////////////////////////

void gui_frame::_iface_add_toolbar_item(wxToolBar * pToolBar, gui_frame::_iface_id id, wxItemKind kind)
{
	const _iface_def * pDef = gui_frame::_get_iface_def(id);

	if (!pDef)
		return;

	int local_id = ((pDef->wx_id != wxID_NONE) ? pDef->wx_id : pDef->id);

	switch (pDef->tbType)
	{
	default:
	case _T_NONE:
		pToolBar->AddTool(local_id,pDef->szToolBarLabel,
						  wxNullBitmap,wxNullBitmap,
						  kind,pDef->szToolBarToolTip,pDef->szStatusBar);
		return;
		
	case _T_XPM_VARIABLE:
		pToolBar->AddTool(local_id,pDef->szToolBarLabel,
						  wxBitmap((const char **)pDef->szToolBarIcon),wxNullBitmap,
						  kind,pDef->szToolBarToolTip,pDef->szStatusBar);
		return;

	case _T_PNG2SRC_VARIABLE:
		pToolBar->AddTool(local_id,pDef->szToolBarLabel,
						  _src2png2bmp((const unsigned char *)pDef->szToolBarIcon),
						  _src2png2bmp((const unsigned char *)pDef->szToolBarIconDisabled),
						  kind,pDef->szToolBarToolTip,pDef->szStatusBar);
		return;
	}

}

//////////////////////////////////////////////////////////////////

void gui_frame::_iface_define_toolbar(ToolBarType tbt)
{
	if (m_pToolBar)
	{
		if (m_tbt == tbt)		// already set to this type, 
			return;				// no need to do anything.

		wxASSERT_MSG( (GetToolBar() == m_pToolBar), _T("Toolbar handles don't match?") );

		// wxBUG: mac version crashes if you remove and delete the existing toolbar.
		// wxBUG: this happens when looking at an empty window and you click on one
		// wxBUG: of the buttons that opens a type of window.  we crash after we
		// wxBUG: go idle after switching to the new toolbar.  it's like it has a
		// wxBUG: pending event for the old toolbar button that we clicked.  calling
		// wxBUG: m_pToolBar->Destroy() doesn't help.  if we just unhook it from the
		// wxBUG: frame but don't delete it, then auto-resize breaks because there is
		// wxBUG: more than one (non tool/status bar) child.
		//
		// SetToolBar(NULL);		// remove current toolbar from frame and delete
		// HACK let it leak -- delete m_pToolBar;
		// m_pToolBar = NULL;

		m_pToolBar->ClearTools();
	}

	// create new toolbar (if necessary) and populate for the frame.

	if (!m_pToolBar)
		m_pToolBar = CreateToolBar(wxTB_FLAT|wxTB_HORIZONTAL|wxTB_NODIVIDER);
	m_pToolBar->SetToolBitmapSize( _get_toolbar_bitmap_size() );
	m_tbt = tbt;

	switch (tbt)
	{
	default:		// not possible, but this does silence compiler warning on Mac.
		wxASSERT_MSG( 0, _T("Coding Error: gui_frame::_iface_define_toolbar: unknown type"));
		return;
		
	case TBT_BASIC:
		_iface_add_toolbar_item(m_pToolBar,MENU_FILE_FOLDER_DIFF);
		_iface_add_toolbar_item(m_pToolBar,MENU_FILE_FILE_DIFF);
		_iface_add_toolbar_item(m_pToolBar,MENU_FILE_FILE_MERGE);
		break;

	case TBT_FOLDER:
		_iface_add_toolbar_item(m_pToolBar,MENU_FOLDER_OPEN_FILES);
		_iface_add_toolbar_item(m_pToolBar,MENU_FOLDER_OPEN_FOLDERS);
		m_pToolBar->AddSeparator();
		_iface_add_toolbar_item(m_pToolBar,MENU_FOLDER_SHOW_EQUAL,wxITEM_CHECK);
		_iface_add_toolbar_item(m_pToolBar,MENU_FOLDER_SHOW_EQUIVALENT,wxITEM_CHECK);
		_iface_add_toolbar_item(m_pToolBar,MENU_FOLDER_SHOW_QUICKMATCH,wxITEM_CHECK);
		_iface_add_toolbar_item(m_pToolBar,MENU_FOLDER_SHOW_SINGLES,wxITEM_CHECK);
		_iface_add_toolbar_item(m_pToolBar,MENU_FOLDER_SHOW_FOLDERS,wxITEM_CHECK);
		break;

	case TBT_DIFF:
		_iface_add_toolbar_item(m_pToolBar,MENU_VIEWFILE_SHOW_ALL,wxITEM_RADIO);
		_iface_add_toolbar_item(m_pToolBar,MENU_VIEWFILE_SHOW_DIF,wxITEM_RADIO);
		_iface_add_toolbar_item(m_pToolBar,MENU_VIEWFILE_SHOW_CTX,wxITEM_RADIO);
		m_pToolBar->AddSeparator();
		_iface_add_toolbar_item(m_pToolBar,MENU_VIEWFILE_LINENUMBERS,wxITEM_CHECK);
		_iface_add_toolbar_item(m_pToolBar,MENU_VIEWFILE_PILCROW,wxITEM_CHECK);
		m_pToolBar->AddSeparator();
		_iface_add_toolbar_item(m_pToolBar,MENU_VIEWFILE_SPLIT_VERTICALLY,wxITEM_RADIO);
		_iface_add_toolbar_item(m_pToolBar,MENU_VIEWFILE_SPLIT_HORIZONTALLY,wxITEM_RADIO);
		m_pToolBar->AddSeparator();
		_iface_add_toolbar_item(m_pToolBar,MENU_FILE_SAVE);
		m_pToolBar->AddSeparator();
		_iface_add_toolbar_item(m_pToolBar,MENU_EDIT_CUT);
		_iface_add_toolbar_item(m_pToolBar,MENU_EDIT_COPY);
		_iface_add_toolbar_item(m_pToolBar,MENU_EDIT_PASTE);
		m_pToolBar->AddSeparator();
		_iface_add_toolbar_item(m_pToolBar,MENU_EDIT_UNDO);
		_iface_add_toolbar_item(m_pToolBar,MENU_EDIT_REDO);
		m_pToolBar->AddSeparator();
		_iface_add_toolbar_item(m_pToolBar,MENU_EDIT_APPLY_DEFAULT_L);
		_iface_add_toolbar_item(m_pToolBar,MENU_EDIT_NEXT_DELTA);
		_iface_add_toolbar_item(m_pToolBar,MENU_EDIT_PREV_DELTA);
		break;

	case TBT_MERGE:
		_iface_add_toolbar_item(m_pToolBar,MENU_VIEWFILE_SHOW_ALL,wxITEM_RADIO);
		_iface_add_toolbar_item(m_pToolBar,MENU_VIEWFILE_SHOW_DIF,wxITEM_RADIO);
		_iface_add_toolbar_item(m_pToolBar,MENU_VIEWFILE_SHOW_CTX,wxITEM_RADIO);
		m_pToolBar->AddSeparator();
		_iface_add_toolbar_item(m_pToolBar,MENU_VIEWFILE_LINENUMBERS,wxITEM_CHECK);
		_iface_add_toolbar_item(m_pToolBar,MENU_VIEWFILE_PILCROW,wxITEM_CHECK);
		m_pToolBar->AddSeparator();
		_iface_add_toolbar_item(m_pToolBar,MENU_VIEWFILE_SPLIT_VERTICALLY,wxITEM_RADIO);
		_iface_add_toolbar_item(m_pToolBar,MENU_VIEWFILE_SPLIT_HORIZONTALLY,wxITEM_RADIO);
		m_pToolBar->AddSeparator();
		_iface_add_toolbar_item(m_pToolBar,MENU_FILE_SAVE);
		m_pToolBar->AddSeparator();
		_iface_add_toolbar_item(m_pToolBar,MENU_EDIT_CUT);
		_iface_add_toolbar_item(m_pToolBar,MENU_EDIT_COPY);
		_iface_add_toolbar_item(m_pToolBar,MENU_EDIT_PASTE);
		m_pToolBar->AddSeparator();
		_iface_add_toolbar_item(m_pToolBar,MENU_EDIT_UNDO);
		_iface_add_toolbar_item(m_pToolBar,MENU_EDIT_REDO);
		m_pToolBar->AddSeparator();
		_iface_add_toolbar_item(m_pToolBar,MENU_EDIT_APPLY_DEFAULT_L);
		_iface_add_toolbar_item(m_pToolBar,MENU_EDIT_APPLY_DEFAULT_R);
		_iface_add_toolbar_item(m_pToolBar,MENU_EDIT_NEXT_DELTA);
		_iface_add_toolbar_item(m_pToolBar,MENU_EDIT_PREV_DELTA);
		m_pToolBar->AddSeparator();
		_iface_add_toolbar_item(m_pToolBar,MENU_EDIT_NEXT_CONFLICT);
		_iface_add_toolbar_item(m_pToolBar,MENU_EDIT_PREV_CONFLICT);
		m_pToolBar->AddSeparator();
		_iface_add_toolbar_item(m_pToolBar,MENU_EDIT_AUTOMERGE);
		break;
	}

	m_pToolBar->Realize();
}

//////////////////////////////////////////////////////////////////
