// fim_ptable.h -- piecetable -- represents document content as
// a list of fragments.
//
// (we do not use STL's <list> because we want the next/prev links
// in the node rather than external in an iterator.)
//////////////////////////////////////////////////////////////////

#ifndef H_FIM_PTABLE_H
#define H_FIM_PTABLE_H

//////////////////////////////////////////////////////////////////

class fim_ptable
{
private:
	friend class fim_ptable_table;
	fim_ptable(fim_buf * pFimBuf=NULL);					// allocate via fim_ptable_table
	~fim_ptable(void);									// destroy via fim_ptable_table

public:
	util_error				loadFile(poi_item * pPoiItem,
									 bool bSniffEncodingBOM,
									 util_encoding enc);				// load from disk file

	util_error				reload(void);													// reload from disk file
	util_error				attemptSave(poi_item * pPoiItem);							// save contents to disk file
	util_error				attemptSaveWithForceIfNecessary(wxWindow * pParent, poi_item * pPoiItem);

	void					rebindPoi(poi_item * pPoiNew);
	void					updateDateTimes(void);

	inline wxDateTime		getDateTimeAsLoaded(void)		const { return m_dtmAsLoaded; };
	inline wxDateTime		getDateTimeLastChecked(void)	const { return m_dtmChecked; };
	bool					hasChangedOnDiskSinceLoading(util_error & ue);
	bool					hasChangedOnDiskSinceLastChecked(util_error & ue, bool bUpdate);

	fl_fl *					getLayout(void);

	inline poi_item *		getPoiItem(void)			const { return m_pPoiItem; };	// may validly return NULL (if never loaded, but created in-memory and not saved)
	inline util_encoding	getEncoding(void)			const { return m_enc; };
	inline bool				getHadUnicodeBOM(void)		const { return m_bHadUnicodeBOM; };

	inline fim_eol_mode		getEolMode(void)			const { return m_eolMode; };
	void					setEolMode(fim_eol_mode mode)     { m_eolMode = mode; };

	wxString				dumpSupportInfo(const wxString & strIndent) const;

public:
	void					insertText(fim_offset docPos, fr_prop prop, const wxChar * pData, fim_length lenData);
	void					insertText(fim_offset docPos, fr_prop prop, const wxChar * pData);
	void					insertText(fim_offset docPos, fr_prop prop, const wxString & pData);

	void					appendText(fr_prop prop, const wxChar * pData, fim_length lenData);

	void					deleteText(fim_offset docPos, fim_length lenData);

	void					replaceText(fim_offset docPos, fim_length lenData, const wxString & pData,                      fr_prop prop);
	void					replaceText(fim_offset docPos, fim_length lenData, const wxChar * pData, fim_length lenNewData, fr_prop prop);

	void					applyPatchSet(fim_patchset * pPatchSet);

	void					clone(const fim_ptable * pPTableSrc, fr_prop propMask);
	void					enableAutoSave(void);

	void					re_clone(const fim_ptable * pPTableSrc, fr_prop propMask);

	void					turnOnProp(fim_frag * pFrag, fr_offset offset, fim_length lenData, fr_prop prop);	// TODO convert to docPos-style interface
	void					turnOffProp(fim_frag * pFrag, fr_offset offset, fim_length lenData, fr_prop prop);	// TODO convert to docPos-style interface

	fim_crec::TAID			beginTransaction(void);
	void					endTransaction(fim_crec::TAID taid);

	bool					undo(fim_offset * pDocPos);
	bool					redo(fim_offset * pDocPos);

	inline const fim_frag *	getFirstFrag(void) const { return m_pFragHead; };

	bool					hasFinalEOL(void) const;

#if 0	// it works, but we don't need it right now.
	fim_length				measureText(const fim_frag * pFrag, fr_offset offset,					// measure between [begin,end)
										const fim_frag * pFragEnd, fr_offset offsetEnd) const;		// does not include end point.
#endif

	void					forceAutoSaveNow(void);

	inline pt_stat			getPtStat(void) const { return _computeStat(); };

public:
	void					addChangeCB(util_cbl_fn pfn, void * pData)	{ m_cblFrag.addCB(pfn,pData); };
	void					delChangeCB(util_cbl_fn pfn, void * pData)	{ m_cblFrag.delCB(pfn,pData); };
	void					addStatusCB(util_cbl_fn pfn, void * pData)	{ m_cblStat.addCB(pfn,pData); };
	void					delStatusCB(util_cbl_fn pfn, void * pData)	{ m_cblStat.delCB(pfn,pData); };

private:
	util_error				_loadFile(poi_item * pPoiItem, bool bSniffEncodingBOM, util_encoding enc);	// load from disk file

	util_error				_reload(void);										// reload from disk file
	util_error				_import_conv(util_encoding enc, const byte * pBufSrc, wxChar ** pWideBuffer, size_t * pLenWideBuffer);

	void					_insertText(fim_offset docPos, 
										const wxChar * pData, fim_length lenData,
										fr_prop prop);

	void					_appendText(fr_prop prop, const wxChar * pData, fim_length lenData);

	void					_deleteText(fim_offset docPos, fim_length lenData);

	void					_replaceText(fim_offset docPos, fim_length lenOldData,
										 const wxChar * pData, fim_length lenNewData,
										 fr_prop prop);

	void					_applyPatchSet(fim_patchset * pPatchSet);

	void					_clone(const fim_ptable * pPTableSrc, fr_prop propMask);
	void					_re_clone(const fim_ptable * pPTableSrc, fr_prop propMask);
	void					_delete_all(void);
	void					_setPoiItem(poi_item * pPoiItem);

	void					_turnOnOrOffProp(bool bOn, fim_frag * pFrag, fr_offset offset, fim_length lenData, fr_prop prop);

	bool					_undo(fim_offset * pDocPos);
	bool					_redo(fim_offset * pDocPos);

	pt_stat					_computeStat(void) const;

	void					_applyCRec            (const fim_crec * pCRec);
	void					_applyCRec_text_insert(const fim_crec * pCRec, fim_frag * pFragHint=NULL, fr_offset offsetFragHint=0);
	void					_applyCRec_text_delete(const fim_crec * pCRec, fim_frag * pFragHint=NULL, fr_offset offsetFragHint=0);
	void					_applyCRec_prop       (const fim_crec * pCRec, fim_frag * pFragHint=NULL, fr_offset offsetFragHint=0);

	void					_linkNewFragBeforeFrag(fim_frag * pFrag, fim_frag * pFragNew, fr_op fop=FOP_INSERT_BEFORE, bool bCoalesce=true);
	void					_linkNewFragAfterFrag (fim_frag * pFrag, fim_frag * pFragNew, fr_op fop=FOP_INSERT_AFTER,  bool bCoalesce=true);
	void					_linkNewFrag          (fim_frag * pFrag);
	void					_unlinkFrag           (fim_frag * pFrag,                      fr_op fop=FOP_DELETE_SELF,   bool bCoalesce=true);
	void					_split                (fim_frag * pFrag, fr_offset offset);
	bool					_coalesceLeft         (fim_frag * pFrag);
	bool					_coalesceRight        (fim_frag * pFrag);
	void					_setProp              (fim_frag * pFrag, fr_prop prop);

public:
	fim_offset				getAbsoluteOffset(const fim_frag * pFrag, fr_offset offsetFrag) const;
	void					getFragAndOffset(fim_offset offsetDoc, fim_frag ** ppFrag, fr_offset * pOffsetFrag) const;
	fim_length				getAbsoluteLength(void);

private:
	void					_getFragAndOffsetFromHintOrDocOffset(fim_frag * pFragHint, fr_offset offsetFragHint, fim_offset offsetDoc,
																 fim_frag ** ppFrag, fr_offset * pOffsetFrag) const;
	void					_fixupFragAndOffset(fim_frag ** ppFrag, fr_offset * pOffsetFrag) const;
	fim_eol_mode			_guessEolMode(void) const;

	void					_considerAutoSave(void);
	util_error				_attemptSave(poi_item * pPoiItem);
	void					_resetAutoSave(void);
	void					_createSaveBuffer(wxString & rString);

private:
	fim_buf	*				m_pFimBuf;		// growbuf for raw document content (without ordering) for frags to reference
	fim_crecvec				m_crecvec;		// undo/redo stack
	util_cbl				m_cblFrag;		// callback list for views wanting to know when document content changes [CB's receive util_cbl_arg(const fim_frag *, fr_op) ]
	util_cbl				m_cblStat;		// callback list for views wanting to know when document status changes [CB's receive util_cbl_arg(,pt_stat)]

	fim_frag *				m_pFragHead;	// head/tail of fragment list
	fim_frag *				m_pFragTail;

	poi_item *				m_pPoiItem;		// reference to the underlying file -- if we have one [see also poi_item::getPTable()]
	wxDateTime				m_dtmAsLoaded;	// the date/timestamp on the file when we loaded it.
	wxDateTime				m_dtmChecked;	// the date/timestamp on the file when we last asked the user to reload it.
	fl_fl *					m_pFlFl;		// we own the layout, since it is sharable just like us.
	util_encoding			m_enc;			// remember the character encoding that we used to load the file (incase we need to save it later)
	bool					m_bHadUnicodeBOM;
	fim_eol_mode			m_eolMode;		// the EOL style for this file
	int						m_autoSave;

	fim_length				m_cachedAbsoluteLength;
	bool					m_bCachedAbsoluteLengthValid;

#ifdef DEBUG
public:
	void					dump(int indent) const;

#if 0
	void					_cb_frag_test(const fim_frag * pFrag, fr_op fop);
	unsigned long			_count_frags(void) const;
	fim_length				_get_doc_length(void) const;
	long					_test_compact(void) const;
	wxString				_get_doc(void) const;
	
	void					test01(void);
	void					test02(void);
#endif
#endif
};

//////////////////////////////////////////////////////////////////

#endif//H_FIM_PTABLE_H
