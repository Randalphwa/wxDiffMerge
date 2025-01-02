// gui_app.h
// a singleton class representing the entire app.
//////////////////////////////////////////////////////////////////

#ifndef H_GUI_APP_H
#define H_GUI_APP_H

//////////////////////////////////////////////////////////////////

class gui_app : public wxApp
{

public:
	gui_app(void);
	virtual ~gui_app(void);
	virtual int		OnRun(void);
	virtual int		OnExit(void);
	virtual bool		OnInit(void);
	virtual void		OnInitCmdLine(wxCmdLineParser & parser);
	virtual bool		OnCmdLineParsed(wxCmdLineParser & parser);
	virtual bool		OnCmdLineError(wxCmdLineParser & parser);
	virtual bool		OnCmdLineHelp(wxCmdLineParser & parser);

#if defined(__WXMAC__)
	virtual void	MacOpenFiles(const wxArrayString & asFilenames);
//	virtual void	MacOpenFile(....);
//	virtual void	MacOpenURL(const wxString & strUrl);
//	virtual void	MacPrintFiles(const wxArrayString & asFilenames);
//	virtual void	MacPrintFile(const wxString & strFilename);
//	virtual void	MacNewFile(void);
//	virtual void	MacReopenApp(void);

	virtual void	SG__MacOpenFilesViaService(const wxArrayString &fileNames);
#endif

	void			onQueryEndSession(wxCloseEvent & e);
	void			onEndSession(wxCloseEvent & e);

	void			ShowHelpContents(void);
	void			ShowWebhelp(void);

	void			ShowVisitSourceGear(void);

	wxString		getExePathnameString(void) const;

	static void		spawnAsyncXT(const xt_tool * pxt, const wxString & rStrXtExt, const wxString & rStrXtArgs);
	static int		spawnSyncXT(const xt_tool * pxt, const wxString & rStrXtExt, const wxString & rStrXtArgs);

	void			setExitStatusStatic(int status);
	void			setExitStatusWaitForMergeResult(gui_frame * pFrameResult, poi_item * pPoiResult);
	int				getExitStatusType(void) { return m_exitStatusType; };
	gui_frame *		getExitStatusMergeFrame(void) const { return m_pFrameMergeResult; };
	poi_item *		getExitStatusMergePoi(void) const { return m_pPoiMergeResult; } ;

	wxString		formatArgvForSupportMessage(void);

private:
	bool			_OnInit_cl_args(void);
	void			_cleanup(void);
	void			_init_help(void);
	void			_cleanup_help(void);

	util_error		_dumpDiffs(bool * pbHadChanges);
	util_error		_batchoutput_diff2(long kSync, bool * pbHadChanges);
	util_error		_batchoutput_folders2(bool * pbHadChanges);

	wxString		_makeUsageString(void) const;
	void			_init_network(void);
	void			_cleanup_network(void);
#if defined(__WXMSW__)
	void			deref_if_shortcut(int n, wxString & msgError);
#endif

private:
	cl_args			m_cl_args;

	const xt_tool *		m_pXtStartup;	// only used if we need to invoke external tool
	wxString		m_strXtExe;		// from the set of files given on the command
	wxString		m_strXtArgs;	// line.
	gui_frame *		m_pFrameXt;

	int			m_exitStatusType;
	int			m_exitStatusValue;
	gui_frame *		m_pFrameMergeResult;	// only valid when type == MY_EXIT_STATUS_TYPE__MERGE
	poi_item *		m_pPoiMergeResult;		// only valid when type == MY_EXIT_STATUS_TYPE__MERGE

#if defined(__WXMSW__)
	wxString		m_strHelpManualPDF;
#endif
#if defined(__WXMAC__)
	wxString		m_strHelpManualPDF;
#endif
#if defined(__WXGTK__)
	wxString		m_strHelpManualPDF;
#endif

private:
	DECLARE_EVENT_TABLE()
};

//////////////////////////////////////////////////////////////////

DECLARE_APP(gui_app);

//////////////////////////////////////////////////////////////////

#endif//H_GUI_APP_H
