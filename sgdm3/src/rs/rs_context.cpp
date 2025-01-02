// rs_context.cpp -- code to identify/deal with RuleSet Contexts.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <poi.h>
#include <rs.h>

//////////////////////////////////////////////////////////////////

rs_context::rs_context(rs_context_attrs attrs,
					   const wxString & strStartPattern,
					   const wxString & strEndPattern,
					   wxChar   chEscape,
					   bool bEndsAtEOL)
	: m_attrs(attrs),
	  m_strStartPattern(strStartPattern),
	  m_strEndPattern(strEndPattern),
	  m_chEscape(chEscape),
	  m_bEndsAtEOL(bEndsAtEOL),
	  m_pReStartPattern(NULL),
	  m_pReEndPattern(NULL)
{
//	wxLogTrace(TRACE_RS_DUMP,_T("rs_context::rs_context: [%p] default ctor"),this);

	//////////////////////////////////////////////////////////////////
	// TODO decide if we need to enforce a pattern without parens.
	//////////////////////////////////////////////////////////////////

	m_strStartPattern.Trim(false);	// ltrim()
	m_strStartPattern.Trim(true);	// rtrim()
	if (m_strStartPattern.Length() > 0)			{ m_pReStartPattern = new wxRegEx(m_strStartPattern,wxRE_ADVANCED); }

	m_strEndPattern.Trim(false);	// ltrim()
	m_strEndPattern.Trim(true);		// rtrim()
	if (m_strEndPattern.Length() > 0)			{ m_pReEndPattern   = new wxRegEx(m_strEndPattern,  wxRE_ADVANCED); }

	//////////////////////////////////////////////////////////////////
	// TODO decide if/how we want to complain if the patterns can't be compiled.
	// TODO need to capture error output -- because pattern errors generate message boxes...
	//////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////
	// TODO consider requiring start-pattern.
	// TODO consider requiring end-pattern or ends-at-eol.
	//////////////////////////////////////////////////////////////////
}

rs_context::~rs_context(void)
{
//	wxLogTrace(TRACE_RS_DUMP,_T("rs_context::rs_context: [%p] dtor"),this);

	delete m_pReStartPattern;
	delete m_pReEndPattern;
}

//////////////////////////////////////////////////////////////////

rs_context::rs_context(const rs_context & ctx)		// copy constructor
{
//	wxLogTrace(TRACE_RS_DUMP,_T("rs_context::rs_context: [%p] copy ctor"),this);

	m_attrs           = ctx.m_attrs;
	m_strStartPattern = ctx.m_strStartPattern;
	m_strEndPattern   = ctx.m_strEndPattern;
	m_chEscape        = ctx.m_chEscape;
	m_bEndsAtEOL      = ctx.m_bEndsAtEOL;

	m_pReStartPattern = (ctx.haveValidStartPattern()) ? new wxRegEx(m_strStartPattern,wxRE_ADVANCED) : NULL;
	m_pReEndPattern   = (ctx.haveValidEndPattern()  ) ? new wxRegEx(m_strEndPattern,  wxRE_ADVANCED) : NULL;
}

//////////////////////////////////////////////////////////////////

bool rs_context::isEqual(const rs_context * pCTX) const
{
	if (!pCTX) return false;

	if (m_attrs != pCTX->m_attrs) return false;
	if (m_chEscape != pCTX->m_chEscape) return false;
	if (m_bEndsAtEOL != pCTX->m_bEndsAtEOL) return false;

	if (m_strStartPattern != pCTX->m_strStartPattern) return false;
	if (m_strEndPattern != pCTX->m_strEndPattern) return false;

	// don't need to compare reg-ex's

	return true;
}

//////////////////////////////////////////////////////////////////

bool rs_context::isStartMatch(const wxChar * szLine, size_t * pOffset, size_t * pLen) const
{
	// find the first span that matches our start-pattern.
	// return true if we find a match;
	// return false if not.

	if (m_pReStartPattern  &&  m_pReStartPattern->IsValid()  &&  m_pReStartPattern->Matches(szLine))
		return m_pReStartPattern->GetMatch(pOffset,pLen,0);

	return false;
}

bool rs_context::_ends_in_escape(const wxChar * szBegin, const wxChar * szEnd) const
{
	// see if the sequence [szBegin,szEnd) ends with an escape.
	// we DO NOT include the end point.

	// scan the line looking to see if we end with the escape
	// character.  we have to scan to because we might have an
	// even or odd number of them consecutively.  [think of a
	// line like ending with a random number of backslashes:
	// -- is it \\LF -- a string ending with a backslash
	// -- or \LF     -- a string continued to the next line]

	if (m_chEscape == 0)					// no escape character.
		return false;

	bool bInEscape = false;
	const wxChar * sz;
	for (sz=szBegin; (sz<szEnd); sz++)
	{
		if (bInEscape)
			bInEscape = false;
		else
			bInEscape = (*sz == m_chEscape);
	}

	return bInEscape;						// do we end with a final open escape
}

bool rs_context::isEndMatch  (const wxChar * szLine, size_t * pOffset, size_t * pLen) const
{
	// find the first span that matches our end-pattern -- provided
	// that it doesn't immediately follow our escape character.
	// if it does, we need to try again.   [think "abc\"def"]
	//
	// WE ASSUME THAT THE GIVEN STRING DOES NOT HAVE THE EOL CHARACTER(S)
	// ON THE END.
	// 
	// return true if we find a match -- we can end the currently-open context.
	// return false if no match -- we cannot end the currently-open context.

	// either we have a valid end-patten or we must end at the EOL
	// [otherwise, how do we leave this context??]

	wxASSERT_MSG( (szLine), _T("Coding Error!") );

	bool bHaveEndPattern = (m_pReEndPattern && m_pReEndPattern->IsValid());
	wxASSERT_MSG( (bHaveEndPattern  ||  m_bEndsAtEOL), _T("Coding Error!"));

	if (bHaveEndPattern)
	{
		const wxChar * sz = szLine;
		while (*sz)
		{
			if (!m_pReEndPattern->Matches(sz))		// if we can't find an end-pattern match anywhere
				break;			
			else
			{
				// we found an end-pattern match, verify that it isn't
				// a false positive -- escaped -- like the dquote that's
				// actually in the string in "\""

				size_t offset;
				size_t len;
				bool bMatch = m_pReEndPattern->GetMatch(&offset,&len,0);
				MY_ASSERT( (bMatch) );

				if (!_ends_in_escape(szLine,sz+offset))
				{
					// if no open escape character immediately prior to match
					// we can use it.

					if (pOffset) *pOffset = (sz+offset - szLine);
					if (pLen)    *pLen    = len;
					return true;
				}

				// match not valid because of open escape sequence.
				// advance string and try again.

				sz += offset+1;
			}
		}
	}

	// no end-pattern or did not find a match for it.
	// see if the EOL should terminate the context.
	
	if (!m_bEndsAtEOL)
		return false;
	
	// context must end at the EOL (provided it isn't escaped).

	const wxChar * szEnd = szLine;
	while (*szEnd)
		szEnd++;

	if (_ends_in_escape(szLine,szEnd))				// line ended with final escape open
		return false;								// so we don't end the context.
	
	// line ended properly (without final escape)
		
	if (pOffset) *pOffset = (szEnd - szLine);		// offset of end
	if (pLen)    *pLen    = 0;						// length of delimiter

	return true;
}

//////////////////////////////////////////////////////////////////

wxString rs_context::getSummaryDescription(void) const
{
	if (!haveValidStartPattern())
		return _("Invalid Pattern");

	bool bHaveEndPattern = haveValidEndPattern() && (getEndPatternString()->Length() > 0);
	bool bEndsAtEOL      = getEndsAtEOL();

	wxASSERT_MSG( (bHaveEndPattern || bEndsAtEOL), _T("Coding Error!") );

	wxString strType = ((RS_ATTRS_IsUnimportant(getContextAttrs()))
						? _("Comment")
						: _("Literal"));

	wxString strESC;
	if (getEscapeChar())
		strESC = wxString::Format(_(" (Escape character %c)"), getEscapeChar());

	if (bHaveEndPattern && bEndsAtEOL)
		return wxString::Format(_("%s: %s to %s or EOL%s"),
								strType.wc_str(),
								getStartPatternString()->wc_str(),
								getEndPatternString()->wc_str(),
								strESC.wc_str());
	else if (bHaveEndPattern)
		return wxString::Format(_("%s: %s to %s%s"),
								strType.wc_str(),
								getStartPatternString()->wc_str(),
								getEndPatternString()->wc_str(),
								strESC.wc_str());
	else if (bEndsAtEOL)
		return wxString::Format(_("%s: %s to EOL%s"),
								strType.wc_str(),
								getStartPatternString()->wc_str(),
								strESC.wc_str());

	return _("Invalid Context");	// to keep compiler quiet
}

//////////////////////////////////////////////////////////////////

#ifdef DEBUG
void rs_context::dump(int indent) const
{
	wxLogTrace(TRACE_RS_DUMP, _T("%*cRS_CONTEXT: [begin %s][end %s][esc %c][eol %d][attr %x][valid %d %d]"),
			   indent,' ',
			   m_strStartPattern.wc_str(),m_strEndPattern.wc_str(),(m_chEscape?m_chEscape:_T('0')),m_bEndsAtEOL,m_attrs,
			   haveValidStartPattern() ? 1 : 0,
			   haveValidEndPattern() ? 1 : 0);
}
#endif
