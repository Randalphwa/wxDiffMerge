// View.h
// base class for gui-layer representation of a view.
// gui_frame will reference a "Doc" and a "View".
// this "View" will be the single client window in the frame.
//////////////////////////////////////////////////////////////////

#ifndef H_VIEW_H
#define H_VIEW_H

//////////////////////////////////////////////////////////////////

#define ID_VIEW_CLIENT				1000
#define ID_VIEWFOLDER_LIST_CTRL		1001


//////////////////////////////////////////////////////////////////

class View : public wxWindow
{
public:
	View(gui_frame * pFrame, Doc * pDoc, const cl_args * pArgs);
	virtual ~View(void);

	virtual void			createLayout(void) = 0;
	virtual void			activated(bool bActive) = 0;
	virtual void			queueEvent(int reason) = 0;
	virtual void			setTopLevelWindowTitle(void) = 0;
	virtual ViewPrintout *	createPrintout(ViewPrintout * pPrintoutRelated) = 0;
	virtual wxString		dumpSupportInfo(const wxString & strIndent) const = 0;

	const wxString			getTitle(int kColumn)	const { return m_title[kColumn]; }

	inline gui_frame               *getFrame(void) const        { return m_pFrame; }
	inline Doc                     *getDoc(void) const          { return m_pDoc; }

	virtual void			specialSetFocus(void) = 0;

	virtual void			doSave(void) = 0;
	virtual void			doSaveAs(void) = 0;
	
	//////////////////////////////////////////////////////////////////
	// Tool bar buttons/Menu events for things visible on folder-diff
	// frames.  They are no-ops on file-based frames.
	//////////////////////////////////////////////////////////////////

	virtual bool			isEnabled_FolderOpenFiles(void)		const { return false; }; // pseudo-pure
	virtual void			onEvent_FolderOpenFiles(void)		const {               }; // pseudo-pure

	virtual bool			isEnabled_FolderExportDiffFiles(void)	const { return false; }; // pseudo-pure
	virtual void			onEvent_FolderExportDiffFiles(int /*gui_frame::_iface_id*/ /*id*/)	const { }; // pseudo-pure

	virtual bool			isEnabled_FolderOpenFolders(void)	const { return false; }; // pseudo-pure
	virtual void			onEvent_FolderOpenFolders(void)		const {               }; // pseudo-pure

#if defined(__WXMSW__)
	virtual bool			isEnabled_FolderOpenShortcuts(void)	const { return false; }; // pseudo-pure
	virtual void			onEvent_FolderOpenShortcuts(void)	const {               }; // pseudo-pure
#endif
#if defined(__WXMAC__) || defined(__WXGTK__)
	virtual bool			isEnabled_FolderOpenSymlinks(void)	const { return false; }; // pseudo-pure
	virtual void			onEvent_FolderOpenSymlinks(void)	const {               }; // pseudo-pure
#endif

	//////////////////////////////////////////////////////////////////
	// Tool bar buttons/Menu events for things visible on file-diff/merge
	// frames.  They are no-ops on folder-based frames.
	//////////////////////////////////////////////////////////////////

	virtual void			onEvent_SetDisplayMode(long /*kSync*/, de_display_ops /*dop*/)					{			};	// pseudo-pure
	virtual void			onEvent_SetDisplayBits(long /*kSync*/, de_display_ops /*dop*/, bool /*bOn*/)	{			};	// pseudo-pure
	virtual de_display_ops		getDisplayOps(long /*kSync*/)							const	{ return 0; };	// pseudo-pure

	virtual bool			getPilcrow(void)										const	{ return false; };	// pseudo-pure
	virtual void			onEvent_SetPilcrow(bool /*bOn*/)								{				};	// pseudo-pure

	virtual void			onEvent_setTabStop(int /*t*/)									{			};	// pseudo-pure
	virtual int				getTabStop(void)										const	{ return 0; };	// pseudo-pure

	virtual bool			getShowLineNumbers(void)								const	{ return false; };	// pseudo-pure
	virtual void			onEvent_SetShowLineNumbers(bool /*bOn*/)						{				};	// pseudo-pure

protected:
	gui_frame *				m_pFrame;	// we do not own this
	Doc *					m_pDoc;		// we do not own this

	wxString				m_title[3];	// individual column titles (as specified on command line for the first window)

	typedef enum { ID_HTML_BANNER = 100,
				   ID_TIMER_MY_QUEUE,
				   ID_TIMER_MY_MOUSE,
				   ID_TIMER_MY_PROGRESS
	} MyViewID;
	
	wxTimer					m_timer_MyQueue;
	wxTimer					m_timer_MyProgress;

private:
//	virtual void				onPaintEvent(wxPaintEvent & e) = 0;
	void					onSizeEvent(wxSizeEvent & e);
	void					onSysColourChangedEvent(wxSysColourChangedEvent & e);

	virtual void				onTimerEvent_MyQueue(wxTimerEvent & e) = 0;
	virtual void				onTimerEvent_MyProgress(wxTimerEvent & e) = 0;

	DECLARE_EVENT_TABLE();
};

//////////////////////////////////////////////////////////////////

#endif//H_VIEW_H
