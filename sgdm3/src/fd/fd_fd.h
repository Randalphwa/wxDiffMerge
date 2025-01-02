// fd_fd.h
// a folder diff
//////////////////////////////////////////////////////////////////

#ifndef H_FD_FD_H
#define H_FD_FD_H

//////////////////////////////////////////////////////////////////
// See also GPL_FOLDER_SHOW_HIDE_FLAGS.

typedef unsigned int FD_SHOW_HIDE_FLAGS;
#define FD_SHOW_HIDE_FLAGS__EQUAL				((FD_SHOW_HIDE_FLAGS)0x01)
#define FD_SHOW_HIDE_FLAGS__EQUIVALENT			((FD_SHOW_HIDE_FLAGS)0x02)
#define FD_SHOW_HIDE_FLAGS__SINGLES				((FD_SHOW_HIDE_FLAGS)0x04)
#define FD_SHOW_HIDE_FLAGS__FOLDERS				((FD_SHOW_HIDE_FLAGS)0x08)
#define FD_SHOW_HIDE_FLAGS__ERRORS				((FD_SHOW_HIDE_FLAGS)0x10)
#define FD_SHOW_HIDE_FLAGS__QUICKMATCH			((FD_SHOW_HIDE_FLAGS)0x20)
#define FD_SHOW_HIDE_FLAGS__MASK_BITS			((FD_SHOW_HIDE_FLAGS)0x3f)

//////////////////////////////////////////////////////////////////

typedef unsigned int FD_EXPORT_FORMAT;
#define FD_EXPORT_FORMAT__UNSET					((FD_EXPORT_FORMAT)0x0000)

#define FD_EXPORT_FORMAT__MASK					((FD_EXPORT_FORMAT)0x00ff)
#define FD_EXPORT_FORMAT__CSV					((FD_EXPORT_FORMAT)0x0001)
#define FD_EXPORT_FORMAT__RQ					((FD_EXPORT_FORMAT)0x0002)
#define FD_EXPORT_FORMAT__HTML					((FD_EXPORT_FORMAT)0x0004)

#define FD_EXPORT__TO_MASK						((FD_EXPORT_FORMAT)0xff00)
#define FD_EXPORT__TO_FILE						((FD_EXPORT_FORMAT)0x0100)
#define FD_EXPORT__TO_CLIPBOARD					((FD_EXPORT_FORMAT)0x0200)

//////////////////////////////////////////////////////////////////

class fd_fd
{
private:
	friend class fd_fd_table;
	fd_fd(void);				// allocate via fd_fd_table
	~fd_fd(void);				// destroy via fd_fd_table


public:
	void				setFolders(poi_item * pPoi0, poi_item * pPoi1);		// set POIs, but does not load from disk
	util_error			loadFolders(util_background_thread_helper * pBGTH,
									bool bUsePreviousSoftmatchResult);

	void				buildVec(void);
	void				clearVec(void);

	FD_SHOW_HIDE_FLAGS	getShowHideFlags(void) const { return m_ShowHideFlags; };
	void				setShowHideFlags(FD_SHOW_HIDE_FLAGS);

	inline poi_item *	getRootPoi(int kItem)		const { return m_pPoiItemRoot[kItem]; };

	inline long			getItemCount(void)			const { return (long)m_vec.size(); };			// loadFolders() should have 
	inline fd_item *	getItem(int kItem)			const { return m_vec[kItem]; };					// been called before any of
	inline long			getStats(fd_item::Status s)	const { return m_table.getStats(s); };			// these can be used.

	fd_item *			findItemByRelativePath(const wxString & strRelativePath, long * pNdxInVec);

	wxString			formatStatsString(void)		const;
	
	util_error			exportVisibleItems(wxWindow * pParent,
										   FD_EXPORT_FORMAT expfmt,
										   const wxString * pStrPath = NULL);

	bool				haveChanges(void) const;

	void				updateProgressMessage(const wxString & strMsg);
	void				updateProgress(int n, int d);

private:
	friend class		_fd_fd_loadFolder;

	util_error			_loadFolder(int k,
									const fd_filter * pFdFilter,
									const fd_softmatch * pFdSoftmatch,
									bool bUsePreviousSoftmatchResult);

	util_error			_exportContents__rq(wxString & strResult);
	util_error			_exportContents_csv(wxString & strResult);
	util_error			_exportContents__html(wxString & strResult) const;
	wxString			_exportContents__html_rows(void) const;
	util_error			_getExportPathnameFromUser(wxWindow * pParent, FD_EXPORT_FORMAT expfmt);

	fd_item_table		m_table;			// table of fd_items in the tree (keyed by path fragment)
	poi_item *			m_pPoiItemRoot[2];	// handles to the 2 root folders we are comparing.

	util_background_thread_helper * m_pBGTH;

	poi_item *			m_pPoiSaveAsPathname;

	FD_SHOW_HIDE_FLAGS	m_ShowHideFlags;
	FD_EXPORT_FORMAT	m_expfmt;

private:
	void batchoutput_html(wxString & strOut,
						  wxString * pStrLabelA,
						  wxString * pStrLabelB) const;
	wxString batchoutput_html__div(wxString * pStrLabelA,
								   wxString * pStrLabelB) const;
	wxString _f2d_array(void) const;
	wxString _f2d_f(void) const;
	wxString _f2d_h(wxString * pStrLabelA, wxString * pStrLabelB) const;
	wxString _f2d_h_stats(void) const;

private:
	typedef std::vector<fd_item *>				TVec; // built from m_table.m_map after treewalk and keyed by index
	typedef TVec::const_iterator				TVecConstIterator;
	typedef TVec::iterator						TVecIterator;

	TVec				m_vec; // table of fd_items in the tree (keyed by position (since maps don't have getNth() (needed by wxListCtrl)))

#ifdef DEBUG
public:
	void				print_stats(const wxString & strMsg) const { m_table.print_stats(strMsg); };
	void				dump(int indent) const;
#endif
};

//////////////////////////////////////////////////////////////////

#endif//H_FD_FD_H
