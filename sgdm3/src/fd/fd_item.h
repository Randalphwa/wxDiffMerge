// fd_item.h
// a line-item in a folder diff window representing a pair of
// files/directories within the 2 trees.
//////////////////////////////////////////////////////////////////

#ifndef H_FD_ITEM_H
#define H_FD_ITEM_H

//////////////////////////////////////////////////////////////////

class fd_item
{
public:
	typedef enum _status { FD_ITEM_STATUS_UNKNOWN=0,	// unknown, not yet computed
						   FD_ITEM_STATUS_ERROR,		// error, see m_err
						   FD_ITEM_STATUS_MISMATCH,		// file vs folder mismatch (shouldn't happen since wx appends '/' to folder entrynames)
						   FD_ITEM_STATUS_BOTH_NULL,	// neither set (probably error)

						   FD_ITEM_STATUS_SAME_FILE,	// they are the same (physical) file
						   FD_ITEM_STATUS_IDENTICAL,	// files are equal
						   FD_ITEM_STATUS_EQUIVALENT,	// soft-match equivalent
						   FD_ITEM_STATUS_QUICKMATCH,	// quick-match equivalent
						   FD_ITEM_STATUS_DIFFERENT,	// files are different

						   FD_ITEM_STATUS_FOLDERS,		// both are folders

						   FD_ITEM_STATUS_FILE_NULL,	// left side only has file; right side null
						   FD_ITEM_STATUS_NULL_FILE,	// left side null, right side has file

						   FD_ITEM_STATUS_FOLDER_NULL,	// left side only has folder; right side null
						   FD_ITEM_STATUS_NULL_FOLDER,	// left side null, right side has folder

						   FD_ITEM_STATUS_SHORTCUT_NULL,	// left side only has .lnk ; right side null
						   FD_ITEM_STATUS_NULL_SHORTCUT,	// left side null, right side has .lnk
						   FD_ITEM_STATUS_SHORTCUTS_SAME,	// the same file
						   FD_ITEM_STATUS_SHORTCUTS_EQ,		// both are .lnk shortcuts and are equal/equivalent
						   FD_ITEM_STATUS_SHORTCUTS_NEQ,	// both are .lnk shortcuts and are different

						   FD_ITEM_STATUS_SYMLINK_NULL,		// left side only has symlink; right side null
						   FD_ITEM_STATUS_NULL_SYMLINK,		// left side null, right side has symlink
						   FD_ITEM_STATUS_SYMLINKS_SAME,	// the same file
						   FD_ITEM_STATUS_SYMLINKS_EQ,		// both are symlinks and the text of their targets are identical
						   FD_ITEM_STATUS_SYMLINKS_NEQ,		// both are symlinks and are different

						   __FD_ITEM_STATUS__COUNT__	// must be last
	} Status;
	static const wxChar *	getStatusText(Status status)
		{
			// get a printable string for this status.
			static const wxChar * s_table[__FD_ITEM_STATUS__COUNT__] = 
				{	_T("Unknown"),
					_T("Error"),
					_T("Mismatch"),
					_T("NULL"),

					_T("Same File"),
					_T("Identical"),
					_T("Equivalent"),
					_T("Assumed Equal"),
					_T("Different"),

					_T("Folders"),

					_T("Peerless File"),
					_T("Peerless File"),

					_T("Peerless Folder"),
					_T("Peerless Folder"),

					_T("Peerless Shortcut"),
					_T("Peerless Shortcut"),
					_T("Same Shortcut"),
					_T("Equivalent Shortcuts"),
					_T("Different Shortcuts"),

					_T("Peerless Symlink"),
					_T("Peerless Symlink"),
					_T("Same Symlink"),
					_T("Equivalent Symlinks"),
					_T("Different Symlinks"),

				};
			return s_table[status];
		};

private:
	friend class fd_item_table;
	fd_item(fd_item_table * pFdItemTable,
			const wxString * pRelativePathname0, const wxString * pRelativePathname1,
			poi_item * pPoiItem0=NULL, poi_item * pPoiItem1=NULL);	// allocate via fd_item_table
	virtual ~fd_item(void);											// destroy via fd_item_table

public:
	void				setPoiItem(int kItem, poi_item * pPoiItemK);
	inline poi_item *	getPoiItem(int kItem)				const { return m_pPoiItem[kItem]; };

	const wxString &	getRelativePathname(int kItem)		const { return m_relativePathname[kItem]; };
	void				setRelativePathname(int kItem, const wxString * pRelativePathname);

	util_error			getError(void)						const { return m_err; };
	void				setError(util_error & e)			      { m_err = e; };

	Status				computeStatus(const fd_filter * pFdFilter,
									  const fd_quickmatch * pFdQuickmatch,
									  const fd_softmatch * pFdSoftmatch,
									  const rs_ruleset_table * pRsRuleSetTable,
									  bool bUsePreviousSoftmatchResult);
	Status				getStatus(void);

	void				setStale(void);			// set transient flags while rebuilding
	bool				deleteStale(void);		// cleanup after rebuilding, return stale flag

	bool				getSoftQuickMatchSummaryMessage(wxString & strMsg) const;

	wxDateTime			getDTM(int k) const { return m_dtm[k]; };

	util_error			clone_item_on_disk_in_other_folder(bool bRecursive);
	util_error			overwrite_item_on_disk(bool bReplaceLeft);

private:
	void				_setStatus(Status newValue);
	void				_computeStatus(const fd_filter * pFdFilter,
									   const fd_quickmatch * pFdQuickmatch,
									   const fd_softmatch * pFdSoftmatch,
									   const rs_ruleset_table * pRsRuleSetTable,
									   bool bUsePreviousSoftmatchResult);
	void				_computeFileDiff(const fd_filter * pFdFilter,
										 const fd_quickmatch * pFdQuickmatch,
										 const fd_softmatch * pFdSoftmatch,
										 const rs_ruleset_table * pRsRuleSetTable,
										 bool bUsePreviousSoftmatchResult);

#if defined(__WXMSW__)
	void				_computeShortcutDiff(void);
#endif
#if defined(__WXMAC__) || defined(__WXGTK__)
	void				_computeSymlinkDiff(void);
	util_error			_my_clone_or_overwrite_symlink(int k_src, int k_dest, bool bOverwrite);
#endif	
	util_error			_clone_iterate(int k_src, int k_dest);
	wxString			_make_clone_target_pathname(int k_src, int k_dest);
	util_error			_my_clone_or_overwrite_file(int k_src, int k_dest, bool bOverwrite);

	fd_item_table *		m_pFdItemTable;			// our container
	wxString			m_relativePathname[2];	// portion of pathname below both roots
	poi_item *			m_pPoiItem[2];
	Status				m_status;
	util_error			m_err;
	bool				m_bStale[2];			// transient flag while rebuilding tree
	wxDateTime			m_dtm[2];

	wxString			m_strSoftQuickMatchInfo;	// tool-tip-like info when soft-match or quick-match equivalent
#ifdef DEBUG
public:
	void				dump(int indent) const;
#endif
};

//////////////////////////////////////////////////////////////////

#endif//H_FD_ITEM_H
