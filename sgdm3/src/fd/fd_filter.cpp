// fd_filter.cpp
// class for determining if a given pathname should be filtered
// out of a folderdiff window.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fd.h>

//////////////////////////////////////////////////////////////////

fd_filter::fd_filter(void)
{
	m_bIgnoreSubdirFilter =       (gpGlobalProps->getLong(GlobalProps::GPL_FOLDER_IGNORE_SUBDIR_FILTER) == 1); /*gui*/
	m_bIgnoreSuffixFilter =       (gpGlobalProps->getLong(GlobalProps::GPL_FOLDER_IGNORE_SUFFIX_FILTER) == 1); /*gui*/
	m_bIgnoreFullFilenameFilter = (gpGlobalProps->getLong(GlobalProps::GPL_FOLDER_IGNORE_FULL_FILENAME_FILTER) == 1); /*gui*/
	m_bIgnorePatternCase =        (gpGlobalProps->getLong(GlobalProps::GPL_FOLDER_IGNORE_PATTERN_CASE) == 1); /*gui*/
	m_bIgnoreMatchupCase =        (gpGlobalProps->getLong(GlobalProps::GPL_FOLDER_IGNORE_MATCHUP_CASE) == 1); /*gui*/

	m_strSubdirs =       gpGlobalProps->getString(GlobalProps::GPS_FOLDER_SUBDIR_FILTER); /*gui*/
	m_strSuffixes =      gpGlobalProps->getString(GlobalProps::GPS_FOLDER_SUFFIX_FILTER); /*gui*/
	m_strFullFilenames = gpGlobalProps->getString(GlobalProps::GPS_FOLDER_FULL_FILENAME_FILTER); /*gui*/

	fd_filter::parse_string_into_set( m_setSubdirs,       m_strSubdirs,       m_bIgnorePatternCase );
	fd_filter::parse_string_into_set( m_setSuffixes,      m_strSuffixes,      m_bIgnorePatternCase );
	fd_filter::parse_string_into_set( m_setFullFilenames, m_strFullFilenames, m_bIgnorePatternCase );
}

fd_filter::~fd_filter(void)
{
}

//////////////////////////////////////////////////////////////////
// isFiltered{Subdir,Suffix}() are called by fd_fd during treewalk.
//////////////////////////////////////////////////////////////////

bool fd_filter::isFilteredSubdir(const wxString & strPathname) const
{
	if (m_bIgnoreSubdirFilter)
		return false;

	// we assume we are given the (absolute or relative) pathname
	// where the final component is the directory in question and
	// without the trailing slash. so we can stuff it into a wxFileName
	// to get the final component as the "filename".

	wxFileName fn(strPathname);
	wxString strDirname(fn.GetFullName());
	
//	wxLogTrace(wxTRACE_Messages, _T("FilterSubdir: [%s][%s]"),
//			   strPathname.wc_str(),
//			   strDirname.wc_str());

	if (m_bIgnorePatternCase)
		strDirname.MakeLower();

	return fd_filter::is_filtered( m_setSubdirs, strDirname, true );
}

bool fd_filter::isFilteredSuffix(const wxString & pathname) const
{
	if (m_bIgnoreSuffixFilter)
		return false;

	wxFileName fn(pathname);
	wxString strSuffix(fn.GetExt());

	return fd_filter::is_filtered( m_setSuffixes, strSuffix, false );
}

bool fd_filter::isFilteredFullFilename(const wxString & strPathname) const
{
	if (m_bIgnoreFullFilenameFilter)
		return false;

	wxFileName fn(strPathname);
	wxString strFilename(fn.GetFullName());

	if (m_bIgnorePatternCase)
		strFilename.MakeLower();

	return fd_filter::is_filtered( m_setFullFilenames, strFilename, true );
}

//////////////////////////////////////////////////////////////////

/**
 * Parse the given string into individual tokens and add each
 * to the given SET.  We overwrite the contents of the given SET
 * first.
 *
 * Unlike the stock strtok() or wxStringTokenizer() approash,
 * we handle some escape/quote sequences.
 *
 */
/*static*/ void fd_filter::parse_string_into_set(fd_filter::TSet & S,
												 const wxString & strInput,
												 bool bIgnorePatternCase)
{
	const wxChar * p;
	wxString strInputLowerCase;
	wxString strToken;
	bool bHaveDQ = false;

	S.clear();

	if (bIgnorePatternCase)
	{
		strInputLowerCase = strInput.Lower();
		p = strInputLowerCase.wc_str();
	}
	else
	{
		p = strInput.wc_str();
	}

	while (*p)
	{
		if (bHaveDQ)	// inside a DQ string
		{
			switch (*p)
			{
			case L'"':
				if (p[1] == L'"')			// we are looking at the DQ after c:  "abc""def"
				{
					strToken += L'"';		// put a single DQ in the buffer and stay in DQ mode
					p += 2;
				}
				else
				{
					bHaveDQ = false;		// turn off DQ mode, but don't emit a token until
					p += 1;					// we see a delimiter:  abc" "def
				}
				break;

				// TODO 2013/05/09 We should probably throw if we see a '/' since
				// TODO            we do the filtering by filename, not pathname.
				//
				// TODO 2013/05/09 Should we also throw for a '\\' since we're not
				// TODO            doing backslash escaping.

			default:
				strToken += *p;				// in DQ mode anything goes into the token
				p++;
				break;
			}
		}
		else
		{
			switch (*p)
			{
			case L'"':
				bHaveDQ = true;				// begin DQ mode, we allow this to be
				p++;						// in the middle of a token.
				break;
				
			case L' ':						// all of the various delimiters
			case L'\t':
			case L'\r':
			case L'\n':
			case L',':
			case L';':
				if (strToken.Length() > 0)
				{
//					wxLogTrace(wxTRACE_Messages, _T("Token: [%s]"), strToken.wc_str());
					S.insert( strToken );
					strToken = _T("");
				}
				p++;
				break;

			default:
				strToken += *p;
				p++;
				break;
			}
		}
	}

	// if the string ended in DQ mode, silently assume one.
	// if no trailing delimiters, write final token.
	
	if (strToken.Length() > 0)
	{
//		wxLogTrace(wxTRACE_Messages, _T("Token: [%s]"), strToken.wc_str());
		S.insert( strToken );
	}

}

/**
 * Do plain and wildcard matching.
 *
 */
/*static*/ bool fd_filter::is_filtered(const fd_filter::TSet & S, const wxString strInput, bool bAlsoDoWildcards)
{
	for (TSetConstIterator it = S.begin(); (it != S.end()); it++)
	{
		const wxString & strPattern = (*it);

		if (strPattern == strInput)
			return true;

		if (bAlsoDoWildcards)
			if (::wxMatchWild(strPattern, strInput, false))
				return true;
	}
	
	return false;
}
