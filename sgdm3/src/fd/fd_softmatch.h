// fd_softmatch.h
// a wrapper for holding folder window soft-match parameters.
//////////////////////////////////////////////////////////////////

#ifndef H_FD_SOFTMATCH_H
#define H_FD_SOFTMATCH_H

//////////////////////////////////////////////////////////////////

class fd_softmatch
{
public:
	fd_softmatch(void);
	~fd_softmatch(void);

	bool				isSimpleMode(void) const;
	bool				isRulesetMode(void) const;
	
	bool				isSimpleModeSuffix(const wxString & pathname) const;
	const rs_ruleset *	findRuleset(const rs_ruleset_table * pRsRuleSetTable,
									poi_item * pPoiItem) const;

	bool				allowDefaultRuleset(void) const;

	// result is {-1 error, 0 diff, 2 equivalent}

	util_error			compareFileSimpleMode(const poi_item * pPoiItem1,
											  const poi_item * pPoiItem2,
											  int * pResult,
											  wxString & strSoftMatchInfo) const;
	util_error			compareFileRulesetMode(const poi_item * pPoiItem1,
											   const poi_item * pPoiItem2,
											   const rs_ruleset * pRS,
											   int * pResult,
											   wxString & strSoftMatchInfo) const;

private:
	void			_setSimpleModeSuffix(void);
	
	typedef std::set<wxString>			TSet;
	typedef TSet::const_iterator		TSetConstIterator;
	typedef TSet::iterator				TSetIterator;
	typedef TSet::value_type			TSetValue;

	TSet			m_setSimpleModeSuffixes;

	fd_SoftMatchMode	m_SoftMatchMode;

	wxString		m_strSimpleModeSuffixes;
	bool			m_bSimpleModeIgnoreEOL;
	bool			m_bSimpleModeIgnoreWhitespace;
	bool			m_bSimpleModeTABisWhitespace;

	bool			m_bRulesetEnableCustom;
	bool			m_bRulesetAutomaticMatch;
	bool			m_bRulesetIgnoreSuffixCase;
	bool			m_bRulesetRequireCompleteMatch;
	bool			m_bRulesetAllowDefault;
	int				m_nRulesetFileLimitMb;

};

//////////////////////////////////////////////////////////////////

#endif//H_FD_SOFTMATCH_H
