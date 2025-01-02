// ViewFile__iface_defs.h
// a portion of class ViewFile primarily concerned with creating the GUI.
// that is the window/child-window topology and routing various events.
//////////////////////////////////////////////////////////////////

#define VIEWFILE_STATUSBAR_CELL_STATUS			0
#define VIEWFILE_STATUSBAR_CELL_RS				1
#define VIEWFILE_STATUSBAR_CELL_CHARENC			2

#define VIEWFILE_STATUSBAR_CELL_STATUS_WIDTH	-4
#define VIEWFILE_STATUSBAR_CELL_RS_WIDTH		-1
#define VIEWFILE_STATUSBAR_CELL_CHARENC_WIDTH	-2

// TODO put character encoding string into status bar field [2].

//////////////////////////////////////////////////////////////////

public:
	virtual void			createLayout(void);
	inline ViewFilePanel *	getPanel(long kSync, PanelIndex kPanel) const { return m_nbPage[kSync].m_pWinPanel[kPanel]; };
	inline long				getCurrentNBSync(void)					const { return m_currentNBSync; };
	inline int				getSync(void)							const { return m_currentNBSync; };	// TODO delete one of these

	inline bool				areSplittersVertical(long kSync)				const { return (m_nbPage[kSync].m_pWinSplitter[SPLIT_V1]->areSplittersVertical()); };
	virtual void			setSplittersVertical(long kSync, bool bVertical) = 0;

	//////////////////////////////////////////////////////////////////

protected:
	virtual void			_create_top_panels(long kSync) = 0;
	void					_createLayout_nb_page(wxWindow * pParent, long kSync);
	void					_layout_enable_edit_panel(void);
	void					_create_find_panel(void);

	wxNotebook *			m_pNotebook;
	long					m_currentNBSync;	// should match value of m_pNotebook->GetSelection() after change notification has finished
	struct _nb
	{
		wxPanel *				m_pNotebookPanel;
		_my_scrolled_win *		m_pWinScroll;
		_my_splitter *			m_pWinSplitter[__NR_SPLITS__];
		ViewFilePanel *			m_pWinPanel[__NR_TOP_PANELS__];
		MyStaticText *			m_pWinTitle[__NR_TOP_PANELS__];		// windows for displaying the column titles
		MyStaticText *			m_pWinChangeStatusText;				// window for displaying "changes %d / conflict %d..." inside the notebook page
		Glance *				m_pWinGlance;
		int						m_panelWithFocus;

	}						m_nbPage[__NR_SYNCS__];


	class _my_notebook : public wxNotebook
	{
	public:
		_my_notebook(wxWindow * pParent, ViewFile * pViewFile, long style)
			: wxNotebook(pParent,wxID_ANY,wxDefaultPosition,wxDefaultSize,style),
			  m_pViewFile(pViewFile)
			{
			};

		virtual bool AcceptsFocus(void) const { return false; };		// don't let notebook panel/tabs steal focus from ViewFilePanel's.

		void onSetFocusEvent(wxFocusEvent & e)
			{
				//wxLogTrace(wxTRACE_Messages,_T("_my_notebook::onSetFocusEvent:"));
				e.Skip();
			};

		void onKeyDownEvent(wxKeyEvent & e);
		
		void on_notebook_page_changing(wxNotebookEvent & e)
			{
				m_pViewFile->onEvent_NotebookChanging(e);
			};

		void on_notebook_page_changed(wxNotebookEvent & e)
			{
				m_pViewFile->onEvent_NotebookChanged(e);
				e.Skip();
			};

	protected:
		ViewFile *			m_pViewFile;

		DECLARE_EVENT_TABLE();
	};

