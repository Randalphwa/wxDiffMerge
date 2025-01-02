// rs_ruleset_table.h -- a container to hold all of the defined RuleSets.
// there should be:
// [] one global instance of this class (in gpRsRuleSetTable)
// [] a temporary copy while the OptionsDlg is up
// [] a snap-shot/temporary copy for use by Folder Window
//    background thread during scan.
// 
// Individual RuleSets will include builtin (pre-defined) sets and
// any user-defined sets.  They are stored in a per-user profile.
//////////////////////////////////////////////////////////////////

#ifndef H_RS_RULESET_TABLE_H
#define H_RS_RULESET_TABLE_H

//////////////////////////////////////////////////////////////////

class rs_ruleset_table
{
public:
	rs_ruleset_table(void);
	rs_ruleset_table(const rs_ruleset_table & rst);		// copy constructor
	~rs_ruleset_table(void);

	bool						isEqual(const rs_ruleset_table * pRST) const;

	void						OnInit(bool bBuiltinOnly=false);
	void						addRuleSet(rs_ruleset * pRS);
	void						replaceRuleSet(int index, rs_ruleset * pNew, bool bDelete=true);
	void						replaceDefaultRuleSet(rs_ruleset * pNewDefault, bool bDelete=true);
	void						deleteRuleSet(int index);
	void						moveRuleSetUpOne(int index);
	void						moveRuleSetDownOne(int index);

	inline int					getCountRuleSets(void)	const { return (int)m_vec.size(); };
	inline const rs_ruleset *	getNthRuleSet(int n)	const { return ((n >= getCountRuleSets()) ? NULL : m_vec[n]); };
	inline const rs_ruleset *	getDefaultRuleSet(void)	const { return m_pRSDefault; };

	const rs_ruleset *			do_choose_ruleset_dialog(wxWindow * pParent, int nrPoi, poi_item * pPoiTable[]) const;
	const rs_ruleset * 			do_automatic_best_guess(bool bRulesetRequireCompleteMatch,
														bool bRulesetIgnoreSuffixCase,
														int nrPoi, poi_item * pPoiTable[],
														poi_item * pPoiResult=NULL) const;
	const rs_ruleset *			findBestRuleSet(wxWindow * pParent, int nrPoi, poi_item * pPoiTable[], poi_item * pPoiResult=NULL) const;

	const rs_ruleset *			findBestRuleSet_without_asking(int nrPoi, poi_item * pPoiTable[], poi_item * pPoiResult=NULL) const;

	const rs_ruleset *			findByID(RS_ID id) const;
	const rs_ruleset *			findByName(const wxString & strName) const;
	const rs_ruleset *			findBySuffixList(const wxString & strSuffixList) const;

	int							allocateArrayOfNames(wxString ** array) const;
	void						freeArrayOfNames(wxString * array) const;

	void						doExport(void) const;

	int							getIndex(const rs_ruleset * pRS) const;

	wxString					dumpSupportInfoRST(void) const;
	wxString					dumpSupportInfoRS(const rs_ruleset * pRS) const;

private:
	void						_exportRuleset(wxDataOutputStream & dos, const rs_ruleset * pRS, int index) const;
	bool						_doImport(const wxString & strEncoded);

private:
	void						_loadBuiltinRuleSets(void);
	void						_loadBuiltin_c(void);
	void						_loadBuiltin_vb(void);
	void						_loadBuiltin_python(void);
	void						_loadBuiltin_java(void);
	void						_loadBuiltin_txt(void);
	void						_loadBuiltin_utf8(void);
	void						_loadBuiltin_default(void);
	void						_loadBuiltin_xml(void);

private:
	typedef std::vector<rs_ruleset *>			TVector_RuleSets;
	typedef TVector_RuleSets::value_type		TVector_RuleSetsValue;
	typedef TVector_RuleSets::iterator			TVector_RuleSetsIterator;
	typedef TVector_RuleSets::const_iterator	TVector_RuleSetsConstIterator;

	TVector_RuleSets			m_vec;
	rs_ruleset *				m_pRSDefault;

#ifdef DEBUG
private:
//	void						_loadBuiltin_test(void);

public:
	void						dump(int indent) const;
#endif
};

//////////////////////////////////////////////////////////////////

#endif//H_RS_RULESET_TABLE_H
