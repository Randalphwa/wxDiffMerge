// ViewFilePanel__kb.h
// a portion of class ViewFilePanel primarily concerned with a
// set of keybindings.
//////////////////////////////////////////////////////////////////

public:
	void				onKeyDownEvent(wxKeyEvent & e);
	void				onCharEvent(wxKeyEvent & e);

private:
	bool				_dispatchKeyDownEvent(wxKeyEvent & e);
	bool				_dispatchCharEvent(wxKeyEvent & e);

	bool				_kb_getEOLCol(int row, int * pColResult) const;
	int					_kb_getEOFRow(void) const;

	bool				_kb_getNextCol(int row, int colArg, int * pColResult);
	bool				_kb_getPrevCol(int row, int colArg, int * pColResult);
	bool				_kb_getThisCol(int row, int colArg, int * pColResult);

	bool				_kb_getNextWordBreakCol(int row, int colArg, int * pColResult);
	bool				_kb_getPrevWordBreakCol(int row, int colArg, int * pColResult);

	bool				_kb_getWordBoundsAtCol(int row, int colArg,
											   int * pRowResultStart, int * pColResultStart,
											   int * pRowResultEnd, int * pColResultEnd);

	bool				_kb_getNextRow(int row, int * pRowResult);
	bool				_kb_getPrevRow(int row, int * pRowResult);

	bool				_kb_getPrev(bool bByWord, const ViewFileCoord * pVFC, ViewFileCoord * pVFCprev, fim_offset * pDocPos);
	bool				_kb_getNext(bool bByWord, const ViewFileCoord * pVFC, ViewFileCoord * pVFCprev, fim_offset * pDocPos);

	void				_kb_ensureVisible(int row, int col, bool bForceRefresh=true);
	void				_kb_ensureCaretVisible(bool bForceRefresh=true);
	int					_kb_ensureVisible_computeBestHThumb(int row, int col);

	bool				_kb_alert(void) const;
	bool				_kb_insert_text(const wxString & str);

private:
#define DCL_KB(fn)	bool	fn(wxKeyEvent & e)

	DCL_KB(_vk_pagedown);
	DCL_KB(_vk_pageup);
	DCL_KB(_vk_home);
	DCL_KB(_vk_end);
	DCL_KB(_vk_left);
	DCL_KB(_vk_right);
	DCL_KB(_vk_up);
	DCL_KB(_vk_down);
	DCL_KB(_vk_escape);
	DCL_KB(_vk_tab);
	DCL_KB(_vk_backspace);
	DCL_KB(_vk_delete);
	DCL_KB(_vk_enter);
	DCL_KB(_vk_insert);
	DCL_KB(_vk__default);
	DCL_KB(_vk_context_menu);

#undef DCL_KB
