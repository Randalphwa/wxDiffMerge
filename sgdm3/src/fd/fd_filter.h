// fd_filter.h
// a wrapper for folderdiff file suffix and subdirectory filtering.
//////////////////////////////////////////////////////////////////

#ifndef H_FD_FILTER_H
#define H_FD_FILTER_H

//////////////////////////////////////////////////////////////////

class fd_filter
{
public:
	fd_filter(void);
	~fd_filter(void);

	bool			isFilteredSubdir(const wxString & relativePathname) const;
	bool			isFilteredSuffix(const wxString & pathname) const;
	bool			isFilteredFullFilename(const wxString & pathname) const;
	bool			isIgnorePatternCase(void) const { return m_bIgnorePatternCase; };
	bool			isIgnoreMatchupCase(void) const { return m_bIgnoreMatchupCase; };

private:
	bool			_setSubdirs(void);
	bool			_setSuffixes(void);
	bool			_setFullFilenames(void);
	bool			_setIgnoreSubdirs(void);
	bool			_setIgnoreSuffixes(void);
	bool			_setIgnoreFullFilenames(void);
	bool			_setIgnorePatternCase(void);
	bool			_setIgnoreMatchupCase(void);

	typedef std::set<wxString>			TSet;
	typedef TSet::const_iterator		TSetConstIterator;
	typedef TSet::iterator				TSetIterator;
	typedef TSet::value_type			TSetValue;

	TSet			m_setSubdirs;
	TSet			m_setSuffixes;
	TSet			m_setFullFilenames;
	bool			m_bIgnoreSubdirFilter;
	bool			m_bIgnoreSuffixFilter;
	bool			m_bIgnoreFullFilenameFilter;
	bool			m_bIgnorePatternCase;
	bool			m_bIgnoreMatchupCase;
	wxString		m_strSubdirs;
	wxString		m_strSuffixes;
	wxString		m_strFullFilenames;

public:
	static void parse_string_into_set(TSet & S,
									  const wxString & strInput,
									  bool bIgnorePatternCase);
	static bool is_filtered(const TSet & S, const wxString strInput, bool bAlsoDoWildcards);

};

//////////////////////////////////////////////////////////////////

#endif//H_FD_FILTER_H
