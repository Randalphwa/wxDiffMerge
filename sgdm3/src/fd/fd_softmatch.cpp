// fd_softmatch.cpp
// class for determining if we should apply soft-match comparison
// rather than exact-match comparisons to a given pathname in a
// folderdiff window.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fd.h>
#include <rs.h>

//////////////////////////////////////////////////////////////////

fd_softmatch::fd_softmatch(void)
{
	m_SoftMatchMode = (fd_SoftMatchMode)gpGlobalProps->getLong(GlobalProps::GPL_FOLDER_SOFTMATCH_MODE); /*gui*/

	_setSimpleModeSuffix();

	m_bSimpleModeIgnoreEOL         = (gpGlobalProps->getLong(GlobalProps::GPL_FOLDER_SOFTMATCH_SIMPLE_IGNORE_EOL) == 1); /*gui*/
	m_bSimpleModeIgnoreWhitespace  = (gpGlobalProps->getLong(GlobalProps::GPL_FOLDER_SOFTMATCH_SIMPLE_IGNORE_WHITESPACE) == 1); /*gui*/
	m_bSimpleModeTABisWhitespace   = (gpGlobalProps->getLong(GlobalProps::GPL_FOLDER_SOFTMATCH_SIMPLE_IGNORE_TAB) == 1); /*gui*/

	m_bRulesetEnableCustom         = (gpGlobalProps->getLong(GlobalProps::GPL_FILE_RULESET_ENABLE_CUSTOM_RULESETS) == 1); /*gui*/
	m_bRulesetAutomaticMatch       = (gpGlobalProps->getLong(GlobalProps::GPL_FILE_RULESET_ENABLE_AUTOMATIC_MATCH) == 1); /*gui*/
	m_bRulesetIgnoreSuffixCase     = (gpGlobalProps->getLong(GlobalProps::GPL_FILE_RULESET_IGNORE_SUFFIX_CASE) == 1); /*gui*/

	// Note 2013/08/13 In theory, we don't need to bother with ruleset-require-complete-match
	// Note            because we always pass pairs with the same file name (give or take
	// Note            the case of the filename).  So we only need to pass 1 of the pair into
	// Note            do_automatic_best_guess() and so complete match won't matter.  However,
	// Note            I'm leaving this here because eventually I want to be able to have the
	// Note            Folder Window be able to do some kind of manual alignment (or other
	// Note            tricks) to compensate for moves/renames -- especially if we do VCS
	// Note            integration.
	m_bRulesetRequireCompleteMatch = (gpGlobalProps->getLong(GlobalProps::GPL_FILE_RULESET_REQUIRE_COMPLETE_MATCH) == 1); /*gui*/

	m_nRulesetFileLimitMb          = gpGlobalProps->getLong(GlobalProps::GPL_FOLDER_SOFTMATCH_RULESET_FILE_LIMIT_MB); /*gui*/
	m_bRulesetAllowDefault         = (gpGlobalProps->getLong(GlobalProps::GPL_FOLDER_SOFTMATCH_RULESET_ALLOW_DEFAULT) == 1); /*gui*/
}

fd_softmatch::~fd_softmatch(void)
{
}

//////////////////////////////////////////////////////////////////

void fd_softmatch::_setSimpleModeSuffix(void)
{
	m_strSimpleModeSuffixes = gpGlobalProps->getString(GlobalProps::GPS_FOLDER_SOFTMATCH_SIMPLE_SUFFIX); /*gui*/

	m_setSimpleModeSuffixes.clear();

	wxStringTokenizer tok(m_strSimpleModeSuffixes, _T(" \t\r\n,;"), wxTOKEN_STRTOK);
	while (tok.HasMoreTokens())
		m_setSimpleModeSuffixes.insert( tok.GetNextToken() );
}

//////////////////////////////////////////////////////////////////

bool fd_softmatch::isSimpleModeSuffix(const wxString & pathname) const
{
	if (!isSimpleMode())
		return false;

	wxFileName f(pathname);
	if (m_setSimpleModeSuffixes.find(f.GetExt()) != m_setSimpleModeSuffixes.end())
		return true;			// we found this files suffix in the set

	return false;				// we did not find the suffix in the set
}

//////////////////////////////////////////////////////////////////

bool fd_softmatch::isSimpleMode(void) const
{
	if (m_SoftMatchMode != FD_SOFTMATCH_MODE_SIMPLE)
		return false;

	// if all features turned off, just say no (and keep our caller
	// from comparing the files twice).

	if (!m_bSimpleModeIgnoreEOL && !m_bSimpleModeIgnoreWhitespace && !m_bSimpleModeTABisWhitespace)
		return false;

	return true;
}

bool fd_softmatch::isRulesetMode(void) const
{
	if (m_SoftMatchMode != FD_SOFTMATCH_MODE_RULESET)
		return false;

	return true;
}

bool fd_softmatch::allowDefaultRuleset(void) const
{
	return m_bRulesetAllowDefault;
}

const rs_ruleset * fd_softmatch::findRuleset(const rs_ruleset_table * pRsRuleSetTable,
											 poi_item * pPoiItem) const
{
	// find a ruleset for the given pathname.

	const rs_ruleset * pRS = NULL;
	const rs_ruleset * pRS_default = pRsRuleSetTable->getDefaultRuleSet();

	if (m_bRulesetEnableCustom && m_bRulesetAutomaticMatch)
	{
		// custom rulesets are enabled and automatic matching by suffix is enabled.
		//
		// use the find-best-without-asking algorithm to lookup
		// the first ruleset with a matching suffix.  this will
		// do the lookup in the same order, but will not ask them
		// to pick one (such as when ASK_IF_NO_MATCH is set).
		// instead it returns the default ruleset.

		pRS = pRsRuleSetTable->do_automatic_best_guess(m_bRulesetRequireCompleteMatch,
													   m_bRulesetIgnoreSuffixCase,
													   1, &pPoiItem,
													   NULL);
		if (!pRS)
			pRS = pRS_default;
	}
	else
	{
		// custom rulesets are turned off [1] -or- automatic suffix matching
		// is turned off [2] -or- both.
		//
		// for [1], file windows will use the default ruleset.  that is, file
		// windows don't work *without* a ruleset.  so we should "suggest"
		// the default ruleset too.
		//
		// for [2], a file window would prompt them for the proper ruleset.
		// this works for opening an individual window, but to get the same
		// result, we would have to ask them to pick a ruleset for each
		// line-item in the folder window!  we can either say no or "suggest"
		// the default ruleset -- it's kind of arbitrary (and contrived).

		pRS = pRS_default;
	}
	
	// using the default ruleset might be dangerous because we are going
	// to apply it to everything we find in a directory.  and if the directory
	// contains binary files (something that the user would never have
	// double-clicked on in the folder list) we could go out to lunch.
	//
	// so we allow them to decide.

	if ((pRS == pRS_default) && !m_bRulesetAllowDefault)
		return NULL;

	// if the chosen ruleset doesn't have a definitive character encoding,
	// (where we'd *always* have to ask the user), we just say no.
	// if it might be able to tell, we let it try.

	if (!pRS->getSniffEncodingBOM()
		&& ((pRS->getEncodingStyle()==RS_ENCODING_STYLE_ASK)
			|| (pRS->getEncodingStyle()==RS_ENCODING_STYLE_ASK_EACH)))
		return NULL;

	return pRS;
}

