// fim_frag.h -- a fragment of a file -- a section with uniform properties.
// a fragment is just a snippet of text from the document.
// it has no notion of where it is in the document.
//////////////////////////////////////////////////////////////////

#ifndef H_FIM_FRAG_H
#define H_FIM_FRAG_H

//////////////////////////////////////////////////////////////////

class fim_frag
{
public:
	fim_frag(fim_buf * pFimBuf, fb_offset offsetData=0, fim_length lenData=0, fr_prop prop=FR_PROP_ZERO);
	~fim_frag(void);

	inline fb_offset		getBufOffset(fr_offset relOffset=0)				const	{ return m_offsetData+relOffset; };
	inline fim_length		getFragLength(void)								const	{ return m_lenData; };
	inline fr_prop			getFragProp(void)								const	{ return m_prop; };
	
	inline fim_frag *		getNext(void)									const	{ return m_next; };
	inline fim_frag *		getPrev(void)									const	{ return m_prev; };

	const wxChar *			getTemporaryDataPointer(fr_offset relOffset=0)	const;

private:
	friend class fim_ptable;	// fim_ptable directly manipulates everything

	bool					_canCoalesceWithNext(void) const;
	bool					_canCoalesceWithPrev(void) const;

private:
	fim_buf *				m_pFimBuf;
	fb_offset				m_offsetData;
	fim_length				m_lenData;
	fr_prop					m_prop;

	fim_frag *				m_next;
	fim_frag *				m_prev;

#ifdef DEBUG
public:
	void					dump(int indent) const;
#endif
};

//////////////////////////////////////////////////////////////////

#endif//H_FIM_FRAG_H
