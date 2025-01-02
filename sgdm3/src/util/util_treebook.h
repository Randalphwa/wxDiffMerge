// util_treebook.h
// a control something like a wxNotebook or wxListbook, but using
// a tree control.
//////////////////////////////////////////////////////////////////

	// TODO we can remove this #ifdef once we get the build system updated to 2.8.
#if wxCHECK_VERSION(2,8,0)
#else
#ifndef H_UTIL_TREEBOOK_H
#define H_UTIL_TREEBOOK_H

//////////////////////////////////////////////////////////////////

class util_treebook : public wxBookCtrlBase
{
public:
	util_treebook(void)
		{
			Init();
		};

	util_treebook(wxWindow * pParent, wxWindowID id,
				  const wxPoint & pos = wxDefaultPosition, const wxSize & size = wxDefaultSize,
				  long style = 0,
				  const wxString & name = wxEmptyString)
		{
			Init();

			(void)Create(pParent,id,pos,size,style,name);
		};

	bool				Create(wxWindow * pParent, wxWindowID id,
							   const wxPoint & pos = wxDefaultPosition, const wxSize & size = wxDefaultSize,
							   long style = 0,
							   const wxString & name = wxEmptyString);

	virtual int			GetSelection(void) const { return m_selection; };
	virtual int			SetSelection(size_t n);
	virtual int			ChangeSelection(size_t n);

	virtual bool		SetPageText(size_t n, const wxString & strText);
	virtual wxString	GetPageText(size_t n) const;

	virtual int			GetPageImage(size_t n) const;
    virtual bool		SetPageImage(size_t n, int imageId);

    virtual wxSize		CalcSizeFromPage(const wxSize& sizePage) const;
    virtual bool		InsertPage(size_t n, wxWindow *page, const wxString& text, bool bSelect = false, int imageId = -1);

    virtual wxWindow *	DoRemovePage(size_t page);
	virtual bool		DeleteAllPages(void);

	wxSize				GetTreeSize(void) const;
	wxRect				GetPageRect(void) const;

	void				OnEventSize(wxSizeEvent & e);
	void				OnEventTreeSelectionChanged(wxTreeEvent & e);
	void				OnNavigationKey(wxNavigationKeyEvent & e);
	void				OnEventTreeKeyDown(wxTreeEvent & e);
	void				OnSetFocus(wxFocusEvent & e);

private:
	void				Init(void);

	wxTreeCtrl *		m_pTreeCtrl;		// the actual tree-control within our tree-book.
	int					m_selection;		// the currently selected page or wxNOT_FOUND if none.

	typedef std::vector<wxTreeItemId>			TVecTreeItemIds;
	typedef TVecTreeItemIds::iterator			TVecTreeItemIdsIterator;
	
	TVecTreeItemIds		m_vecTreeItemIds;

	DECLARE_EVENT_TABLE();
	DECLARE_DYNAMIC_CLASS_NO_COPY(util_treebook);
};

//////////////////////////////////////////////////////////////////

class util_treebook_event : public wxBookCtrlBaseEvent
{
public:
	util_treebook_event(wxEventType commandType = wxEVT_NULL, int id = 0,
						int nSel = wxNOT_FOUND, int nOldSel = wxNOT_FOUND)
		: wxBookCtrlBaseEvent(commandType,id,nSel,nOldSel)
		{};

private:
	DECLARE_DYNAMIC_CLASS_NO_COPY(util_treebook_event);
};

//////////////////////////////////////////////////////////////////

extern const wxEventType UTIL_COMMAND_TREEBOOK_PAGE_CHANGED;

typedef void (wxEvtHandler::*util_treebook_event_function)(util_treebook_event &);

#define util_treebook_event_handler(func)	(wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(util_treebook_event_function), &func)

#define EVT_util_treebook_PAGE_CHANGED(winid, fn)	wx__DECLARE_EVT1(UTIL_COMMAND_TREEBOOK_PAGE_CHANGED, winid, util_treebook_event_handler(fn))

//////////////////////////////////////////////////////////////////

#endif//H_UTIL_TREEBOOK_H
#endif//!version_check
