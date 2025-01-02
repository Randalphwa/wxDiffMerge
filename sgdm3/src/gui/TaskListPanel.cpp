// TaskListPanel.cpp -- show task list below main windows
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
#if 0	// we are not currently using the task-list
//////////////////////////////////////////////////////////////////

#define POP2CTX(pop)		(wxID_HIGHEST + 100 + (pop))
#define CTX2POP(ctx)		((ctx) - wxID_HIGHEST - 100)
#define CTX__FIRST__		(POP2CTX(POP__FIRST__))
#define CTX__LAST__			(POP2CTX(POP__LAST__))

//////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(TaskListPanel_AutoMerge,wxPanel)
	EVT_BUTTON(ID_BTN_CANCEL,					TaskListPanel_AutoMerge::onButton_Cancel)
	EVT_BUTTON(ID_BTN_APPLY,					TaskListPanel_AutoMerge::onButton_Apply)
	EVT_LIST_ITEM_SELECTED(ID_LIST_CTRL,		TaskListPanel_AutoMerge::OnListItem_Selected)
	EVT_LIST_ITEM_RIGHT_CLICK(ID_LIST_CTRL,		TaskListPanel_AutoMerge::OnListItem_RightClick)
	EVT_MENU_RANGE(CTX__FIRST__,CTX__LAST__,	TaskListPanel_AutoMerge::OnMenuItem_Command)
END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////

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

static void s_cb_de_changed(void * pThis, const util_cbl_arg & arg)	{ ((TaskListPanel_AutoMerge *)pThis)->cb_de_changed(arg); }

//////////////////////////////////////////////////////////////////

TaskListPanel::TaskListPanel(wxWindow * pParent, ViewFile * pViewFile)
	: wxPanel(pParent,wxID_ANY,wxDefaultPosition,wxDefaultSize,MY_PANEL_STYLE),
	  m_pViewFile(pViewFile), m_pDeDe(NULL)
{
	wxLogTrace(wxTRACE_Messages,_T("TaskListPanel:ctor"));

	bind_dede(m_pViewFile->getDE());
}

TaskListPanel::~TaskListPanel(void)
{
	wxLogTrace(wxTRACE_Messages,_T("TaskListPanel:dtor"));

	bind_dede(NULL);
}

//////////////////////////////////////////////////////////////////

TaskListPanel_AutoMerge::TaskListPanel_AutoMerge(wxWindow * pParent, ViewFile * pViewFile,
												 fim_patchset * pPatchSet, int nrConflicts)
	: TaskListPanel(pParent,pViewFile),
	  m_pPatchSet(pPatchSet),
	  m_nrConflicts(nrConflicts),
	  m_bPatchSetIsStale(false)
{
	wxLogTrace(wxTRACE_Messages,_T("TaskListPanel_AutoMerge:ctor"));

	createLayout();
}

TaskListPanel_AutoMerge::~TaskListPanel_AutoMerge(void)
{
	if (m_pPatchSet)
		delete m_pPatchSet;

	wxLogTrace(wxTRACE_Messages,_T("TaskListPanel_AutoMerge:dtor"));
}

void TaskListPanel::bind_dede(de_de * pDeDe)
{
	if (m_pDeDe)	m_pDeDe->delChangeCB(s_cb_de_changed,this);

	m_pDeDe = pDeDe;
	
	if (m_pDeDe)	m_pDeDe->addChangeCB(s_cb_de_changed,this);
}

//////////////////////////////////////////////////////////////////

fim_patchset * TaskListPanel_AutoMerge::removePatchSet(void)
{
	fim_patchset * p = m_pPatchSet;

	m_pPatchSet = NULL;

	return p;
}

//////////////////////////////////////////////////////////////////

#define M 7
#define FIX 0
#define VAR 1

void TaskListPanel_AutoMerge::createLayout(void)
{
	wxBoxSizer * vSizerTop = new wxBoxSizer(wxVERTICAL);
	{
		wxNotebook * pNotebook = new wxNotebook(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxNB_BOTTOM);
		{
			m_pPanelBody = new wxPanel(pNotebook,wxID_ANY,wxDefaultPosition,wxDefaultSize);
			{
				wxBoxSizer * nbpSizer = new wxBoxSizer(wxHORIZONTAL);
				{
					m_pSizerListCtrl = new wxBoxSizer(wxVERTICAL);
					{
						// if the error panel is needed, it gets inserted here.

						m_pListCtrl = new wxListCtrl(m_pPanelBody,ID_LIST_CTRL,wxDefaultPosition,wxDefaultSize,
													 wxLC_REPORT|wxLC_SINGLE_SEL|MY_BORDER_STYLE|MY_RULES_STYLE);
						_define_columns();
						_populate_list();
						m_pSizerListCtrl->Add( m_pListCtrl, VAR, wxGROW, 0);
					}
					nbpSizer->Add( m_pSizerListCtrl, VAR, wxGROW|wxALL, M);

					wxBoxSizer * vSizerButtons = new wxBoxSizer(wxVERTICAL);
					{
						m_pButtonApply = new wxButton(m_pPanelBody,ID_BTN_APPLY,_("Perform Auto-Merge"));
						vSizerButtons->Add( m_pButtonApply, FIX, wxGROW, 0);
						vSizerButtons->AddSpacer(M);

						m_pButtonCancel = new wxButton(m_pPanelBody,ID_BTN_CANCEL,_("Cancel Auto-Merge"));
						vSizerButtons->Add( m_pButtonCancel, FIX, wxGROW, 0);
						vSizerButtons->AddSpacer(M);
						vSizerButtons->AddStretchSpacer(VAR);
					}
					nbpSizer->Add( vSizerButtons,FIX,wxGROW|wxALL,M);
				}
				m_pPanelBody->SetSizer(nbpSizer);
				//nbpSizer->SetSizeHints(m_pPanelBody);
				//nbpSizer->Fit(m_pPanelBody);
			}
			pNotebook->AddPage(m_pPanelBody,_("Auto-Merge Task List"));
		}
		vSizerTop->Add(pNotebook,VAR,wxGROW,0);
	}

	SetSizer(vSizerTop);
	vSizerTop->SetSizeHints(this);
	vSizerTop->Fit(this);

	_enable_apply();
}

//////////////////////////////////////////////////////////////////

#define COL_STATUS				0
#define COL_LINES				1
#define COL_DESCRIPTION			2

void TaskListPanel_AutoMerge::_define_columns(void)
{
	wxListItem liCol;

	liCol.SetText(_("Status"));
	liCol.SetAlign(wxLIST_FORMAT_LEFT);
	liCol.SetMask(wxLIST_MASK_TEXT|wxLIST_MASK_DATA);
	m_pListCtrl->InsertColumn(COL_STATUS,liCol);

	liCol.SetText(_("Lines"));
	liCol.SetAlign(wxLIST_FORMAT_LEFT);
	liCol.SetMask(wxLIST_MASK_TEXT);
	m_pListCtrl->InsertColumn(COL_LINES,liCol);

	liCol.SetText(_("Description"));
	liCol.SetAlign(wxLIST_FORMAT_LEFT);
	liCol.SetMask(wxLIST_MASK_TEXT);
	m_pListCtrl->InsertColumn(COL_DESCRIPTION,liCol);

	m_pListCtrl->SetBackgroundColour(*wxWHITE);
	m_pListCtrl->SetTextColour(*wxBLACK);
}

void TaskListPanel_AutoMerge::_populate_list(void)
{
	long ndxInList;
	long nrItems = m_pPatchSet->getNrPatches();
	wxASSERT_MSG( (nrItems > 0), _T("Coding Error") );
	
	for (long kItem=0; (kItem<nrItems); kItem++)
	{
		// we built the list in reverse order, so let's reverse it here
		// so that we show the user the correct order -- from top to bottom.

		long kIndexPatch = nrItems - kItem - 1;
		fim_patch * pFimPatch = m_pPatchSet->getNthPatch(kIndexPatch);
		de_patch * pDePatch = static_cast<de_patch *>(pFimPatch);

		switch (pDePatch->getPatchOpOriginal())
		{
		default:
		case POP_IGNORE:
		case POP_DELETE:
		case POP_INSERT_L:
		case POP_INSERT_R:
		case POP_REPLACE_L:
		case POP_REPLACE_R:
			break;			// for now, we only put conflicts in the listctrl.

		case POP_CONFLICT:
			ndxInList = m_pListCtrl->InsertItem(kItem,_("Conflict -- Decision Needed"));
			m_pListCtrl->SetItemData(ndxInList,kIndexPatch);

			_set_listctrl_fields(ndxInList,pDePatch,false);
			break;
		}
	}

	m_pListCtrl->SetColumnWidth(COL_STATUS,200);
	m_pListCtrl->SetColumnWidth(COL_LINES,wxLIST_AUTOSIZE);
	m_pListCtrl->SetColumnWidth(COL_DESCRIPTION,wxLIST_AUTOSIZE);
}

void TaskListPanel_AutoMerge::_set_listctrl_fields(long ndxInList, de_patch * pDePatch, bool bSetStatusColumn)
{
	if (bSetStatusColumn)
	{
		wxString strAction;
		switch (pDePatch->getPatchOpCurrent())
		{
		default:			wxASSERT_MSG( (0), _T("Coding Error") );			break;
		case POP_IGNORE:	strAction = _("Conflict/Ignored by Auto-Merge");	break;
		case POP_DELETE:	strAction = _("Conflict/Delete from Center");		break;
		case POP_INSERT_L:	strAction = _("Conflict/Insert from Left");			break;
		case POP_INSERT_R:	strAction = _("Conflict/Insert from Right");		break;
		case POP_REPLACE_L: strAction = _("Conflict/Replace from Left");		break;
		case POP_REPLACE_R: strAction = _("Conflict/Replace from Right");		break;
		case POP_CONFLICT:	strAction = _("Conflict -- Decision Needed");		break;
		}
		m_pListCtrl->SetItem(ndxInList,COL_STATUS,strAction);
	}

	m_pListCtrl->SetItem(ndxInList,COL_LINES,
						 wxString::Format(_T("%s / %s / %s"),
										  pDePatch->format_src_lines(PANEL_T0),
										  pDePatch->format_edit_lines(),
										  pDePatch->format_src_lines(PANEL_T2)));
	m_pListCtrl->SetItem(ndxInList,COL_DESCRIPTION,
						 pDePatch->format_summary_msg());

	switch (pDePatch->getPatchOpCurrent())
	{
	default:
	case POP_DELETE:
	case POP_INSERT_L:
	case POP_INSERT_R:
	case POP_REPLACE_L:
	case POP_REPLACE_R:
		m_pListCtrl->SetItemTextColour(ndxInList,*wxBLACK);
		break;

	case POP_IGNORE:
		m_pListCtrl->SetItemTextColour(ndxInList,*wxLIGHT_GREY);
		break;

	case POP_CONFLICT:	
		m_pListCtrl->SetItemTextColour(ndxInList,*wxRED);
		break;
	}
}

void TaskListPanel_AutoMerge::_enable_apply(void)
{
	// don't allow APPLY button until they have decided what to with each conflict.
	// don't allow APPLY button if the documents have changed behind our backs.

	bool bEnable = (   (m_nrConflicts == 0)
					&& (!m_bPatchSetIsStale));
	
	m_pButtonApply->Enable(bEnable);
}

//////////////////////////////////////////////////////////////////

void TaskListPanel_AutoMerge::onButton_Cancel(wxCommandEvent & /*e*/)
{
	wxLogTrace(wxTRACE_Messages,_T("TLP_AM::onButton_Cancel"));

	// discard the patchset and close the task-list.

	m_pViewFile->hideTaskList();	// this will cause 'this' to be deleted
}

void TaskListPanel_AutoMerge::onButton_Apply(wxCommandEvent & /*e*/)
{
	wxLogTrace(wxTRACE_Messages,_T("TLP_AM::onButton_Apply"));

	// apply the patches in the patchset and close the task-list.

	if (m_pDeDe)
		m_pDeDe->unsetPatchHighlight(SYNC_EDIT);

	// make the task list window disappear from the frame before
	// we apply the patches.  this prevents any user confusion or
	// race conditions -- where the patches we apply cause a callback
	// on our cb_de_changed() (which would cause us to complain).
	//
	// WARNING: but when we hide the task list, we will be deleted,
	// WARNING: so we need to cache whatever variables we need (and
	// WARNING: remove the patchset from our class so that our dtor
	// WARNING: won't delete it).

	fim_patchset * pPatchSet = removePatchSet();
	ViewFile * pViewFile = m_pViewFile;

	pViewFile->hideTaskList();	// this will cause 'this' to be deleted

	pViewFile->completeAutoMerge(pPatchSet);

	delete pPatchSet;
}

//////////////////////////////////////////////////////////////////

void TaskListPanel_AutoMerge::OnListItem_Selected(wxListEvent & e)
{
	if (m_bPatchSetIsStale)
	{
		::wxBell();
		return;
	}
	
	long ndxInList = e.GetIndex();
	long ndxPatch  = m_pListCtrl->GetItemData(ndxInList);

	wxLogTrace(wxTRACE_Messages,_T("TLP_AM::OnListItem_Selected: [ndxList %ld][ndxPatch %ld]"),ndxInList,ndxPatch);

	fim_patch * pFimPatch = m_pPatchSet->getNthPatch(ndxPatch);
	de_patch * pDePatch = static_cast<de_patch *>(pFimPatch);

	// cause view-file-panel to highlight this patch and make sure it's visible.

	// TODO force top panel's notebook to show SYNC_EDIT...

	long rowStart = pDePatch->getRowStart();
	if (m_pDeDe)
		m_pDeDe->setPatchHighlight(SYNC_EDIT,rowStart,false);
	m_pViewFile->warpScrollCentered(SYNC_EDIT,rowStart);
	if (m_pViewFile->getPanelWithFocus() != -1)
		if (m_pViewFile->getPanel(SYNC_EDIT,(PanelIndex)m_pViewFile->getPanelWithFocus()))
			m_pViewFile->getPanel(SYNC_EDIT,(PanelIndex)m_pViewFile->getPanelWithFocus())->setBogusCaret();
}

void TaskListPanel_AutoMerge::OnListItem_RightClick(wxListEvent & e)
{
	if (m_bPatchSetIsStale)
	{
		::wxBell();
		return;
	}

	// a right-mouse-click on an item.  we ASSUME that the mouse click
	// will also have caused a SELECT event (prior to this event).

	long ndxInList = e.GetIndex();
	long ndxPatch  = m_pListCtrl->GetItemData(ndxInList);

	wxLogTrace(wxTRACE_Messages,_T("TLP_AM::OnListItem_RightClick: [ndxList %ld][ndxPatch %ld]"),ndxInList,ndxPatch);

	fim_patch * pFimPatch = m_pPatchSet->getNthPatch(ndxPatch);
	de_patch * pDePatch = static_cast<de_patch *>(pFimPatch);

	// raise context menu and let them decide what to do with this patch

	wxMenu menu;

	switch (pDePatch->getPatchOpOriginal())
	{
	default:
	//case POP_IGNORE:
	//case POP_DELETE:
	//case POP_INSERT_L:
	//case POP_INSERT_R:
	//case POP_REPLACE_L:
	//case POP_REPLACE_R:
		wxASSERT_MSG( (0), _T("Coding Error") );
		return;

	case POP_CONFLICT:
		if (!pDePatch->hasContentEdit())
		{
			if (pDePatch->hasContentT0())
				menu.Append(POP2CTX(POP_INSERT_L),_("Insert into Center from Left"));
			if (pDePatch->hasContentT2())
				menu.Append(POP2CTX(POP_INSERT_R),_("Insert into Center from Right"));
		}
		else
		{
			if (!pDePatch->hasContentT0() || !pDePatch->hasContentT2())
				menu.Append(POP2CTX(POP_DELETE),_("Delete from Center"));
			if (pDePatch->hasContentT0())
				menu.Append(POP2CTX(POP_REPLACE_L),_("Replace Center with Left"));
			if (pDePatch->hasContentT2())
				menu.Append(POP2CTX(POP_REPLACE_R),_("Replace Center with Right"));
		}
		menu.AppendSeparator();
		menu.Append(POP2CTX(POP_IGNORE), _("Have Auto-Merge Ignore this Conflict"));
		menu.AppendSeparator();
		menu.Append(POP2CTX(POP_CONFLICT), _("Defer Decision"));
		break;
	}

	m_ndxInList_for_Popup = ndxInList;
	PopupMenu(&menu);
}

//////////////////////////////////////////////////////////////////

void TaskListPanel_AutoMerge::OnMenuItem_Command(wxCommandEvent & e)
{
	if (m_bPatchSetIsStale)
	{
		::wxBell();
		return;
	}

	fim_patch_op opNew = (fim_patch_op)(CTX2POP(e.GetId()));

	long ndxInList = m_ndxInList_for_Popup;
	long ndxPatch  = m_pListCtrl->GetItemData(ndxInList);

	wxLogTrace(wxTRACE_Messages,_T("TLP_AM::OnMenuItem_Command: [ndxList %ld][ndxPatch %ld]"),ndxInList,ndxPatch);

	fim_patch * pFimPatch = m_pPatchSet->getNthPatch(ndxPatch);
	de_patch * pDePatch = static_cast<de_patch *>(pFimPatch);

	// update m_nrConflicts based upon the old and new current-op.

	fim_patch_op opOld = pDePatch->getPatchOpCurrent();
	if ((opOld == POP_CONFLICT) && (opNew != POP_CONFLICT))
		m_nrConflicts--;
	else if ((opOld != POP_CONFLICT) && (opNew == POP_CONFLICT))
		m_nrConflicts++;

	// set op-current in the patch to the value of the menu item they chose.

	pDePatch->setPatchOpCurrent(opNew);

	// update listctrl with new status for this item

	_set_listctrl_fields(ndxInList,pDePatch,true);

	_enable_apply();
}
	
//////////////////////////////////////////////////////////////////	

void TaskListPanel_AutoMerge::cb_de_changed(const util_cbl_arg & arg)
{
	// TODO do we also need to trap cb_fl_changed??

	wxLogTrace(wxTRACE_Messages,_T("TLP_AM:cb_de_changed: [0x%lx]"),arg.m_l);

	long chg = arg.m_l;

	// if something in the documents changed or a setting (such as
	// the current ruleset) changed, then we need to abort the
	// auto-merge.
	//
	// we don't check for DE_CHG_EDIT_DISP because that is fired
	// everytime we call setPatchHighlight().

	if ((chg & (DE_CHG_EDIT_CHG|DE_CHG_EDIT_RUN)) != 0)
		_set_stale();
}

void TaskListPanel_AutoMerge::_set_stale(void)
{
	// TODO this is somewhat of a hack -- a we give up -- when the document,
	// TODO ruleset, or global-pref changes.  it'd be nice to give them a "recompute"
	// TODO button that would recompute the auto-merge list and compare it
	// TODO with the current one and update the current-op on the ones that
	// TODO they had updated (when the lists are comparable) and allow them
	// TODO continue.   but this has a few nasty cases (such as no conflicts
	// TODO the second time) and we don't know the nature of the change, so
	// TODO we don't know if it even makes sense to correlate them.
	// TODO
	// TODO besides, if they have a pending auto-merge, they shouldn't be
	// TODO making a bunch of changes.  so they shouldn't get here that often.
	// TODO so let's just disable the "perform" button, give them a nice
	// TODO warning and let them cancel the auto-merge -- they can always
	// TODO re-start it in a moment (and hopefully after they're finished
	// TODO tinkering around).

	if (m_bPatchSetIsStale)
		return;

	m_bPatchSetIsStale = true;

	// we don't disable the listctrl because it doesn't draw right on Win32.
	// and besides, they may want to scroll thru it and remember what their
	// choices were before we go away.

	m_pListCtrl->SetBackgroundColour(wxColour(0xe0,0xe0,0xe0));
	m_pListCtrl->SetTextColour(wxColour(0x70,0x70,0x70));
	long nrItems = m_pListCtrl->GetItemCount();
	for (long kItem=0; kItem<nrItems; kItem++)
		m_pListCtrl->SetItemTextColour(kItem,wxColour(0x70,0x70,0x70));
	m_pListCtrl->Refresh();

	_enable_apply();

	// we don't automatically close the task-list because the user may want
	// to see what their choices were (even though we're going to discard
	// them shortly).
	//
	// WARNING: we can't raise a modal dialog (message box) here because we
	// may not be the active window.  the user may have 2 top-level windows
	// open with one or more files common between the 2 windows and be editing
	// in the other window --OR-- may have changed a RuleSet or global-pref
	// in the other window that affected this window.  so we just add a static
	// text message to the body of this panel.
	//
	// this error panel gets added above the listctrl
	//

	m_pPanelError = new wxPanel(m_pPanelBody,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxSIMPLE_BORDER);
	{
		wxBoxSizer * vSizer = new wxBoxSizer(wxVERTICAL);
		{
			wxString strError = wxGetTranslation(L"The pending Auto-Merge cannot be completed.  Changes to one of the\n"
												 L"referenced documents or changes to the configuration options require\n"
												 L"that Auto-Merge be cancelled and restarted."
				);
			
			wxStaticText * pStaticText = new wxStaticText(m_pPanelError,wxID_ANY,
														  strError,
														  wxDefaultPosition,wxDefaultSize,
														  wxALIGN_CENTER);
			wxFont font(pStaticText->GetFont());
			font.SetWeight(wxFONTWEIGHT_BOLD);
			pStaticText->SetFont(font);
			pStaticText->SetBackgroundColour(wxColour(0xff,0xff,0xd8));
			pStaticText->SetForegroundColour(*wxRED);
			vSizer->Add( pStaticText, FIX, wxGROW, 0);
		}
		m_pPanelError->SetSizer(vSizer);
	}

	m_pSizerListCtrl->Prepend(m_pPanelError, FIX, wxGROW|wxBOTTOM, M);
	m_pSizerListCtrl->Layout();

	Refresh();
}
#endif//0
