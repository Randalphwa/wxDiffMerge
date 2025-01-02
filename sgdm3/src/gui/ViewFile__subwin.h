// ViewFile__subwin.cpp
// a portion of class ViewFile that provides a simple wrapper
// around wxWindow that properly handles size events and causes
// a Layout().
//////////////////////////////////////////////////////////////////
#if 0
protected:
	class _my_sub_win : public wxWindow
	{
	public:
		_my_sub_win(wxWindow * pParent, long style);

private:
		void onSizeEvent(wxSizeEvent & e);

		DECLARE_EVENT_TABLE();
	};
#endif
