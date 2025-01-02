// rs_ruleset_table.cpp
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <poi.h>
#include <rs.h>

//////////////////////////////////////////////////////////////////

rs_ruleset_table::rs_ruleset_table(void)
	: m_pRSDefault(NULL)
{
	// do minor initialization.  save big stuff for OnInit().  see gui_app::OnInit().
}

rs_ruleset_table::~rs_ruleset_table(void)
{
	// delete in-memory datastructures.

	for (TVector_RuleSetsIterator it = m_vec.begin(); (it != m_vec.end()); it++)
	{
		rs_ruleset * pRS = (*it);
		delete pRS;
	}
	m_vec.clear();

	delete m_pRSDefault;
}

//////////////////////////////////////////////////////////////////

rs_ruleset_table::rs_ruleset_table(const rs_ruleset_table & rst)	// copy constructor
{
	for (TVector_RuleSetsConstIterator it = rst.m_vec.begin(); (it != rst.m_vec.end()); it++)
	{
		const rs_ruleset * pRS = (*it);
		rs_ruleset * pNew = new rs_ruleset(*pRS);

		m_vec.push_back(pNew);
	}

	m_pRSDefault = new rs_ruleset(*rst.m_pRSDefault);
}

//////////////////////////////////////////////////////////////////

bool rs_ruleset_table::isEqual(const rs_ruleset_table * pRST) const
{
	if (!pRST) return false;

	if (!m_pRSDefault->isEqual(pRST->m_pRSDefault)) return false;

	if (m_vec.size() != pRST->m_vec.size()) return false;
	size_t n = m_vec.size();
	for (size_t k=0; k<n; k++)
		if (!m_vec[k]->isEqual(pRST->m_vec[k]))
			return false;

	return true;
}

//////////////////////////////////////////////////////////////////

void rs_ruleset_table::OnInit(bool bBuiltinOnly)
{
	if (bBuiltinOnly)
	{
		_loadBuiltinRuleSets();
	}
	else
	{
		// load only the rulesets saved in the registry.  if we get
		// an error or a bogus value, fall back to the builtin set.

		wxString strSaved = gpGlobalProps->getString(GlobalProps::GPS_FILE_RULESET_SERIALIZED); /*init*/
		if ((strSaved.Length() == 0) || (!_doImport(strSaved)))
			_loadBuiltinRuleSets();
	}
	
//	dump(10);
}

//////////////////////////////////////////////////////////////////

void rs_ruleset_table::addRuleSet(rs_ruleset * pRS)
{
	m_vec.push_back(pRS);
}

void rs_ruleset_table::replaceRuleSet(int index, rs_ruleset * pNew, bool bDelete)
{
	rs_ruleset * pOld = m_vec[index];

	m_vec[index] = pNew;

	if (bDelete)
		delete pOld;
}

void rs_ruleset_table::replaceDefaultRuleSet(rs_ruleset * pNewDefault, bool bDelete)
{
	rs_ruleset * pOld = m_pRSDefault;

	m_pRSDefault = pNewDefault;

	if (bDelete)
		delete pOld;
}

void rs_ruleset_table::deleteRuleSet(int index)
{
	// what i want to do is a: vec.erase[k] that removes the kth
	// cell from the vector (and making it shorter).  but erase()
	// only takes iterators, so we do it the hard way.

	int k = 0;
	
	for (TVector_RuleSetsIterator it = m_vec.begin(); (it != m_vec.end()); it++, k++)
		if (k == index)
		{
			rs_ruleset * pRS = (*it);
			delete pRS;
			m_vec.erase(it);
			return;
		}
}

void rs_ruleset_table::moveRuleSetUpOne(int index)
{
	// move the item at v[index] to v[index-1] shifting the one at
	// v[index-1] to v[index]

	wxASSERT_MSG( (index < (int)m_vec.size()), _T("Coding Error!") );

	if (index < 1)		// bogus call,
		return;			// nothing to do.

	TVector_RuleSetsIterator itCurrent = m_vec.begin() + index;
	if (itCurrent == m_vec.end())			// should not happen
		return;

	rs_ruleset * pRS = (*itCurrent);
	m_vec.erase(itCurrent);

	TVector_RuleSetsIterator itNew = m_vec.begin() + (index - 1);

	m_vec.insert(itNew,pRS);
}

void rs_ruleset_table::moveRuleSetDownOne(int index)
{
	// move the item at v[index] to v[index+1] shifting the one at
	// v[index+1] to v[index]

	wxASSERT_MSG( (index+1 < (int)m_vec.size()), _T("Coding Error!") );

	moveRuleSetUpOne(index+1);
}

//////////////////////////////////////////////////////////////////

const rs_ruleset * rs_ruleset_table::do_choose_ruleset_dialog(wxWindow * pParent, int nrPoi, poi_item * pPoiTable[]) const
{
	int nrRulesets = getCountRuleSets();

	if (nrRulesets == 0)		// no use raising dialog if nothing to pick
		return m_pRSDefault;
	
	rs_choose_dlg__ruleset dlg(pParent,-2,nrPoi,pPoiTable,this);
	int nr = dlg.run();

	if (nr >= 0)
		return getNthRuleSet(nr);

	if (nr == -1)	// use default
		return m_pRSDefault;

	return NULL;	// cancel
}

//////////////////////////////////////////////////////////////////

const rs_ruleset * rs_ruleset_table::do_automatic_best_guess(bool bRulesetRequireCompleteMatch,
															 bool bRulesetIgnoreSuffixCase,
															 int nrPoi, poi_item * pPoiTable[],
															 poi_item * pPoiResult) const
{
	// try to automatically guess the best ruleset for the given set of files.
	// we also use the suffix for the result-pathname, if given.
	// 
	// this can either be easy (when opening a file-diff window on "a.c"
	// vs. "b.c") or more involved (when tmp or version number suffixes
	// are used).
	//
	// WARNING: both Vault and SOS use a lot of TEMP names.

	// we have 2 automatic matching strategies:
	// [] match all -- all N file suffixes must match
	// [] match any -- any file can trigger a match (useful when "a.c" vs "a.c~")
	// either way, we take the first match we come to.

	int nrItemsToMatch;
	if (bRulesetRequireCompleteMatch)
		nrItemsToMatch = nrPoi + ((pPoiResult) ? 1 : 0);
	else
		nrItemsToMatch = 1;

	for (TVector_RuleSetsConstIterator it = m_vec.begin(); (it != m_vec.end()); it++)
	{
		const rs_ruleset * pRS = (*it);
		if (!pRS) continue;
			
		int cVotes = 0;
		for (int k=0; k<nrPoi; k++)
		{
			if (pRS->testPathnameSuffix(bRulesetIgnoreSuffixCase, pPoiTable[k]))
				cVotes++;
		}
		if (pPoiResult)
			if (pRS->testPathnameSuffix(bRulesetIgnoreSuffixCase, pPoiResult))
				cVotes++;
		
		if (cVotes >= nrItemsToMatch)
		{
			//wxLogTrace(TRACE_RS_DUMP, _T("RS:do_automatic_best_guess: matched [%s]"), pRS->getName().wc_str());
			return pRS;
		}
	}

	// could not find a match

	return NULL;
}

//////////////////////////////////////////////////////////////////
	
const rs_ruleset * rs_ruleset_table::findBestRuleSet(wxWindow * pParent, int nrPoi, poi_item * pPoiTable[], poi_item * pPoiResult) const
{
	// find the best ruleset to represent the set of documents given.
	// if we are given a poi for the result-pathname, we also search using its suffix.

	//////////////////////////////////////////////////////////////////
	// WE DO NOT REFERENCE COUNT RULESETS.  file-diff/-merge windows, their
	// diff-engines, and the file-sets will all reference the "RuleSet" and
	// these are fairly long-lasting things.  if the user (conceptually)
	// makes a change to a ruleset or adds/deletes one from the ruleset-table,
	// the options dialog builds a new ruleset-table and then fs_fs_table
	// does an "apply" that changes the ruleset on each fs_fs to the
	// "corresponding" one in the new table.  both tables (and both lists of
	// rulesets) exist while the transition is going on -- so that random
	// paint events and the like don't cause crashes.
	//////////////////////////////////////////////////////////////////

	// if custom rulesets are turned off, they always get the default ruleset.

	if (!gpGlobalProps->getBool(GlobalProps::GPL_FILE_RULESET_ENABLE_CUSTOM_RULESETS)) /*gui*/
	{
		wxLogTrace(TRACE_RS_DUMP, _T("RS:findBestRuleSet: custom rulesets disabled -- using default"));
		return m_pRSDefault;
	}

	// if automatic match turned off, we always ask them.
	// we do not include the result-pathname in the chooser dialog, because
	// this file should not exist yet.

	if (!gpGlobalProps->getBool(GlobalProps::GPL_FILE_RULESET_ENABLE_AUTOMATIC_MATCH)) /*gui*/
		return do_choose_ruleset_dialog(pParent,nrPoi,pPoiTable);

	// custom is enabled and automatic turned on.

	bool bRulesetRequireCompleteMatch = gpGlobalProps->getBool(GlobalProps::GPL_FILE_RULESET_REQUIRE_COMPLETE_MATCH); /*gui*/
	bool bRulesetIgnoreSuffixCase = gpGlobalProps->getBool(GlobalProps::GPL_FILE_RULESET_IGNORE_SUFFIX_CASE); /*gui*/

	const rs_ruleset * pRS_Guess = do_automatic_best_guess(bRulesetRequireCompleteMatch,
														   bRulesetIgnoreSuffixCase,
														   nrPoi,pPoiTable,
														   pPoiResult);
	if (pRS_Guess)
		return pRS_Guess;

	// none of them matched.  ask the user if requested.
	// we do not include the result-pathname in the chooser dialog, because
	// this file should not exist yet.

	if (gpGlobalProps->getBool(GlobalProps::GPL_FILE_RULESET_ASK_IF_NO_MATCH)) /*gui*/
		return do_choose_ruleset_dialog(pParent,nrPoi,pPoiTable);
		
	// otherwise, use our hard-coded, builtin default.

	wxLogTrace(TRACE_RS_DUMP, _T("RS:findBestRuleSet: using default"));
	return m_pRSDefault;
}

const rs_ruleset * rs_ruleset_table::findBestRuleSet_without_asking(int nrPoi, poi_item * pPoiTable[], poi_item * pPoiResult) const
{
	// find the best ruleset to represent the set of documents given ***WITHOUT ASKING THE USER***.
	// if we are given a poi for the result-pathname, we also search using its suffix.

	// if custom rulesets are turned off, they always get the default ruleset.
	// if automatic match turned off, we use default instead of asking.

	if (   !gpGlobalProps->getBool(GlobalProps::GPL_FILE_RULESET_ENABLE_CUSTOM_RULESETS) /*ok*/
		|| !gpGlobalProps->getBool(GlobalProps::GPL_FILE_RULESET_ENABLE_AUTOMATIC_MATCH)) /*ok*/
		return m_pRSDefault;

	// custom is enabled and automatic turned on.

	bool bRulesetRequireCompleteMatch = gpGlobalProps->getBool(GlobalProps::GPL_FILE_RULESET_REQUIRE_COMPLETE_MATCH); /*ok*/
	bool bRulesetIgnoreSuffixCase = gpGlobalProps->getBool(GlobalProps::GPL_FILE_RULESET_IGNORE_SUFFIX_CASE); /*ok*/

	const rs_ruleset * pRS_Guess = do_automatic_best_guess(bRulesetRequireCompleteMatch,
														   bRulesetIgnoreSuffixCase,
														   nrPoi,pPoiTable,
														   pPoiResult);
	if (pRS_Guess)
		return pRS_Guess;

	// none of them matched.  use use the default instead of asking.

	return m_pRSDefault;
}

//////////////////////////////////////////////////////////////////

const rs_ruleset * rs_ruleset_table::findByID(RS_ID id) const
{
	for (TVector_RuleSetsConstIterator it = m_vec.begin(); (it != m_vec.end()); it++)
	{
		const rs_ruleset * pRS = (*it);
		if (id == pRS->getID())
			return pRS;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////

const rs_ruleset * rs_ruleset_table::findByName(const wxString & strName) const
{
	for (TVector_RuleSetsConstIterator it = m_vec.begin(); (it != m_vec.end()); it++)
	{
		const rs_ruleset * pRS = (*it);
		if (strName == pRS->getName())
			return pRS;
	}

	return NULL;
}

const rs_ruleset * rs_ruleset_table::findBySuffixList(const wxString & strSuffixList) const
{
	for (TVector_RuleSetsConstIterator it = m_vec.begin(); (it != m_vec.end()); it++)
	{
		const rs_ruleset * pRS = (*it);
		if (strSuffixList == pRS->getSuffixes())
			return pRS;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////

void rs_ruleset_table::_loadBuiltinRuleSets(void)
{
	_loadBuiltin_default();

	_loadBuiltin_c();
	_loadBuiltin_vb();
	_loadBuiltin_python();
	_loadBuiltin_java();
	_loadBuiltin_txt();
	_loadBuiltin_utf8();
	_loadBuiltin_xml();
#if 0
#ifdef DEBUG
	_loadBuiltin_test();
#endif
#endif
}

//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// a few #defines to clarify the flags on m_matchStripAttrs
//////////////////////////////////////////////////////////////////

#define MATCH_EOL				RS_ATTRS_RESPECT_EOL
#define FOLD_EOL				0

#define MATCH_WHITE				RS_ATTRS_RESPECT_WHITE
#define STRIP_WHITE				0

#define MATCH_CASE				RS_ATTRS_RESPECT_CASE
#define FOLD_CASE				0

#define KEEP_TABS				0
#define FOLD_TABS				RS_ATTRS_TAB_IS_WHITE

// the recommended settings for match/strip -- this controls how we
// preprocess each line before handing to the diff-engine -- that is,
// what we do to it before computing the line's hash.

#define MS_RECOMMENDED			(FOLD_EOL | FOLD_CASE | STRIP_WHITE | FOLD_TABS)

// the recommended settings for ruleset-based equivalence testing
// in the folder window -- this controls what the soft-match code
// is allowed to ignore and consider equivalent.

#define FDEQ_RECOMMENDED_CASE		(FOLD_EOL | MATCH_CASE | STRIP_WHITE | FOLD_TABS)
#define FDEQ_RECOMMENDED_FOLDCASE	(FOLD_EOL | FOLD_CASE  | STRIP_WHITE | FOLD_TABS)

//////////////////////////////////////////////////////////////////
// a few #defines to clarify the flags when setting context attrs
//////////////////////////////////////////////////////////////////

#define X_UNIMP					RS_ATTRS_UNIMPORTANT
#define X_IMP					0

#define X_EOL					RS_ATTRS_RESPECT_EOL
#define X_WHITE					RS_ATTRS_RESPECT_WHITE
#define X_CASE					RS_ATTRS_RESPECT_CASE

#define X_TAB_IS_WHITE			RS_ATTRS_TAB_IS_WHITE
#define X_TAB_IS_TAB			0

//////////////////////////////////////////////////////////////////


void rs_ruleset_table::_loadBuiltin_default(void)
{
	// builtin default ruleset -- used when no suffixes match -- don't put this in the vector.

	rs_ruleset * pRS = new rs_ruleset( _("_Default_"), _T("*"),
									   true,RS_ENCODING_STYLE_LOCAL, wxFONTENCODING_DEFAULT,
									   MS_RECOMMENDED,
									   X_IMP | X_EOL | X_WHITE | X_CASE | X_TAB_IS_TAB,
									   FDEQ_RECOMMENDED_CASE);
	m_pRSDefault = pRS;
}

void rs_ruleset_table::_loadBuiltin_c(void)
{
	// builtin ruleset for C/C++
	//
	// for C/C++/C#:
	//    normal code: important, respect-case, ignore-white, tab-is-white, ignore-eols
	//    comments:    unimportant, ignore-case, ignore-white, ignore-tabs, ignore-eols
	//    strings:     important, respect-case, respect-white, tab-is-tab, respect-eols
	//       (because c strings end at eol unless escaped (and because they are usually
	//       stripped) the -eols setting doesn't usually matter)

	rs_ruleset * pRS = new rs_ruleset( _("C/C++/C# Source"), _T("c cpp cs h"),
									   true,RS_ENCODING_STYLE_NAMED1,wxFONTENCODING_ISO8859_1,
									   MS_RECOMMENDED,
									   X_IMP | X_CASE | X_TAB_IS_WHITE,
									   FDEQ_RECOMMENDED_CASE);

	// double-backslashes -- one to get past the C compiler and one to get
	// past the RegEx compiler.

	// comment contexts

	pRS->addContext( new rs_context(X_UNIMP | X_TAB_IS_WHITE,                _T("/\\*"), _T("\\*/"),        0, false));
	pRS->addContext( new rs_context(X_UNIMP | X_TAB_IS_WHITE,                _T("//"),   _T(""),     _T('\\'),  true));

	// string literal contexts

	pRS->addContext( new rs_context(X_IMP | X_EOL | X_WHITE | X_CASE | X_TAB_IS_TAB, _T("\""),  _T("\""),    _T('\\'),  true));
	pRS->addContext( new rs_context(X_IMP | X_EOL | X_WHITE | X_CASE | X_TAB_IS_TAB, _T("'"),   _T("'"),     _T('\\'),  true));

	addRuleSet(pRS);
}

void rs_ruleset_table::_loadBuiltin_vb(void)
{
	// builtin ruleset for Visual Basic
	//
	// for Visual Basic:
	//    normal code: important, ignore-case, tab-is-white, ignore-eols
	//    comments:    unimportant, ignore-case, ignore-white, ignore-tabs, ignore-eols
	//    strings:     important, respect-case, respect-white, tab-is-tab, respect-eols

	rs_ruleset * pRS = new rs_ruleset( _("Visual Basic Source"), _T("bas frm cls vbp ctl vbs"),
									   true,RS_ENCODING_STYLE_NAMED1,wxFONTENCODING_ISO8859_1,
									   MS_RECOMMENDED,
									   X_IMP | X_TAB_IS_WHITE,
									   FDEQ_RECOMMENDED_FOLDCASE);

	// double-backslashes -- one to get past the C compiler and one to get
	// past the RegEx compiler.

	// comment contexts

	pRS->addContext( new rs_context(X_UNIMP | X_TAB_IS_WHITE,                        _T("'"),    _T(""),    0, true));

	// string literal contexts

	pRS->addContext( new rs_context(X_IMP | X_EOL | X_WHITE | X_CASE | X_TAB_IS_TAB, _T("\""),  _T("\""),   0, false));

	addRuleSet(pRS);
}

void rs_ruleset_table::_loadBuiltin_python(void)
{
	// builtin ruleset for Python
	//
	// for Python:
	//    normal code: important, respect-case, respect-white, tab-is-tab, ignore-eols
	//    comments:    unimportant, ignore-case, ignore-white, tab-is-white, ignore-eols
	//    strings:     important, respect-case, respect-white, tab-is-tab, respect-eols

	rs_ruleset * pRS = new rs_ruleset( _("Python Source"), _T("py"),
									   true,RS_ENCODING_STYLE_NAMED1,wxFONTENCODING_ISO8859_1,
									   MS_RECOMMENDED,
									   X_IMP | X_CASE | X_WHITE | X_TAB_IS_TAB,
									   FDEQ_RECOMMENDED_CASE);

	// double-backslashes -- one to get past the C compiler and one to get
	// past the RegEx compiler.

	// comment contexts

	pRS->addContext( new rs_context(X_UNIMP | X_TAB_IS_WHITE,                        _T("#"),    _T(""),    0, true));

	// string literal contexts

	pRS->addContext( new rs_context(X_IMP | X_EOL | X_WHITE | X_CASE | X_TAB_IS_TAB, _T("\""),  _T("\""),   0, false));

	addRuleSet(pRS);
}

void rs_ruleset_table::_loadBuiltin_java(void)
{
	// builtin ruleset for Java
	//
	// for Java:
	//    normal code: important, respect-case, ignore-white, tab-is-white, ignore-eols
	//    comments:    unimportant, ignore-case, ignore-white, ignore-tabs, ignore-eols
	//    strings:     important, respect-case, respect-white, tab-is-tab, respect-eols
	//       (because strings end at eol unless escaped (and because they are usually
	//       stripped) the -eols setting doesn't usually matter)

	rs_ruleset * pRS = new rs_ruleset( _("Java Source"), _T("java jav"),
									   true,RS_ENCODING_STYLE_NAMED1,wxFONTENCODING_ISO8859_1,
									   MS_RECOMMENDED,
									   X_IMP | X_CASE | X_TAB_IS_WHITE,
									   FDEQ_RECOMMENDED_CASE);

	// double-backslashes -- one to get past the C compiler and one to get
	// past the RegEx compiler.

	// comment contexts

	pRS->addContext( new rs_context(X_UNIMP | X_TAB_IS_WHITE,                _T("/\\*"), _T("\\*/"),        0, false));
	pRS->addContext( new rs_context(X_UNIMP | X_TAB_IS_WHITE,                _T("//"),   _T(""),     _T('\\'),  true));

	// string literal contexts

	pRS->addContext( new rs_context(X_IMP | X_EOL | X_WHITE | X_CASE | X_TAB_IS_TAB, _T("\""),  _T("\""),    _T('\\'),  true));

	addRuleSet(pRS);
}

void rs_ruleset_table::_loadBuiltin_txt(void)
{
	// builtin ruleset for plain text files

	rs_ruleset * pRS = new rs_ruleset( _("Text Files"), _T("txt text"),
									   true,RS_ENCODING_STYLE_NAMED1,wxFONTENCODING_ISO8859_1,
									   MS_RECOMMENDED,
									   X_IMP | X_EOL | X_WHITE | X_CASE | X_TAB_IS_TAB,
									   FDEQ_RECOMMENDED_CASE);

	addRuleSet(pRS);
}

void rs_ruleset_table::_loadBuiltin_utf8(void)
{
	// builtin ruleset for UTF-8 files

	rs_ruleset * pRS = new rs_ruleset( _("UTF-8 Text Files"), _T("utf utf8"),
									   true,RS_ENCODING_STYLE_NAMED1,wxFONTENCODING_UTF8,
									   MS_RECOMMENDED,
									   X_IMP | X_EOL | X_WHITE | X_CASE | X_TAB_IS_TAB,
									   FDEQ_RECOMMENDED_CASE);

	addRuleSet(pRS);
}

void rs_ruleset_table::_loadBuiltin_xml(void)
{
	// builtin ruleset for XML files.  we assume utf8.

	rs_ruleset * pRS = new rs_ruleset( _("XML Files"), _T("xml"),
									   true,RS_ENCODING_STYLE_NAMED1,wxFONTENCODING_UTF8,
									   MS_RECOMMENDED,
									   X_IMP | X_CASE | X_TAB_IS_WHITE,
									   FDEQ_RECOMMENDED_CASE);

	pRS->addContext( new rs_context(X_UNIMP | X_TAB_IS_WHITE, _T("<!--"), _T("-->"), 0, false) );

	addRuleSet(pRS);
}

#if 0
#ifdef DEBUG
void rs_ruleset_table::_loadBuiltin_test(void)
{
	// builtin ruleset for test files

	rs_ruleset * pRS = new rs_ruleset( _("DEBUGGING Test Files"), _T("xxx"),
									   true,RS_ENCODING_STYLE_ASK,wxFONTENCODING_DEFAULT,
									   MS_RECOMMENDED,
									   X_IMP | X_EOL | X_TAB_IS_TAB,
									   FDEQ_RECOMMENDED_CASE);

	// for testing purposes, let's suppose that the page header on a report looks like
	//
	// 1: ^L
	// 2: ABC Company.... Page 1
	// 3: Report Name   Date....
	// 4: <blank line>
	pRS->addLOmit( _T("\\f"),4);

	// lines starting with "foo"

	pRS->addLOmit( _T("^foo"),2);
	
	// omit blank lines

	pRS->addLOmit( _T("^[[:blank:]]*$"),1);
	
	addRuleSet(pRS);
}
#endif
#endif	

//////////////////////////////////////////////////////////////////

int rs_ruleset_table::allocateArrayOfNames(wxString ** array) const
{
	int kLimit = getCountRuleSets();
	if (kLimit == 0)
	{
		*array = NULL;
		return 0;
	}

	*array = new wxString[kLimit];

	for (int k=0; k<kLimit; k++)
		(*array)[k] = getNthRuleSet(k)->getName();

	return kLimit;
}

void rs_ruleset_table::freeArrayOfNames(wxString * array) const
{
	delete [] array;
}

//////////////////////////////////////////////////////////////////

int rs_ruleset_table::getIndex(const rs_ruleset * pRS) const
{
	// lookup the given ruleset and return it's index in the vector.
	// return -1 when it is the default ruleset.
	// return -2 when not found.

	if (pRS == m_pRSDefault)
		return -1;

	int kLimit = getCountRuleSets();
	for (int k=0; k<kLimit; k++)
		if (pRS == getNthRuleSet(k))
			return k;

	return -2;
}

//////////////////////////////////////////////////////////////////
// Exporting -- Serializing Rulesets into a string so that it can
// be saved in the config file between sessions.
//////////////////////////////////////////////////////////////////
// Using a memory-output-stream and a data-output-stream, we create
// a binary BLOB from the ruleset table (details later in this file).
// For each variable that we wish to save in the blob, we create a
// RECORD.  Our overall BLOB will contain a variable number of these
// variable sized RECORDS.  Each RECORD has the following structure:
// [] key -- what the record is
// [] type -- what data type follows -- that is, the type of the value
// [] value -- a variant -- encoded by wxDataOutputStream

typedef enum K { TABLE_VERSION,			// do not reorder or insert in the middle of this enum
				 TABLE_EOF,
				 RULESET_BEGIN,
				 RULESET_NAME,
				 RULESET_SUFFIXES,
				 RULESET_ENCODING_STYLE,		// version 7 added values to _style enum
				 RULESET_ENCODING_SETTING1,		// pre-version 7 only setting, with 7 this is encoding1
				 RULESET_LOMIT_BEGIN,
				 RULESET_LOMIT_PATTERN,
				 RULESET_LOMIT_SKIP,
				 RULESET_LOMIT_END,
				 RULESET_CONTEXT_BEGIN,
				 RULESET_CONTEXT_ATTRS,
				 RULESET_CONTEXT_BEGPAT,
				 RULESET_CONTEXT_ENDPAT,
				 RULESET_CONTEXT_ESCAPE,
				 RULESET_CONTEXT_EOL,
				 RULESET_CONTEXT_END,
				 RULESET_DEFAULT_ATTRS,
				 RULESET_MATCH_STRIP_ATTRS,
				 RULESET_END,
				 RULESET_SNIFF_ENCODING_BOM,
				 RULESET_EQUIVALENCE_ATTRS,		// added in version 6
				 RULESET_ENCODING_SETTING2,		// added in version 7
				 RULESET_ENCODING_SETTING3,		// added in version 7
				 __LAST__FIELD__,
} T_RecordKey;

typedef enum T { T_BYTE		= 0x42,		// 'B'
				 T_LONG		= 0x4C,		// 'L'
				 T_STRING	= 0x53,		// 'S'
} T_RecordType;

struct _my_rs_record
{
	unsigned char		m_key;			// see T_RecordKey
	unsigned char		m_type;			// see T_RecordType

	unsigned char		m_u8Value;		// treat all values as a union
	wxUint32		m_ulValue;
	wxString			m_strValue;
};

static void _exportByte(  wxDataOutputStream & dos, unsigned char u8Key, unsigned char u8Value)		{dos.Write8(u8Key); dos.Write8(T_BYTE);   dos.Write8(u8Value);       }
static void _exportLong(  wxDataOutputStream & dos, unsigned char u8Key, wxUint32 ulValue)		{dos.Write8(u8Key); dos.Write8(T_LONG);   dos.Write32(ulValue);      }
static void _exportString(wxDataOutputStream & dos, unsigned char u8Key, const wxString & strValue)	{dos.Write8(u8Key); dos.Write8(T_STRING); dos.WriteString(strValue); }
static void _exportString(wxDataOutputStream & dos, unsigned char u8Key, const wxString * strValue)	{dos.Write8(u8Key); dos.Write8(T_STRING); dos.WriteString(*strValue);}

static bool _import_record(wxDataInputStream & dis, struct _my_rs_record * pRec)
{
	pRec->m_key = dis.Read8();
	pRec->m_type = dis.Read8();

	switch (pRec->m_type)
	{
	case T_BYTE:	pRec->m_u8Value  = dis.Read8();      return true;
	case T_LONG:	pRec->m_ulValue  = dis.Read32();     return true;
	case T_STRING:	pRec->m_strValue = dis.ReadString(); return true;
	default:		return false;
	}
}

//////////////////////////////////////////////////////////////////

// version 6 added folder-diff-equivalence-attrs
// version 7 added multiple named character encodings

#define STREAM_FORMAT_VERSION	7

//////////////////////////////////////////////////////////////////
// Encoding -- Converting data BLOB into a string so that we can
// write it to the config files and/or the registry.
//////////////////////////////////////////////////////////////////

static wxString _encodeBlob(wxMemoryOutputStream & mos)
{
	// copy the data-output-stream into a raw buffer -- a BLOB.

	off_t lenDos = mos.TellO();
	unsigned char * buf = (unsigned char *)calloc(lenDos,sizeof(char));
	mos.CopyTo((char *)buf,lenDos);

#if 0
#ifdef DEBUG
	wxString strDebug;
	for (int k=0; k<lenDos; k++)
		if ((buf[k] >= 0x20) && (buf[k] < 0x7f))
			strDebug += buf[k];
		else
			strDebug += wxString::Format(_T("\\x%02hhx"),buf[k]);
	wxLogTrace(TRACE_RS_DUMP, _T("EncodingBlob:\n%s\n"), strDebug.wc_str());
#endif
#endif

	// take the BLOB and encode it in BASE16 in a string variable.
	// [yes, i can hear you groaning now.]  this solves a couple
	// of potential problems:
	// 
	// [] global props (and the wxConfig stuff) only take strings
	//    and since we are in a unicode build, wxConfig won't be
	//    able to safely digest the blob (it'll try to convert it
	//    to UTF8 before writing) **AND** we'll have lots of
	//    zeroes within our buffer, so various string functions
	//    won't work.
	//    
	// [] base64 might be more kosher, but i have to dig up the
	//    routines somewhere **AND** base64 uses <>= (and maybe
	//    other chars) that XML finds useful, so we might have
	//    another quoting/escaping problem if the wxConfig layer
	//    on a platform uses XML under the hood.  [they might
	//    take care of it, but i don't want to rely on it.]
	//
	// so, we base16 it and be done with it.

	static const wxChar * szHex = _T("0123456789abcdef");

	wxString strEncoded;
	for (int kEnc=0; kEnc<lenDos; kEnc++)
	{
		wxChar ch = szHex[((buf[kEnc] >> 4) & 0x0f)];
		wxChar cl = szHex[((buf[kEnc]     ) & 0x0f)];

		strEncoded += ch;
		strEncoded += cl;
	}
	free(buf);

//	wxLogTrace(TRACE_RS_DUMP, _T("EncodedBlob: \n%s\n"), strEncoded.wc_str());

	return strEncoded;
}

static bool _decodeBlob(const wxString & strEncoded, unsigned char ** ppbuf, size_t * pLenBuf)
{
//	wxLogTrace(TRACE_RS_DUMP, _T("DecodeBlob: \n%s\n"), strEncoded.wc_str());

	// decode the base16 string back into a blob.

	size_t lenMis = strEncoded.Length() / 2;
	if (lenMis == 0)
		return false;
	
	const wxChar * szEncoded = strEncoded.wc_str();
	
	unsigned char * buf = (unsigned char *)calloc(lenMis,sizeof(char));

	for (size_t k=0; k<lenMis; k++)
	{
		wxChar ch = *szEncoded++;
		wxChar cl = *szEncoded++;

		if (      (ch >= _T('0')) && (ch <= _T('9')) )	buf[k] = (unsigned char)(( ch - _T('0')      ) << 4);
		else if ( (ch >= _T('a')) && (ch <= _T('f')) )	buf[k] = (unsigned char)(( ch - _T('a') + 10 ) << 4);
		else if ( (ch >= _T('A')) && (ch <= _T('F')) )	buf[k] = (unsigned char)(( ch - _T('A') + 10 ) << 4);
		else goto Failed;

		if (      (cl >= _T('0')) && (cl <= _T('9')) )	buf[k] |= (unsigned char)( cl - _T('0')      );
		else if ( (cl >= _T('a')) && (cl <= _T('f')) )	buf[k] |= (unsigned char)( cl - _T('a') + 10 );
		else if ( (cl >= _T('A')) && (cl <= _T('F')) )	buf[k] |= (unsigned char)( cl - _T('A') + 10 );
		else goto Failed;
	}
	
#if 0
#ifdef DEBUG
	{
		wxString strDebug;
		for (size_t k=0; k<lenMis; k++)
			if ((buf[k] >= 0x20) && (buf[k] < 0x7f))
				strDebug += buf[k];
			else
				strDebug += wxString::Format(_T("\\x%02hhx"),buf[k]);
		wxLogTrace(TRACE_RS_DUMP, _T("DecodedBlob:\n%s\n"), strDebug.wc_str());
	}
#endif
#endif

	*ppbuf = buf;
	*pLenBuf = lenMis;
	return true;

Failed:
	free(buf);
	return false;
}
	
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////

void rs_ruleset_table::doExport(void) const
{
	// stream all ruleset info into a buffer.  i hate using
	// streams, but XML seems a bit overkill (and there are
	// library issues and the one built-into wxWidgets is marked
	// as "don't use; api will change soon").
	//
	// besides we have to eventually get this data into a global
	// property so that it can be stored in the config file.
	// [and we don't want to pollute the global config file with
	// the value of each and every field in each and every ruleset.]
	//
	// as an alternative, we could build a big buffer/string with
	// a bunch of sprintf's/Format's, but have the problem that we
	// have UNICODE data (as is everything (and which sprintf and
	// %s barf on)), and we need to worry about quote, escape, and
	// \n, \r, and etc characters when we get ready to read it in.
	//
	// so, we dump it to a memory stream using the data-output
	// formatter -- which will take care of UNICODE->UTF8, and
	// it writes {length,array-of-converted-chars} for strings
	// (so we don't have to worry about quotes & escapes & etc)
	// and it takes care of byte order for free (which we really
	// don't care about, but is good to have).

	wxMemoryOutputStream mos;
	wxDataOutputStream dos(mos);

	_exportByte(dos,TABLE_VERSION, STREAM_FORMAT_VERSION);
	_exportRuleset(dos,m_pRSDefault,-1);

	int k=0;
	for (TVector_RuleSetsConstIterator it = m_vec.begin(); (it != m_vec.end()); it++)
	{
		const rs_ruleset * pRS = (*it);
		wxASSERT_MSG( (pRS), _T("Coding Error!") );

		_exportRuleset(dos,pRS,k++);
	}

	_exportByte(dos,TABLE_EOF, STREAM_FORMAT_VERSION);

	wxString strEncoded = _encodeBlob(mos);

	gpGlobalProps->setString(GlobalProps::GPS_FILE_RULESET_SERIALIZED,strEncoded); /*ok*/
}

void rs_ruleset_table::_exportRuleset(wxDataOutputStream & dos, const rs_ruleset * pRS, int index) const
{
	// emit a ruleset -- start with header record and then dump all fields within it.

	_exportLong(  dos, RULESET_BEGIN,				index);

	_exportString(dos, RULESET_NAME,				pRS->m_strName);
	_exportString(dos, RULESET_SUFFIXES,			pRS->m_strSuffixes);

	_exportByte(  dos, RULESET_ENCODING_STYLE,		pRS->m_encodingStyle);
	_exportLong(  dos, RULESET_ENCODING_SETTING1,	pRS->m_encodingSetting1);
	_exportLong(  dos, RULESET_ENCODING_SETTING2,	pRS->m_encodingSetting2);
	_exportLong(  dos, RULESET_ENCODING_SETTING3,	pRS->m_encodingSetting3);
	_exportByte(  dos, RULESET_SNIFF_ENCODING_BOM,	pRS->m_bSniffEncodingBOM);

	int kOmit = 0;
	for (rs_ruleset::TVector_LOmit_ConstIterator it = pRS->m_vecLOmit.begin(); (it != pRS->m_vecLOmit.end()); it++)
	{
		const rs_lomit * pLO = (*it);
		_exportLong(  dos, RULESET_LOMIT_BEGIN,		kOmit);
		_exportString(dos, RULESET_LOMIT_PATTERN,	pLO->getPattern());
		_exportLong(  dos, RULESET_LOMIT_SKIP,   	pLO->getNrLinesToSkip());
		_exportLong(  dos, RULESET_LOMIT_END,		kOmit++);
	}

	int kContext = 0;
	for (rs_ruleset::TVector_Context_ConstIterator it = pRS->m_vecContext.begin(); (it != pRS->m_vecContext.end()); it++)
	{
		const rs_context * pCXT = (*it);
		_exportLong(  dos, RULESET_CONTEXT_BEGIN,	kContext);
		_exportLong(  dos, RULESET_CONTEXT_ATTRS,	pCXT->getContextAttrs());
		_exportString(dos, RULESET_CONTEXT_BEGPAT,	pCXT->getStartPatternString());
		_exportString(dos, RULESET_CONTEXT_ENDPAT,	pCXT->getEndPatternString());
		_exportByte(  dos, RULESET_CONTEXT_ESCAPE,	pCXT->getEscapeChar());
		_exportByte(  dos, RULESET_CONTEXT_EOL,		pCXT->getEndsAtEOL());
		_exportLong(  dos, RULESET_CONTEXT_END,		kContext++);
	}

	_exportLong(  dos, RULESET_DEFAULT_ATTRS,		pRS->m_defaultContextAttrs);
	_exportLong(  dos, RULESET_MATCH_STRIP_ATTRS,	pRS->m_matchStripAttrs);
	_exportLong(  dos, RULESET_EQUIVALENCE_ATTRS,	pRS->m_equivalenceAttrs);
	_exportLong(  dos, RULESET_END,					index);
}

//////////////////////////////////////////////////////////////////

bool rs_ruleset_table::_doImport(const wxString & strEncoded)
{
	// convert the given string (containing a base16-encoded data stream)
	// into our rulesets.

	unsigned char * buf = NULL;
	size_t lenBuf = 0;

	if (!_decodeBlob(strEncoded,&buf,&lenBuf))
		return false;
	
	// put blob into memory-input-stream and let data-input-stream
	// start reading fields -- this takes care of UTF8->UNICODE and
	// byte-swap issues and etc.

	wxMemoryInputStream mis(buf,lenBuf);	// we must free buf
	wxDataInputStream dis(mis);

	// read the records and process -- we dispatch on key,type as
	// we try to adapt to what we find --- as opposed to assuming
	// that we find exactly what we wrote last time --- i'd hate
	// to go berzerk here because someone edited the value in registry...

	rs_ruleset * pRS = NULL;
	wxString strLOmit;
	int nrLOmit = 1;
	rs_context_attrs attrs = 0;
	wxString strStartPattern, strEndPattern;
	wxChar chEsc = 0;
	bool bEndsAtEOL = false;
	bool bSeenEquivalenceAttrs = false;			// equivalence attrs field not present in version 5.

	struct _my_rs_record rec;
	while (_import_record(dis,&rec))
	{
		switch (rec.m_key)
		{
		case TABLE_VERSION:
			if ((rec.m_type != T_BYTE) || (rec.m_u8Value > STREAM_FORMAT_VERSION))		goto Failed;	// written by a newer version, puke
			break;

		case TABLE_EOF:
			goto SawEOF;

		case RULESET_BEGIN:
			pRS = new rs_ruleset();
			bSeenEquivalenceAttrs = false;
			break;
			
		case RULESET_END:
			if (rec.m_type != T_LONG)	goto Failed;

			if (!bSeenEquivalenceAttrs)
			{
				// equivalence attrs was added in version 6.  it was not present
				// in older versions of the stream (version 5 and before).  if
				// we are importing an older version, silently supply the overall
				// line handling as a guess for the equivalence attrs.
				pRS->setEquivalenceAttrs( pRS->getMatchStripAttrs() );
			}

			if (rec.m_ulValue == (wxUint32)-1)
			{
				delete m_pRSDefault;
				m_pRSDefault = pRS;
			}
			else
				m_vec.push_back(pRS);
			pRS = NULL;
			break;

		case RULESET_NAME:
			if ((rec.m_type != T_STRING) || (!pRS))	goto Failed;
			pRS->setName(rec.m_strValue);
			break;
			
		case RULESET_SUFFIXES:
			if ((rec.m_type != T_STRING) || (!pRS))	goto Failed;
			pRS->setSuffixes(rec.m_strValue);
			break;

		case RULESET_ENCODING_STYLE:
			if ((rec.m_type != T_BYTE) || (!pRS))	goto Failed;
			pRS->setEncodingStyle((RS_ENCODING_STYLE)rec.m_u8Value);
			break;

		case RULESET_ENCODING_SETTING1:
			if ((rec.m_type != T_LONG) || (!pRS))	goto Failed;
			pRS->setEncoding1((util_encoding)rec.m_ulValue);
			break;

		case RULESET_ENCODING_SETTING2:
			if ((rec.m_type != T_LONG) || (!pRS))	goto Failed;
			pRS->setEncoding2((util_encoding)rec.m_ulValue);
			break;

		case RULESET_ENCODING_SETTING3:
			if ((rec.m_type != T_LONG) || (!pRS))	goto Failed;
			pRS->setEncoding3((util_encoding)rec.m_ulValue);
			break;

		case RULESET_SNIFF_ENCODING_BOM:
			if ((rec.m_type != T_BYTE) || (!pRS))	goto Failed;
			pRS->setSniffEncodingBOM( (rec.m_u8Value != 0) );
			break;

		case RULESET_LOMIT_BEGIN:
			strLOmit = _T("");
			nrLOmit = 1;
			break;

		case RULESET_LOMIT_PATTERN:
			if (rec.m_type != T_STRING)		goto Failed;
			strLOmit = rec.m_strValue;
			break;

		case RULESET_LOMIT_SKIP:
			if (rec.m_type != T_LONG)		goto Failed;
			nrLOmit = rec.m_ulValue;
			break;

		case RULESET_LOMIT_END:
			if (rec.m_type != T_LONG)		goto Failed;
			pRS->addLOmit(strLOmit,nrLOmit);
			break;

		case RULESET_CONTEXT_BEGIN:
			attrs = RS_ATTRS__DEFAULT;
			strStartPattern = _T("");
			strEndPattern = _T("");
			chEsc = 0;
			bEndsAtEOL = true;
			break;
			
		case RULESET_CONTEXT_ATTRS:
			if (rec.m_type != T_LONG)		goto Failed;
			attrs = rec.m_ulValue;
			break;
			
		case RULESET_CONTEXT_BEGPAT:
			if (rec.m_type != T_STRING)		goto Failed;
			strStartPattern = rec.m_strValue;
			break;

		case RULESET_CONTEXT_ENDPAT:
			if (rec.m_type != T_STRING)		goto Failed;
			strEndPattern = rec.m_strValue;
			break;

		case RULESET_CONTEXT_ESCAPE:
			if (rec.m_type != T_BYTE)		goto Failed;
			chEsc = rec.m_u8Value;
			break;

		case RULESET_CONTEXT_EOL:
			if (rec.m_type != T_BYTE)		goto Failed;
			bEndsAtEOL = (rec.m_u8Value != 0);
			break;
			
		case RULESET_CONTEXT_END:
			if (!pRS)						goto Failed;
			pRS->addContext( new rs_context(attrs,strStartPattern,strEndPattern,chEsc,bEndsAtEOL) );
			break;

		case RULESET_DEFAULT_ATTRS:
			if ((rec.m_type != T_LONG) || (!pRS))	goto Failed;
			pRS->setDefaultContextAttrs( rec.m_ulValue );
			break;

		case RULESET_MATCH_STRIP_ATTRS:
			if ((rec.m_type != T_LONG) || (!pRS))	goto Failed;
			pRS->setMatchStripAttrs( rec.m_ulValue );
			break;

		case RULESET_EQUIVALENCE_ATTRS:
			if ((rec.m_type != T_LONG) || (!pRS))	goto Failed;
			pRS->setEquivalenceAttrs( rec.m_ulValue );
			bSeenEquivalenceAttrs = true;
			break;

		default:
			goto Failed;
		}
	}
	// if _import_record() failed, we had bogus BLOB or we ran off
	// the end (and thus didn't the EOF marker), so let's puke.
	goto Failed;

SawEOF:
	delete pRS;
	free(buf);

	return true;

Failed:		
	delete pRS;
	free(buf);

	// we could not load the rulesets from the string given.
	// destroy any everything that we created.

	for (TVector_RuleSetsIterator it = m_vec.begin(); (it != m_vec.end()); it++)
	{
		rs_ruleset * pRSItem = (*it);
		delete pRSItem;
	}
	m_vec.clear();
	
	delete m_pRSDefault;
	m_pRSDefault = NULL;

	return false;
}

//////////////////////////////////////////////////////////////////

wxString rs_ruleset_table::dumpSupportInfoRST(void) const
{
	// build a string containing a human readable dump of the
	// complete ruleset table.  this is used to populate the
	// support dialog.

	wxString str;

	str += wxString::Format(_T("Ruleset Table: [Version %d]\n"), STREAM_FORMAT_VERSION);

	str += dumpSupportInfoRS(m_pRSDefault);

	for (TVector_RuleSetsConstIterator it = m_vec.begin(); (it != m_vec.end()); it++)
	{
		const rs_ruleset * pRS = (*it);
		str += dumpSupportInfoRS(pRS);
	}

	str += _T("\n");

	return str;
}

#define TorF(v)				((v) ? _T("true") : _T("false"))
#define IorTorF(v1,v2)		((v1) ? TorF(v2) : _T("N/A"))

wxString rs_ruleset_table::dumpSupportInfoRS(const rs_ruleset * pRS) const
{
	wxString str;

	//////////////////////////////////////////////////////////////////
	// name & suffix list
	//////////////////////////////////////////////////////////////////
	
	str += wxString::Format(_T("\tRuleset: %s\n"), pRS->m_strName.wc_str());
	str += wxString::Format(_T("\t\tSuffixes: %s\n"), pRS->m_strSuffixes.wc_str());

	//////////////////////////////////////////////////////////////////
	// overall line handling
	//////////////////////////////////////////////////////////////////

	str += wxString::Format(_T("\t\tLine Match Handling: [0x%08x]\n"),pRS->m_matchStripAttrs);
	str += wxString::Format(_T("\t\t\tIgnore/Strip EOLs: %s\n"),
							TorF(!RS_ATTRS_RespectEOL(pRS->m_matchStripAttrs)));
	str += wxString::Format(_T("\t\t\tIgnore/Fold Case: %s\n"),
							TorF(!RS_ATTRS_RespectCase(pRS->m_matchStripAttrs)));
	str += wxString::Format(_T("\t\t\tStrip Whitespace: %s\n"),
							TorF(!RS_ATTRS_RespectWhite(pRS->m_matchStripAttrs)));
	str += wxString::Format(_T("\t\t\t\tAlso Treat TABs as Whitespace: %s\n"),
							IorTorF(!RS_ATTRS_RespectWhite(pRS->m_matchStripAttrs),
									RS_ATTRS_TabIsWhite(pRS->m_matchStripAttrs)));

	//////////////////////////////////////////////////////////////////
	// folder-diff equivalence settings
	//////////////////////////////////////////////////////////////////

	str += wxString::Format(_T("\t\tFolder Diff Ruleset Equivalence: [0x%08x]\n"),pRS->m_equivalenceAttrs);
	str += wxString::Format(_T("\t\t\tIgnore/Strip EOLs: %s\n"),
							TorF(!RS_ATTRS_RespectEOL(pRS->m_equivalenceAttrs)));
	str += wxString::Format(_T("\t\t\tIgnore/Fold Case: %s\n"),
							TorF(!RS_ATTRS_RespectCase(pRS->m_equivalenceAttrs)));
	str += wxString::Format(_T("\t\t\tStrip Whitespace: %s\n"),
							TorF(!RS_ATTRS_RespectWhite(pRS->m_equivalenceAttrs)));
	str += wxString::Format(_T("\t\t\t\tAlso Treat TABs as Whitespace: %s\n"),
							IorTorF(!RS_ATTRS_RespectWhite(pRS->m_equivalenceAttrs),
									RS_ATTRS_TabIsWhite(pRS->m_equivalenceAttrs)));
	
	//////////////////////////////////////////////////////////////////
	// context settings
	//////////////////////////////////////////////////////////////////

	str += wxString::Format(_T("\t\tDefault Context Guidelines: [0x%08x]\n"),pRS->m_defaultContextAttrs);
	str += wxString::Format(_T("\t\t\tClassify Differences as Important: %s\n"),
							TorF(!RS_ATTRS_IsUnimportant(pRS->m_defaultContextAttrs)));
	str += wxString::Format(_T("\t\t\t\tEOL differences are important: %s\n"),
							IorTorF((   !RS_ATTRS_IsUnimportant(pRS->m_defaultContextAttrs)
									 && RS_ATTRS_RespectEOL(pRS->m_matchStripAttrs)),
									RS_ATTRS_RespectEOL(pRS->m_defaultContextAttrs)));
	str += wxString::Format(_T("\t\t\t\tCase differences are important: %s\n"),
							IorTorF(!RS_ATTRS_IsUnimportant(pRS->m_defaultContextAttrs),
									RS_ATTRS_RespectCase(pRS->m_defaultContextAttrs)));
	str += wxString::Format(_T("\t\t\t\tWhitespace differences are important: %s\n"),
							IorTorF(!RS_ATTRS_IsUnimportant(pRS->m_defaultContextAttrs),
									RS_ATTRS_RespectWhite(pRS->m_defaultContextAttrs)));
	str += wxString::Format(_T("\t\t\t\t\tTreat TABs as Whitespace: %s\n"),
							IorTorF((   !RS_ATTRS_IsUnimportant(pRS->m_defaultContextAttrs)
									 && !RS_ATTRS_RespectWhite(pRS->m_defaultContextAttrs)),
									RS_ATTRS_TabIsWhite(pRS->m_defaultContextAttrs)));

	int kContextLimit = pRS->getCountContexts();
	str += wxString::Format(_T("\t\tCustom Contexts: [%d contexts]\n"),kContextLimit);

	for (int k=0; k<kContextLimit; k++)
	{
		const rs_context * pCTX = pRS->getNthContext(k);
		rs_context_attrs attrs = pCTX->getContextAttrs();
		
		wxString strContextDetail = pCTX->getSummaryDescription();
		str += wxString::Format(_T("\t\t\tContext[%d]: %s\n"),k,strContextDetail.wc_str());
		str += wxString::Format(_T("\t\t\t\tGuidelines: [0x%08x]\n"),attrs);

		str += wxString::Format(_T("\t\t\t\t\tClassify Differences as Important: %s\n"),
								TorF(!RS_ATTRS_IsUnimportant(attrs)));
		str += wxString::Format(_T("\t\t\t\t\t\tEOL differences are important: %s\n"),
								IorTorF((   !RS_ATTRS_IsUnimportant(attrs)
										 && RS_ATTRS_RespectEOL(pRS->m_matchStripAttrs)),
										RS_ATTRS_RespectEOL(attrs)));
		str += wxString::Format(_T("\t\t\t\t\t\tCase differences are important: %s\n"),
								IorTorF(!RS_ATTRS_IsUnimportant(attrs),
										RS_ATTRS_RespectCase(attrs)));
		str += wxString::Format(_T("\t\t\t\t\t\tWhitespace differences are important: %s\n"),
								IorTorF(!RS_ATTRS_IsUnimportant(attrs),
										RS_ATTRS_RespectWhite(attrs)));
		str += wxString::Format(_T("\t\t\t\t\t\t\tTreat TABs as Whitespace: %s\n"),
								IorTorF((   !RS_ATTRS_IsUnimportant(attrs)
										 && !RS_ATTRS_RespectWhite(attrs)),
										RS_ATTRS_TabIsWhite(attrs)));
	}

	//////////////////////////////////////////////////////////////////
	// character encoding settings
	//////////////////////////////////////////////////////////////////

	wxString strEncodingStyle;
	switch (pRS->m_encodingStyle)
	{
	default:	// quiets compiler
	case RS_ENCODING_STYLE_LOCAL:
		strEncodingStyle = _T("Use System Local/Default");
		break;
	case RS_ENCODING_STYLE_ASK:
		strEncodingStyle = _T("Ask for Each Window");
		break;
	case RS_ENCODING_STYLE_ASK_EACH:
		strEncodingStyle = _T("Ask for Each File in Each Window");
		break;
	case RS_ENCODING_STYLE_NAMED1:
		strEncodingStyle
			= wxString::Format(_T("Assume [%s]"),
							   wxFontMapper::GetEncodingDescription((wxFontEncoding)pRS->m_encodingSetting1).wc_str());
		break;
	case RS_ENCODING_STYLE_NAMED2:
		strEncodingStyle
			= wxString::Format(_T("Assume [%s, %s]"),
							   wxFontMapper::GetEncodingDescription((wxFontEncoding)pRS->m_encodingSetting1).wc_str(),
							   wxFontMapper::GetEncodingDescription((wxFontEncoding)pRS->m_encodingSetting2).wc_str());
		break;
	case RS_ENCODING_STYLE_NAMED3:
		strEncodingStyle
			= wxString::Format(_T("Assume [%s, %s, %s]"),
							   wxFontMapper::GetEncodingDescription((wxFontEncoding)pRS->m_encodingSetting1).wc_str(),
							   wxFontMapper::GetEncodingDescription((wxFontEncoding)pRS->m_encodingSetting2).wc_str(),
							   wxFontMapper::GetEncodingDescription((wxFontEncoding)pRS->m_encodingSetting3).wc_str());
		break;
	}
	str += wxString::Format(_T("\t\tCharacter Encoding:\n"));
	str += wxString::Format(_T("\t\t\tAutomatically detect Unicode BOM: %s\n"), TorF(pRS->m_bSniffEncodingBOM));
	str += wxString::Format(_T("\t\t\tFallback Handling: %s\n"),strEncodingStyle.wc_str());

	//////////////////////////////////////////////////////////////////
	// lines to omit settings
	//////////////////////////////////////////////////////////////////

	int kLOmitLimit = pRS->getCountLOmit();
	str += wxString::Format(_T("\t\tLines To Omit: [%d patterns]\n"),kLOmitLimit);
	for (int k=0; k<kLOmitLimit; k++)
	{
		const rs_lomit * pLOmit = pRS->getNthLOmit(k);
		wxString strLOmitDetail = pLOmit->getSummaryDescription();
		str += wxString::Format(_T("\t\t\tLOmit[%d]: %s\n"),k,strLOmitDetail.wc_str());
	}

	return str;
}

//////////////////////////////////////////////////////////////////

#ifdef DEBUG
void rs_ruleset_table::dump(int indent) const
{
	wxLogTrace(TRACE_RS_DUMP, _T("%*cRS_RULESET_TABLE: [count %d]"), indent, ' ', (int)m_vec.size());
	for (TVector_RuleSetsConstIterator it = m_vec.begin(); (it != m_vec.end()); it++)
	{
		const rs_ruleset * pRS = (*it);
		if (!pRS) continue;

		pRS->dump(indent+5);
	}
	m_pRSDefault->dump(indent+5);
}
#endif
