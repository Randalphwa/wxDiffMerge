// rs_lomit.cpp -- lines to be omitted from diff computation.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <poi.h>
#include <rs.h>

//////////////////////////////////////////////////////////////////

rs_lomit::rs_lomit(const wxString & str, int nr)
	: m_strPattern(str),
	  m_nrLinesToSkip(nr)
{
	// TODO decide if we also want wxRE_ICASE

	//////////////////////////////////////////////////////////////////
	// TODO decide if we need to enforce a pattern without parens.
	//////////////////////////////////////////////////////////////////

	m_rePattern.Compile(m_strPattern,wxRE_ADVANCED|wxRE_NOSUB);

	//////////////////////////////////////////////////////////////////
	// TODO decide if/how we want to complain if the pattern can't be compiled.
	//////////////////////////////////////////////////////////////////
}

rs_lomit::rs_lomit(const rs_lomit & lo)		// copy constructor
{
	m_strPattern = lo.m_strPattern;
	m_nrLinesToSkip = lo.m_nrLinesToSkip;

	m_rePattern.Compile(m_strPattern,wxRE_ADVANCED|wxRE_NOSUB);
}

//////////////////////////////////////////////////////////////////

bool rs_lomit::isEqual(const rs_lomit * pLO) const
{
	if (!pLO) return false;

	if (m_nrLinesToSkip != pLO->m_nrLinesToSkip) return false;

	if (m_strPattern != pLO->m_strPattern) return false;

	// we don't need to compare the compiled reg-ex's.

	return true;
}

//////////////////////////////////////////////////////////////////

bool rs_lomit::isMatch(const wxChar * szTest) const
{
	// return true if the given string matches our RegEx.

	return m_rePattern.IsValid() && m_rePattern.Matches(szTest);
}

//////////////////////////////////////////////////////////////////

wxString rs_lomit::getSummaryDescription(void) const
{
	if (!isValid())
		return _("Invalid Entry");

	int skip = getNrLinesToSkip();

	if (skip == 1)
		return wxString::Format( _("Each Line Matching: %s"), getPattern()->wc_str());
	else
		return wxString::Format( _("%d Lines at Each Match of: %s"), skip, getPattern()->wc_str());
}

//////////////////////////////////////////////////////////////////

#ifdef DEBUG
void rs_lomit::dump(int indent) const
{
	wxLogTrace(TRACE_RS_DUMP, _T("%*cRS_LOMIT: [pattern %s][nrLines %d]"),
			   indent,' ',
			   m_strPattern.wc_str(),m_nrLinesToSkip);
}
#endif
