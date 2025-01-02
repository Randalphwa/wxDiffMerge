// fl_fl.h -- a file layout -- line-number,column layout on the
// contents of a file (piecetable).  a data structure to express
// contents in terms of lines and runs within a line.
//////////////////////////////////////////////////////////////////

#ifndef H_FL_FL_H
#define H_FL_FL_H

//////////////////////////////////////////////////////////////////

class fl_fl
{
private:
	friend class fim_ptable;
	fl_fl(fim_ptable * pPTable);	// only fim_ptable should call this
	~fl_fl(void);					// only fim_ptable should call this

public:
	void							addChangeCB(util_cbl_fn pfn, void * pData)	{ m_cblChange.addCB(pfn,pData); };
	void							delChangeCB(util_cbl_fn pfn, void * pData)	{ m_cblChange.delCB(pfn,pData); };

	inline fl_line *				getFirstLine(void)			const	{ return m_pLineHead; };
	inline fl_line *				getLastLine(void)			const	{ return m_pLineTail; };
	fl_line *						getNthLine(long n);

	inline int						getFormattedLineNrs(void)	{ if (FL_INV_IS_SET(m_invUnion,FL_INV_LINENR)) format(); return m_totalLineNrs; };
	inline int						getFormattedCols(void)		{ if (FL_INV_IS_SET(m_invUnion,FL_INV_COL))    format(); return m_totalCols; };

	void							format(void);

	fl_slot							claimSlot(de_de * pDeDe);
	void							releaseSlot(de_de * pDeDe, fl_slot slot);

	bool							getLineAndColumnOfFragAndOffset(const fim_frag * pFrag, fr_offset offset,
																	int cColTabWidth,
																	const fl_line ** ppFlLine, int * pCol) const;

private:
	inline void						_invalidateFull(void)				{                            m_invUnion |= FL_INV_FULL;    };
	inline void						_invalidateCols(fl_line * pLine)	{ pLine->invalidateCols();   m_invUnion |= FL_INV_COL;     };
	inline void						_invalidateLineNrs(fl_line * pLine)	{ pLine->invalidateLineNr(); m_invUnion |= FL_INV_LINENR;  };
	inline void						_invalidateProp(fl_line * pLine)	{ pLine->invalidateProp();   m_invUnion |= FL_INV_PROP;    };

	fl_run *						_get_first_run_in_frag(const fim_frag * pFrag);
	fl_run *						_get_last_run_in_frag(const fim_frag * pFrag);

	void							_delete_line(fl_line * pLine);
	fl_line *						_append_line(fl_line * pLineNew);
	fl_line *						_insert_line_after(fl_line * pLineNew, fl_line * pLineExisting);
	fl_line *						_fixup_lines_after_insert(fl_run_list_endpoints & ep);
	fl_line *						_fixup_lines_after_delete(fl_run_list_endpoints & ep);
	void							_invalidate_coords(fl_run_list_endpoints & ep);

	void							_map_insert(const fim_frag * pFrag, fl_run_list_endpoints & ep);
	void							_map_delete(const fim_frag * pFrag);

	void							_do_initial_layout(void);
	void							_fop_insert_before(const fim_frag * pFrag);
	void							_fop_insert_after(const fim_frag * pFrag);
	void							_fop_split(const fim_frag * pFrag);
	void							_fop_join_before(const fim_frag * pFrag);
	void							_fop_join_after(const fim_frag * pFrag);
	void							_fop_delete_self(const fim_frag * pFrag);
	void							_fop_set_prop(const fim_frag * pFrag);

private:
	fim_ptable *					m_pPTable;		// we *DO NOT* own the piecetable and we *DO NOT* hold a reference.

	fl_run_list						m_listRun;

	fl_line *						m_pLineHead;	// we own the line list
	fl_line *						m_pLineTail;
	TMap_FragToRunList				m_mapFrag;

	util_cbl						m_cblChange;	// callback list for objects wanting to know when we change. [can send FL_INV_ or FL_MOD_ events]

	int								m_totalLineNrs;
	int								m_totalCols;
	fl_invalidate					m_invUnion;

	typedef std::vector<de_de *>	TVec_Slots;
	typedef TVec_Slots::iterator	TVec_SlotsIterator;

	TVec_Slots						m_vecSlots;

public:
	void							_cb_frag_change(const fim_frag * pFrag, fr_op fop);
//	void							_gp_cb_long(GlobalProps::EnumGPL id);
//	void							_gp_cb_string(GlobalProps::EnumGPS id);

#ifdef DEBUG
public:
	void							dump(int indent) const;
#endif
};

//////////////////////////////////////////////////////////////////

#endif//H_FL_FL_H
