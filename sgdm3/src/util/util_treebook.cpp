// util_treebook.cpp
// a control something like a wxNotebook or wxListbook, but using
// a tree control.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>

	// TODO we can remove this #ifdef once we get the build system updated to 2.8.
#if wxCHECK_VERSION(2,8,0)
#else
//////////////////////////////////////////////////////////////////

#define MARGIN 7

//////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC_CLASS(util_treebook, wxControl)
IMPLEMENT_DYNAMIC_CLASS(util_treebook_event, wxNotifyEvent)

const wxEventType UTIL_COMMAND_TREEBOOK_PAGE_CHANGED = wxNewEventType();
const int ID_treebook_treectrl = wxNewId();

BEGIN_EVENT_TABLE(util_treebook, wxBookCtrlBase)
	EVT_SIZE(util_treebook::OnEventSize)
	EVT_TREE_SEL_CHANGED(ID_treebook_treectrl, util_treebook::OnEventTreeSelectionChanged)
	EVT_NAVIGATION_KEY(util_treebook::OnNavigationKey)
	EVT_TREE_KEY_DOWN(ID_treebook_treectrl, util_treebook::OnEventTreeKeyDown)
	EVT_SET_FOCUS(util_treebook::OnSetFocus)
END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////

#define IS_VALID_PAGE(nPage)		((nPage) < GetPageCount())

//////////////////////////////////////////////////////////////////

void util_treebook::Init(void)
{
	m_pTreeCtrl = NULL;
	m_selection = wxNOT_FOUND;
}

//////////////////////////////////////////////////////////////////

bool util_treebook::Create(wxWindow * pParent, wxWindowID id,
						   const wxPoint & pos, const wxSize & size,
						   long style,
						   const wxString & name)
{
#ifdef wxBK_LEFT
	// in wxWidgets 2.8.0, they added wxBK_ flags and wxBookCtrlBase
	// complains/asserts if you don't use one of them.
	style |= wxBK_LEFT;
#endif

	// on MSW, we need to set no-border because the default is a stupid looking
	// sunken border.
	//
	// on MSW, we need to force tab-traversal to get from-self navigation events.
	// (it has no effect on GTK.)

	if (!wxControl::Create(pParent,id,pos,size,style|wxBORDER_NONE|wxTAB_TRAVERSAL,wxDefaultValidator,name))
		return false;

	m_pTreeCtrl = new wxTreeCtrl(this,ID_treebook_treectrl,wxDefaultPosition,wxSize(200,-1),
								 wxTAB_TRAVERSAL|wxTR_NO_BUTTONS|wxTR_LINES_AT_ROOT|wxTR_HIDE_ROOT|wxTR_SINGLE);
	m_pTreeCtrl->AddRoot(_T("Root"));
	
	return true;
}

//////////////////////////////////////////////////////////////////

int util_treebook::SetSelection(size_t n)
{
	if (!IS_VALID_PAGE(n))
		return wxNOT_FOUND;

	int nOldSelection = m_selection;
	int nNewSelection = (int)n;

	if (nNewSelection != nOldSelection)
	{
		if (nOldSelection != wxNOT_FOUND)
			m_pages[nOldSelection]->Hide();

		m_selection = nNewSelection;

		if (nNewSelection != wxNOT_FOUND)
		{
			wxWindow * pPage = m_pages[nNewSelection];
			pPage->SetSize(GetPageRect());
			pPage->Show();

			m_pTreeCtrl->SelectItem(m_vecTreeItemIds[nNewSelection],true);
			m_pTreeCtrl->EnsureVisible(m_vecTreeItemIds[nNewSelection]);
		}

		// send PAGE-CHANGED event to container of the treebook.

		util_treebook_event event(UTIL_COMMAND_TREEBOOK_PAGE_CHANGED, m_windowId, nNewSelection, nOldSelection);
		event.SetEventObject(this);
		(void)GetEventHandler()->ProcessEvent(event);
	}

	return nOldSelection;
}

int util_treebook::ChangeSelection(size_t n)
{
	if (!IS_VALID_PAGE(n))
		return wxNOT_FOUND;

	int nOldSelection = m_selection;
	int nNewSelection = (int)n;

	if (nNewSelection != nOldSelection)
	{
		if (nOldSelection != wxNOT_FOUND)
			m_pages[nOldSelection]->Hide();

		m_selection = nNewSelection;

		if (nNewSelection != wxNOT_FOUND)
		{
			wxWindow * pPage = m_pages[nNewSelection];
			pPage->SetSize(GetPageRect());
			pPage->Show();

			m_pTreeCtrl->SelectItem(m_vecTreeItemIds[nNewSelection],true);
			m_pTreeCtrl->EnsureVisible(m_vecTreeItemIds[nNewSelection]);
		}
	}

	return nOldSelection;
}

bool util_treebook::SetPageText(size_t n, const wxString & strText)
{
	m_pTreeCtrl->SetItemText(m_vecTreeItemIds[n],strText);
	return true;
}

wxString util_treebook::GetPageText(size_t n) const
{
	return m_pTreeCtrl->GetItemText(m_vecTreeItemIds[n]);
}

int util_treebook::GetPageImage(size_t /*n*/) const
{
	// NOT IMPLEMENTED
	return wxNOT_FOUND;
}

bool util_treebook::SetPageImage(size_t /*n*/, int /*imageId*/)
{
	// NOT IMPLEMENTED
	return false;
}

//////////////////////////////////////////////////////////////////

wxSize util_treebook::GetTreeSize(void) const
{
	// compute size of the tree-control within this tree-book.
	// we assume left alignment.

    const wxSize sizeClient = GetClientSize();
	const wxSize sizeTree = m_pTreeCtrl->GetSize();

    wxSize size;
	size.x = sizeTree.x;
	size.y = sizeClient.y;

    return size;
}

wxSize util_treebook::CalcSizeFromPage(const wxSize& sizePage) const
{
	// calculate the size the entire tree-book (containing the tree-control
	// on the left and the book-pages on the right).

	wxSize sizeTotal = sizePage;
	wxSize sizeTreeCtrl = GetTreeSize();

	sizeTotal.x += sizeTreeCtrl.x + MARGIN;

	return sizeTotal;
}

//////////////////////////////////////////////////////////////////

bool util_treebook::InsertPage(size_t n, wxWindow * pPage, const wxString & strText, bool bSelect, int imageId)
{
	if (!wxBookCtrlBase::InsertPage(n,pPage,strText,bSelect,imageId))
		return false;

	wxTreeItemId idNew = m_pTreeCtrl->InsertItem(m_pTreeCtrl->GetRootItem(),n,strText,imageId);
	m_vecTreeItemIds.insert(m_vecTreeItemIds.begin()+n,idNew);

	if ((int)n <= m_selection)		// if insert before the currently selected page, update
	{								// the index of the selected page.
		m_selection++;
		m_pTreeCtrl->SelectItem(m_vecTreeItemIds[m_selection],true);
		m_pTreeCtrl->EnsureVisible(m_vecTreeItemIds[m_selection]);
	}

	// this logic comes from wxWidgets-2.6.3/src/generic/listbkg.cpp

	// ensure that something is selected

	int nNewSelection = wxNOT_FOUND;
	if (bSelect)
		nNewSelection = (int)n;
	else if (m_selection == wxNOT_FOUND)
		nNewSelection = 0;

	if (nNewSelection != m_selection)
		pPage->Hide();

	if (nNewSelection != wxNOT_FOUND)
		SetSelection(nNewSelection);
		
    InvalidateBestSize();
	return true;
}

wxWindow * util_treebook::DoRemovePage(size_t kPage)
{
	const size_t nPages = GetPageCount();

	wxWindow * pWin = wxBookCtrlBase::DoRemovePage(kPage);

	if (pWin)
	{
		m_pTreeCtrl->Delete(m_vecTreeItemIds[kPage]);
		m_vecTreeItemIds.erase(m_vecTreeItemIds.begin()+kPage);

		// this logic comes from wxWidgets-2.6.3/src/generic/listbkg.cpp

		if (m_selection >= (int)kPage)		// currently selected page is after this item, update
		{									// the index of the selected page.
			int nNewSelection = m_selection - 1;
			if (nPages == 1)
				nNewSelection = wxNOT_FOUND;
			else if ((nPages == 2) || (nNewSelection == -1))
				nNewSelection = 0;

			// force selection invalid if deleting the current page
			m_selection = ((m_selection == (int)kPage) ? wxNOT_FOUND : m_selection - 1);

			if ((nNewSelection != wxNOT_FOUND) && (nNewSelection != m_selection))
				SetSelection(nNewSelection);
		}
	}

	return pWin;
}

//////////////////////////////////////////////////////////////////

bool util_treebook::DeleteAllPages(void)
{
	m_pTreeCtrl->DeleteAllItems();
	m_vecTreeItemIds.clear();
	return wxBookCtrlBase::DeleteAllPages();
}

//////////////////////////////////////////////////////////////////

wxRect util_treebook::GetPageRect(void) const
{
	const wxSize sizeTree = m_pTreeCtrl->GetSize();

	wxPoint pt;
	wxRect rectPage(pt,GetClientSize());

	rectPage.x = sizeTree.x + MARGIN;
	rectPage.width -= sizeTree.x + MARGIN;

	return rectPage;
}

//////////////////////////////////////////////////////////////////

void util_treebook::OnEventSize(wxSizeEvent & e)
{
	e.Skip();

	if (!m_pTreeCtrl)		// if not fully created
		return;

    const wxSize sizeClient = GetClientSize();
	const wxSize sizeBorder = m_pTreeCtrl->GetSize() - m_pTreeCtrl->GetClientSize();
	wxSize sizeTree = GetTreeSize();
	wxSize sizeTreeInterior = sizeTree - sizeBorder;

	m_pTreeCtrl->SetClientSize(sizeTreeInterior);

	if (m_selection != wxNOT_FOUND)
	{
		wxWindow * pPage = m_pages[m_selection];
		if (pPage)
			pPage->SetSize(GetPageRect());
	}
}

void util_treebook::OnEventTreeSelectionChanged(wxTreeEvent & e)
{
	// tree control gives us an ID -- not an index

	wxTreeItemId idNew = e.GetItem();

	// map ID into index using the vector

	int nNewSelection = wxNOT_FOUND;

	int k = 0;
	TVecTreeItemIdsIterator it = m_vecTreeItemIds.begin();
	while (it != m_vecTreeItemIds.end())
	{
		if (*it == idNew)
		{
			nNewSelection = k;
			break;
		}
		
		k++;
		it++;
	}

	wxASSERT_MSG( (nNewSelection != wxNOT_FOUND), _T("Coding Error") );

	if (nNewSelection == m_selection)		// bogus or duplicate event 
		return;								// possibly from our call to Select()

	SetSelection(nNewSelection);

	// if vetoed (or some other problem), put it back.
	// (may not be necessary.)

	if (m_selection != nNewSelection)
	{
		m_pTreeCtrl->SelectItem(m_vecTreeItemIds[m_selection],true);
		m_pTreeCtrl->EnsureVisible(m_vecTreeItemIds[m_selection]);
	}
}

//////////////////////////////////////////////////////////////////

void util_treebook::OnNavigationKey(wxNavigationKeyEvent & e)
{
	if (e.IsWindowChange())
	{
		wxLogTrace(wxTRACE_Messages,_T("util_treebook::OnNavigationKey: ChangePages [flags %lx][bForward %d]"),e.m_flags,e.GetDirection());

		AdvanceSelection(e.GetDirection());		// cause page change in book base class
		m_pTreeCtrl->SetFocus();
		return;
	}

	wxWindow * const parent = GetParent();

	const bool isFromParent = (e.GetEventObject() == (wxObject*)parent);
	const bool isFromSelf = (e.GetEventObject() == (wxObject*)this);
	const bool isFromTreeCtrl = (e.GetEventObject() == (wxObject*)m_pTreeCtrl);
		
	if (isFromParent)				// from our parent (probably OK/Cancel buttons)
	{
		wxLogTrace(wxTRACE_Messages,_T("util_treebook::OnNavigationKey: FromParent [flags %lx][bForward %d]"),
				   e.m_flags,e.GetDirection());

		if (e.GetDirection())		// forward direction
		{
			// if they TAB past the parent's controls and into ours,
			// we should give focus to the tree ctrl.

			m_pTreeCtrl->SetFocus();
		}
		else						// backward direction
		{
			// if they SHIFT-TAB past the parent's controls, we
			// should give focus to the last field on the page.

			e.SetEventObject(this);
			if ((m_selection != wxNOT_FOUND) && m_pages[m_selection])
				if (!m_pages[m_selection]->GetEventHandler()->ProcessEvent(e))
					m_pages[m_selection]->SetFocus();
		}
	}
	else if (isFromSelf)
	{
#if defined(__WXMSW__)
		// we only seem to get from-self events on MSW (because
		// in src/msw/evtloop.cpp:PreProcessMessage() does not
		// call MSWProcessMessage() for the window causing the
		// event -- they start with the parent -- that's us.
		// 
		// these events and GTK's from-tree-control events are
		// treated the same.
#endif

		wxLogTrace(wxTRACE_Messages,_T("util_treebook::OnNavigationKey: [flags %lx] fromSelf"),e.m_flags);

		if (e.GetDirection())		// forward direction (TAB) on tree control
		{
			// if they TAB on the tree control, we should put focus
			// on the first field on the current page.

			e.SetEventObject(this);
			if ((m_selection != wxNOT_FOUND) && m_pages[m_selection])
				if (!m_pages[m_selection]->GetEventHandler()->ProcessEvent(e))
					m_pages[m_selection]->SetFocus();
		}
		else						// backward direction (SHIFT-TAB) on tree control
		{
			// if they SHIFT-TAB on the tree control, we should hand it 
			// up to our parent and let it hand it to the next sibling.
			// (this is probably the OK/Cancel buttons).

			e.SetCurrentFocus(this);
			if (parent)
				parent->GetEventHandler()->ProcessEvent(e);
		}
	}
	else if (isFromTreeCtrl)
	{
		wxLogTrace(wxTRACE_Messages,_T("util_treebook::OnNavigationKey: FromTree [flags %lx][bForward %d][child %x]"),
				   e.m_flags,e.GetDirection(),e.GetEventObject());

#if defined(__WXGTK__)
		// we only seem to get from-tree-control events on GTK.
		// 
		// these and the MSW' from-self events are treated the same.
#endif

		if (e.GetDirection())		// forward direction (TAB) on tree control
		{
			// if they TAB on the tree control, we should put focus
			// on the first field on the current page.

			e.SetEventObject(this);
			if ((m_selection != wxNOT_FOUND) && m_pages[m_selection])
				if (!m_pages[m_selection]->GetEventHandler()->ProcessEvent(e))
					m_pages[m_selection]->SetFocus();
		}
		else						// backward direction (SHIFT-TAB) on tree control
		{
			// if they SHIFT-TAB on the tree control, we should hand it 
			// up to our parent and let it hand it to the next sibling.
			// (this is probably the OK/Cancel buttons).

			e.SetCurrentFocus(this);
			if (parent)
				parent->GetEventHandler()->ProcessEvent(e);
		}
	}
	else							// event is from the child of one of the pages.
	{
		wxLogTrace(wxTRACE_Messages,_T("util_treebook::OnNavigationKey: FromChild [flags %lx][bForward %d][child %x]"),
				   e.m_flags,e.GetDirection(),e.GetEventObject());

		if (e.GetDirection())		// forward direction
		{
			// if they TAB out of the last field on the child page,
			// hand it up to our parent and let it hand it to the
			// next sibling.  (this is probably the OK/Cancel buttons.)

			e.SetCurrentFocus(this);
			if (parent)
				parent->GetEventHandler()->ProcessEvent(e);
		}
		else						// backward direction
		{
			// if they SHIFT-TAB out of the first field on the child page,
			// we should give focus to the tree ctrl.

			m_pTreeCtrl->SetFocus();
		}
	}
}

//////////////////////////////////////////////////////////////////

void util_treebook::OnEventTreeKeyDown(wxTreeEvent & e)
{
	int code = e.GetKeyCode();

	const wxKeyEvent ke = e.GetKeyEvent();
	bool bShift = ke.ShiftDown();
	bool bControl = ke.ControlDown();
	
	wxLogTrace(wxTRACE_Messages,_T("util_treebook::OnEventTreeKeyDown: [code %d][shift %d][control %d]"),code,bShift,bControl);

	if (code == WXK_TAB)
	{
		// let Ctrl-TAB and Ctrl-Shift-TAB on the tree control do a page change -- just like in a Notebook.
		// let TAB and Shift-TAB do the normal field traversal --  just like in a Notebook.

		wxNavigationKeyEvent ne;
		ne.SetWindowChange( bControl );
		ne.SetDirection( !bShift );
		ne.SetEventObject( m_pTreeCtrl );
		ne.SetCurrentFocus( m_pTreeCtrl );

		if (GetEventHandler()->ProcessEvent(ne))
			return;
	}
	
	e.Skip();
}

//////////////////////////////////////////////////////////////////

void util_treebook::OnSetFocus(wxFocusEvent & e)
{
	wxLogTrace(wxTRACE_Messages,_T("util_treebook::OnSetFocus()"));

	if (m_selection != wxNOT_FOUND)
		m_pages[m_selection]->SetFocus();
	else if (m_pTreeCtrl)
		m_pTreeCtrl->SetFocus();
	else
		e.Skip();
}

#endif//!version_check
