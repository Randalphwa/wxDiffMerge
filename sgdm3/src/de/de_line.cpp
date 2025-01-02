// de_line.cpp
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <poi.h>
#include <fim.h>
#include <fs.h>
#include <fl.h>
#include <rs.h>
#include <de.h>

//////////////////////////////////////////////////////////////////
de_line::de_line(const fl_line * pFlLine)
		: m_pFlLine(pFlLine),
		  m_bValidUID(false),
		  m_bOmitted(false),
		  m_pSpanFirst(NULL),
		  m_pSpanLast(NULL),
		  m_pCTXEOL(NULL),
		  m_ndxLineCmp(-1)
{
	m_rowNr[SYNC_VIEW] = 0;
	m_rowNr[SYNC_EDIT] = 0;
	m_pDeMark[SYNC_VIEW] = NULL;
	m_pDeMark[SYNC_EDIT] = NULL;

	m_cacheEditOpCounter = pFlLine->getEditOpCounter() - 1;	// invalidate cache
	m_cacheUIDParams = 0x00;

	DEBUG_CTOR(T_DE_LINE,L"de_line");
}

de_line::~de_line(void)
{
	wxASSERT_MSG( (!hasMark(SYNC_VIEW)), _T("Coding Error") );
	wxASSERT_MSG( (!hasMark(SYNC_EDIT)), _T("Coding Error") );
		
	DELETE_LIST(de_span,m_pSpanFirst);

	DEBUG_DTOR(T_DE_LINE);
}

//////////////////////////////////////////////////////////////////

const rs_context * de_line::setStringsAndComputeContext(const rs_ruleset * pRS, const rs_context * pCTX, bool bCacheRSChanged, bool * pbCacheWasValid)
{
	// take all the runs on this line and build a single string so that we
	// can scan it easily.  we keep the specific chars in the EOL separate.
	// [keeping EOL's out of the body helps make RegEx's work better because
	// we don't have to use something like (\r+\n+)$ in order to use $.
	//
	// for now, we store a copy of the string of the entire line so that we can
	// quickly subscript/index into it as we scan for contexts and later as we
	// draw/highlight.  this may not be necessary, so we may want to revisit this.

	//////////////////////////////////////////////////////////////////
	// every time the diff-engine runs we need to build this string
	// by concatenating the runs on the line in the layout.  99+% of
	// the time this string will be identical to the string we constructed
	// the last time the diff-engine ran --- except for the individual
	// lines that were edited.  the fl_lines now keep an edit-op-counter
	// that is incremented when a line is edited (and decremented when
	// the edit is due to UNDO).  we can use this as an indicator of
	// when the line is dirty (with respect to the last time the diff-
	// engine ran).

	if (m_cacheEditOpCounter == m_pFlLine->getEditOpCounter())
	{
		// the layout-line has not changed since the last time we were
		// called for this line.  therefore, we shouldn't need to rebuild
		// the string.

		*pbCacheWasValid = true;

#if 0
#ifdef _DEBUG
		// if cached op counter valid, ensure contents identical.

		wxString strFooLine, strFooEOL;
		m_pFlLine->buildStringsFromRuns(false,&strFooLine,&strFooEOL);
		bool bLineSame = (strFooLine == m_strLine);
		bool bEOLSame = (strFooEOL == m_strEOL);
		bool bContentSame = (bLineSame && bEOLSame);
	
		wxASSERT_MSG( (bContentSame), _T("Coding Error") );		
#endif
#endif

		//////////////////////////////////////////////////////////////////
		// now see if we can salvage the span-list and context information.
		// that is, if the line hasn't changed and the starting context
		// that we were given matches the context in the first span (what
		// it was last time we were called for this line), then we should
		// be able to reuse the span list and just return the final
		// context for this line (the same value we returned the last time
		// we were called for this line).
		//
		// WARNING: we assume here that if the ruleset changes between
		// WARNING: runs that we will get different pointers for the
		// WARNING: context; that is, when the ruleset is edited in the
		// WARNING: options dialog and the new version is installed, the
		// WARNING: set of contexts will be allocated and have a different
		// WARNING: pointer value.  [this won't be true for the NULL
		// WARNING: context -- so we also need to cache the ruleset used.]

		if ( !bCacheRSChanged  &&  m_pSpanFirst  &&  (m_pSpanFirst->getContext() == pCTX))
			return m_pCTXEOL;

		// otherwise, the context has changed.  so we need to
		// compute it again.
	}
	else
	{
		// cached op counter invalid, contents may or may not match.
		// so we need to rebuild the strings (and then re-compute the
		// contexts).

		*pbCacheWasValid = false;

		m_pFlLine->buildStringsFromRuns(false,&m_strLine,&m_strEOL);
		m_cacheEditOpCounter = m_pFlLine->getEditOpCounter();
	}

	// we are given the contents of the document for the line
	// we represent and the specific chars in the EOL.
	// we assume that we are in the given context (such as in
	// a multi-line comment or not).
	//
	// cut the line into spans based upon the initial context
	// (if any) and any context changes we find within the line.
	// return the ending context (if we have one).

	if (m_pSpanFirst)
	{
		// we are re-running (for example, after a change to the
		// ruleset or an edit).
		// kill the existing span list before we try to build it.

		DELETE_LIST(de_span,m_pSpanFirst);
		m_pSpanFirst = NULL;
		m_pSpanLast = NULL;
	}

	size_t offset;		// offset of beginning of delimiter from sz
	size_t len;			// length of delimiter

	const wxChar * szBegin = m_strLine.wc_str();
	const wxChar * szEnd   = szBegin + m_strLine.Len();
	const wxChar * sz      = szBegin;

	while (sz < szEnd)
	{
//		wxLogTrace(TRACE_DE_DUMP,_T("setAndComputeContext: sz [%s]"),sz);

		if (pCTX)
		{
			// we are currently in a context, try to end it.

			if (pCTX->isEndMatch(sz,&offset,&len))
			{
				// we found an end-delimiter.  put all content upto
				// and including the end-delimiter into the span.
				// close the currently open context and loop back
				// to deal with the rest of the line.

				appendSpan(szBegin,pCTX,(sz-szBegin),offset+len);
				sz += offset+len;
				pCTX = NULL;
			}
			else
			{
				// we did not find an end-delimiter.  so the rest of
				// the line must be part of this context.  consume it
				// all and return the currently open context.

				if (szEnd > sz)
					appendSpan(szBegin,pCTX,(sz-szBegin),(szEnd-sz));
				m_pCTXEOL = pCTX;
				return pCTX;
			}
		}
		else
		{
			// we are not currently in a context.  see if can
			// start one.

			pCTX = pRS->findStartContext(sz,&offset,&len);

			if (pCTX)
			{
#if 0
				// we found the start of a new context, put the
				// part before the start-delimiter into the null
				// context and the start-delimiter into the new
				// context.  loop back to deal with the rest of
				// the line.

				if (offset > 0)
				{
					appendSpan(szBegin,NULL,(sz-szBegin),offset);
					sz += offset;
				}
				
				appendSpan(szBegin,pCTX,(sz-szBegin),len);
				sz += len;
#else
				// we found the start of a new context, put the
				// part before the start-delimiter *AND* the
				// start-delimiter itself into the NULL context.
				// loop back to deal with the rest of the line.

				appendSpan(szBegin,NULL,(sz-szBegin),offset+len);
				sz += offset+len;
#endif
			}
			else
			{
				// we did not find a start-delimiter.  so the rest
				// of the line must be in the null context.  consume
				// it all and return.

				if (szEnd > sz)
					appendSpan(szBegin,NULL,(sz-szBegin),(szEnd-sz));
				m_pCTXEOL = NULL;
				return NULL;
			}
		}
	}

	// if we get here, the line ended with either the
	// begin- or end-delimiter.
	//
	// if the line ended with the begin-delimiter,
	// normally we'd want to return this "open"
	// context for the next line but if the context
	// has bEndsAtEOL set, we implicitly close it.

	if (pCTX && pCTX->isEndMatch(sz,&offset,&len))
		pCTX = NULL;

	m_pCTXEOL = pCTX;
	return pCTX;
}

//////////////////////////////////////////////////////////////////

void de_line::appendSpan(const wxChar * sz, const rs_context * pCTX, size_t offset, size_t len)
{
	// sz[offset .. offset+len-1] represents part of a span of text
	// that is in the given context.  it may be adjacent to other
	// spans in the same or in different contexts.
	//
	// we need to create a de_span for it.
	//
	// to facilitate later whitespace handling (changes in whitespace
	// in important contexts can be marked unimportant), we chop this
	// span on whitespace into (potentially) many sub spans.  (and
	// assume that the lower level does not coalesce them.)
	//
	// [the reason we do this is because when the intra-line sync-list
	// gets built, we chop it up into non-context-crossing sync nodes.
	// and we do that by chopping it up on span boundaries -- having
	// a few extra spans won't hurt anything.]

	wxASSERT_MSG( (len > 0), _T("Coding Error!") );
	wxASSERT_MSG( (   (( m_pSpanLast) && (offset == m_pSpanLast->m_offset+m_pSpanLast->m_len))
				   || ((!m_pSpanLast) && (offset == 0))),
				  _T("Coding Error!") );

//	wxLogTrace(TRACE_DE_DUMP,_T("     appendSpan: sz [%s] offset [%ld] len [%ld] : [%.*s]"),sz,offset,len,len,sz+offset);

	size_t end = offset+len;
	size_t begin = offset;
	size_t e = begin;

	while (begin < end)
	{
		// create sub-span for non-white
		while ((e < end) && ((sz[e] != 0x00020) && (sz[e] != 0x0009)))
			e++;
		if (e != begin)
			appendSpan(pCTX,begin,(e-begin));
		begin = e;

		// create sub-span for whitespace
		while ((e < end) && ((sz[e] == 0x00020) || (sz[e] == 0x0009)))
			e++;
		if (e != begin)
			appendSpan(pCTX,begin,(e-begin));
		begin = e;
	}
}
	
void de_line::appendSpan(const rs_context * pCTX, size_t offset, size_t len)
{
	wxASSERT_MSG( (len > 0), _T("Coding Error!") );
	wxASSERT_MSG( (   (( m_pSpanLast) && (offset == m_pSpanLast->m_offset+m_pSpanLast->m_len))
				   || ((!m_pSpanLast) && (offset == 0))),
				  _T("Coding Error!") );

//	wxLogTrace(TRACE_DE_DUMP,_T("          appendSpan: offset [%ld] len [%ld]"), offset, len);

	// we do not attempt to coalesce spans.

	de_span * pSpan = new de_span(pCTX,offset,len);

	pSpan->m_prev = m_pSpanLast;
	if (m_pSpanLast)
		m_pSpanLast->m_next = pSpan;
	m_pSpanLast = pSpan;

	if (!m_pSpanFirst)
		m_pSpanFirst = pSpan;
}

//////////////////////////////////////////////////////////////////

#ifdef DEBUG
void de_line::dump(int indent) const
{
	wxLogTrace(TRACE_DE_DUMP,_T("%*cDE_LINE: [%s]"),indent,_T(' '),m_strLine.wc_str());
	for (const de_span * pSpan=m_pSpanFirst; pSpan; pSpan=pSpan->m_next)
		wxLogTrace(TRACE_DE_DUMP,_T("%*cDE_SPAN: [%p][%ld,%ld]"),indent+5,' ',pSpan->m_pCTX,pSpan->m_offset,pSpan->m_len);
	wxLogTrace(TRACE_DE_DUMP,_T("%*cDE_SPAN: [%p] eol"),indent+5,_T(' '),m_pCTXEOL);
}
#endif
