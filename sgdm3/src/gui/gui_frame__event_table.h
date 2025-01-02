// gui_frame__event_table.h
// a portion of class gui_frame.
//////////////////////////////////////////////////////////////////

	void onSizeEvent(wxSizeEvent & e);
	void onMoveEvent(wxMoveEvent & e);

	void onWebhelp(wxCommandEvent & e);
	void onVisitSG(wxCommandEvent & e);

	//////////////////////////////////////////////////////////////////
	// see gui_frame__events_closing.cpp
	//////////////////////////////////////////////////////////////////

	void onCloseEvent(wxCloseEvent & e);

	//////////////////////////////////////////////////////////////////
	// see gui_frame__events_frame.cpp
	//////////////////////////////////////////////////////////////////

	void onActivateEvent(wxActivateEvent & e);
	void onSetFocusEvent(wxFocusEvent & e);

	//////////////////////////////////////////////////////////////////
	// see gui_frame__events.cpp
	//////////////////////////////////////////////////////////////////

	void onFileFolderDiff(wxCommandEvent & e);
	void onFileFileDiff(wxCommandEvent & e);
	void onFileFileMerge(wxCommandEvent & e);
	void onFileReload(wxCommandEvent & e);			void onUpdateFileReload(wxUpdateUIEvent & e);
	void onFileChangeRuleset(wxCommandEvent & e);	void onUpdateFileChangeRuleset(wxUpdateUIEvent & e);
	void onFileSave(wxCommandEvent & e);			void onUpdateFileSave(wxUpdateUIEvent & e);
	void onFileSaveAs(wxCommandEvent & e);			void onUpdateFileSaveAs(wxUpdateUIEvent & e);
	void onFileSaveAll(wxCommandEvent & e);			void onUpdateFileSaveAll(wxUpdateUIEvent & e);
	void onFileCloseWindow(wxCommandEvent & e);
	void onFileExit(wxCommandEvent & e);

	void onEditUndo(wxCommandEvent & e);			void onUpdateEditUndo(wxUpdateUIEvent & e);
	void onEditRedo(wxCommandEvent & e);			void onUpdateEditRedo(wxUpdateUIEvent & e);
	void onEditCut(wxCommandEvent & e);				void onUpdateEditCut(wxUpdateUIEvent & e);
	void onEditCopy(wxCommandEvent & e);			void onUpdateEditCopy(wxUpdateUIEvent & e);
	void onEditPaste(wxCommandEvent & e);			void onUpdateEditPaste(wxUpdateUIEvent & e);
	void onEditSelectAll(wxCommandEvent & e);		void onUpdateEditSelectAll(wxUpdateUIEvent & e);
	void onEditNextDelta(wxCommandEvent & e);		void onUpdateEditNextDelta(wxUpdateUIEvent & e);
	void onEditPrevDelta(wxCommandEvent & e);		void onUpdateEditPrevDelta(wxUpdateUIEvent & e);
	void onEditNextConflict(wxCommandEvent & e);	void onUpdateEditNextConflict(wxUpdateUIEvent & e);
	void onEditPrevConflict(wxCommandEvent & e);	void onUpdateEditPrevConflict(wxUpdateUIEvent & e);
	void onEditAutoMerge(wxCommandEvent & e);		void onUpdateEditAutoMerge(wxUpdateUIEvent & e);	bool _isEditAutoMergeEnabled(void);

	void onSettingsPreferences(wxCommandEvent & e);

	void onHelpContents(wxCommandEvent & e);
	void onHelpAbout(wxCommandEvent & e);

	void onFolderOpenFiles(wxCommandEvent & e);		void onUpdateFolderOpenFiles(wxUpdateUIEvent & e);
	void onFolderOpenFolders(wxCommandEvent & e);	void onUpdateFolderOpenFolders(wxUpdateUIEvent & e);
#if defined(__WXMSW__)
	void onFolderOpenShortcuts(wxCommandEvent & e);	void onUpdateFolderOpenShortcuts(wxUpdateUIEvent & e);
#endif
#if defined(__WXMAC__) || defined(__WXGTK__)
	void onFolderOpenSymlinks(wxCommandEvent & e);	void onUpdateFolderOpenSymlinks(wxUpdateUIEvent & e);
#endif

	void onFolderShow__x(FD_SHOW_HIDE_FLAGS f);
	void onUpdateFolderShow__x(wxUpdateUIEvent & e, FD_SHOW_HIDE_FLAGS f);

	void onFolderShowEqual(wxCommandEvent & e);			void onUpdateFolderShowEqual(wxUpdateUIEvent & e);
	void onFolderShowEquivalent(wxCommandEvent & e);	void onUpdateFolderShowEquivalent(wxUpdateUIEvent & e);
	void onFolderShowQuickMatch(wxCommandEvent & e);	void onUpdateFolderShowQuickMatch(wxUpdateUIEvent & e);
	void onFolderShowSingles(wxCommandEvent & e);		void onUpdateFolderShowSingles(wxUpdateUIEvent & e);
	void onFolderShowFolders(wxCommandEvent & e);		void onUpdateFolderShowFolders(wxUpdateUIEvent & e);
	void onFolderShowErrors(wxCommandEvent & e);		void onUpdateFolderShowErrors(wxUpdateUIEvent & e);

	void onViewFileShowAll(wxCommandEvent & e);		void onUpdateViewFileShowAll(wxUpdateUIEvent & e);
	void onViewFileShowDif(wxCommandEvent & e);		void onUpdateViewFileShowDif(wxUpdateUIEvent & e);
	void onViewFileShowCtx(wxCommandEvent & e);		void onUpdateViewFileShowCtx(wxUpdateUIEvent & e);
//	void onViewFileShowEql(wxCommandEvent & e);		void onUpdateViewFileShowEql(wxUpdateUIEvent & e);

	void onViewFileIgnUnimportant(wxCommandEvent & e);	void onUpdateViewFileIgnUnimportant(wxUpdateUIEvent & e);
	void onViewFileHideOmitted(wxCommandEvent & e);		void onUpdateViewFileHideOmitted(wxUpdateUIEvent & e);
	void onViewFileLineNumbers(wxCommandEvent & e);		void onUpdateViewFileLineNumbers(wxUpdateUIEvent & e);
	void onViewFilePilcrow(wxCommandEvent & e);		void onUpdateViewFilePilcrow(wxUpdateUIEvent & e);

	void onViewFileTab2(wxCommandEvent & e);		void onUpdateViewFileTab2(wxUpdateUIEvent & e);
	void onViewFileTab4(wxCommandEvent & e);		void onUpdateViewFileTab4(wxUpdateUIEvent & e);
	void onViewFileTab8(wxCommandEvent & e);		void onUpdateViewFileTab8(wxUpdateUIEvent & e);

	void onPrint(wxCommandEvent & e);			void onUpdatePrint(wxUpdateUIEvent & e);
	void onPrintPreview(wxCommandEvent & e);	void onUpdatePrintPreview(wxUpdateUIEvent & e);
	void onPageSetup(wxCommandEvent & e);

	void onViewFileInsertMark(wxCommandEvent & e);	void onUpdateViewFileInsertMark(wxUpdateUIEvent & e);	bool isViewFileInsertMarkEnabled(void);
	void onViewFileDeleteAllMark(wxCommandEvent & e);	void onUpdateViewFileDeleteAllMark(wxUpdateUIEvent & e);

	void onViewFileEdit_Find_GoTo(wxCommandEvent & e);  void onUpdateViewFileEdit_Find_GoTo(wxUpdateUIEvent & e);

	void onViewFileEditFind__Next__Prev(wxCommandEvent & e);
	void onUpdateViewFileEditFind__Next__Prev(wxUpdateUIEvent & e);

	void onViewFileEditUseSelectionForFind(wxCommandEvent & e); void onUpdateViewFileEditUseSelectionForFind(wxUpdateUIEvent & e);

	void onViewFileEditApplyDefaultActionL(wxCommandEvent & e);		void onUpdateViewFileEditApplyDefaultActionL(wxUpdateUIEvent & e);
	void onViewFileEditApplyDefaultActionR(wxCommandEvent & e);		void onUpdateViewFileEditApplyDefaultActionR(wxUpdateUIEvent & e);


	void onViewFileSplitVertically(wxCommandEvent & e);		void onUpdateViewFileSplitVertically(wxUpdateUIEvent & e);
	void onViewFileSplitHorizontally(wxCommandEvent & e);	void onUpdateViewFileSplitHorizontally(wxUpdateUIEvent & e);

	void onExport__FolderSummary__id(wxCommandEvent & e);	void onUpdate__Export__FolderSummary__id(wxUpdateUIEvent & e);

	void onFileDiffExport__id(wxCommandEvent & e);	void onUpdate__FileDiffExport__id(wxUpdateUIEvent & e);
