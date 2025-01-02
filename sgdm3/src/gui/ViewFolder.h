// ViewFolder.h
// a folder diff view -- a container for a list control.
//////////////////////////////////////////////////////////////////

#ifndef H_VIEWFOLDER_H
#define H_VIEWFOLDER_H

//////////////////////////////////////////////////////////////////

#define VIEWFOLDER_QUEUE_EVENT_NOTHING			0x0000
#define VIEWFOLDER_QUEUE_EVENT_BUILD			0x0001		// just build-vec
#define VIEWFOLDER_QUEUE_EVENT_LOAD				0x0011		// rescan-tree + build-vec
#define VIEWFOLDER_QUEUE_EVENT_LOAD_SOFTMATCH	0x0111		// invalidate softmatch cache + rescan-tree + build-vec

#define VIEWFOLDER_STATUSBAR_CELL_STATS			1
#define VIEWFOLDER_STATUSBAR_CELL_STATS_WIDTH	-1

//////////////////////////////////////////////////////////////////

class ViewFolder : public View
{
public:
	ViewFolder(gui_frame * pFrame, Doc * pDoc, const cl_args * pArgs = NULL);
	virtual ~ViewFolder(void);

	virtual void					createLayout(void);
	virtual void					activated(bool bActive);
	virtual void					queueEvent(int reason);
	virtual void					setTopLevelWindowTitle(void);
	virtual ViewPrintout *			createPrintout(ViewPrintout * pPrintoutRelated=NULL);
	virtual wxString				dumpSupportInfo(const wxString & strIndent) const;

	virtual bool					isEnabled_FolderOpenFiles(void)		const { return (m_pListCtrl && m_pListCtrl->isEnabled_FolderOpenFiles()); };
	virtual void					onEvent_FolderOpenFiles(void)		const { if     (m_pListCtrl)   m_pListCtrl->onEvent_FolderOpenFiles(); };

	virtual bool					isEnabled_FolderExportDiffFiles(void)	const { return (m_pListCtrl && m_pListCtrl->isEnabled_FolderExportDiffFiles()); };
	virtual void					onEvent_FolderExportDiffFiles(int /*gui_frame::_iface_id*/ id) const { if     (m_pListCtrl)   m_pListCtrl->onEvent_FolderExportDiffFiles(id); };

	virtual bool					isEnabled_FolderOpenFolders(void)	const { return (m_pListCtrl && m_pListCtrl->isEnabled_FolderOpenFolders()); };
	virtual void					onEvent_FolderOpenFolders(void)		const { if     (m_pListCtrl)   m_pListCtrl->onEvent_FolderOpenFolders(); };

#if defined(__WXMSW__)
	virtual bool					isEnabled_FolderOpenShortcuts(void)	const { return (m_pListCtrl && m_pListCtrl->isEnabled_FolderOpenShortcuts()); };
	virtual void					onEvent_FolderOpenShortcuts(void)	const { if     (m_pListCtrl)   m_pListCtrl->onEvent_FolderOpenShortcuts(); };
#endif
#if defined(__WXMAC__) || defined(__WXGTK__)
	virtual bool					isEnabled_FolderOpenSymlinks(void)	const { return (m_pListCtrl && m_pListCtrl->isEnabled_FolderOpenSymlinks()); };
	virtual void					onEvent_FolderOpenSymlinks(void)	const { if     (m_pListCtrl)   m_pListCtrl->onEvent_FolderOpenSymlinks(); };
#endif

//	virtual bool					getSelectedRowIndex(long * pRow)	const { return (m_pListCtrl && m_pListCtrl->getSelectedRowIndex(pRow)); };

//	virtual void					onPaintEvent(wxPaintEvent & e);
	virtual void					onTimerEvent_MyQueue(wxTimerEvent & e);
	virtual void					onTimerEvent_MyProgress(wxTimerEvent & e);

	virtual void					specialSetFocus(void);

	virtual void					doSave(void);
	virtual void					doSaveAs(void);
	//int							showModalDialogSuppressingActivationChecks(wxDialog & dlg);

	virtual void					doExportAs(FD_EXPORT_FORMAT expfmt);

private:
	void							_formatStats(void);
	void							_timer_reload_folders(void);
	util_error						_run_scan_using_background_thread(bool bUsePreviousSoftmatchResult,
																	  const wxString & strSelected);
	void							_postprocess_reload(const wxString & strSelected);
	void							_create_progress_panel(void);

	ViewFolder_ListCtrl	*			m_pListCtrl;
	int								m_nQueueEventReason;
	bool							m_bTemporarilySuppressActivationChecks;

	util_background_thread_helper * m_pBGTH;
	wxString						m_BGTH_strSelected;		// only defined when m_pBGTH.
	int								m_BGTH_last_cProgress;	// only defined when m_pBGTH.

	wxPanel *						m_pPanelProgress;
	wxStaticText *					m_pStaticTextProgress;
	wxGauge *						m_pGaugeProgress;
	wxBusyCursor *					m_pBusyCursor;

public:
	void							gp_cb_long(GlobalProps::EnumGPL id);
	void							gp_cb_string(GlobalProps::EnumGPS id);
};

//////////////////////////////////////////////////////////////////

#endif//H_VIEWFOLDER_H
