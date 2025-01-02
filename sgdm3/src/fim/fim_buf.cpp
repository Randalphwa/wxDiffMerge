// fim_buf.cpp -- a "growbuf" for a piecetable.
// text is appended to this buffer but NEVER removed.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <poi.h>
#include <fim.h>

//////////////////////////////////////////////////////////////////

fim_buf::fim_buf(fim_length lenPreallocateHint)
	: m_pBuf(NULL),
	  m_cSizeInUse(0),
	  m_cSizeAllocated(0)
{
	if (lenPreallocateHint)
		hintSpaceNeeded(lenPreallocateHint);
}

fim_buf::~fim_buf(void)
{
	FREEP(m_pBuf);
}

//////////////////////////////////////////////////////////////////

const wxChar * fim_buf::getTemporaryPointer(fb_offset offset) const
{
	// convert offset into TEMPORARY pointer.

	// the assert should be strictly '<' for the pointer to refer to content
	// within a fragment.
	//
	// but we allow '<=' so that requests for pointers on zero length frags
	// can be honored (for dump())

	wxASSERT_MSG( (m_pBuf && (offset <= m_cSizeInUse)), _T("Coding Error: fim_buf::getTemporaryPointer") );

	return m_pBuf+offset;
}

bool fim_buf::_getOffset(const wxChar * pData, fb_offset * pOffset) const
{
	// see if pData is contained with our grow buffer.
	// if so, convert it to a relative offset within our buffer.

	if (!m_pBuf)
		return false;

	if (pData < m_pBuf)
		return false;
	
	fb_offset offset = pData - m_pBuf;

	if (offset >= m_cSizeInUse)
		return false;

	if (pOffset)
		*pOffset = offset;

	return true;
}

//////////////////////////////////////////////////////////////////

void fim_buf::append(const wxChar * pData, fim_length lenData, fb_offset * pOffset)
{
	if (!pData || !lenData)
		return;
	
	if (lenData > (m_cSizeAllocated - m_cSizeInUse))
	{
		// need more space in buffer.  realloc buffer (generously rounding up space required).
		// 
		// sanity check that the source data is not within the buffer we're about to move.
		// (this should not happen because the caller should just reference the existing
		// data (via the magic of frags) rather than copying.  we should only be appending
		// data from other growbufs (during cut/paste operations) and from keystroke/editing
		// data.

		fb_offset srcOffset;
		bool bOverlap = _getOffset(pData,&srcOffset);

		_grow(lenData);

		if (bOverlap)
			pData = getTemporaryPointer(srcOffset);
	}

	if (pOffset)
		*pOffset = m_cSizeInUse;

	memmove(m_pBuf+m_cSizeInUse,pData,lenData*sizeof(wxChar));
	m_cSizeInUse += lenData;
}

void fim_buf::copyFrom(const fim_buf * pFimBuf, fb_offset offsetData, fim_length lenData, fb_offset * pOffset)
{
	const wxChar * pData = pFimBuf->getTemporaryPointer(offsetData);

	append(pData,lenData,pOffset);
}

//////////////////////////////////////////////////////////////////

void fim_buf::hintSpaceNeeded(fim_length lenPreallocateHint)
{
	if (lenPreallocateHint > m_cSizeAllocated)
		_grow(lenPreallocateHint - m_cSizeAllocated);
}

void fim_buf::_grow(fim_length spaceNeeded)
{
#define CHUNK_GBUF			(1024*1024)

	fim_length newSize = ((m_cSizeAllocated + spaceNeeded + CHUNK_GBUF - 1)/CHUNK_GBUF) * CHUNK_GBUF;

	m_pBuf = (wxChar *)realloc(m_pBuf,newSize * sizeof(wxChar));

	m_cSizeAllocated = newSize;

//	wxLogTrace(TRACE_FIMBUF_DUMP, _T("fim_buf: expanding [%p] to [buf %p][size %ld][allocated %ld]"),
//			   this,m_pBuf,m_cSizeInUse,m_cSizeAllocated);

#undef CHUNK_GBUF
}

//////////////////////////////////////////////////////////////////

void fim_buf::resetBuffer(void)
{
	// when we do a non-undo-able _delete_all() on the piece-table,
	// we don't need to keep the raw-buffer data.  because as we
	// reload/reclone the document we'll only be referencing new
	// raw-text.

	m_cSizeInUse = 0;
}

//////////////////////////////////////////////////////////////////

#ifdef DEBUG
void fim_buf::dump(int /*indent*/) const
{
	wxLogTrace(TRACE_FIMBUF_DUMP, _T("fim_buf: [%p][buf %p][size %ld][allocated %ld]"),
			   this,m_pBuf,m_cSizeInUse,m_cSizeAllocated);
}
#endif
