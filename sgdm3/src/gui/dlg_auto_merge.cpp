// dlg_auto_merge.cpp -- show preview of what auto-merge will do and
// let them adjust it a little.
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
#if 0

#define DLG_TITLE			_("Automatic Merge")
#define DLG_HELP_TITLE		_("Help for Automatic Merge")

#define M 7
#define FIX 0
#define VAR 1

#define CTX_APPLY		(wxID_HIGHEST+100)
#define CTX_OMIT		(wxID_HIGHEST+101)
#define CTX_DELETE		(wxID_HIGHEST+200+POP_DELETE)
#define CTX_INSERT_L	(wxID_HIGHEST+200+POP_INSERT_L)
#define CTX_INSERT_R	(wxID_HIGHEST+200+POP_INSERT_R)
#define CTX_REPLACE_L	(wxID_HIGHEST+200+POP_REPLACE_L)
#define CTX_REPLACE_R	(wxID_HIGHEST+200+POP_REPLACE_R)

//////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(dlg_auto_merge,wxDialog)
	EVT_LIST_ITEM_SELECTED(ID_LIST_CTRL, dlg_auto_merge::onListItemSelected)
	EVT_LIST_ITEM_RIGHT_CLICK(ID_LIST_CTRL, dlg_auto_merge::onListItemRightClick)
	EVT_MENU(CTX_APPLY, dlg_auto_merge::onCtxMenuEvent_Apply)
	EVT_MENU(CTX_OMIT,  dlg_auto_merge::onCtxMenuEvent_Omit)
	EVT_MENU(CTX_DELETE,	dlg_auto_merge::onCtxMenuEvent_Command)
	EVT_MENU(CTX_INSERT_L,	dlg_auto_merge::onCtxMenuEvent_Command)
	EVT_MENU(CTX_INSERT_R,	dlg_auto_merge::onCtxMenuEvent_Command)
	EVT_MENU(CTX_REPLACE_L,	dlg_auto_merge::onCtxMenuEvent_Command)
	EVT_MENU(CTX_REPLACE_R,	dlg_auto_merge::onCtxMenuEvent_Command)
	EVT_BUTTON(ID_SHOW_HELP, dlg_auto_merge::onButton_ShowHelp)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(dlg_auto_merge_help,wxDialog)
END_EVENT_TABLE()

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
// TODO we should have a "? Verify" status for patches that are
// TODO near edits already made in the file.  this would let them
// TODO see and force them to confirm that a particular patch is
// TODO what they want --- the problem is that for some kinds of
// TODO patches (like "a a <void>" where if they used the default
// TODO right-mouse context menu and deleted the 'a' (giving "a <void> <void>")
// TODO and then run auto-merge, we're going to suggest putting it back....)
//////////////////////////////////////////////////////////////////

dlg_auto_merge::dlg_auto_merge(gui_frame * pFrame, ViewFile * pViewFile, fim_patchset * pPatchSet, int nrConflicts)
	: m_pFrame(pFrame),
	  m_pViewFile(pViewFile),
	  m_pPatchSet(pPatchSet),
	  m_nrConflicts(nrConflicts)
{
#if defined(__WXMAC__)
	SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif

	Create(pFrame,-1,DLG_TITLE,wxDefaultPosition,wxDefaultSize,
		   wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX);

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		wxStaticText * pHelp;
		if (m_nrConflicts == 0)
			pHelp = new wxStaticText(this,-1,
									 wxGetTranslation( L"Actions that Auto-Merge will perform."
										 ));
		else
			pHelp = new wxStaticText(this,-1,
									 wxGetTranslation( L"Actions that Auto-Merge will perform.\n"
													   L"\n"
													   L"WARNING: There are conflicts."
										 ));
		vSizerTop->Add( pHelp, FIX, wxGROW|wxALL, M);

		m_pListCtrl = new wxListCtrl(this,ID_LIST_CTRL,wxDefaultPosition,wxSize(650,150),wxLC_REPORT|wxLC_SINGLE_SEL|MY_BORDER_STYLE|MY_RULES_STYLE);
		_build_image_list();
		_define_columns();
		_populate_list();
		vSizerTop->Add( m_pListCtrl, VAR, wxGROW|wxALL, M);
		
		//////////////////////////////////////////////////////////////////

		m_pStaticTextSummary = new wxStaticText(this,ID_SUMMARY,_T(""));
		vSizerTop->Add(m_pStaticTextSummary,FIX,wxGROW|wxALL,M);
		
		//////////////////////////////////////////////////////////////////

		// put horizontal line and set of ok/cancel buttons across the bottom of the dialog

		vSizerTop->Add( new wxStaticLine(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxLI_HORIZONTAL), FIX, wxGROW|wxLEFT|wxRIGHT, M);
		wxBoxSizer * hSizerButtons = new wxBoxSizer(wxHORIZONTAL);
		{
			m_pButtonHelp = new wxButton(this,ID_SHOW_HELP,_("&Help..."));
			hSizerButtons->Add( m_pButtonHelp, FIX, wxALIGN_CENTER_VERTICAL, 0);
			hSizerButtons->AddStretchSpacer(VAR);
			hSizerButtons->Add( CreateButtonSizer(wxOK | wxCANCEL), FIX, 0, 0);
		}
		vSizerTop->Add(hSizerButtons,FIX,wxGROW|wxALL,M);

	}
	SetSizer(vSizerTop);
	vSizerTop->SetSizeHints(this);
	vSizerTop->Fit(this);

	// center dialog on parent window rather than arbitrary location
	// (screen-centered on Win32 & GTK, upper-left corner of window in MAC).
	// WXBUG: this doesn't appear to work on WIN32 -- oh well.

	Centre(wxBOTH);

}

dlg_auto_merge::~dlg_auto_merge(void)
{
	delete m_pImageList;
}

//////////////////////////////////////////////////////////////////

#define COL_STATUS		0
#define COL_ACTION		1
#define COL_LEFT		2
#define COL_CENTER		3
#define COL_RIGHT		4

#define IMG_PLUS		0
#define IMG_MINUS		1
#define IMG_CHECK		2
#define IMG_X			3
#define IMG_AT			4
#define IMG_QUESTION	5

//////////////////////////////////////////////////////////////////

void dlg_auto_merge::_build_image_list(void)
{
	m_pImageList = new wxImageList(16,16,true,4);

#define AddXPM(_item_,_xpm_) { int k=m_pImageList->Add(wxBitmap((const char **)(_xpm_))); MY_ASSERT( (k==_item_) ); }

	AddXPM(IMG_PLUS,     gXpm_plusmark_16x16);
	AddXPM(IMG_MINUS,    gXpm_minusmark_16x16);
	AddXPM(IMG_CHECK,    gXpm_checkmark_16x16);
	AddXPM(IMG_X,        gXpm_xmark_16x16);
	AddXPM(IMG_AT,       gXpm_atmark_16x16);
	AddXPM(IMG_QUESTION, gXpm_questionmark_16x16);

	m_pListCtrl->SetImageList(m_pImageList,wxIMAGE_LIST_SMALL);
}

void dlg_auto_merge::_define_columns(void)
{
	wxListItem liCol;

	liCol.SetText(_("Status"));
	liCol.SetAlign(wxLIST_FORMAT_LEFT);
	liCol.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_DATA);
	m_pListCtrl->InsertColumn(COL_STATUS,liCol);

	liCol.SetText(_("Action"));
	liCol.SetAlign(wxLIST_FORMAT_LEFT);
	liCol.SetMask(wxLIST_MASK_TEXT);
	m_pListCtrl->InsertColumn(COL_ACTION,liCol);

	liCol.SetText(_("Left Panel"));
	liCol.SetAlign(wxLIST_FORMAT_CENTER);
	liCol.SetMask(wxLIST_MASK_TEXT);
	m_pListCtrl->InsertColumn(COL_LEFT,liCol);

	liCol.SetText(_("Center Panel"));
	liCol.SetAlign(wxLIST_FORMAT_CENTER);
	liCol.SetMask(wxLIST_MASK_TEXT);
	m_pListCtrl->InsertColumn(COL_CENTER,liCol);

	liCol.SetText(_("Right Panel"));
	liCol.SetAlign(wxLIST_FORMAT_CENTER);
	liCol.SetMask(wxLIST_MASK_TEXT);
	m_pListCtrl->InsertColumn(COL_RIGHT,liCol);
}

//////////////////////////////////////////////////////////////////

static long _start_insert_item(wxListCtrl * pListCtrl, fim_patch * pPatch, long kItem, long kData)
{
	// kItem is the 0...n item index in the list.
	// kData is our private data (the subscript we use to get back to the patch).

	bool bActive = (pPatch->getPatchOpCurrent() != POP_IGNORE);
	
	long ndx;		// the list's index of where this item is.

	if (bActive)
		ndx = pListCtrl->InsertItem(kItem, _("Apply"),IMG_CHECK);
	else
		ndx = pListCtrl->InsertItem(kItem, _("Omit"), IMG_X);
	
	pListCtrl->SetItemData(ndx,kData);

	return ndx;
}
	
//////////////////////////////////////////////////////////////////

void dlg_auto_merge::_populate_list(void)
{
	wxString strBlanks(_T(' '),20);		// blank padding for unused cells -- keeps the auto-column-width stuff working/honest.

	m_pListCtrl->SetBackgroundColour(*wxWHITE);
	m_pListCtrl->SetTextColour(*wxBLACK);

	long nrItems = m_pPatchSet->getNrPatches();
	wxASSERT_MSG( (nrItems > 0), _T("Coding Error") );
	
	for (long kItem=0; (kItem<nrItems); kItem++)
	{
		// we built the list in reverse order, so let's reverse it here
		// so that we show the user the correct order -- from top to bottom.

		long kIndex = nrItems - kItem - 1;
		fim_patch * pFimPatch = m_pPatchSet->getNthPatch(kIndex);
		de_patch * pDePatch = static_cast<de_patch *>(pFimPatch);

		long ndxInList = _start_insert_item(m_pListCtrl,pDePatch,kItem,kIndex);

		switch (pDePatch->getPatchOpOriginal())
		{
		default:
			wxASSERT_MSG( (0), _T("Coding Error") );
			break;

		case POP_DELETE:
			m_pListCtrl->SetItem(ndxInList,COL_ACTION,_("Delete"));
			m_pListCtrl->SetItem(ndxInList,COL_LEFT,  strBlanks);
			m_pListCtrl->SetItem(ndxInList,COL_CENTER,pDePatch->format_edit_lines(),		IMG_MINUS);
			m_pListCtrl->SetItem(ndxInList,COL_RIGHT, strBlanks);
			break;
			
		case POP_INSERT_L:
			m_pListCtrl->SetItem(ndxInList,COL_ACTION,_("Insert"));
			m_pListCtrl->SetItem(ndxInList,COL_LEFT,  pDePatch->format_src_lines(PANEL_T0), IMG_PLUS);
			m_pListCtrl->SetItem(ndxInList,COL_CENTER,pDePatch->format_edit_lines(),        IMG_AT);
			m_pListCtrl->SetItem(ndxInList,COL_RIGHT, strBlanks);
			break;

		case POP_INSERT_R:
			m_pListCtrl->SetItem(ndxInList,COL_ACTION,_("Insert"));
			m_pListCtrl->SetItem(ndxInList,COL_LEFT,  strBlanks);
			m_pListCtrl->SetItem(ndxInList,COL_CENTER,pDePatch->format_edit_lines(),        IMG_AT);
			m_pListCtrl->SetItem(ndxInList,COL_RIGHT, pDePatch->format_src_lines(PANEL_T2), IMG_PLUS);
			break;

		case POP_REPLACE_L:
			m_pListCtrl->SetItem(ndxInList,COL_ACTION,_("Replace"));
			m_pListCtrl->SetItem(ndxInList,COL_LEFT,  pDePatch->format_src_lines(PANEL_T0), IMG_PLUS);
			m_pListCtrl->SetItem(ndxInList,COL_CENTER,pDePatch->format_edit_lines(),        IMG_MINUS);
			m_pListCtrl->SetItem(ndxInList,COL_RIGHT, strBlanks);
			break;

		case POP_REPLACE_R:
			m_pListCtrl->SetItem(ndxInList,COL_ACTION,_("Replace"));
			m_pListCtrl->SetItem(ndxInList,COL_LEFT,  strBlanks);
			m_pListCtrl->SetItem(ndxInList,COL_CENTER,pDePatch->format_edit_lines(),        IMG_MINUS);
			m_pListCtrl->SetItem(ndxInList,COL_RIGHT, pDePatch->format_src_lines(PANEL_T2), IMG_PLUS);
			break;

		case POP_CONFLICT:
			m_pListCtrl->SetItem(ndxInList,COL_ACTION,_("Conflict"));
			m_pListCtrl->SetItem(ndxInList,COL_LEFT,  pDePatch->format_src_lines(PANEL_T0), IMG_QUESTION);
			m_pListCtrl->SetItem(ndxInList,COL_CENTER,pDePatch->format_edit_lines(),        IMG_QUESTION);
			m_pListCtrl->SetItem(ndxInList,COL_RIGHT, pDePatch->format_src_lines(PANEL_T2), IMG_QUESTION);

			//m_pListCtrl->SetItemBackgroundColour(ndxInList,gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_CONFLICT_BG)); // TODO get this to work.
			m_pListCtrl->SetItemTextColour(ndxInList,gpGlobalProps->getColor(GlobalProps::GPL_FILE_COLOR_CONFLICT_FG));
			break;
		}
	}

	m_pListCtrl->SetColumnWidth(COL_STATUS,60);	//	m_pListCtrl->SetColumnWidth(COL_STATUS,wxLIST_AUTOSIZE);
	m_pListCtrl->SetColumnWidth(COL_ACTION,200);	//	m_pListCtrl->SetColumnWidth(COL_ACTION,wxLIST_AUTOSIZE);
	m_pListCtrl->SetColumnWidth(COL_LEFT,  wxLIST_AUTOSIZE);
	m_pListCtrl->SetColumnWidth(COL_CENTER,wxLIST_AUTOSIZE);
	m_pListCtrl->SetColumnWidth(COL_RIGHT, wxLIST_AUTOSIZE);
}

//////////////////////////////////////////////////////////////////

void dlg_auto_merge::onListItemSelected(wxListEvent & e)
{
	long kItem = e.GetIndex();
	long kData = m_pListCtrl->GetItemData(kItem);

	//wxLogTrace(wxTRACE_Messages,_T("dlg_auto_merge::onListItemSelected: [kItem %ld][kData %ld]"), kItem,kData);

	// stuff a nice sentence into the static text field describing what we're going to do for this patch.

	fim_patch * pFimPatch = m_pPatchSet->getNthPatch(kData);
	de_patch * pDePatch = static_cast<de_patch *>(pFimPatch);

	m_pStaticTextSummary->SetLabel( pDePatch->format_summary_msg() );

	// cause view-file-panel to highlight this patch and make sure it's visible.

	long rowStart = pDePatch->getRowStart();
	m_pViewFile->getDE()->setPatchHighlight(SYNC_EDIT,rowStart,false);
	m_pViewFile->warpScrollCentered(SYNC_EDIT,rowStart);
	if (m_pViewFile->getPanelWithFocus() != -1)
		if (m_pViewFile->getPanel(SYNC_EDIT,(PanelIndex)m_pViewFile->getPanelWithFocus()))
			m_pViewFile->getPanel(SYNC_EDIT,(PanelIndex)m_pViewFile->getPanelWithFocus())->setBogusCaret();
}

//////////////////////////////////////////////////////////////////

void dlg_auto_merge::onListItemRightClick(wxListEvent & e)
{
	// a right-mouse-click on an item.  we ASSUME that the mouse click will also cause a SELECT event.

	long kItem = e.GetIndex();
	long kData = m_pListCtrl->GetItemData(kItem);

	//wxLogTrace(wxTRACE_Messages,_T("dlg_auto_merge::onListItemRightMouse: [kItem %ld][kData %ld]"), kItem,kData);

	fim_patch * pFimPatch = m_pPatchSet->getNthPatch(kData);
	de_patch * pDePatch = static_cast<de_patch *>(pFimPatch);

	m_itemContextMenu = kItem;
	wxMenu menu;

	switch (pDePatch->getPatchOpOriginal())
	{
	default:
		wxASSERT_MSG( (0), _T("Coding Error") );
		return;
		
	case POP_DELETE:
	case POP_INSERT_L:
	case POP_INSERT_R:
	case POP_REPLACE_L:
	case POP_REPLACE_R:
		menu.Append(CTX_APPLY,_("Apply"));
		menu.Append(CTX_OMIT, _("Omit"));
		break;
		
	case POP_CONFLICT:
		if (!pDePatch->hasContentEdit())
		{
			if (pDePatch->hasContentT0())
				menu.Append(CTX_INSERT_L,_("Insert from Left"));
			if (pDePatch->hasContentT2())
				menu.Append(CTX_INSERT_R,_("Insert from Right"));
		}
		else
		{
			if (!pDePatch->hasContentT0() || !pDePatch->hasContentT2())
				menu.Append(CTX_DELETE,_("Delete"));
			if (pDePatch->hasContentT0())
				menu.Append(CTX_REPLACE_L,_("Replace with Left"));
			if (pDePatch->hasContentT2())
				menu.Append(CTX_REPLACE_R,_("Replace with Right"));
		}
		menu.Append(CTX_OMIT, _("Omit"));
		break;
	}
	
	PopupMenu(&menu);
}

//////////////////////////////////////////////////////////////////

void dlg_auto_merge::onCtxMenuEvent_Apply(wxCommandEvent & /*e*/)
{
	long kItem = m_itemContextMenu;
	long kData = m_pListCtrl->GetItemData(kItem);

	fim_patch * pFimPatch = m_pPatchSet->getNthPatch(kData);
	de_patch * pDePatch = static_cast<de_patch *>(pFimPatch);

	wxASSERT_MSG( (pDePatch->getPatchOpOriginal() != POP_CONFLICT), _T("Coding Error") );

	pDePatch->setPatchOpCurrent( pDePatch->getPatchOpOriginal() );
	
	m_pStaticTextSummary->SetLabel( pDePatch->format_summary_msg() );

	m_pListCtrl->SetItem(kItem,COL_STATUS,_("Apply"),IMG_CHECK);

	//m_pListCtrl->SetItemBackgroundColour(kItem,*wxWHITE);		// TODO get this to work
	m_pListCtrl->SetItemTextColour(kItem,*wxBLACK);
}

void dlg_auto_merge::onCtxMenuEvent_Omit(wxCommandEvent & /*e*/)
{
	long kItem = m_itemContextMenu;
	long kData = m_pListCtrl->GetItemData(kItem);

	fim_patch * pFimPatch = m_pPatchSet->getNthPatch(kData);
	de_patch * pDePatch = static_cast<de_patch *>(pFimPatch);

	pDePatch->setPatchOpCurrent(POP_IGNORE);

	m_pListCtrl->SetItem(kItem,COL_STATUS,_("Omit"),IMG_X);

	wxString strAction;
	switch (pDePatch->getPatchOpOriginal())
	{
	default:			wxASSERT_MSG( (0), _T("Coding Error") );	break;
	case POP_DELETE:	strAction = _("Delete");					break;
	case POP_INSERT_L:
	case POP_INSERT_R:	strAction = _("Insert");					break;
	case POP_REPLACE_L:
	case POP_REPLACE_R: strAction = _("Replace");					break;
	case POP_CONFLICT:	strAction = _("Conflict");					break;
	}
	m_pListCtrl->SetItem(kItem,COL_ACTION,strAction);

	//m_pListCtrl->SetItemBackgroundColour(kItem,*wxWHITE);	// TODO get this to work
	m_pListCtrl->SetItemTextColour(kItem,*wxLIGHT_GREY);
}

void dlg_auto_merge::onCtxMenuEvent_Command(wxCommandEvent & e)
{
	// will only be used for menu commands on conflicts (other than apply & omit)

	int id = e.GetId() - wxID_HIGHEST - 200;

	long kItem = m_itemContextMenu;
	long kData = m_pListCtrl->GetItemData(kItem);

	fim_patch * pFimPatch = m_pPatchSet->getNthPatch(kData);
	de_patch * pDePatch = static_cast<de_patch *>(pFimPatch);

	pDePatch->setPatchOpCurrent( (fim_patch_op)id );
	
	m_pStaticTextSummary->SetLabel( pDePatch->format_summary_msg() );

	m_pListCtrl->SetItem(kItem,COL_STATUS,_("Apply"),IMG_CHECK);

	wxString strAction;
	switch (pDePatch->getPatchOpCurrent())
	{
	default:			wxASSERT_MSG( (0), _T("Coding Error") );		break;
	case POP_DELETE:	strAction = _("Conflict/Delete");				break;
	case POP_INSERT_L:	strAction = _("Conflict/Insert from Left");		break;
	case POP_INSERT_R:	strAction = _("Conflict/Insert from Right");	break;
	case POP_REPLACE_L: strAction = _("Conflict/Replace from Left");	break;
	case POP_REPLACE_R: strAction = _("Conflict/Replace from Right");	break;
	}
	m_pListCtrl->SetItem(kItem,COL_ACTION,strAction);

	//m_pListCtrl->SetItemBackgroundColour(kItem,*wxWHITE);		// TODO get this to work
	m_pListCtrl->SetItemTextColour(kItem,*wxBLACK);
}

//////////////////////////////////////////////////////////////////

void dlg_auto_merge::onButton_ShowHelp(wxCommandEvent & /*e*/)
{
	dlg_auto_merge_help dlg(this);
	dlg.ShowModal();
}

//////////////////////////////////////////////////////////////////

dlg_auto_merge_help::dlg_auto_merge_help(wxWindow * pParent)
{
#if defined(__WXMAC__)
	SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif

	Create(pParent,-1,DLG_TITLE,wxDefaultPosition,wxDefaultSize,
		   wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX);

	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		wxStaticText * pHelp = new wxStaticText(this,-1,
												wxGetTranslation( L"The Auto Merge Dialog lists the set of actions that the Auto-Merge Feature\n"
																  L"will perform.\n"
																  L"\n"
																  L"Auto-Merge evaluates each change/conflict in the documents and has selected\n"
																  L"an appropriate action based upon each context.  For each change the\n"
																  L"action matches the default action in the center panel's context menu.\n"
																  L"\n"
																  L"Each list item gives the document line number(s) involved in the change\n"
																  L"and describes that action that will be taken.  You may click on each\n"
																  L"list item and see the change highlighted in the main window.\n"
																  L"\n"
																  L"If you DO NOT want one of the listed actions performed, right click on\n"
																  L"it and mark it omitted.\n"
																  L"\n"
																  L"WARNING: If you have already edited the center panel, please double-check\n"
																  L"WARNING: the list for items near your edits and verify that you really\n"
																  L"WARNING: want the actions performed."
													));
//		pHelp->SetFont( *wxITALIC_FONT );
		vSizerTop->Add( pHelp, FIX, wxGROW|wxALL, M);

		//////////////////////////////////////////////////////////////////

		// put horizontal line and set of ok/cancel buttons across the bottom of the dialog

//		vSizerTop->AddStretchSpacer(VAR);
		vSizerTop->Add( new wxStaticLine(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxLI_HORIZONTAL), FIX, wxGROW|wxLEFT|wxRIGHT, M);
		wxBoxSizer * hSizerButtons = new wxBoxSizer(wxHORIZONTAL);
		{
			hSizerButtons->AddStretchSpacer(VAR);
			hSizerButtons->Add( new wxButton(this,wxID_CANCEL,_("&Close")), FIX, wxALL, M);	// using wxID_CANCEL lets ESC key close dialog
		}
		vSizerTop->Add(hSizerButtons,FIX,wxGROW|wxALL,M);

	}
	SetSizer(vSizerTop);
	vSizerTop->SetSizeHints(this);
	vSizerTop->Fit(this);

	// center dialog on parent window rather than arbitrary location
	// (screen-centered on Win32 & GTK, upper-left corner of window in MAC).
	// WXBUG: this doesn't appear to work on WIN32 -- oh well.

	Centre(wxBOTH);
}
#endif
