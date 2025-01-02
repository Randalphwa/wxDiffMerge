// poi_item.h
// "pathname of interest" item -- provides a handle for a pathname.
// a handle that can be referenced by multiple windows when they
// share a file/directory.  also a place where a window can
// register for notification when the file/directory is changed
// (either by us in another window or by another app behind our backs).
//////////////////////////////////////////////////////////////////

#ifndef H_POI_ITEM_H
#define H_POI_ITEM_H

//////////////////////////////////////////////////////////////////

typedef enum {
		POI_T_NULL		= 0,
		POI_T_FILE		= 1,
		POI_T_DIR		= 2,
		POI_T_SHORTCUT	= 3,	// a MSW shortcut .lnk -- do not ifdef this
		POI_T_SYMLINK	= 4,	// a MAC/Linux symlink -- do not ifdef this
		POI_T__LIMIT	// must be last
} POI_TYPE;

//////////////////////////////////////////////////////////////////

class poi_item
{
private:
	friend class		poi_item_table;
	poi_item(void);							// should only be called by poi_item_table
	~poi_item(void);						// should only be called by poi_item_table

public:
	// WARNING: the isDir() and isFile() here actually touch the
	// WARNING: filesystem (calls stat() & etc) and are not necessarily
	// WARNING: the inverses of each other -- filename may be bogus or
	// WARNING: path may refer to a block device, in which case both
	// WARNING: would return false.
	// WARNING:
	// WARNING: the IsDir() method in wxFileName just checks for a blank
	// WARNING: entryname and extension -- is the path of the directory
	// WARNING: form -- our version actually tests for an existing directory.

	bool				isDir(void)					const;
	bool				isFile(void)				const;
#if defined(__WXMSW__)
	bool				isLnk(void)					const;
	util_error			get_lnk_target(util_shell_lnk ** ppLnk) const;
#endif
#if defined(__WXMAC__) || defined(__WXGTK__)
	bool				isSymlink(void)				const;
	util_error 			get_symlink_target_raw(char * pBuf, int bufLen) const;
	util_error			get_symlink_target(wxString & rStrTarget) const;
#endif

	POI_TYPE getPoiType(void) const;
	static POI_TYPE getPoiType(const poi_item * poi);

	bool				tryToMakeWritable(void)		const;

	inline fim_ptable *	getPTable(void)				const { return m_pPTable; };
	void				setPTable(fim_ptable * pPTable);

	wxString			getFileSizeAsString(void) const;
	util_error			getFileSize(wxULongLong * pULL) const;
	util_error			getDateTimeModified(wxDateTime * pDTM) const;
	util_error			compareFileExact(const poi_item * pPoiItem, int * pResult); // result is {-1 error, 0 diff, 1 equal}

	util_error			setFileName(const wxFileName & f);
	const wxFileName	getFileName(void)			const { return m_filename; };
	
	wxString			getFullPath(void)			const { wxASSERT(m_filename.IsOk()); return m_filename.GetFullPath(); };
	wxString			getFullName(void)			const { wxASSERT(m_filename.IsOk()); return m_filename.GetFullName(); };

	void				setPoiEditBuffer(poi_item * pPoi);
	poi_item *			getPoiEditBuffer(void)		const { return m_pPoiEditBuffer; };

	void				setPoiEditSrc(poi_item * pPoi);
	poi_item *			getPoiEditSrc(void)			const { return m_pPoiEditSrc; };

	// we currently only use the saveCount to try to keep track of how many
	// times the merge result has been saved so that we can properly set the
	// application exit status.  we don't bother making more use than this
	// because we still have to check for other applications modifying files
	// behind our backs.
	void				incrementSaveCount(void)	      { m_saveCount++; };
	unsigned int		getSaveCount(void)			const { return m_saveCount; };
	
private:
	wxFileName			m_filename;
	fim_ptable *		m_pPTable;			// piecetable if POI is open in one or more file-views.

	poi_item *			m_pPoiEditBuffer;	// POI (tempfile,piecetable) of edit buffer on this document (valid when this document is a VIEW and being edited somewhere).
	poi_item *			m_pPoiEditSrc;		// POI (original file, piecetable) of the reference document (valid when this document is an edit buffer; refers to VIEW).

	unsigned int		m_saveCount;		// how many times have we written the file using fim_ptable:_attemptSave().

#ifdef DEBUG
public:
	void				dump(int indent) const;
#endif
};

//////////////////////////////////////////////////////////////////

#endif//H_POI_ITEM_H
