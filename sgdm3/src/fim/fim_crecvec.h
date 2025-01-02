// fim_crecvec.h -- an unbounded vector of change records.
// represents the sequence of changes/transactions applied
// to a piecetable.
//
// each crec is of fine-enough granularity that it is a
// "reversible" operation -- for undo/redo.
//
// think of this vector as a "stack" where the "top (aka tail)" is
// the most recent operation performed -- an UNDO uses the info
// in the top crec to undo the change.  i named this a "vector"
// and not a "stack" because i don't "pop on undo" -- we keep
// an index of the "actual top" and the "undo position".
//
// to "redo" we replay the crec.
//
// we defer the "pop/destroy" until the user makes a new change
// and invalidates the redo.
//////////////////////////////////////////////////////////////////

#ifndef H_FIM_CRECVEC_H
#define H_FIM_CRECVEC_H

//////////////////////////////////////////////////////////////////

class fim_crecvec
{
public:
	fim_crecvec(void);
	~fim_crecvec(void);

	// in all of the following that return a "fim_crec *".  this pointer is
	// a TEMPORARY pointer.  use it immediately as the storage may move when
	// the next crec is pushed.

	const fim_crec *		getUndoCRec(void);		// fetch crec for undo, move undo pos
	const fim_crec *		getRedoCRec(void);		// fetch crec for redo, move redo pos

	const fim_crec *		push_ta_begin(void);					// push a "do"
	const fim_crec *		push_ta_end(fim_crec::TAID taidBegin);	// push a "do"
	const fim_crec *		push_text(fim_crec::Verb v, fb_offset offsetBuf, fim_length lenData, fim_offset offsetDoc, fr_prop prop,
									  const fim_buf * pFimBuf); // push a "do"
	const fim_crec *		push_prop(fb_offset offsetBuf, fim_length lenData, fim_offset offsetDoc, fr_prop prop, fr_prop propNew); // push a "do"

	inline bool				canUndo(void)			const	{ return m_cSizePos > 0; };
	inline bool				canRedo(void)			const	{ return m_cSizePos < m_cSizeTop; };

	inline bool				isDirty(void)			const	{ return m_cSizePos != m_cSizeClean; };
	inline void				markClean(void)					{ m_cSizeClean = m_cSizePos; };

	// 2011/12/02 J5726: I'm simplifying the rules for when they can use the auto-merge button.
	//                   In the past we tried to let them use it once, but allowed editing before
	//                   they used it.  And we tried to let it intertwine with the UNDO/REDO
	//                   state.
	//
	//                   Now we say that it must be the FIRST thing they do or else they can use it.
	//                   Normally this isn't a problem since Vault and Veracity will usually launch
	//                   us with the --automerge flag anyway.
	inline bool				canAutoMerge(void)		const	{ return m_cSizePos == 0; };

	inline fim_crec::TAID	getNewTAID(void)				{ return m_taidNext++; };

	void					reset(void);

private:
	fim_crec *				_push_crec(void);

	fim_crec *				m_pVec;					// a moveable array of crec's

	long					m_cSizeAllocated;		// nr of elements allocated (think realloc chunking)
	long					m_cSizeTop;				// pos of last crec (end of redo)
	long					m_cSizePos;				// current pos of undo/redo
	long					m_cSizeClean;			// pos at last checkpoint

	fim_crec::TAID			m_taidNext;				// next transaction id to assign (these are per-crecvec unique)

	fim_crec				m_crecGlobTemp;			// used when we glob

};

//////////////////////////////////////////////////////////////////

#endif//H_FIM_CRECVEC_H
