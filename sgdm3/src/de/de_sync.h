// de_sync.h -- list and nodes to represent synchronized document
// content across all docs in the set.
//////////////////////////////////////////////////////////////////

#ifndef H_DE_SYNC_H
#define H_DE_SYNC_H

//////////////////////////////////////////////////////////////////
// de_sync -- a "sync" node -- each node represents a corresponding
// portion (ndx,len) within each doc in a set.  when we are doing
// the main diff, each portion will represent a line ndx (within
// de_de::m_vecLineCmp) and a number of lines in the portion.  when
// we are doing an intra-line diff, ndx represents the index of the
// string (within de_line::m_strLine or de_line::m_strEOL).
//
// sub-classing for lines vs strings proved more trouble than it was
// worth -- since the css algorithm and important parts of the sync
// list sync'ing (load_T*) are independent of types (see de_css_src
// and de_css_src_{lines,strings}.
//////////////////////////////////////////////////////////////////

class de_sync
{
public:
	friend class de_sync_list;

	de_sync(void);
	de_sync(PanelIndex kPanel_a, long ndx_a, long len_a,
			PanelIndex kPanel_b, long ndx_b, long len_b,
			de_attr attr);
	de_sync(PanelIndex kPanel_a, long ndx_a, long len_a,
			PanelIndex kPanel_b, long ndx_b, long len_b,
			PanelIndex kPanel_c, long ndx_c, long len_c,
			de_attr attr);
			
	virtual ~de_sync(void);

public:
	inline de_sync *			getNext(void)					const { return m_next; };
	inline de_sync *			getPrev(void)					const { return m_prev; };

	inline long					getNdx(PanelIndex kPanel)		const { return m_ndx[kPanel]; };
	inline long					getLen(PanelIndex kPanel)		const { return m_len[kPanel]; };
	long						getMaxLen(void)					const;
	long						getMinLen(void)					const;

	inline de_attr				getAttr(void)					const { return m_attr;        };
	void						updateAttrType(de_attr attr);

	inline bool					isSameType(de_attr attr)		const { return ((m_attr & DE_ATTR__TYPE_MASK) == (attr & DE_ATTR__TYPE_MASK)); };
	inline bool					isSameKind(de_attr attr)		const { return ((m_attr & DE_ATTR__KIND_MASK) == (attr & DE_ATTR__KIND_MASK)); };
	inline bool					isDiffKind(void)				const { return (isSameKind(DE_ATTR__DIF_KIND));                                };
	inline bool					isMergeKind(void)				const { return (isSameKind(DE_ATTR__MRG_KIND));                                };

	inline bool					isEND(void)						const { return (isSameType(DE_ATTR_EOF));                                      };
	inline bool					isOmitted(void)					const { return (isSameType(DE_ATTR_OMITTED));                                  };
	inline bool					isMARK(void)					const { return (isSameType(DE_ATTR_MARK));                                     };

	inline bool					isConflict(void)				const { return ((m_attr & DE_ATTR_CONFLICT)    == DE_ATTR_CONFLICT   ); };
	inline bool					isUnimportant(void)				const { return ((m_attr & DE_ATTR_UNIMPORTANT) == DE_ATTR_UNIMPORTANT); };
	inline bool					isPartial(void)					const { return ((m_attr & DE_ATTR_PARTIAL)     == DE_ATTR_PARTIAL    ); };
	inline bool					isSmoothed(void)				const { return ((m_attr & DE_ATTR_SMOOTHED)    == DE_ATTR_SMOOTHED   ); };
	inline bool					isMLMember(void)				const { return ((m_attr & DE_ATTR_ML_MEMBER)   == DE_ATTR_ML_MEMBER  ); };
	inline bool					isNonExactEQ(void)				const { return ((m_attr & DE_ATTR_NONEXACTEQ)  == DE_ATTR_NONEXACTEQ ); };

	inline bool					isAnyChangeType2(void)			const { return (isDiffKind()  && !isSameType(DE_ATTR_DIF_2EQ));	};
	inline bool					isAnyChangeType3(void)			const { return (isMergeKind() && !isSameType(DE_ATTR_MRG_3EQ));	};
	inline bool					isAnyChangeType(void)			const { return (isAnyChangeType2() || isAnyChangeType3());      };
	
	inline void					setConflict(void)					  { m_attr |= DE_ATTR_CONFLICT;    };
	inline void					setUnimportant(bool bUn)			  { m_attr = ((bUn) ? (m_attr | DE_ATTR_UNIMPORTANT) : (m_attr & ~DE_ATTR_UNIMPORTANT)); };

	inline void					setSmoothed(de_attr attr)			  { wxASSERT_MSG( ((m_attr&DE_ATTR__MODIFIER_MASK)==0), _T("Coding Error")); m_attr=attr|DE_ATTR_SMOOTHED; };
	inline void					setNonExactEQ(de_attr attr)			  { wxASSERT_MSG( ((m_attr&DE_ATTR__MODIFIER_MASK)==0), _T("Coding Error")); m_attr=attr|DE_ATTR_NONEXACTEQ; };

	void						changeToMark(de_mark * pMark);
	inline de_mark *			getMark(void)					const { wxASSERT_MSG( (isMARK()), _T("Coding Error")); return m_pMark; };

	void						unionAttr(de_attr attr);

protected:
	void						_init(void);

	de_sync *					m_next;
	de_sync *					m_prev;

	long						m_ndx[__NR_TOP_PANELS__];		// index into vector of {lines,strings} we are comparing
	long						m_len[__NR_TOP_PANELS__];		// length within vector of {lines,strings}

	de_attr						m_attr;
	de_mark *					m_pMark;	// only valid when isMARK()

	//////////////////////////////////////////////////////////////////
	// The following should only be used when we are a line-oriented
	// sync node.
	//
	// for line-oriented sync-nodes that represent changes, we need
	// to do intra-line diffs -- both for display purposes, and for
	// properly classifying (important/unimportant) the line based
	// upon the conext (within the line) of where (on the line) the
	// change occurs.
	//////////////////////////////////////////////////////////////////
public:
	// TODO consider moving isOmitted() here -- until we add column omitting.

	void						createIntraLineSyncListVector(long cReserve);
	void						appendIntraLineSyncList(de_sync_list * pSyncList);
	de_sync_list *				getIntraLineSyncList(long row)	const;
	bool						haveIntraLineSyncInfo(void)		const { return (m_pVecSyncList != NULL); };
	
private:
	typedef std::vector<de_sync_list *>				TVector_SyncList;
	typedef TVector_SyncList::value_type			TVector_SyncList_Value;
	typedef TVector_SyncList::iterator				TVector_SyncList_Iterator;
	typedef TVector_SyncList::const_iterator		TVector_SyncList_ConstIterator;

	TVector_SyncList *			m_pVecSyncList;		// ptr to intra-line sync lists for each line in [0...max(m_len[...])]

	//////////////////////////////////////////////////////////////////
	// The following should only be used when we are a string-oriented
	// (intra-line) sync node.
	//////////////////////////////////////////////////////////////////
public:
	inline void					setSpanContextAttrs(PanelIndex kPanel, rs_context_attrs attr)			{ m_contextAttrsSpan[kPanel] = attr; };
	inline rs_context_attrs		getSpanContextAttrs(PanelIndex kPanel)							 const	{ return m_contextAttrsSpan[kPanel]; };

private:
	rs_context_attrs			m_contextAttrsSpan[__NR_TOP_PANELS__];

	//////////////////////////////////////////////////////////////////

#ifdef DEBUG
public:
	virtual void				dump(int indent) const;
#endif
};
	
//////////////////////////////////////////////////////////////////

#endif//H_DE_SYNC_H
