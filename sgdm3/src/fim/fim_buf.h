// fim_buf.h -- a "grow buf" for storing the raw content of a file.
//////////////////////////////////////////////////////////////////

#ifndef H_FIM_BUF_H
#define H_FIM_BUF_H

//////////////////////////////////////////////////////////////////

class fim_buf
{
public:
	fim_buf(fim_length lenPreallocateHint=0);
	~fim_buf(void);

	const wxChar *			getTemporaryPointer(fb_offset offset) const; // for temporary use only (until next realloc)

	void					append(const wxChar * pData, fim_length lenData, fb_offset * pOffset);
	void					copyFrom(const fim_buf * pFimBuf, fb_offset offsetData, fim_length lenData, fb_offset * pOffset);

	void					resetBuffer(void);

	void					hintSpaceNeeded(fim_length lenPreallocateHint);

protected:
	void					_grow(fim_length spaceNeeded);
	bool					_getOffset(const wxChar * pData, fb_offset * pOffset=NULL) const;

	wxChar *				m_pBuf;				// an array of contiguous elements
	fim_length				m_cSizeInUse;		// nr of elements currently in-use
	fim_length				m_cSizeAllocated;	// nr of elements allocated

#ifdef DEBUG
public:
	void					dump(int indent) const;
#endif
};

//////////////////////////////////////////////////////////////////

#endif//H_FIM_BUF_H
