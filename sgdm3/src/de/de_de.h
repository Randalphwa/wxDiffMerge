// de_de.h -- diff-engine
//////////////////////////////////////////////////////////////////

#ifndef H_DE_DE_H
#define H_DE_DE_H

//////////////////////////////////////////////////////////////////
// TSet_Hash
// 
// we construct a hash table to hash every line in all documents
// into a unique token.  then we can say lines in 2 different
// documents are equal iff they have the same unique token.
// we use a line's set iterator as the unique token.

typedef std::set<wxString>					TSet_Hash;
typedef TSet_Hash::iterator					TSet_HashIterator;
typedef TSet_Hash::value_type				TSet_HashValue;
typedef std::pair<TSet_HashIterator,bool>	TSet_HashInsertResult;

//////////////////////////////////////////////////////////////////
// de_line
//
// since the layout (fl_fl) and the lines (fl_line) within it are
// shared by all views onto the document, we cannot have (per-view,
// per-diff, per-etc) private data on the lines.  we store line-specific
// private data in de_line.  the fl_line has a vector of "slots" that
// link to us.  [the de_de asks for a slot index for each panel, and
// we store our de_line in the corresponding slot in each fl_line.]

class de_line
{
public:
	friend class de_css_src_string;

	de_line(const fl_line * pFlLine);
	~de_line(void);

	inline void					setUIDs(TSet_HashIterator itUID, TSet_HashIterator itExactUID, unsigned long cacheUIDParams)
		{
			m_bValidUID = true;
			m_itUID = itUID;
			m_itExactUID = itExactUID;
			m_cacheUIDParams = cacheUIDParams;
		};
	inline void					unsetUIDs(void)						{ m_bValidUID = false; m_cacheUIDParams = 0x00; };
	inline unsigned long		getCachedUIDParams(void) const		{ return m_cacheUIDParams; };

	inline void					setOmitted(bool bOmitted)			{ m_bOmitted = bOmitted; };

	inline const fl_line *		getFlLine(void) const				{ return m_pFlLine;	};
	inline TSet_HashIterator	getUID(void)	const				{ wxASSERT_MSG( (m_bValidUID), _T("Coding Error") ); return m_itUID; };
	inline TSet_HashIterator	getExactUID(void) const				{ wxASSERT_MSG( (m_bValidUID), _T("Coding Error") ); return m_itExactUID; };
	inline bool					getUIDValid(void) const				{ return m_bValidUID; };

	inline bool					isOmitted(void)	const				{ return (m_bOmitted); };

	inline void					setRowNr(long kSync, size_t rowNr)	{ m_rowNr[kSync] = rowNr; };
	inline size_t				getRowNr(long kSync) const			{ return m_rowNr[kSync]; };

	const rs_context *			setStringsAndComputeContext(const rs_ruleset * pRS, const rs_context * pCTX, bool bCacheRSChanged, bool * pbCacheWasValid);

	void						appendSpan(const wxChar * sz, const rs_context * pCTX, size_t offset, size_t len);
	void						appendSpan(const rs_context * pCTX, size_t offset, size_t len);
	inline const de_span *		getFirstSpan(void) const			{ return m_pSpanFirst; };
	inline const rs_context *	getContextEOL(void) const			{ return m_pCTXEOL; };

	const wxString &			getStrLine(void) const				{ return m_strLine; };
	const wxString &			getStrEOL(void) const				{ return m_strEOL; };

	inline bool					hasMark(long kSync) const				{ return (m_pDeMark[kSync] != NULL); };
	inline de_mark *			getMark(long kSync) const				{ wxASSERT_MSG((hasMark(kSync)), _T("Coding Error")); return m_pDeMark[kSync]; };
	inline void					setMark(long kSync, de_mark * pMark)	{ m_pDeMark[kSync] = pMark; };

	inline void					setNdxLineCmp(long ndx)				{ m_ndxLineCmp = ndx; };
	inline long					getNdxLineCmp(void) const			{ return m_ndxLineCmp; };

private:
	const fl_line *				m_pFlLine;					// we do not own this.  a back-ptr to the line we refer to.
	TSet_HashIterator			m_itUID;					// hash for this line.  used while running CSS algorithm.  hash value w/o whitespace/eol/case as per global flags.
	TSet_HashIterator			m_itExactUID;				// hash value for exact content of this line (ignoring global flags).
	bool						m_bValidUID;				// true iff m_itUID & m_itExactUID are set.
	bool						m_bOmitted;					// true iff line omitted from diff comparison

	wxString					m_strLine;
	wxString					m_strEOL;
	de_span *					m_pSpanFirst;
	de_span *					m_pSpanLast;
	const rs_context *			m_pCTXEOL;					// we do not own this

	size_t						m_rowNr[__NR_SYNCS__];		// row number in display lists [index into m_vecDisplay[kSync]]

	long						m_cacheEditOpCounter;		// see de_line::setStringsAndComputeContext()
	unsigned long				m_cacheUIDParams;			// see de_de::_build_vector()

	de_mark *					m_pDeMark[__NR_SYNCS__];	// valid only-if a mark on line [for de_lines in T0,T2 both might be used, for T1-->View, for ED-->Edit]
	long						m_ndxLineCmp;

#ifdef DEBUG
public:
	void						dump(int indent) const;
#endif
};

//////////////////////////////////////////////////////////////////
// TVector_LineCmp
//
// we construct a vector that maps from a line number to info for that
// line so we can use the de_css algorithm. note that this line number
// is from the point of view of the de_css algorithm -- it may or may
// not match the line number in the document -- for example, if the
// active ruleset permits lines to be COMPLETELY OMITTED BEFORE the
// diff-engine runs.

typedef std::vector<de_line *>				TVector_LineCmp;		// we do not own pointers
typedef TVector_LineCmp::value_type			TVector_LineCmpValue;

//////////////////////////////////////////////////////////////////
// de_css_src_lines
//
// we define a little helper class to access our vector and yet
// implement the de_css_src interface (required by the diff-engine
// algorithm).  we use a sub-class so that we can hide some
// subscripting tricks.

class de_css_src_lines : public de_css_src
{
public:
	de_css_src_lines(TVector_LineCmp * pVecA, TVector_LineCmp * pVecB)
		: m_pVecA(pVecA), m_pVecB(pVecB)
		{};
		
	virtual ~de_css_src_lines(void) {};

	virtual long		getLenA(void)			const	{ return (long)(m_pVecA->size()); };
	virtual long		getLenB(void)			const	{ return (long)(m_pVecB->size()); };
	virtual bool		equal(long a, long b)	const	{ return ( (*m_pVecA)[a]->getUID() == (*m_pVecB)[b]->getUID() ); };

	virtual bool		identical_A(long a1, long a2, long len)	const
		{
			for (long k=0; k<len; k++)
				if ((*m_pVecA)[a1+k]->getUID() != (*m_pVecA)[a2+k]->getUID())
					return false;
			return true;
		};
	
	virtual bool		identical_B(long b1, long b2, long len)	const
		{
			for (long k=0; k<len; k++)
				if ((*m_pVecB)[b1+k]->getUID() != (*m_pVecB)[b2+k]->getUID())
					return false;
			return true;
		};

private:
	TVector_LineCmp *	m_pVecA;
	TVector_LineCmp *	m_pVecB;
};

//////////////////////////////////////////////////////////////////
// de_css_src_string
//
// we define a css_src that will let us diff 2 strings so that we
// can do intra-line diffs.

class de_css_src_string : public de_css_src
{
public:
	de_css_src_string(const de_line * pDeLineA, const de_line * pDeLineB, bool bMatchEOLs);
	virtual ~de_css_src_string(void) {};

	virtual long				getLenA(void)			const { return m_lenA + m_lenAeol; };
	virtual long				getLenB(void)			const { return m_lenB + m_lenBeol; };
	virtual bool				equal(long a, long b)	const { return _getCharA(a) == _getCharB(b); };

	virtual bool				identical_A(long a1, long a2, long len)	const
		{
			for (long k=0; k<len; k++)
				if (_getCharA(a1+k) != _getCharA(a2+k))
					return false;
			return true;
		};

	virtual bool				identical_B(long b1, long b2, long len)	const
		{
			for (long k=0; k<len; k++)
				if (_getCharB(b1+k) != _getCharB(b2+k))
					return false;
			return true;
		};

private:
	inline const wxChar 		_getCharA(long a) const { return ((a < m_lenA) ? m_szA[a] : m_szAeol[a-m_lenA]); };
	inline const wxChar			_getCharB(long b) const { return ((b < m_lenB) ? m_szB[b] : m_szBeol[b-m_lenB]); };

	const de_line *				m_pDeLineA;
	const de_line *				m_pDeLineB;
	bool						m_bMatchEOLs;

	const wxChar *				m_szA;
	const wxChar *				m_szB;
	const wxChar *				m_szAeol;
	const wxChar *				m_szBeol;

	long						m_lenA;
	long						m_lenB;
	long						m_lenAeol;
	long						m_lenBeol;
};
//////////////////////////////////////////////////////////////////

class de_css_src_simple_strings : public de_css_src
{
 public:
	de_css_src_simple_strings(const wxString & strA, const wxString & strB);
	virtual ~de_css_src_simple_strings(void) {};

	virtual long 				getLenA(void)			const { return m_lenA; };
	virtual long 				getLenB(void)			const { return m_lenB; };
//	virtual bool				equal(long a, long b)	const { return _getCharA(a)==_getCharB(b); };
	virtual bool				equal(long a, long b)	const { return m_szA[a]==m_szB[b]; };

	virtual bool				identical_A(long a1, long a2, long len)	const
		{
			for (long k=0; k<len; k++)
				if (_getCharA(a1+k) != _getCharA(a2+k))
					return false;
			return true;
		};

	virtual bool				identical_B(long b1, long b2, long len)	const
		{
			for (long k=0; k<len; k++)
				if (_getCharB(b1+k) != _getCharB(b2+k))
					return false;
			return true;
		};

private:
	inline const wxChar			_getCharA(long a) const { return m_szA[a]; };
	inline const wxChar			_getCharB(long b) const { return m_szB[b]; };

	const wxChar *				m_szA;
	const wxChar *				m_szB;

	long						m_lenA;
	long						m_lenB;
};


//////////////////////////////////////////////////////////////////
// de_span
//
// a de_line represents an entire line in a source document (and
// there's a correspondence to a fl_line.
// 
// a de_span represents a portion of a line that has similar
// properties/attibutes -- such as the RuleSet context and
// intra-line diff results.  we are *like* fl_run's, but *NOT*.
// fl_run's are created in response to the piecetable fragmentation
// on the line.  de_span's are created in response to changes in
// the context or diff results -- we do not also model the frags.

class de_span
{
public:
	friend class de_line;
	
	de_span(const rs_context * pCTX, size_t offset, size_t len)
		: m_next(NULL), m_prev(NULL),
		  m_pCTX(pCTX),
		  m_offset(offset), m_len(len)
		{
			DEBUG_CTOR(T_DE_SPAN,L"de_span");
		};

	~de_span(void)
		{
			DEBUG_DTOR(T_DE_SPAN);
		};

	inline de_span *			getNext(void)		const	{ return m_next; };
	inline de_span *			getPrev(void)		const	{ return m_prev; };

	inline size_t				getOffset(void)		const	{ return m_offset; };
	inline size_t				getLen(void)		const	{ return m_len; };

	inline const rs_context *	getContext(void)	const	{ return m_pCTX; };

private:
	de_span *			m_next;
	de_span *			m_prev;

	const rs_context *	m_pCTX;		// we do not own this
	size_t				m_offset;
	size_t				m_len;
};

//////////////////////////////////////////////////////////////////
// de_row
//
// defines info for a screen row in the display list.  that is,
// we tie together one de_line for each panel.

class de_row
{
public:
	de_row(const de_sync * pSync, long offset, bool bGap,
		   de_line * pT0=NULL, de_line * pT1=NULL, de_line * pT2=NULL)
		: m_pSync(pSync),
		  m_offset(offset),
		  m_bEOF(false),
		  m_bGap(bGap),
		  m_pMark(NULL),
		  m_rowPrevChange(-1), m_rowNextChange(-1),
		  m_rowPrevConflict(-1), m_rowNextConflict(-1)
		{
			m_pDeLines[PANEL_T0] = pT0;
			m_pDeLines[PANEL_T1] = pT1;
			m_pDeLines[PANEL_T2] = pT2;

			DEBUG_CTOR(T_DE_ROW,L"de_row");
		};

	de_row(const de_sync * pSync, bool bGap)
		: m_pSync(pSync),
		  m_offset(0),
		  m_bEOF(true),
		  m_bGap(bGap),
		  m_pMark(NULL),
		  m_rowPrevChange(-1), m_rowNextChange(-1),
		  m_rowPrevConflict(-1), m_rowNextConflict(-1)
		{
			m_pDeLines[PANEL_T0] = NULL;
			m_pDeLines[PANEL_T1] = NULL;
			m_pDeLines[PANEL_T2] = NULL;			

			DEBUG_CTOR(T_DE_ROW,L"de_row");
		};

	de_row(const de_sync * pSync, de_mark * pMark, bool bGap)
		: m_pSync(pSync),
		  m_offset(0),
		  m_bEOF(false),
		  m_bGap(bGap),
		  m_pMark(pMark),
		  m_rowPrevChange(-1), m_rowNextChange(-1),
		  m_rowPrevConflict(-1), m_rowNextConflict(-1)
		{
			m_pDeLines[PANEL_T0] = NULL;
			m_pDeLines[PANEL_T1] = NULL;
			m_pDeLines[PANEL_T2] = NULL;			

			DEBUG_CTOR(T_DE_ROW,L"de_row");
		};

	de_row(const de_row & row)
		{
			m_pSync					= row.m_pSync;
			m_offset				= row.m_offset;
			m_bEOF					= row.m_bEOF;
			m_bGap					= row.m_bGap;
			m_pMark					= row.m_pMark;
			m_pDeLines[PANEL_T0]	= row.m_pDeLines[PANEL_T0];
			m_pDeLines[PANEL_T1]	= row.m_pDeLines[PANEL_T1];
			m_pDeLines[PANEL_T2]	= row.m_pDeLines[PANEL_T2];
			m_rowPrevChange			= row.m_rowPrevChange;
			m_rowNextChange			= row.m_rowNextChange;
			m_rowPrevConflict		= row.m_rowPrevConflict;
			m_rowNextConflict		= row.m_rowNextConflict;
			
			DEBUG_CTOR(T_DE_ROW,L"de_row");
		};
	
	~de_row(void)
		{
			DEBUG_DTOR(T_DE_ROW);
		};
	
	inline void				setPanel(PanelIndex kPanel, de_line * pDeLine)	{ m_pDeLines[kPanel] = pDeLine; };
	inline de_line *		getPanelLine(PanelIndex kPanel) const			{ return m_pDeLines[kPanel]; };
	inline const de_sync *	getSync(void) const								{ return m_pSync; };
	inline long				getOffsetInSync(void) const						{ return m_offset; };
	inline bool				haveGap(void) const								{ return m_bGap; };
	inline bool				isEOF(void) const								{ return m_bEOF; };
	inline bool				isMARK(void) const								{ return m_pMark != NULL; };

	inline long				getPrevChange(void) const						{ return m_rowPrevChange;   };
	inline long				getNextChange(void) const						{ return m_rowNextChange;   };
	inline long				getPrevConflict(void) const						{ return m_rowPrevConflict; };
	inline long				getNextConflict(void) const						{ return m_rowNextConflict; };

	inline void				setPrevChange(long r)							{ m_rowPrevChange   = r; };
	inline void				setNextChange(long r)							{ m_rowNextChange   = r; };
	inline void				setPrevConflict(long r)							{ m_rowPrevConflict = r; };
	inline void				setNextConflict(long r)							{ m_rowNextConflict = r; };

	inline de_mark *		getMark(void) const								{ return m_pMark; };

private:
	const de_sync *			m_pSync;					// we do not own this.  link to the sync node we are a part of
	long					m_offset;					// our offset within the sync node
	bool					m_bEOF;
	bool					m_bGap;						// true if display list is hiding something before this line.
	de_mark *				m_pMark;					// only valid if m_pSync->isMARK()
	de_line *				m_pDeLines[__NR_TOP_PANELS__];	// we do not own these.  links to the lines in each panel on this line.

	long					m_rowPrevChange,   m_rowNextChange;			// quick links for enabling/disabling
	long					m_rowPrevConflict, m_rowNextConflict;		// prev/next change/conflict buttons

#ifdef DEBUG
public:
	void					dump(int indent) const;
#endif
};

//////////////////////////////////////////////////////////////////
// TVector_Display
//
// we construct a vector that maps from a screen row number to info
// for the lines that should be displayed on it.  that is, an index
// from scroll position to the de_lines.  this is our display list.

typedef std::vector<de_row>					TVector_Display;
typedef TVector_Display::value_type			TVector_DisplayValue;
typedef TVector_Display::iterator			TVector_DisplayIterator;
typedef TVector_Display::const_iterator		TVector_DisplayConstIterator;

//////////////////////////////////////////////////////////////////
// TVector_DisplayIndex
//
// we construct a vector to remember the row number of the start of
// each change/conflict in the display list.  THESE ARE RELATIVE TO
// the current DOP.  that is, these are the row numbers that the
// Next/Prev Change/Conflict commands should warp to.  Note, we already
// have this info in the individual de_rows, but it took a little bit
// to compute and might be useful (for the status bar summary, for
// example).

typedef std::vector<long>					TVector_DisplayIndex;

//////////////////////////////////////////////////////////////////

class de_stats2
{
public:
	de_stats2(void)
		{
			init();
		};

	void		init(void)	
		{
			m_cImportantChanges = 0;
			m_cUnimportantChanges = 0;
			m_cOmittedChunks = 0;
		};

	wxString	formatStatsString(void) const
		{
			wxString msg;
			
			if (m_cUnimportantChanges > 0)
				msg += wxString::Format( _("Changes: (%ld Imp, %ld Unimp)"), m_cImportantChanges, m_cUnimportantChanges);
			else
				msg += wxString::Format( _("Changes: %ld"), m_cImportantChanges);
		
			if (m_cOmittedChunks > 0)
				msg += wxString::Format( _("; Omissions: %ld"), m_cOmittedChunks);

			return msg;
		};
			
public:
	long		m_cImportantChanges;
	long		m_cUnimportantChanges;
	long		m_cOmittedChunks;
};


class de_stats3
{
public:
	de_stats3(void)
		{
			init();
		};

	void		init(void)	
		{
			m_cImportantChanges = 0;
			m_cUnimportantChanges = 0;
			m_cOmittedChunks = 0;
			m_cImportantConflicts = 0;
			m_cUnimportantConflicts = 0;
		};

	wxString	formatStatsString(void) const
		{
			wxString msg;
		
			if (m_cUnimportantChanges > 0)
				msg += wxString::Format( _("Changes: (%ld Imp, %ld Unimp)"), m_cImportantChanges, m_cUnimportantChanges);
			else
				msg += wxString::Format( _("Changes: %ld"), m_cImportantChanges);

			if (m_cUnimportantConflicts > 0)
				msg += wxString::Format( _("; Conflicts: (%ld Imp, %ld Unimp)"), m_cImportantConflicts, m_cUnimportantConflicts);
			else
				msg += wxString::Format( _("; Conflicts: %ld"), m_cImportantConflicts);
		
			if (m_cOmittedChunks > 0)
				msg += wxString::Format( _("; Omissions: %ld"), m_cOmittedChunks);

			return msg;
		};
	
public:
	long		m_cImportantChanges;
	long		m_cUnimportantChanges;
	long		m_cOmittedChunks;
	long		m_cImportantConflicts;
	long		m_cUnimportantConflicts;
};

//////////////////////////////////////////////////////////////////

class de_de
{
public:
	de_de(fs_fs * pFsFs,
		  de_display_ops dopsView, de_display_ops dopsEdit,
		  de_detail_level detailLevel);
	~de_de(void);

	void					enableEditing(bool bEditBufferIsNew);

	void					addChangeCB(util_cbl_fn pfn, void * pData)	{ m_cblChange.addCB(pfn,pData); };
	void					delChangeCB(util_cbl_fn pfn, void * pData)	{ m_cblChange.delCB(pfn,pData); };

	void					cb_fl_changed_T0T2(PanelIndex kPanel, const util_cbl_arg & arg);
	void					cb_fl_changed_T1ED(long kSync, PanelIndex kPanel, const util_cbl_arg & arg);

	void					setDisplayOps(long kSync, de_display_ops dops);

	void					cb_rs_changed(void);
	void					cb_threshold_changed(void);
	void					cb_detail_level_changed(void);
	void					cb_multiline_detail_changed(void);

	fim_ptable *			getPTable(long kSync, PanelIndex kPanel) const;
	fl_fl *					getLayout(long kSync, PanelIndex kPanel) const;
	fs_fs *					getFsFs(void) const { return m_pFsFs; };
	de_display_ops			getDisplayOps(long kSync) const { return m_dops[kSync]; };
	de_detail_level			getDetailLevel(void) const { return m_detailLevel; };

	void					run(void);
	TVector_Display *		getDisplayList(long kSync);
	const fl_line *			getFlLineFromDisplayListRow(long kSync, PanelIndex kPanel, int row);
	bool					getDisplayListRowFromFlLine(long kSync, PanelIndex kPanel, const fl_line * pFlLine, int * pRow);
	bool					mapCoordToDocPosition(long kSync, PanelIndex kPanel, int row, int col, int colTabWidth, fim_offset * pDocPos);
	bool					mapCoordToDocPosition2(long kSync, PanelIndex kPanel, int row, int col, int colTabWidth, fim_offset * pDocPos, bool bSkipVoids);
	bool					mapDocPositionToRowCol(long kSync, PanelIndex kPanel, int colTabWidth, fim_offset docPos, int * pRow, int * pCol);
	
	long					getNthChangeDisplayIndex(long kSync, long ndx) const;

	inline const de_stats2 *	getStats2View(void)			const { return &m_stats2view; };
	inline const de_stats2 *	getStats2Edit(void)			const { return &m_stats2edit; };
	inline const de_stats2 *	getStats2(long kSync)		const { return ((kSync==SYNC_VIEW) ? getStats2View() : getStats2Edit()); };
	inline const de_stats3 *	getStats3View(void)			const { return &m_stats3view; };
	inline const de_stats3 *	getStats3Edit(void)			const { return &m_stats3edit; };
	inline const de_stats3 *	getStats3(long kSync)		const { return ((kSync==SYNC_VIEW) ? getStats3View() : getStats3Edit()); };

	inline wxString			getStats2ViewMsg(void)			const { return m_stats2view.formatStatsString(); };
	inline wxString			getStats2EditMsg(void)			const { return m_stats2edit.formatStatsString(); };
	inline wxString			getStats2Msg(int sync)			const { return ((sync==SYNC_VIEW) ? getStats2ViewMsg() : getStats2EditMsg()); };
	inline wxString			getStats3ViewMsg(void)			const { return m_stats3view.formatStatsString(); };
	inline wxString			getStats3EditMsg(void)			const { return m_stats3edit.formatStatsString(); };
	inline wxString			getStats3Msg(int sync)			const { return ((sync==SYNC_VIEW) ? getStats3ViewMsg() : getStats3EditMsg()); };

	inline long				getTotalRows(long kSync)		const { return m_cTotalRows[kSync]; };
	const de_sync_list *	getSyncList(long kSync)			const { return &m_sync_list[kSync]; };

	inline const de_line *	getDeLineFromNdx(long kSync, PanelIndex kPanel, long ndx) { return (*_lookupVecLineCmp(kSync,kPanel))[ndx]; };
	de_line *				getDeLineFromFlLine(long kSync, PanelIndex kPanel, const fl_line * pFlLine);

	void					unsetPatchHighlight(long kSync);
	bool					isPatch(long kSync, int yRowClick);
	void					setPatchHighlight(long kSync, long yRowClick, bool bDontExtend, bool bQuietly=false);
	bool					getPatchHighlight(long kSync, long * pyRow=NULL, long * pyRowFirst=NULL, long * pyRowLast=NULL);
	bool					isPatchAVoid(long kSync, PanelIndex kPanel, bool bAssert_kSync=true);
	bool					isPatchEqual(long kSync, PanelIndex kPanel, PanelIndex jPanel);
	wxString				getPatchSrcString(long kSync, PanelIndex kPanel);
	bool 					getPatchStartDocPosition(long kSync, PanelIndex kPanel, fim_offset * pDocPos);
	bool					getPatchEndDocPosition(long kSync, PanelIndex kPanel, fim_offset * pDocPos);
	bool					getPatchLineNrs(long kSync, PanelIndex kPanel, long * pLineNrFirst, long * pLineNrLast);
	bool					getPatchLineNrAfter(long kSync, PanelIndex kPanel, long * pLineNrAfter);

	de_patch *				createPatch(fim_patch_op op, int kSync);

	bool					isMark(long kSync, long yRowClick, de_mark ** ppDeMark=NULL);
	void					deleteMark(long kSync, long yRowClick);
	void					deleteMark(long kSync, de_mark * pDeMark);
	util_error				createMark(int kSync, de_mark_type markType, int nrFiles, long * alLineNr, de_mark ** ppDeMark, PanelIndex * pPanelError=NULL);
	int						getMarkRowNr(const de_mark * pDeMark);
	int						getNrMarks(int kSync) const;
	de_mark *				getNthMark(int kSync, int k) const;
	void					deleteAllMark(int kSync);
	de_mark *				getTrivialMark(int kSync) const;

	const fl_line *			getNthLine(int kSync, PanelIndex kPanel, long lineNr);

	inline bool				isRunBusy(void) const { return (m_nestedCalls > 0); };
	inline bool				isRunNeeded(void) const { return (m_bNeedRun); };

	wxString				dumpSupportInfo(const wxString & strIndent) const;

	wxString				getDisplayModeString(long kSync, int colTabWidth) const;

	void					batchoutput_text_traditional_diff2(long kSync, wxString & strOut, bool * pbHadChanges);
	void					batchoutput_text_unified_diff2(long kSync, wxString & strOut, bool * pbHadChanges,
														   wxString * pStrLabelA=NULL,
														   wxString * pStrLabelB=NULL);


	// return a complete html document of a sxs diff.
	void					batchoutput_html_sxs_diff2(long kSync,
													   wxString & strOut,
													   bool * pbHadChanges,
													   int colTabWidth=8,
													   wxString * pStrLabelA=NULL,
													   wxString * pStrLabelB=NULL);
	// return just the CSS used in a html sxs diff.
	static wxString			batchoutput_html_sxs_diff2__css(int colTabWidth=8);

	// return just the html div/table of a sxs diff.
	wxString				batchoutput_html_sxs_diff2__div(long kSync,
															bool * pbHadChanges,
															int colTabWidth,
															wxString * pStrLabelA,
															wxString * pStrLabelB);


	// return a complete html document of a unified diff.
	void					batchoutput_html_unified_diff2(long kSync,
														   wxString & strOut,
														   bool * pbHadChanges,
														   int colTabWidth=8,
														   wxString * pStrLabelA=NULL,
														   wxString * pStrLabelB=NULL);
	// return just the CSS used in a html unified diff.
	static wxString			batchoutput_html_unified_diff2__css(int colTabWidth=8);

	// return just the html div/table of a unified diff.
	wxString				batchoutput_html_unified_diff2__div(long kSync,
																bool * pbHadChanges,
																int colTabWidth,
																wxString * pStrLabelA,
																wxString * pStrLabelB);


	// return a complete html document of a traditional diff.
	void					batchoutput_html_traditional_diff2(long kSync,
															   wxString & strOut,
															   bool * pbHadChanges,
															   int colTabWidth=8,
															   wxString * pStrLabelA=NULL,
															   wxString * pStrLabelB=NULL);
	// return just the CSS used in a html traditional diff.
	static wxString			batchoutput_html_traditional_diff2__css(int colTabWidth=8);

	// return just the html div/table of a traditional diff.
	wxString				batchoutput_html_traditional_diff2__div(long kSync,
																	bool * pbHadChanges,
																	int colTabWidth,
																	wxString * pStrLabelA,
																	wxString * pStrLabelB);


private:
	util_cbl				m_cblChange;	// callback list for objects wanting to know when we change.
	long					m_chgs;			// (DE_CHG_...) stuff currently dirty/reasons for needing to re-run diff-engine
	de_display_ops			m_dops[__NR_SYNCS__];		// display-options (_ALL, _CTX, etc)
	de_detail_level			m_detailLevel;	// lines or lines-n-chars -- controls both views
	bool					m_bNeedRun;		// true if there is work to do
	bool					m_bCacheRSChanged;
	int						m_nestedCalls;	// don't let someone cause run() to be called recursively (via lazy eval)

	fs_fs *					m_pFsFs;

	// count the number of changes/conflicts while running the
	// diff -- this is independent of the current display mode.

	de_stats2				m_stats2view;
	de_stats2				m_stats2edit;
	de_stats3				m_stats3view;
	de_stats3				m_stats3edit;

	// since the content of most lines don't change, we can
	// just build this when the files are loaded and leave it around.
	// as the bottom panel is edited, we'll need to add hash table entries.
	// we don't bother deleting hash entries for lines no longer in the document.
	// we *WILL* have to rebuild it if they change something in the ruleset.

	TSet_Hash				m_setHash;

	// we create one vector for each document in the fs_fs.
	// these vectors can persist between runs of the diff engine,
	// provided that they are invalidated/rebuilt when the document
	// or the active rulset changes.  this cacheing might be important,
	// performance-wise, if the ruleset is complicated.
	// we share T0 and T2's data on both SYNC_ pages, so we only use the
	// SYNC_VIEW cell for them.

	bool					m_bVecValid [__NR_SYNCS__][__NR_TOP_PANELS__];
	TVector_LineCmp			m_vecLineCmp[__NR_SYNCS__][__NR_TOP_PANELS__];
	long					m_sumOmitted[__NR_SYNCS__][__NR_TOP_PANELS__];	// nr of lines omitted when vector built

	// in order to allow for manual alignment, we partition the
	// vecLineCmp into chunks and run the CSS algorithm on each
	// chunk.  think clipping here.  each sync-point is defined
	// by a "mark".  the mark-list should always have at least
	// one entry -- a trivial mark to represent the complete
	// contents of all the documents in the set.  furthermore,
	// since marks represent the sync-points, we let the first
	// one in the list be special and reflect the partion from
	// the beginning of the document to the first actual sync-point.
	// 
	// we keep a mark list for both SYNC_VIEW and SYNC_EDIT;  the
	// latter should inherit the former's marks when editing is
	// enabled, but after that they are independent.  [this lets us
	// use marks to bound the area affected when a patches is applied.]

	bool					m_bCloneMarks;	// when we need to queue a request to clone marks in SYNC_VIEW when creating SYNC_EDIT page; see enableEditing().
	bool					m_bMarksChanged[__NR_SYNCS__];
	TList_DeMark			m_listMarks[__NR_SYNCS__];

#if 0
	// we need to run the diff-engine pairwise between various
	// pairs of documents in the set (T1 vs T0, T1 vs T2)
	// depending on the type of view we have.  since we always
	// compare against T1, we keep a simple array of lists
	// (and don't use m_cssl[*][T1]).
	// 
	// since the top panels should remain relatively constant (as
	// editing is done to the edit panel), we should be able to
	// cache the css lists -- we must invalidate them if the ruleset
	// changes or if either of the 2 documents referenced changes.
	// remember, the css lists only give us the pair-wise common
	// sub-sequences; we still need another step to get the complete
	// picture.

	de_css_list				m_cssl[__NR_SYNCS__][__NR_TOP_PANELS__];	// note: we don't use m_cssl[SYNC_VIEW][PANEL_T1] and m_cssl[SYNC_EDIT][PANEL_EDIT]
#endif

	// the net-net of all of these complicated/convoluted data
	// structures is the de_sync_list -- a list of lines, ranges,
	// and status, sync'd across all files in the file set.
	// we build one sync list [0==SYNC_VIEW] for the REFERENCE
	// panels and one sync list [1==SYNC_EDIT] for the editing page.

	bool					m_bSyncValid[__NR_SYNCS__];
	de_sync_list			m_sync_list[__NR_SYNCS__];
	
	// from the sync list (and in conjunction with the current
	// display op settings), we build a display list.  in our case,
	// this is a vector of information for each row we should
	// display.  we allow the display list for the VIEW panels
	// to be different than the display list for the EDIT panel
	// -- the user may only
	// want to see diffs without context in the VIEW panels, but
	// the whole file in the EDIT panel, for example.  so we
	// populate 2 independent display lists.  the de_lines will
	// have a set of rowNr[__NR_SYNCS__] indexes that are a
	// back link to the absolute vertical position of the line
	// in the corresponding display list.

	bool					m_bDisplayValid[__NR_SYNCS__];
	TVector_Display			m_vecDisplay[__NR_SYNCS__];

	// the next/prev change/conflict links

	TVector_DisplayIndex	m_vecDisplayIndex_Changes[__NR_SYNCS__];
	TVector_DisplayIndex	m_vecDisplayIndex_Conflicts[__NR_SYNCS__];

	// we hold a slot on each layout we're using.

	fl_slot					m_slot[__NR_SYNCS__][__NR_TOP_PANELS__];

	// total display rows in sync-list without regard to display-ops

	long					m_cTotalRows[__NR_SYNCS__];

	// patch highlighting -- when the user clicks on a change
	// we highlight the corresponding parts on all
	// panels on the page.  then, on the SYNC_EDIT page, various
	// patch operations are available -- like replace, insert-before,
	// etc.

	bool					m_bPatchHighlight[__NR_SYNCS__];		// is a patch/change selected
	long					m_yRowPatchClick[__NR_SYNCS__];			// the row the user clicked on
	long					m_yRowPatch[__NR_SYNCS__][2];			// the bounds in display-row-space (affected by DOP)
//	const de_sync *			m_pSyncPatch[__NR_SYNCS__][2];			// the actual bounds of the patch

private:
	fl_slot *				_lookupSlot(long kSync, PanelIndex kPanel);
	TVector_LineCmp *		_lookupVecLineCmp(long kSync, PanelIndex kPanel);

	void					_install_fl_cb(long kSync, PanelIndex kPanel, void (*pfn)(void *, const util_cbl_arg &));
	void					_uninstall_fl_cb(long kSync, PanelIndex kPanel, void (*pfn)(void *, const util_cbl_arg &));

	long					_totalOmitted(long kSync) const;
	void					_setSumOmitted(long kSync, PanelIndex kPanel, long value);
	long					_incrSumOmitted(long kSync, PanelIndex kPanel);
	long					_getSumOmitted(long kSync, PanelIndex kPanel) const;
	
	void					_run_sync2(long kSync);
	void					_run_sync3(long kSync);
	void					_run_algorithm(PanelIndex kPanelA, long kSyncB, PanelIndex kPanelB);
	bool					_build_vector(long kSync, PanelIndex kPanel);
	void					_insert_omitted_lines_into_sync_list(long kSync);
//	de_sync *				__insert_omitted_lines_into_sync_list(long kSync,de_sync * pDeSyncHead);
	void					_build_display(long kSync);
	void					_add_node_to_display(long kSync, const de_sync * pDeSync, bool & bGap);
	void					_emit_normal(long kSync, const de_sync * pDeSync, long kRowBegin, long kRowEnd, bool & bGap);
	void					_emit_omitted(long kSync, const de_sync * pDeSync, long kRowBegin, long kRowEnd, bool & bGap);
	void					_emit_eof(long kSync, const de_sync * pDeSync, bool & bGap);
	void					_emit_mark(long kSync, const de_sync * pDeSync, bool & bGap);
	void					_maybe_add_context_to_display(long kSync, const de_sync * pDeSync, de_display_ops dops, long lenGoal, long & lenHead, bool & bGap);
	long					_classify_type(const de_sync * pDeSync, de_display_ops dops);
	long					_classify_for_display(const de_sync * pDeSync, de_display_ops dops);

	void					_fixup_next_prev_change_links(long kSync);
	void					_fixup_npcl_all(long kSync);
	void					_fixup_npcl_dif(long kSync);
//	void					_fixup_npcl_ctx(long kSync);
//	long					_fixup_npcl_ctx_look_forward_for_change(long kSync, long rowStart, long ctxGoal);
	void					_fixup_next_links(long kSync);

	void					_do_intraline2(long kSync);
	void					_do_intraline3(long kSync);

	void					_do_simple_intraline2_sync(long kSync, de_sync * pSync);
	void					_do_simple_intraline3_sync(long kSync, de_sync * pSync);

	void					_do_build_multiline_string(long kSync,PanelIndex kPanel,
													   long ndxStart, long nrLines,
													   bool bMatchEOLs,
													   wxString * pStr,
													   const de_line ** ppDeLineLast,
													   TVecLongs * pVecLens,
													   TVecBools * pVecEols);

	de_sync *				_do_complex_intraline2_sync(long kSync, de_sync * pSync);
	de_sync *				_do_complex_intraline3_sync(long kSync, de_sync * pSync);

	de_sync *				_extractLinesFromMultiLineIntraLine2(long kSync,
																 de_sync * pSyncLine,
																 de_sync_list * pSyncListIntraLine,
																 const wxString & strT1,
																 long lenVecT1, const std::vector<long> & vecLineLensT1,
																 long lenVecT0, const std::vector<long> & vecLineLensT0,
																 rs_context_attrs defaultContextAttrs);
	de_sync *				_extractLinesFromMultiLineIntraLine3(long kSync,
																 de_sync * pSyncLine,
																 de_sync_list * pSyncListIntraLine,
																 const wxString & strT1,
																 long lenVecT1, const std::vector<long> & vecLineLensT1,
																 long lenVecT0, const std::vector<long> & vecLineLensT0,
																 long lenVecT2, const std::vector<long> & vecLineLensT2,
																 rs_context_attrs defaultContextAttrs);

	void					_cloneMarks(void);
	long					_get_ndx_for_de_line(long kSync, PanelIndex kPanel, de_line * pDeLineThis);
	de_css_src_clipped *	_create_clip_src_for_mark(de_css_src_lines * pSrc, long kSync, TList_DeMarkIterator it, PanelIndex kPanelA, PanelIndex kPanelB);
	void					_deal_with_mark_before_delete_line(long kSync, PanelIndex kPanel,fl_line * pFlLine);
	void					_delete_line(long kSync, PanelIndex kPanel,fl_line * pFlLine);

	void					_get_patch_line_nrs_allowing_for_voids_or_eof(long kSync, PanelIndex kPanel, long * pLineNrFirst, long * pLineNrLast);

	void					_split_up_eqs2(long kSync);
	void					_split_up_eqs3(long kSync);
	void					_join_up_neqs2(long kSync);
	void					_join_up_neqs3(long kSync);
	void					_force_resync(void);
	
#ifdef DEBUG
public:
	void					dump_row_vector(int indent, long kSync, de_display_ops dops) const;
	void					dump_links(int indent, long kSync);
#endif
};

//////////////////////////////////////////////////////////////////

bool unified_diff__compute_row_end_of_chunk(TVector_Display * pDis,
											long nrRows,
											long kRowStart,
											long * pkRowEnd);

struct _unified_diff__line_nr_mapping
{
	bool bVoid[2];
	long lineNrFirst[2];
	long lineNrLast[2];
	long len[2];
};

void unified_diff__map_rows_to_line_numbers(de_de * pDeDe, TVector_Display * pDis, long kSync,
											long kRowStart, long kRowEnd,
											struct _unified_diff__line_nr_mapping * p);

wxString _traditional_diff__format_change_header(const struct _unified_diff__line_nr_mapping & lnm);

wxString html_diff_escape_string(const wxString & strInput,
								 bool bGlobalRespectEOL = false);

//////////////////////////////////////////////////////////////////

#endif//H_DE_DE_H
