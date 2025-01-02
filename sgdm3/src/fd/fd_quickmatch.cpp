// fd_quickmatch.cpp
// class for determining if we should apply "quick" comparison
// rather than exact-match comparisons to a given pathname in a
// folder diff window.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fd.h>
#include <rs.h>

//////////////////////////////////////////////////////////////////

fd_quickmatch::fd_quickmatch(void)
{
	m_bEnable = gpGlobalProps->getBool(GlobalProps::GPL_FOLDER_QUICKMATCH_ENABLED);
	_setSuffixes();
}

fd_quickmatch::~fd_quickmatch(void)
{
}

//////////////////////////////////////////////////////////////////

void fd_quickmatch::_setSuffixes(void)
{
	m_strSuffixes = gpGlobalProps->getString(GlobalProps::GPS_FOLDER_QUICKMATCH_SUFFIX); /*gui*/

	m_setSuffixes.clear();

	wxStringTokenizer tok(m_strSuffixes, _T(" \t\r\n,;"), wxTOKEN_STRTOK);
	while (tok.HasMoreTokens())
		m_setSuffixes.insert( tok.GetNextToken() );
}

bool fd_quickmatch::isQuickSuffix(const wxString & pathname) const
{
	if (!isQuickMatchEnabled())
		return false;

	wxFileName f(pathname);
	if (m_setSuffixes.find(f.GetExt()) != m_setSuffixes.end())
		return true;			// we found this files suffix in the set

	return false;				// we did not find the suffix in the set
}

bool fd_quickmatch::isQuickMatchEnabled(void) const
{
	if (!m_bEnable)
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////

// use "quick" mode to compare the files.
util_error fd_quickmatch::compareQuick(const poi_item * pPoiItem1,
									   const poi_item * pPoiItem2,
									   int * pResult,
									   wxString & strQuickMatchInfo) const
{
	util_error ue;
	wxULongLong len1 = 0;
	wxULongLong len2 = 0;

#define RESULT_ERROR	-1
#define RESULT_DIFF		0
#define RESULT_EQUIV	2
#define RESULT_QUICK	3

	strQuickMatchInfo.Empty();

	ue = pPoiItem1->getFileSize(&len1);
	if (ue.isErr())
	{
		*pResult = RESULT_ERROR;
		return ue;
	}

	ue = pPoiItem2->getFileSize(&len2);
	if (ue.isErr())
	{
		*pResult = RESULT_ERROR;
		return ue;
	}

	if (len1 == len2)
	{
		*pResult = RESULT_QUICK;
		strQuickMatchInfo = _T("Assumed equal using Quick Match");
	}
	else
	{
		*pResult = RESULT_DIFF;
		strQuickMatchInfo = _T("Assumed different using Quick Match");
	}
	return ue;
}
