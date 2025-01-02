// fd_quickmatch.h
// a wrapper for quick match details
//////////////////////////////////////////////////////////////////

#ifndef H_FD_QUICKMATCH_H
#define H_FD_QUICKMATCH_H

//////////////////////////////////////////////////////////////////

class fd_quickmatch
{
public:
	fd_quickmatch(void);
	~fd_quickmatch(void);

	bool			isQuickSuffix(const wxString & pathname) const;
	bool			isQuickMatchEnabled(void) const;
	util_error		compareQuick(const poi_item * pPoiItem1,
								 const poi_item * pPoiItem2,
								 int * pResult,
								 wxString & strQuickMatchInfo) const;

private:
	void			_setSuffixes(void);

private:
	bool			m_bEnable;
	wxString		m_strSuffixes;

	typedef std::set<wxString>			TSet;
	typedef TSet::const_iterator		TSetConstIterator;
	typedef TSet::iterator				TSetIterator;
	typedef TSet::value_type			TSetValue;

	TSet			m_setSuffixes;
};

//////////////////////////////////////////////////////////////////

#endif//H_FD_QUICKMATCH_H
