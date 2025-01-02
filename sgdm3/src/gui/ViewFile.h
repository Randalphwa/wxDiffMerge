// ViewFile.h
// gui-layer representation of a "View" for file-based frames
// (as opposed to a folder-diff window).
// the "gui_frame" is our wxWidgets top-level window.
// the "Doc" is a container/handle for the set of files.
//
// the "ViewFile" is a container/handle for all the various
// widgets/controls that appear in the client area of the
// frame (between the toolbar and status bar).
//
// This IS NOT a window for editing or viewing an individual
// text file.
//////////////////////////////////////////////////////////////////

#define VIEWFILE_QUEUE_EVENT_NOTHING			0x0000
#define VIEWFILE_QUEUE_EVENT_LOAD				0x0001
#define VIEWFILE_QUEUE_EVENT_RELOAD				0x0002
#define VIEWFILE_QUEUE_EVENT_FORCE_RELOAD		0x0003
#define VIEWFILE_QUEUE_EVENT_CHECKFILES			0x0004

//////////////////////////////////////////////////////////////////

class ViewFile : public View
{
public:
	ViewFile(gui_frame * pFrame, Doc * pDoc, const cl_args * pArgs);
	virtual ~ViewFile(void);

#include <ViewFile__scroll.h>
#include <ViewFile__splitter.h>
#include <ViewFile__iface_defs.h>
#include <ViewFile__subwin.h>

public:
	virtual int						getNrTopPanels(void) const = 0;
	virtual void					updateTitleWindows(long kSync) = 0;
	virtual void					activated(bool bActive);
	virtual void					queueEvent(int reason);
	virtual void					setTopLevelWindowTitle(void) = 0;
	virtual ViewPrintout *			createPrintout(ViewPrintout * pPrintoutRelated) = 0;
	virtual wxString				dumpSupportInfo(const wxString & strIndent) const = 0;
	

	inline fim_ptable *	getPTable(long kSync, PanelIndex kPanel) { return getDoc()->getFsFs()->getPTable(kSync,kPanel); };
	inline fl_fl *		getLayout(long kSync, PanelIndex kPanel) { fim_ptable * pPTable=getPTable(kSync, kPanel); return ((pPTable) ? pPTable->getLayout() : NULL); };
	inline de_de *		getDE(void) const				{ return m_pDeDe; };

	void				cb_de_changed(const util_cbl_arg & arg);
	void				prePaint(void);

	virtual void			onEvent_SetDisplayMode(long kSync, de_display_ops dop);
	virtual void			onEvent_SetDisplayBits(long kSync, de_display_ops dop, bool bOn);
	virtual de_display_ops	getDisplayOps(long kSync)	const	{ return m_dop[kSync]; };

//	virtual void			onPaintEvent(wxPaintEvent & e);
	virtual void			onTimerEvent_MyQueue(wxTimerEvent & e);
	virtual void			onTimerEvent_MyProgress(wxTimerEvent & e);

	long				getNextChange(bool bConflict);
	long				getPrevChange(bool bConflict);
	long				getNextChange(bool bConflict, int rowRelative);
	long				getPrevChange(bool bConflict, int rowRelative);

	virtual bool		getPilcrow(void) const { return m_bPilcrow; };
	virtual void		onEvent_SetPilcrow(bool bOn);

	virtual void		onEvent_setTabStop(int t);
	virtual int			getTabStop(void) const { return m_cTabStop; };

	virtual bool		getShowLineNumbers(void) const { return m_bShowLineNumbers; };
	virtual void		onEvent_SetShowLineNumbers(bool bOn);

	int					getPanelWithFocus(void);
	void				setPanelWithFocus(int kPanel);
	void				cycleFocus(bool bForward);

	void				onEvent_NotebookChanging(wxNotebookEvent & e);
	void				onEvent_NotebookChanged(wxNotebookEvent & e);

	inline bool			getEditPanelPresent(void) const { return (!m_pDoc->getFsFs()->getReadOnly());  };

	void				createEditPanel(void);
	void				cb_EditPanelStatusChanged(const util_cbl_arg & arg);
	inline pt_stat		getEditPanelStatus(void) const { return m_ptStatEdit; };

	void				cb_T1PanelStatusChanged(const util_cbl_arg & arg);

	FindResult			find(int kSync, PanelIndex kPanel,
							 const wxString & strPattern,
							 bool bIgnoreCase, bool bForward);
	FindResult			findAgain(bool bForward);
	inline FindResult	findNext(void) { return findAgain(true); };
	inline FindResult	findPrev(void) { return findAgain(false); };

	int					showModalDialogSuppressingActivationChecks(wxDialog & dlg);
	int					showModal_util_dont_show_again_msgbox_SuppressingActivationChecks(const wxString & strTitle,
																						  const wxString & strMessage,
																						  int key,
																						  long buttonFlags=wxOK);
	void				completeAutoMerge(fim_patchset * pPatchSet);

	void				deleteMark(long kSync, long yRowClick);
	void				deleteMark(long kSync, de_mark * pDeMark);
	util_error			createMark(int kSync, de_mark_type markType, int nrFiles, long * alLineNr, de_mark ** ppDeMark, PanelIndex * pPanelError=NULL);
	void				deleteAllMark(int kSync);

	int					computeDefaultAction(PanelIndex kPanelFrom);
	void				applyDefaultAction(PanelIndex kPanelFrom, bool bAutoAdvance);

	void				doExport(int /*gui_frame::_iface_id*/ id, long kSync);

	static util_error s_doExportToString(de_de * pDeDe, long kSync, int /*gui_frame::_iface_id*/ id,
										 wxString & strOutput,
										 bool * pbHadChanges,
										 int cTabStop=8,
										 const wxString & strTitleA=wxEmptyString,
										 const wxString & strTitleB=wxEmptyString);
	static util_error s_doExportStringToDestination(de_de * pDeDe, long kSync, int /*gui_frame::_iface_id*/ id,
													wxString & strOutput,
													poi_item * pPoiDestination);
	static util_error s_getExportPathnameFromUser(wxWindow * pParent,
												  poi_item * pPoiSeed,
												  int /*gui_frame::_iface_id*/ idPrev,
												  int /*gui_frame::_iface_id*/ id,
												  poi_item ** ppPoiReturned);


protected:
	util_error			_getExportPathnameFromUser(int /*gui_frame::_iface_id*/ id,
												   poi_item ** ppPoiReturned);
	util_error			_doExport(int /*gui_frame::_iface_id*/ id, long kSync);

	void				_set_display_rs_name(void);
	void				_set_display_encodings(void);
	void				_set_stats_msg(int kSync);
	void				_set_display_ops(long kSync, de_display_ops dop);
	void				_updateTitle(long kSync, PanelIndex kPanel);
	void				_timer_reload_files(bool bForceReload=false);
	void				_timer_load_files(void);
	void				_finishLoading(void);
	bool				_checkForFilesChangedOnDisk(void);

protected:
	de_de *				m_pDeDe;		// we own this
	long				m_chg;			// set of DE_GHG_ bits we've received notice of since last prePaint()
	int					m_cTabStop;
	de_display_ops		m_dop[__NR_SYNCS__];
	int					m_nQueueEventReason;
	bool				m_bNeed_WarpSyncViewToFirstChange;
	bool				m_bNeed_WarpSyncEditToFirstChange;
	bool				m_bPilcrow;
	bool				m_bShowLineNumbers;
	bool				m_bRaiseAutoMergeDialogAfterLoading;
	bool				m_bTemporarilySuppressActivationChecks;
	pt_stat				m_ptStatEdit;

	wxString			m_strLastStatsMsg;
	wxString			m_strLastRuleSetMsg;

	poi_item *			m_pPoiExportDiffsPathname;
	int					m_lastExportToFileFormat;

public:
    virtual bool		AcceptsFocus(void) const { return false; };		// don't let ViewFile take focus on LeftMouseClicks
	virtual void		specialSetFocus(void);

	virtual void		doSave(void);
	virtual void		doSaveAs(void);

	//////////////////////////////////////////////////////////////////

public:
	void showFindPanel(bool bShow, bool bFocusFind, bool bFocusGoTo);
	bool isFindPanelVisible(void) const { return m_pPanelFind->IsShown(); };

protected:
	wxPanel * m_pPanelFind;
	wxTextCtrl * m_pTextFind;
	wxButton * m_pButtonFindClose;
	wxButton * m_pButtonFindNext;
	wxButton * m_pButtonFindPrev;
	wxCheckBox * m_pCheckFindMatchCase;
	bool m_bSearchForward;
	unsigned int m_uiDataGoTo;
	wxTextCtrl * m_pTextGoTo;

	void preloadFindPanelFields(void);
	void onFindPanel_Close(wxCommandEvent & e);
	void onFindPanel_Text(wxCommandEvent & e);
	void onFindPanel_TextEnter(wxCommandEvent & e);
	void onFindPanel_Next(wxCommandEvent & e);
	void onFindPanel_Prev(wxCommandEvent & e);
	void onFindPanel_MatchCase(wxCommandEvent & e);
	FindResult onFindPanel__do_search(bool bForward);
	void onFindPanel_GoToTextEnter(wxCommandEvent & e);
	
	typedef enum {	ID_FINDPANEL_TEXT=100,
					ID_FINDPANEL_NEXT,
					ID_FINDPANEL_PREV,
					ID_FINDPANEL_MATCHCASE,
					ID_FINDPANEL_CLOSE,
					ID_FINDPANEL_GOTO
	} ID;

	DECLARE_EVENT_TABLE();
};

//////////////////////////////////////////////////////////////////

class ViewFileDiff : public ViewFile
{
public:
	ViewFileDiff(gui_frame * pFrame, Doc * pDoc, const cl_args * pArgs = NULL);
	virtual ~ViewFileDiff(void) {};

	virtual int						getNrTopPanels(void) const { return 2; };
	virtual void					updateTitleWindows(long kSync);
	virtual void					setTopLevelWindowTitle(void);
	virtual ViewPrintout *			createPrintout(ViewPrintout * pPrintoutRelated);
	virtual wxString				dumpSupportInfo(const wxString & strIndent) const;
	virtual void					setSplittersVertical(long kSync, bool bVertical);

protected:
	virtual void					_create_top_panels(long kSync);
};

//////////////////////////////////////////////////////////////////

class ViewFileMerge : public ViewFile
{
public:
	ViewFileMerge(gui_frame * pFrame, Doc * pDoc, const cl_args * pArgs = NULL);
	virtual ~ViewFileMerge(void) {};

	virtual int						getNrTopPanels(void) const { return 3; };
	virtual void					updateTitleWindows(long kSync);
	virtual void					setTopLevelWindowTitle(void);
	virtual ViewPrintout *			createPrintout(ViewPrintout * pPrintoutRelated);
	virtual wxString				dumpSupportInfo(const wxString & strIndent) const;
	virtual void					setSplittersVertical(long kSync, bool bVertical);

protected:
	virtual void					_create_top_panels(long kSync);
};
