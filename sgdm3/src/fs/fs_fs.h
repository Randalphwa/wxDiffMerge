// fs_fs.h -- a "file set" -- a set of files -- the set of documents
// in a 2-way "file diff" or a 3-way "file merge", for example.
// contains everything needed to manage 1 or more documents as a group.
// may have one or more views onto it.
//////////////////////////////////////////////////////////////////

#ifndef H_FS_FS_H
#define H_FS_FS_H

//////////////////////////////////////////////////////////////////

class fs_fs
{
private:
	friend class fs_fs_table;
	fs_fs(const cl_args * pArgs);
	~fs_fs(void);

public:
	void						setFiles(poi_item * pPoi0, poi_item * pPoi1, poi_item * pPoi2=NULL);
	poi_item *					getPoi(long kSync, PanelIndex kPanel) const;
	fim_ptable *				getPTable(long kSync, PanelIndex kPanel) const;

	util_error					loadFiles(wxWindow * pParent);
#ifdef FEATURE_BATCHOUTPUT
	util_error					loadFiles_without_asking(void);
#endif
	bool						reloadFiles(wxWindow * pParent, bool bForceReload=false);

	void						makeLabelForExportUnifiedHeader(long kSync,
																wxString & strLabelA,
																wxString & strLabelB,
																const wxString & strTitleA=wxEmptyString,
																const wxString & strTitleB=wxEmptyString) const;
	util_error					writeStringInEncodingToFile(long kSync,
															const poi_item * pPoiDestination,
															const wxString & strOutput,
															bool bForceUTF8) const;
	

	bool						setupEditing(void);
	void						forceAutoSaveNow(void);
	void						deleteAutoSaveFile(void);

	inline bool					getLoaded(void)					const { return m_bLoaded; };
	inline util_error			getLoadError(void)				const { return m_errLoad; };

	inline const rs_ruleset *	getRuleSet(void)				const { return m_pRuleSet; };
	inline int					getNrTops(void)					const { return m_nrTops; };

	inline poi_item **			getPoiRefTable(void)			{ return m_pPoiItem; };				// this is &poi[SYNC_VIEW][0..2] only
	inline poi_item **			getPoiRefTable(int kStart)		{ return &m_pPoiItem[kStart]; };	// this is &poi[SYNC_VIEW][k] only

	void						changeRuleset(const rs_ruleset * pRS);

	void						addRsChangeCB(util_cbl_fn pfn, void * pData)	{ m_cblRsChange.addCB(pfn,pData); };
	void						delRsChangeCB(util_cbl_fn pfn, void * pData)	{ m_cblRsChange.delCB(pfn,pData); };

	inline poi_item *			getResultPOI(void)				const { return m_pPoiResult; };

	util_error					fileSave(wxWindow * pParent);
	util_error					fileSaveAs(wxWindow * pParent);

	wxString					dumpSupportInfo(const wxString & strIndent) const;

	bool						getReadOnly(void)				const { return m_bReadOnly; };

	void						rebindPoi(poi_item * pPoiOld, poi_item * pPoiNew);

 private:
	bool						_askToProceedIfFileHasChangedOnDisk(wxWindow * pParent, fim_ptable * pPTable) const;
	util_error					_getSaveAsPathname(wxWindow * pParent, poi_item ** ppPoiNew) const;
	util_error					_fileSaveAs(wxWindow * pParent, poi_item * pPoiOldT1, poi_item * pPoiNew);
	void						_bom_and_nul_byte_in_file_error_dialog(wxWindow * pParent, int kPanel) const;
	bool						_nul_byte_in_file_error_dialog(wxWindow * pParent, int kPanel) const;
	void						_do_raw_comparison(wxString & strResultMessage) const;
	bool						_try_load_one_file(wxWindow * pParent, int kPanel,
												   RS_ENCODING_STYLE encStyle,
												   int nrEnc, util_encoding aEnc[]);

	util_error					m_errLoad;
	bool						m_bLoaded;						// sanity check
	bool						m_bReadOnly;					// /ro used on command line for this fs_fs
	int							m_nrTops;						// number of top panels defined (2 or 3)
	const rs_ruleset *			m_pRuleSet;						// we do not own this

	poi_item *					m_pPoiItem[__NR_TOP_PANELS__];		// handles to the files we are comparing
	poi_item *					m_pPoiItemEdit;

	fim_ptable *				m_pPTable [__NR_TOP_PANELS__];		// piecetable of each file
	fim_ptable *				m_pPTableEdit;

	util_cbl					m_cblRsChange;	// callback list for objects wanting to know when our ruleset changes.

	poi_item *					m_pPoiResult;					// POI for /result:<pathname> given on command-line (only valid when merge and first window)

#ifdef DEBUG
public:
	void						dump(int indent) const;
#endif
};

//////////////////////////////////////////////////////////////////

#endif//H_FS_FS_H
