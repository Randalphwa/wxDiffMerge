// fl_line.h -- a "line" -- describes a line of text, a sequence of runs.
//////////////////////////////////////////////////////////////////

#ifndef H_FL_LINE_H
#define H_FL_LINE_H

//////////////////////////////////////////////////////////////////

class fl_line
{
private:
	friend class fl_fl;
	fl_line(fl_fl * pFlFl, fl_run * pRunFirst=NULL);
	~fl_line(void);

public:
	inline fl_line *			getNext(void)						const	{ return m_next; };
	inline fl_line *			getPrev(void)						const	{ return m_prev; };
	inline fl_fl *				getFlFl(void)						const	{ return m_pFlFl; };

	inline int					getLineNr(void)						const	{ return m_lineNr; };

	inline fl_run *				getFirstRunOnLine(void)				const	{ return m_pRunFirst; };
	inline void					setFirstRunOnLine(fl_run * pRun)			{ m_pRunFirst = pRun; };

	inline bool					isUnlinked(void)					const	{ return ((m_next==NULL) && (m_prev==NULL)); };

	inline fl_invalidate		getInvalidate(void)					const	{ return m_invalidate; };
	inline void					invalidateLineNr(void)						{ m_invalidate |= FL_INV_LINENR; };
	inline void					invalidateCols(void)						{ m_invalidate |= FL_INV_COL; };
	inline void					invalidateProp(void)						{ m_invalidate |= FL_INV_PROP; };
	inline void					invalidate(fl_invalidate inv)				{ m_invalidate |= inv; };

	inline void					validate(void)								{ m_invalidate = 0; };

	inline void					setLineNr(int lineNr)						{ m_lineNr = lineNr; };

	de_line *					getSlotValue(fl_slot slot) const;
	void						setSlotValue(fl_slot slot, de_line * pDeLine);

	void						buildStringsFromRuns(bool bIncludeEOL, wxString * pstrLine, wxString * pstrEOL=NULL) const;
	wxString					buildTabExpandedStringFromRuns(bool bIncludeEOL, int cColTabWidth) const;

	void						getFragAndOffsetOfColumn(int col, int cColTabWidth,
														 const fim_frag ** ppFrag, fr_offset * pOffset) const;
	int							getColumnOfFragAndOffset(int cColTabWidth, const fim_frag * pFrag, fr_offset offset) const;

	inline void					updateEditOpCounter(void)					{ m_editOpCounter++; };
	inline long					getEditOpCounter(void)				const	{ return m_editOpCounter; };

private:
	fl_line *					m_next;
	fl_line *					m_prev;
	
	fl_fl *						m_pFlFl;		// back ptr to the fl_fl layout that contains this line. 
	fl_run *					m_pRunFirst;	// first run on this line (in fl_fl's run list)

	fl_invalidate				m_invalidate;	// invalidate various (lineNr,col,prop) for this line and/or at least run on this line.
	int							m_lineNr;		// absolute lineNr number (0-based line number)

	long						m_editOpCounter;

	typedef std::vector<de_line *>		TVec_SlotValue;			// v[slot]=value -- allow anyone (de_de) making use of our layout to do a setprop().
	typedef TVec_SlotValue::iterator	TVec_SlotValueIterator;	//               -- we index by slot since layout is shared.

	TVec_SlotValue				m_vecSlotValue;
	
#ifdef DEBUG
public:
	void						dump(int indent) const;
#endif
};

//////////////////////////////////////////////////////////////////

#endif//H_FL_LINE_H
