// fim_frag.cpp -- a fragment of a file -- a section with uniform properties
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <poi.h>
#include <fim.h>

//////////////////////////////////////////////////////////////////

fim_frag::fim_frag(fim_buf * pFimBuf, fb_offset offsetData, fim_length lenData, fr_prop prop)
	: m_pFimBuf(pFimBuf),		// we do not claim a reference on fim_buf -- we rely on our containing fim_ptable's
	  m_offsetData(offsetData),
	  m_lenData(lenData),
	  m_prop(prop),
	  m_next(NULL),
	  m_prev(NULL)
{
	// we allow zero-length frags -- but think of them as a transitional
	// state -- as in we're stealing the content from one frag during a
	// coalesce and haven't delete it yet.  or we allocated it before we
	// knew the exact content we were going to assign to it.

	DEBUG_CTOR(T_FIM_FRAG,L"fim_frag");
}

fim_frag::~fim_frag(void)
{
	DEBUG_DTOR(T_FIM_FRAG);
}

//////////////////////////////////////////////////////////////////

const wxChar * fim_frag::getTemporaryDataPointer(fr_offset relOffset) const
{
	// the assert should be strictly '<' for the pointer to refer to content
	// within the fragment.
	//
	// but we allow '<=' so that requests for pointers on zero length frags
	// can be honored (for dump())

	wxASSERT_MSG( (relOffset <= m_lenData), _T("Coding Error: fim_frag::getTemporaryDataPointer") );
	
	return m_pFimBuf->getTemporaryPointer(m_offsetData + relOffset);
}

//////////////////////////////////////////////////////////////////

bool fim_frag::_canCoalesceWithNext(void) const
{
	// see if we can coalesce the content of the frag following ours onto the tail of ours.
	//
	// in order to be able to coalesce, we require:
	// [] we have a next frag (duh)
	// [] both frags to refer to the same growbuf (be in the same document)
	// [] both frags to refer to contiguous storage within the growbuf (so that [offset1,len1)+[offset2,len2)==>[offset1,len1+len2) )
	// [] the props on both frags to be the same.

	return (   (m_next)
			&& (m_next->m_pFimBuf    == m_pFimBuf)
			&& (m_next->m_offsetData == (m_offsetData+m_lenData))
			&& (m_next->m_prop       == m_prop) );
}

bool fim_frag::_canCoalesceWithPrev(void) const
{
	return ((m_prev) && (m_prev->_canCoalesceWithNext()));
}

//////////////////////////////////////////////////////////////////

#ifdef DEBUG
void fim_frag::dump(int indent) const
{
	wxLogTrace(TRACE_FRAG_DUMP, _T("%*cFIM_FRAG: [%p][prop %lx][len %ld]"), indent,' ',this,m_prop,m_lenData);
	util_print_buffer_cl(TRACE_FRAG_DUMP,getTemporaryDataPointer(),m_lenData);
}
#endif
