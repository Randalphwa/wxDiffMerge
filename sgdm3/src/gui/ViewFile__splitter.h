// ViewFile__splitter.h
// a portion of class ViewFile primarily concerned with managing the
// various splitter windows that we have.
//////////////////////////////////////////////////////////////////

#if defined(__WXMSW__)
#	define MY_V_SPLITTER_MIN	20
#	define MY_SPLITTER_STYLE	wxSP_NOBORDER|wxSP_LIVE_UPDATE | wxCLIP_CHILDREN		/* turn off 3d borders because of nested splitters */
#elif defined(__WXGTK__)
#	define MY_V_SPLITTER_MIN	20
#	define MY_SPLITTER_STYLE	wxSP_NOBORDER|wxSP_LIVE_UPDATE		/* turn off 3d borders because of nested splitters */
#elif defined(__WXMAC__)
#	define MY_V_SPLITTER_MIN	20
#	define MY_SPLITTER_STYLE	wxSP_NOBORDER|wxSP_LIVE_UPDATE		/* turn off 3d borders because of nested splitters */
#endif

//////////////////////////////////////////////////////////////////

public:
	typedef enum _split { SPLIT_V1=0, SPLIT_V2=1, __NR_SPLITS__ } Split;

protected:
	//////////////////////////////////////////////////////////////////
	// _my_splitter -- a simple wrapper around wxSplitterWindow that
	// handles some of its quirks.
	// this splitter is used to separate the 2 or 3 panels.
	//////////////////////////////////////////////////////////////////

	class _my_splitter : public wxSplitterWindow
	{
	public:
		_my_splitter(wxWindow * pParent, ViewFile * pViewFile, Split s, long kSync, float proportion=0.5f)
			: wxSplitterWindow(pParent,wxID_ANY,wxDefaultPosition,wxDefaultSize,MY_SPLITTER_STYLE),
			  m_pViewFile(pViewFile),
			  m_split(s),
			  m_kSync(kSync),
			  m_proportion(proportion)
			{
				SetSashGravity(m_proportion);
				SetMinimumPaneSize(1);		// prevents unsplit
			};

		void SetProportion(float proportion=0.5f)
			{
				m_proportion = proportion;
				SetSashPosition(_getPositionFromProportion(proportion),true);
				SetSashGravity(m_proportion);
			};

		void MySplitVertically(wxWindow * pWin1, wxWindow * pWin2, float proportion=0.5f)
			{
				m_proportion = proportion;
				wxSplitterWindow::SplitVertically(pWin1,pWin2,_getPositionFromProportion_Vertical(m_proportion));
				SetSashGravity(m_proportion);

				// caller should call adjust{Vertical,Horizontal}Scrollbar() after they
				// are thru messing with the layout.  (this isn't necessary during window
				// creation.)
			};

		void MySplitHorizontally(wxWindow * pWin1, wxWindow * pWin2, float proportion=0.5f)
			{
				m_proportion = proportion;
				wxSplitterWindow::SplitHorizontally(pWin1,pWin2,_getPositionFromProportion_Horizontal(m_proportion));
				SetSashGravity(m_proportion);

				// caller should call adjust{Vertical,Horizontal}Scrollbar() after they
				// are thru messing with the layout.  (this isn't necessary during window
				// creation.)
			};

		bool areSplittersVertical(void) const
			{
				return (GetSplitMode() == wxSPLIT_VERTICAL);
			};
		
	private:
		void onDoubleClickEvent(wxSplitterEvent & e)
			{
				m_pViewFile->onSplitterDoubleClick(m_split,m_kSync,e);
			};

		void onSashPosChangedEvent(wxSplitterEvent & e)
			{
				if (areSplittersVertical())
					onSashPosChangedEvent_Vertical(e);
				else
					onSashPosChangedEvent_Horizontal(e);
			};
		
		void onSashPosChangedEvent_Vertical(wxSplitterEvent & e)
			{
				// a vertical splitter was moved (left or right) so we need to
				// adjust children based upon the new widths.
				int sashPos = e.GetSashPosition();
				int parentSize = GetParent()->GetClientSize().GetWidth();
				if ((parentSize > 0) && (sashPos > 0) && (sashPos < parentSize))
				{
					m_proportion = (float)sashPos / (float)parentSize;
					SetSashGravity(m_proportion);
				}

				m_pViewFile->adjustHorizontalScrollbar(m_kSync);
//				wxLogTrace(wxTRACE_Messages, _T("Splitter::SashChangedVertical: [split %d][proportion %f][position %d]"),
//						   m_split,m_proportion,sashPos);
			};

		void onSashPosChangedEvent_Horizontal(wxSplitterEvent & e)
			{
				int sashPos = e.GetSashPosition();
				int parentSize = GetParent()->GetClientSize().GetHeight();
				if ((parentSize > 0) && (sashPos > 0) && (sashPos < parentSize))
				{
					m_proportion = (float)sashPos / (float)parentSize;
					SetSashGravity(m_proportion);
				}

				m_pViewFile->adjustVerticalScrollbar(m_kSync);
//				wxLogTrace(wxTRACE_Messages, _T("Splitter::SashChangedHorizontal: [split %d][proportion %f][position %d]"),
//						   m_split,m_proportion,sashPos);
			};
		
		void onSize(wxSizeEvent & /*e*/)					{ SetSashPosition(_getPositionFromProportion(m_proportion),true); };

		int  _getPositionFromProportion(float proportion=0.5f)
			{
				if (areSplittersVertical())
					return _getPositionFromProportion_Vertical(proportion);
				else
					return _getPositionFromProportion_Horizontal(proportion);
			}
		
		int  _getPositionFromProportion_Vertical(float proportion=0.5f)
			{
				int parentSize = GetParent()->GetClientSize().GetWidth();
				int sash = (int)(parentSize * proportion);
				return sash;
			};
		
		int  _getPositionFromProportion_Horizontal(float proportion=0.5f)
			{
				int parentSize = GetParent()->GetClientSize().GetHeight();
				int sash = (int)(parentSize * proportion);
				return sash;
			};

	private:
		ViewFile *		m_pViewFile;
		Split			m_split;
		long			m_kSync;
		float			m_proportion;

		DECLARE_EVENT_TABLE();
	};

	//////////////////////////////////////////////////////////////////
	// end of _my_splitter
	//////////////////////////////////////////////////////////////////

public:
	// a double click on panel splitter get handed to us
	void				onSplitterDoubleClick(Split s, long kSync, wxSplitterEvent & e);
