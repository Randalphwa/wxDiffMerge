// FrameFactory.h
// a singleton container class for gui_frame maintaining a list of open top-level windows.
//////////////////////////////////////////////////////////////////

#ifndef H_FRAMEFACTORY_H
#define H_FRAMEFACTORY_H

//////////////////////////////////////////////////////////////////

class FrameFactory
{
public:
	FrameFactory(void);		// TODO make this private and have friend which creates all global classes.
	~FrameFactory(void);	// TODO make this private and have friend which deletes all global classes.

	typedef std::map<gui_frame *,gui_frame *>	TMap;			// TODO change this to a <set>
	typedef TMap::iterator						TMapIterator;
	typedef TMap::const_iterator				TMapConstIterator;
	typedef TMap::value_type					TMapValue;

	void onMenuFileExit(wxFrame * pFrame);

	gui_frame *		newFrame(ToolBarType tbt=TBT_BASIC, const wxString & caption = VER_APP_TITLE);

	gui_frame *		openFirstFrame(const cl_args * pArgs,
								   const xt_tool ** ppxt, wxString & rStrXtExe, wxString & rStrXtArgs);

	gui_frame *		openFolderFrame(const wxString & s0, const wxString & s1, const cl_args * pArgs);
	gui_frame *		openFileDiffFrame(const wxString & s0, const wxString & s1,
									  const cl_args * pArgs,
									  const xt_tool ** ppxt, wxString & rStrXtExe, wxString & rStrXtArgs);
	gui_frame *		openFileMergeFrame(const wxString & s0, const wxString & s1, const wxString & s2,
									   const cl_args * pArgs,
									   const xt_tool ** ppxt, wxString & rStrXtExe, wxString & rStrXtArgs);

	void			openFrameRequestedByFinderOrDND(gui_frame * pFrame,
													const wxArrayString & asFilenames);

	void			openFileDiffFrameOrSpawnAsyncXT(const wxString & s0, const wxString & s1,
													const cl_args * pArgs);
	void			openFileMergeFrameOrSpawnAsyncXT(const wxString & s0, const wxString & s1, const wxString & s2,
													 const cl_args * pArgs);

	void			openFoldersFromDialogs(gui_frame * pDialogParent);
	void			openFileDiffFromDialogs(gui_frame * pDialogParent);
	void			openFileMergeFromDialogs(gui_frame * pDialogParent);

	void			raiseFrame(gui_frame * pFrame) const;

	gui_frame *		findFolderFrame(const fd_fd * pFdFd);
	gui_frame *		findFileDiffFrame(const fs_fs * pFsFs);
	gui_frame *		findFileMergeFrame(const fs_fs * pFsFs);
	gui_frame *		findEmptyFrame(void);

	bool			closeAllFrames(bool bForce);
	void			unlinkFrame(gui_frame * pFrame);

	gui_frame *		findFirstEditableFileFrame(void);
	void			saveAll(wxCommandEvent & eFoo);

	int			countFrames(void);
	gui_frame *		getFirstFrame(void);

	bool			haveFrameAtPosition(const wxPoint & pos);

	wxString		dumpSupportInfoFF(void);

	static void					getExternalToolArgs(int nrParams,
													const wxString & s0,
													const wxString & s1,
													const wxString & s2,
													const cl_args * pArgs,
													const xt_tool * pxt,
													wxString & rStrExe,
													wxString & rStrArgs);
	static bool					lookupExternalTool(int nrParams,
												   const wxString & s0,
												   const wxString & s1,
												   const wxString & s2,
												   const cl_args * pArgs,	// only used for titles, result path -- may be null
												   const xt_tool ** ppxt,
												   wxString & rStrXtExe,
												   wxString & rStrXtArgs);

private:
	gui_frame *		_createFirstFrame(const cl_args * pArgs);

	bool			_checkForUniqueFolders(gui_frame * pDialogParent,
										   const wxString & s0,			// input only
										   const wxString & s1);		// input only
	bool			_raiseOpenFoldersDialog(gui_frame * pDialogParent,
											const cl_args * pArgs,
											wxString & s0,	// returned only
											wxString & s1);	// returned only

	bool			_checkForUniqueFiles(gui_frame ** ppDialogParent,
										 const cl_args * pArgs,			// use for caption only
										 const wxString & s0,			// input only
										 const wxString & s1);		// input only
	bool			_raiseOpenFileDiffDialog(gui_frame ** ppDialogParent,
											const cl_args * pArgs,
											wxString & s0,	// returned only
											wxString & s1);	// returned only

	bool			_checkForUniqueFiles(gui_frame ** ppDialogParent,
										 const cl_args * pArgs,			// use for caption only
										 const wxString & s0,			// input only
										 const wxString & s1,			// input only
										 const wxString & s2);			// input only
	bool			_raiseOpenFileMergeDialog(gui_frame ** ppDialogParent,
											  const cl_args * pArgs,
											  wxString & s0,	// returned only
											  wxString & s1,	// returned only
											  wxString & s2);	// returned only

	static int		_ask_about_temp_file_usage(wxFrame * pDialogParent, poi_item * pPoiEditBuffer);

private:

	// critical section to protect the hash of open frames for m_map
	wxCriticalSection	m_cs_map; 

	TMap			m_map;
};

//////////////////////////////////////////////////////////////////

#endif//H_FRAMEFACTORY_H
