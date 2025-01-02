// rs_ruleset.h -- declaration for a single RuleSet -- a set of
// parameters for determining how we load (charset/character encoding)
// and compare (diff-engine rules) the files in a 2- or 3-way window.
//
// There will be many rulesets -- one each for the various types of
// source files.  This lets us understand the comment-syntax of C
// source vs. Makefiles, for example -- so that we can say that changes
// within a comment are unimportant (still there, but unimportant).
//////////////////////////////////////////////////////////////////

#ifndef H_RS_RULESET_H
#define H_RS_RULESET_H

//////////////////////////////////////////////////////////////////

typedef unsigned long RS_ID;

//////////////////////////////////////////////////////////////////

class rs_ruleset
{
public:
	friend class rs_ruleset_table;

	rs_ruleset(void);
	rs_ruleset(const rs_ruleset & rs);			// copy constructor
	rs_ruleset(const wxString & strName, const wxString & strSuffixes,
			   bool bSniff, RS_ENCODING_STYLE style, util_encoding enc,
			   rs_context_attrs attrsMatchStrip, rs_context_attrs attrsDefaultContext,
			   rs_context_attrs attrsEquivalence);

	~rs_ruleset(void);

	rs_ruleset *					clone(void) const;

	bool							isEqual(const rs_ruleset * pRS) const;

//////////////////////////////////////////////////////////////////
// put a hidden ID in each ruleset so that we can do a better
// job of matching the corresponding ruleset in another rule-set-table
// when fs_fs::_apply_new_ruleset_cb_lookup() runs.
//////////////////////////////////////////////////////////////////

public:
	inline bool						doIDsMatch(const rs_ruleset * pRS)	const { return (m_ID == pRS->m_ID); };
	inline RS_ID					getID(void)							const { return m_ID; };

private:
	RS_ID							m_ID;	// this does not get serialized; it is for in-memory use only.
	
//////////////////////////////////////////////////////////////////
// RuleSets have a unique name (such as "C/C++/C# Source Code")
// and a list of suffixes (such as "c cpp cs h") for when it
// should be used (instead of another rule or the default rule).
//////////////////////////////////////////////////////////////////

public:
	void							setName(const wxString & str);
	void							setSuffixes(const wxString & str);

//	inline wxString					getName(void)					const { return m_strName; };
	inline const wxString &			getName(void)					const { return m_strName; };
//	inline wxString					getSuffixes(void)				const { return m_strSuffixes; };
	inline const wxString &			getSuffixes(void)				const { return m_strSuffixes; };
	bool							testPathnameSuffix(bool bRulesetIgnoreSuffixCase,
													   const poi_item * pPoi) const;

private:
	wxString						m_strName;						// user-visible name for this ruleset
	wxString						m_strSuffixes;					// space delimited list of suffixes (without '.')

//////////////////////////////////////////////////////////////////
// RuleSets define a Character Encoding (C source might be US-ASCII
// or Latin-1, whereas a report from SQLserver might be Unicode).
// We are limited to the sets that wxWindows understands.  And we
// DO NOT do any of the fall-back like we did in the native versions
// of SGDM.
//////////////////////////////////////////////////////////////////

public:
	void							setSniffEncodingBOM(bool bSniff);
	void							setEncodingStyle(RS_ENCODING_STYLE style);
	void							setEncoding1(util_encoding enc);
	void							setEncoding2(util_encoding enc);
	void							setEncoding3(util_encoding enc);
	inline bool						getSniffEncodingBOM(void)		const { return m_bSniffEncodingBOM; };
	inline RS_ENCODING_STYLE		getEncodingStyle(void)			const { return m_encodingStyle; };
	inline util_encoding			getEncoding1(void)				const { return m_encodingSetting1; };
	inline util_encoding			getEncoding2(void)				const { return m_encodingSetting2; };
	inline util_encoding			getEncoding3(void)				const { return m_encodingSetting3; };

	int								getNamedEncodingArray(int lenEnc, util_encoding aEnc[]) const;

private:
	bool							m_bSniffEncodingBOM;			// sniff for Unicode BOM before using encoding-style/-settings
	RS_ENCODING_STYLE				m_encodingStyle;
	util_encoding					m_encodingSetting1;				// only used if style is _NAMED
	util_encoding					m_encodingSetting2;				// only used if style is _NAMED
	util_encoding					m_encodingSetting3;				// only used if style is _NAMED

//////////////////////////////////////////////////////////////////
// RuleSets allow lines to be omitted/excluded from the diff
// computation.  This can be used to "hide" page-headers/-footers
// and etc. from the diff-engine.
//////////////////////////////////////////////////////////////////

public:
	bool							addLOmit(const wxString & strPattern, int nrLines);
	bool							addLOmit(rs_lomit * pLOmit);
	void							replaceLOmit(int index, rs_lomit * pLOmit_New, bool bDelete=true);
	void							deleteLOmit(int index);
	int								testLOmit(const wxChar * szTest) const;

	inline int						getCountLOmit(void)		const	{ return (int)m_vecLOmit.size(); };
	inline const rs_lomit *			getNthLOmit(int n)		const	{ return ((n < getCountLOmit()) ? m_vecLOmit[n] : NULL); };

private:
	typedef std::vector<rs_lomit *>				TVector_LOmit;
	typedef TVector_LOmit::value_type			TVector_LOmit_Value;
	typedef TVector_LOmit::iterator				TVector_LOmit_Iterator;
	typedef TVector_LOmit::const_iterator		TVector_LOmit_ConstIterator;

	TVector_LOmit							m_vecLOmit;

//////////////////////////////////////////////////////////////////
// RuleSets allow document content to be grouped into various
// "Contexts" -- such as a string literal or a comment.  These
// can then be given special attributes, such as whether whitespace
// is significant.
//
// Each "Context" defines a starting delimiter (RegEx) and an
// optional ending delimiter.  Each Context is given a set of
// attributes.  There is a default set of attributes for all
// document content not matching a context.
//////////////////////////////////////////////////////////////////

public:
	void							addContext(rs_context * pCXT);
	void							replaceContext(int index, rs_context * pCXT_New, bool bDelete=true);
	void							deleteContext(int index);
	const rs_context *				findStartContext(const wxChar * szLine, size_t * pOffset, size_t * pLen) const;

	inline int						getCountContexts(void)	const	{ return (int)m_vecContext.size(); };
	inline const rs_context *		getNthContext(int n)	const	{ return ((n < getCountContexts()) ? m_vecContext[n] : NULL); };

private:
	typedef std::vector<rs_context *>			TVector_Context;
	typedef TVector_Context::value_type			TVector_Context_Value;
	typedef TVector_Context::iterator			TVector_Context_Iterator;
	typedef TVector_Context::const_iterator		TVector_Context_ConstIterator;

	TVector_Context					m_vecContext;

	//////////////////////////////////////////////////////////////////

public:
	inline void						setMatchStripAttrs(rs_context_attrs attrs)				{ m_matchStripAttrs = attrs; };
	inline rs_context_attrs			getMatchStripAttrs(void)						const	{ return m_matchStripAttrs; };

	inline void						setDefaultContextAttrs(rs_context_attrs attrs)			{ m_defaultContextAttrs = attrs; };
	inline rs_context_attrs			getDefaultContextAttrs(void)					const	{ return m_defaultContextAttrs; };

	inline void						setEquivalenceAttrs(rs_context_attrs attrs)				{ m_equivalenceAttrs = attrs; };
	inline rs_context_attrs			getEquivalenceAttrs(void)						const	{ return m_equivalenceAttrs; };

private:
	rs_context_attrs				m_matchStripAttrs;
	rs_context_attrs				m_defaultContextAttrs;
	rs_context_attrs				m_equivalenceAttrs;

	//////////////////////////////////////////////////////////////////

#ifdef DEBUG
public:
	void							dump(int indent) const;
#endif
};

//////////////////////////////////////////////////////////////////

#endif//H_RS_RULESET_H
