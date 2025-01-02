// fl_run.h -- a "run" -- a span of text that can be displayed
// with in a particular way/style on a particular line.  includes
// some pixel placement info.  maps to a portion of a fragment;
// may not cover more than one frag.
//
// all runs in the document are in a single double-linked list.
// 
// fl_runs on the same display line all refer to the same fl_line.
// 
// fl_lines have a link to the first fl_run on the display line.
//////////////////////////////////////////////////////////////////

#ifndef H_FL_RUN_H
#define H_FL_RUN_H

//////////////////////////////////////////////////////////////////

class fl_run
{
private:
	friend class fl_run_list;
	fl_run(fl_line * pLine,
		   const fim_frag * pFrag=NULL, fr_offset offset=0, fim_length len=0, fr_prop prop=FR_PROP_ZERO);
	~fl_run(void);

public:
	inline fl_run *					getNext(void)					const	{ return m_next; };
	inline fl_run *					getPrev(void)					const	{ return m_prev; };

	inline fl_line *				getLine(void)					const	{ return m_pLine; };
	void							setLine(fl_line * pLine)				{ m_pLine = pLine; };

	inline const fim_frag *			getFrag(void)					const	{ return m_pFrag; };
	inline fr_offset				getFragOffset(void)				const	{ return m_offsetFrag; };
	inline fim_length				getLength(void)					const	{ return m_lenInFrag; };
	inline fr_offset				getFragEndOffset(void)			const	{ return m_offsetFrag+m_lenInFrag; };

	inline fr_prop					getFragProp(void)				const	{ return m_propFrag; };

//	inline int						getCol(void)					const	{ return m_col; };
//	inline int						getNrCols(void)					const	{ return m_nrCols; };

	const wxChar *					getContent(void)				const;	// a temporary pointer to current content
	bool							isLF(void)						const;
	bool							isCR(void)						const;
	bool							isTAB(void)						const;

	inline bool						isUnlinked(void)				const	{ return ((m_next==NULL) && (m_prev==NULL)); };

	inline void						setFrag(const fim_frag * pFrag)			{ m_pFrag = pFrag; };
	inline void						setFragOffset(fr_offset offset)			{ m_offsetFrag = offset; };
	inline void						setFragProp(fr_prop prop)				{ m_propFrag = prop; };

//	inline void						setCol(int col, int nrCols)				{ m_col = col; m_nrCols = nrCols; };

private:
	fl_run *						m_next;
	fl_run *						m_prev;

	fl_line *						m_pLine;		// the fl_line (representing the display line) that this fl_run is on.

	const fim_frag *				m_pFrag;		// [ (frag,offset)...(frag,offset+len) ) describes
	fr_offset						m_offsetFrag;	// the content in the piecetable.
	fim_length						m_lenInFrag;	// note: offset+len < length-of-frag

	fr_prop							m_propFrag;		// attributes

//	int								m_col;		// column number of start of run
//	int								m_nrCols;	// width in columns of run (compensates for tabs) WARNING: Obsolete

#ifdef DEBUG
public:
	void							dump(int indent) const;
#endif
};

//////////////////////////////////////////////////////////////////

#endif//H_FL_RUN_H
