// ViewFile__scroll.h
// a portion of class ViewFile primarily concerned with scrolling the
// various window/child-windows and routing scroll-related events.
//////////////////////////////////////////////////////////////////

public:
	void				onScrollEvent(long kSync, wxScrollWinEvent & e);

	void				adjustHorizontalScrollbar(long kSync, int thumbGiven = -1);
	void				adjustVerticalScrollbar(long kSync, int thumbGiven = -1);
	void				adjustScrollbar(long kSync, int orientation, int thumbGiven = -1);

	void				warpScroll(long kSync, int orientation, int thumb);
	void				warpScrollCentered(long kSync, int row);

	void				keyboardScroll_top(long kSync, int orientation);
	void				keyboardScroll_bottom(long kSync, int orientation);
	void				keyboardScroll_lineup(long kSync, int orientation);
	void				keyboardScroll_linedown(long kSync, int orientation);
	void				keyboardScroll_pageup(long kSync, int orientation);
	void				keyboardScroll_pagedown(long kSync, int orientation);

	void				mouseScroll(long kSync, int vscroll, int hscroll);

	void				keyboardScroll_delta(bool bNext, bool bConflict);

	//////////////////////////////////////////////////////////////////
	// _my_scrolled_win -- a simple wrapper around wxWindow to redirect
	// scroll events back into ViewFile without having to worry about
	// the topology of nested child windows and splitters.
	//////////////////////////////////////////////////////////////////

protected:
	class _my_scrolled_win : public wxWindow
	{
	public:
		_my_scrolled_win(wxWindow * pParent, ViewFile * pViewFile, long style, long kSync)
			: wxWindow(pParent,wxID_ANY,wxDefaultPosition,wxDefaultSize,
					   style | wxVSCROLL | wxHSCROLL | wxALWAYS_SHOW_SB),
			  m_hackSize(0,0),
			  m_pViewFile(pViewFile),
			  m_pWinScrollChild(NULL),
			  m_kSync(kSync)
			{
			};

		void onScrollEvent(wxScrollWinEvent & e);
		
		void onSizeEvent(wxSizeEvent & /*e*/)
			{
				wxSize s = GetClientSize();
				if (s.x==m_hackSize.x && s.y==m_hackSize.y)
					return;
				m_hackSize.x = s.x;
				m_hackSize.y = s.y;

				if (m_pWinScrollChild)
					m_pWinScrollChild->SetSize(s);

				m_pViewFile->adjustVerticalScrollbar(m_kSync);
				m_pViewFile->adjustHorizontalScrollbar(m_kSync);
			};

		virtual bool AcceptsFocus(void) const
			{
				// lie to wxWidgets and claim that we accept focus.
				// if wxControlContainer were documented, we could
				// use it to handle our child....

				return true;
			};

		void onSetFocusEvent(wxFocusEvent & /*e*/)
			{
				// wxWidgets wants to give us the focus.  this happens
				// when the user has focus on the top-level (SYNC-VIEW
				// vs SYNC-EDIT) notebook and hits TAB.  we need to
				// forward focus to one of the VFP's.  try to hand it
				// to our child (the splitter window) and let it forward
				// it to the proper child.

				//wxLogTrace(wxTRACE_Messages,_T("_my_scroll_win::onSetFocusEvent:"));

				if (m_pWinScrollChild)
					m_pWinScrollChild->SetFocus();
			};
		
		inline void setChild(wxWindow * pChild)
			{
				m_pWinScrollChild = pChild;
			};
		
	private:
		wxSize			m_hackSize;
		ViewFile *		m_pViewFile;
		wxWindow *		m_pWinScrollChild;
		long			m_kSync;

		DECLARE_EVENT_TABLE();
	};

	//////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////

public:
	inline int			getScrollThumbCharPosition(long kSync, int orientation)	const
	{
		return m_nbPage[kSync].m_pWinScroll->GetScrollPos(orientation);
	};
	inline int			getScrollRangeChar(long kSync, int orientation) const
	{
		return m_nbPage[kSync].m_pWinScroll->GetScrollRange(orientation);
	};
	inline int			getScrollThumbSize(long kSync, int orientation) const
	{
		return m_nbPage[kSync].m_pWinScroll->GetScrollThumb(orientation);
	};
	inline int			getScrollThumbMax(long kSync, int orientation) const
	{
		return getScrollRangeChar(kSync,orientation) - getScrollThumbSize(kSync,orientation);
	};
