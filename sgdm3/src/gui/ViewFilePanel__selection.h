// ViewFilePanel__selection.h
// portion of ViewFilePanel primarily concerned with selection.
//////////////////////////////////////////////////////////////////

public:
	static void			convertEOLs(wxString & str, fim_eol_mode modeInBuf, fim_eol_mode modeDesired);

	bool				haveSelection(void) const;
	void				clearSelection(void);
	void				selectAll(void);
	long				getCaretOrSelectionRow(bool bEnd);

	bool				prepare_to_copy_selection(wxString & strBuf) const;
	void				copySelectionForFind(void) const;
	void				copyToClipboard(void) const;
	void				cutToClipboard(void);
	void				pasteFromClipboard(void);

	void				undo(void);
	void				redo(void);

	fim_patchset *		_build_patchset(int kSync, int * pNrConflicts);
	void				autoMerge(void);

	FindResult			find(const wxString & strPattern, bool bIgnoreCase, bool bForward);

	void				setBogusCaret(void);
	
	void				gotoLine(int line);		// goto document line-number NOT display-list-row-number

protected:
	void				_doUpdateCaret(bool bContinueSelection, int row, int col);
	void				_doUpdateCaret(bool bContinueSelection, const ViewFileCoord & vfc);
	bool				_withinSelection(int row, int col) const;
	bool				_intersectSelection(int row, int col0, int col1) const;
	bool				_containsCaret(int row, int col0, int col1) const;
	void				_setGoal(const ViewFileCoord & vfc);
	void				_setGoal(int col);
	int					_getGoal(void) const;

	const fl_line *		_mapRowToFlLine(int row)						const;

	bool				_mapCoordToDocPosition(const ViewFileCoord * pVFC, fim_offset * pDocPos) const;
	bool				_mapCoordToDocPosition2(const ViewFileCoord * pVFC, fim_offset * pDocPos, bool bSkipVoids) const;
	bool				_mapDocPositionToRowCol(fim_offset docPosition,
												int * pRow, int * pCol);
	void				_deleteText(fim_offset docPos0, fim_offset docPos1);
	void				_insertText(const wxString & str, bool bSelectNewText=false);
	void				_placeCaret(fim_offset docPos);
	void				_placeSelection(fim_offset docPos0, fim_offset docPos1);
	bool				_getTextFromClipboard(wxTextDataObject & tdo);

	FindResult			_find_forward(bool bUseRegEx, bool bIgnoreCase, bool bWrapAround, const wxRegEx & rePatternWork, const wxString & strPatternWork);
	FindResult			_find_backwards(bool bIgnoreCase, bool bWrapAround, const wxString & strPatternWork);

	bool				_find_forward_on_row(int kRow, int colStart, bool bUseRegEx, bool bIgnoreCase,
											 const wxRegEx & rePatternWork, const wxString & strPatternWork);
	bool				_find_backward_on_row(int kRow, int colStart, bool bIgnoreCase, const wxString & strPatternWork);

private:
	ViewFileCoord		m_vfcSelection0, m_vfcSelection1, m_vfcAnchor, m_vfcCaret;
	ViewFileCoord		m_vfcWordSelection0, m_vfcWordSelection1;	// only used when selection started with word-select
	int					m_colGoal;

//////////////////////////////////////////////////////////////////
// see ViewFile::onEvent_setTabStop()

public:
	class caretState
	{
	public:
		bool			bValidSelection0, bValidSelection1, bValidCaret;
		fim_offset		docPosSelection0, docPosSelection1, docPosCaret;
	};

	void				beforeTabStopChange(caretState * p);
	void				afterTabStopChange(caretState * p);

//
//////////////////////////////////////////////////////////////////
