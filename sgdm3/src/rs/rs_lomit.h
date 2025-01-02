// rs_lomit.h -- part of RuleSet -- describes patterns for lines to
// be omitted from diff computation.
//
// you might use this for a report header (maybe a '^L' and several
// lines of page header), for example.  or you might use it to get
// blank lines to completely disappear.
//////////////////////////////////////////////////////////////////

#ifndef H_RS_LOMIT_H
#define H_RS_LOMIT_H

//////////////////////////////////////////////////////////////////

class rs_lomit
{
public:
	rs_lomit(const wxString & str, int nr);
	rs_lomit(const rs_lomit & lo);			// copy constructor

	bool						isEqual(const rs_lomit * pLO) const;

	inline bool					isValid(void)				const { return (m_nrLinesToSkip > 0) && (m_rePattern.IsValid()); };
	inline int					getNrLinesToSkip(void)		const { return m_nrLinesToSkip; };
	inline const wxString *		getPattern(void)			const { return &m_strPattern; };

	bool						isMatch(const wxChar * szTest)	const;

	wxString					getSummaryDescription(void) const;

protected:
	wxString					m_strPattern;			// the RegEx Source string
	wxRegEx						m_rePattern;			// the compiled RegEx
	int							m_nrLinesToSkip;		// number of lines (including match)

	//////////////////////////////////////////////////////////////////

#ifdef DEBUG
public:
	void							dump(int indent) const;
#endif
};

//////////////////////////////////////////////////////////////////

#endif//H_RS_LOMIT_H
