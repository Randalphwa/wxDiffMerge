// ViewFilePanel__mouse.h
// a portion of class ViewFilePanel primarily concerned with the mouse.
//////////////////////////////////////////////////////////////////

public:
	void				onMouseLeftDown(wxMouseEvent & e);
	void				onMouseLeftDClick(wxMouseEvent & e);
	void				onMouseLeftUp(wxMouseEvent & e);
	void				onMouseRightDown(wxMouseEvent & e);
	void				onMouseMotion(wxMouseEvent & e);
	void				onMouseEnterWindow(wxMouseEvent & e);
	void				onMouseLeaveWindow(wxMouseEvent & e);
	void				onMouseEventWheel(wxMouseEvent & e);
	void				onMouseMiddleDown(wxMouseEvent & e);

	void				onTimerEvent_MyMouse(wxTimerEvent & e);

	int					computeDefaultPatchAction(void);
	void				doPatchOperation(int op);
	void				doPatchOperationSetCaret(int op);

private:
	void				_raise_content_context_menu(int row, bool bDontExtend, bool bAutoApply);

	bool				_mapMouseFindCrossing(wxDC & dc, int xMouse,
											  const wxChar * sz, long len,
											  bool bDisplayInvisibles, int cColTabWidth,
											  wxCoord & x0, wxCoord & x1,
											  int & col0, int & col1);
	void				_mapMouseEventToCoord(int xMouse, int yMouse,
											  int * pRow, int * pVScroll,
											  int * pCol, int * pHScroll);
	void				_rememberMousePosition(bool bMotionEvent, const ViewFileCoord & vfc);

	int					_build_context_menu__edit(wxMenu * pMenu);
	int					_build_context_menu__t0(wxMenu * pMenu);
	int					_build_context_menu__t2(wxMenu * pMenu);
	int					_build_context_menu(wxMenu * pMenu);

	void				_onMenuEvent_CTX_DELETE(wxCommandEvent & e);
	void				_onMenuEvent_CTX_INSERT_L(wxCommandEvent & e);
	void				_onMenuEvent_CTX_INSERT_BEFORE_L(wxCommandEvent & e);
	void				_onMenuEvent_CTX_INSERT_AFTER_L(wxCommandEvent & e);
	void				_onMenuEvent_CTX_REPLACE_L(wxCommandEvent & e);
	void				_onMenuEvent_CTX_INSERT_R(wxCommandEvent & e);
	void				_onMenuEvent_CTX_INSERT_BEFORE_R(wxCommandEvent & e);
	void				_onMenuEvent_CTX_INSERT_AFTER_R(wxCommandEvent & e);
	void				_onMenuEvent_CTX_REPLACE_R(wxCommandEvent & e);
	void				_onMenuEvent_CTX_DELETE_MARK(wxCommandEvent & e);
	void				_onMenuEvent_CTX_CUT(wxCommandEvent & e);
	void				_onMenuEvent_CTX_COPY(wxCommandEvent & e);
	void				_onMenuEvent_CTX_PASTE(wxCommandEvent & e);
	void				_onMenuEvent_CTX_SELECT_ALL(wxCommandEvent & e);
	void				_onMenuEvent_CTX_NEXT_CHANGE(wxCommandEvent & e);
	void				_onMenuEvent_CTX_PREV_CHANGE(wxCommandEvent & e);
	void				_onMenuEvent_CTX_NEXT_CONFLICT(wxCommandEvent & e);
	void				_onMenuEvent_CTX_PREV_CONFLICT(wxCommandEvent & e);
	void				_onMenuEvent_CTX_SELECT_PATCH(wxCommandEvent & e);
	void				_onMenuEvent_CTX_DIALOG_MARK(wxCommandEvent & e);

	void				_doNextPrevOperation(int row);

	void				_do_context_menu__mark(long yRow, wxMouseEvent & e);
	void				_do_context_menu__selection(void);
	void				_do_context_menu__nonselection(int row, bool bOnPatch);

private:
	bool				m_bHaveMotion;
	bool				m_bWeCapturedTheMouse;
	bool				m_bDClick;

	ViewFileCoord		m_vfcMouse;

	long				m_rowRightMouse;	// only valid while right-mouse context menu up
