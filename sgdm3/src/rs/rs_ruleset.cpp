// rs_ruleset.cpp
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <poi.h>
#include <rs.h>

//////////////////////////////////////////////////////////////////

static RS_ID g_RS_ID_Next = 0;

//////////////////////////////////////////////////////////////////

rs_ruleset::rs_ruleset(void)
	: m_bSniffEncodingBOM(true),
	  m_encodingStyle(RS_ENCODING_STYLE_LOCAL),
	  m_encodingSetting1(wxFONTENCODING_DEFAULT),
	  m_encodingSetting2(wxFONTENCODING_DEFAULT),
	  m_encodingSetting3(wxFONTENCODING_DEFAULT),
	  m_matchStripAttrs( RS_ATTRS_TAB_IS_WHITE ),
	  m_defaultContextAttrs( RS_ATTRS_RESPECT_EOL | RS_ATTRS_RESPECT_WHITE | RS_ATTRS_RESPECT_CASE ),
	  m_equivalenceAttrs( RS_ATTRS_TAB_IS_WHITE )
{
	m_ID = g_RS_ID_Next++;

//	wxLogTrace(TRACE_RS_DUMP,_T("rs_ruleset::rs_ruleset: [%p] default ctor"),this);
}

rs_ruleset::~rs_ruleset(void)
{
//	wxLogTrace(TRACE_RS_DUMP,_T("rs_ruleset::rs_ruleset: [%p] dtor"),this);

	for (TVector_LOmit_Iterator it = m_vecLOmit.begin(); (it != m_vecLOmit.end()); it++)
	{
		rs_lomit * pLO = (*it);
		delete pLO;
	}

	for (TVector_Context_Iterator it = m_vecContext.begin(); (it != m_vecContext.end()); it++)
	{
		rs_context * pCXT = (*it);
		delete pCXT;
	}
}

//////////////////////////////////////////////////////////////////

rs_ruleset::rs_ruleset(const rs_ruleset & rs)	// copy constructor
{
//	wxLogTrace(TRACE_RS_DUMP,_T("rs_ruleset::rs_ruleset: [%p] copy ctor"),this);

	m_ID = rs.m_ID;
	
	m_strName = rs.m_strName;
	m_strSuffixes = rs.m_strSuffixes;

	m_bSniffEncodingBOM = rs.m_bSniffEncodingBOM;
	m_encodingStyle = rs.m_encodingStyle;
	m_encodingSetting1 = rs.m_encodingSetting1;
	m_encodingSetting2 = rs.m_encodingSetting2;
	m_encodingSetting3 = rs.m_encodingSetting3;

	for (TVector_LOmit_ConstIterator it = rs.m_vecLOmit.begin(); (it != rs.m_vecLOmit.end()); it++)
	{
		const rs_lomit * pLO = (*it);
		rs_lomit * pNew = new rs_lomit(*pLO);	// deep-copy LOmit from source ruleset
		
		m_vecLOmit.push_back(pNew);
	}

	for (TVector_Context_ConstIterator it = rs.m_vecContext.begin(); (it != rs.m_vecContext.end()); it++)
	{
		const rs_context * pCTX = (*it);
		rs_context * pNew = new rs_context(*pCTX);	// deep-copy context from source

		m_vecContext.push_back(pNew);
	}

	m_matchStripAttrs = rs.m_matchStripAttrs;
	m_defaultContextAttrs = rs.m_defaultContextAttrs;
	m_equivalenceAttrs = rs.m_equivalenceAttrs;
}

//////////////////////////////////////////////////////////////////

rs_ruleset::rs_ruleset(const wxString & strName, const wxString & strSuffixes,
					   bool bSniff, RS_ENCODING_STYLE style, util_encoding enc,
					   rs_context_attrs attrsMatchStrip, rs_context_attrs attrsDefaultContext,
					   rs_context_attrs attrsEquivalence)
	: m_strName(strName), m_strSuffixes(strSuffixes),
	  m_bSniffEncodingBOM(bSniff), m_encodingStyle(style),
	  m_encodingSetting1(enc), m_encodingSetting2(enc), m_encodingSetting3(enc),
	  m_matchStripAttrs(attrsMatchStrip), m_defaultContextAttrs(attrsDefaultContext),
	  m_equivalenceAttrs(attrsEquivalence)
{
	m_ID = g_RS_ID_Next++;
}

//////////////////////////////////////////////////////////////////

rs_ruleset * rs_ruleset::clone(void) const
{
	// clone this ruleset and update some of the fields
	// so that we can distinguish it from the original.

	rs_ruleset * pNew = new rs_ruleset(*this);

	// update the name to be "copy of our name" -- this does not guarantee
	// that it's unique in the ruleset-table, but helps the user somewhat.
	// TODO consider appending number or something to force it to be unique.

	pNew->setName( wxString::Format( _("Copy of %s"), m_strName.wc_str()) );

	// update the ID and force it to be unique -- see rs_ruleset.h

	pNew->m_ID = g_RS_ID_Next++;

	return pNew;
}

//////////////////////////////////////////////////////////////////

bool rs_ruleset::isEqual(const rs_ruleset * pRS) const
{
	if (!pRS) return false;

	if (m_ID != pRS->m_ID) return false;

	if (m_strName != pRS->m_strName) return false;
	if (m_strSuffixes != pRS->m_strSuffixes) return false;

	if (m_bSniffEncodingBOM != pRS->m_bSniffEncodingBOM) return false;
	if (m_encodingStyle != pRS->m_encodingStyle) return false;
	if (m_encodingSetting1 != pRS->m_encodingSetting1) return false;
	if (m_encodingSetting2 != pRS->m_encodingSetting2) return false;
	if (m_encodingSetting3 != pRS->m_encodingSetting3) return false;

	if (m_vecLOmit.size() != pRS->m_vecLOmit.size()) return false;
	size_t nLOmit = m_vecLOmit.size();
	for (size_t kLOmit=0; kLOmit<nLOmit; kLOmit++)
		if (!m_vecLOmit[kLOmit]->isEqual(pRS->m_vecLOmit[kLOmit]))
			return false;

	if (m_vecContext.size() != pRS->m_vecContext.size()) return false;
	size_t nContext = m_vecContext.size();
	for (size_t kContext=0; kContext<nContext; kContext++)
		if (!m_vecContext[kContext]->isEqual(pRS->m_vecContext[kContext]))
			return false;

	if (m_matchStripAttrs != pRS->m_matchStripAttrs) return false;
	if (m_defaultContextAttrs != pRS->m_defaultContextAttrs) return false;
	if (m_equivalenceAttrs != pRS->m_equivalenceAttrs) return false;

	return true;
}

//////////////////////////////////////////////////////////////////

void rs_ruleset::setName(const wxString & str)
{
	// this should only be called by rs_ruleset_table
	// because it needs to make sure our name is unique
	// over the collection of rulesets.

	m_strName = str;
}

void rs_ruleset::setSuffixes(const wxString & str)
{
	m_strSuffixes = str;
}

//////////////////////////////////////////////////////////////////

bool rs_ruleset::testPathnameSuffix(bool bRulesetIgnoreSuffixCase,
									const poi_item * pPoi) const
{
	// see if the suffix of the given pathname matches one of the
	// ones in our set of suffixes.  if so, this ruleset could be
	// used for this document.  [provided that we match the other
	// documents in the document set.]
	//
	// return true if we have a match.

	wxStringTokenizer tkz(m_strSuffixes);

	if (tkz.HasMoreTokens() == false)			// list of suffixes is effectively blank
		return true;

	wxFileName fn = pPoi->getFileName();

	// fetch suffix of pathname -- if it doesn't have a suffix, use
	// the base-name (filename without path).

	wxString sfx;
	if (fn.HasExt()  &&  (fn.GetExt().Length() > 0))
		sfx = fn.GetExt();
	else
		sfx = fn.GetName();
	
	while (tkz.HasMoreTokens())
	{
		wxString token = tkz.GetNextToken();

		if (token == _T("*"))					// let .* match anything
			return true;
		
		bool bEqual;
		if (bRulesetIgnoreSuffixCase)
			bEqual = (sfx.CmpNoCase(token) == 0);
		else
			bEqual = (sfx.Cmp(token) == 0);

		if (bEqual)
			return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////

void rs_ruleset::setEncodingStyle(RS_ENCODING_STYLE style)
{
	// update the character encoding values for this
	// ruleset.  this only affects future document sets
	// because it determines whether we use a named encoding
	// or if we ask the user to specify one as a document
	// is loaded.

	m_encodingStyle = style;
}

void rs_ruleset::setEncoding1(util_encoding enc)
{
	// update the character encoding values for this
	// ruleset.  this only affects future document sets
	// since we can't change the encoding of an already
	// loaded document set without reloading from disk.
	// and that should be explicitly requested by the
	// user.

	m_encodingSetting1 = enc;
}
void rs_ruleset::setEncoding2(util_encoding enc)
{
	m_encodingSetting2 = enc;
}

void rs_ruleset::setEncoding3(util_encoding enc)
{
	m_encodingSetting3 = enc;
}

void rs_ruleset::setSniffEncodingBOM(bool bSniff)
{
	m_bSniffEncodingBOM = bSniff;
}

//////////////////////////////////////////////////////////////////

bool rs_ruleset::addLOmit(const wxString & strPattern, int nrLines)
{
	rs_lomit * p = new rs_lomit(strPattern,nrLines);

	return addLOmit(p);
}

bool rs_ruleset::addLOmit(rs_lomit * p)
{
	bool bValid = (p  &&  p->isValid());
	if (bValid)
		m_vecLOmit.push_back(p);
	else
		delete p;

	return bValid;
}

void rs_ruleset::replaceLOmit(int index, rs_lomit * pLOmit_New, bool bDelete)
{
	rs_lomit * pLOmit_Old = m_vecLOmit[index];

	m_vecLOmit[index] = pLOmit_New;

	if (bDelete)
		delete pLOmit_Old;
}

void rs_ruleset::deleteLOmit(int index)
{
	int k = 0;

	for (TVector_LOmit_Iterator it = m_vecLOmit.begin(); (it != m_vecLOmit.end()); it++, k++)
	{
		if (k == index)
		{
			rs_lomit * pLOmit = (*it);
			delete pLOmit;
			m_vecLOmit.erase(it);
			return;
		}
	}
}

int rs_ruleset::testLOmit(const wxChar * szTest) const
{
	// test all LOmit patterns against the given string
	// and return the number of lines to skip of the
	// first match.
	//
	// return 0 if none were matched. (the expected case)

	for (TVector_LOmit_ConstIterator it=m_vecLOmit.begin(); it != m_vecLOmit.end(); it++)
	{
		const rs_lomit * p = (*it);
		if (!p) continue;

//		wxLogTrace(TRACE_RS_DUMP, _T("test: RegEx __%s__ [%s] yields %d"), p->getPattern()->wc_str(), szTest, p->isMatch(szTest));

		if (p->isMatch(szTest))
			return p->getNrLinesToSkip();
	}

	return 0;
}

//////////////////////////////////////////////////////////////////

void rs_ruleset::addContext(rs_context * pCXT)
{
	m_vecContext.push_back(pCXT);
}

void rs_ruleset::replaceContext(int index, rs_context * pCXT_New, bool bDelete)
{
	rs_context * pCXT_Old = m_vecContext[index];

	m_vecContext[index] = pCXT_New;

	if (bDelete)
		delete pCXT_Old;
}

void rs_ruleset::deleteContext(int index)
{
	int k = 0;
	
	for (TVector_Context_Iterator it = m_vecContext.begin(); (it != m_vecContext.end()); it++, k++)
	{
		if (k == index)
		{
			rs_context * pCXT = (*it);
			delete pCXT;
			m_vecContext.erase(it);
			return;
		}
	}
}

const rs_context * rs_ruleset::findStartContext(const wxChar * szLine, size_t * pOffset, size_t * pLen) const
{
	// walk vector of contexts and test the given line of text.
	// return the context that matches the first.  that is, the
	// one that matches earliest in the line -- not the first in
	// the vector.
	//
	// if the input is:
	// 			abc /* def "hello" ghi */
	// we want to start a comment-context not a string-literal-context.

	const rs_context * pCXTBest = NULL;
	size_t offsetBest = LONG_MAX;
	size_t lenBest = 0;

	for (TVector_Context_ConstIterator it = m_vecContext.begin(); (it != m_vecContext.end()); it++)
	{
		const rs_context * pCXT = (*it);

		size_t offset;
		size_t len;
		if (pCXT->isStartMatch(szLine,&offset,&len))
		{
			if (offset < offsetBest)
			{
				pCXTBest = pCXT;
				offsetBest = offset;
				lenBest = len;
			}
		}
	}

	if (pCXTBest)
	{
		if (pOffset) *pOffset = offsetBest;
		if (pLen)    *pLen    = lenBest;
	}

	return pCXTBest;
}

//////////////////////////////////////////////////////////////////

#ifdef DEBUG
void rs_ruleset::dump(int indent) const
{
	wxLogTrace(TRACE_RS_DUMP, _T("%*cRS_RULESET: [%p][%d][%s]"),
			   indent,' ',
			   this, m_ID, m_strName.wc_str());
	wxLogTrace(TRACE_RS_DUMP, _T("%*c[sniffBOM %d][EncStyle %d][Encoding1: %d=%s][Encoding2: %d=%s][Encoding3: %d=%s]"),
			   indent+5,' ',
			   m_bSniffEncodingBOM,
			   m_encodingStyle,
			   m_encodingSetting1,wxFontMapper::GetEncodingName(m_encodingSetting1).wc_str(),
			   m_encodingSetting2,wxFontMapper::GetEncodingName(m_encodingSetting2).wc_str(),
			   m_encodingSetting3,wxFontMapper::GetEncodingName(m_encodingSetting3).wc_str());

	for (TVector_LOmit_ConstIterator it = m_vecLOmit.begin(); (it != m_vecLOmit.end()); it++)
	{
		const rs_lomit * pLO = (*it);
		pLO->dump(indent+5);
	}

	for (TVector_Context_ConstIterator it = m_vecContext.begin(); (it != m_vecContext.end()); it++)
	{
		const rs_context * pCXT = (*it);
		pCXT->dump(indent+5);
	}

	wxLogTrace(TRACE_RS_DUMP, _T("%*cDefaultContextAttrs: [%x]"),
			   indent+5,' ',
			   m_defaultContextAttrs);

	wxLogTrace(TRACE_RS_DUMP, _T("%*cMatchStripAttrs: [%x]"),
			   indent+5,' ',
			   m_matchStripAttrs);
	
	wxLogTrace(TRACE_RS_DUMP, _T("%*cEquivalenceAttrs: [%x]"),
			   indent+5,' ',
			   m_equivalenceAttrs);
}
#endif

//////////////////////////////////////////////////////////////////

int rs_ruleset::getNamedEncodingArray(int /*lenEnc*/, util_encoding aEnc[]) const
{
	int nr = 0;

	switch (m_encodingStyle)
	{
	case RS_ENCODING_STYLE_NAMED1:
		aEnc[ nr++ ] = m_encodingSetting1;
		break;

	case RS_ENCODING_STYLE_NAMED2:
		aEnc[ nr++ ] = m_encodingSetting1;
		if (m_encodingSetting2 != m_encodingSetting1)
			aEnc[ nr++ ] = m_encodingSetting2;
		break;

	case RS_ENCODING_STYLE_NAMED3:
		aEnc[ nr++ ] = m_encodingSetting1;
		if (m_encodingSetting2 != m_encodingSetting1)
			aEnc[ nr++ ] = m_encodingSetting2;
		if ((m_encodingSetting3 != m_encodingSetting1) && (m_encodingSetting3 != m_encodingSetting2))
			aEnc[ nr++ ] = m_encodingSetting3;
		break;

	default:
		// not worth asserting
		aEnc[ nr++ ] = wxFONTENCODING_DEFAULT;
		break;
	}

	return nr;
}

