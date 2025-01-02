// fd_item_table.h
// a table of fd_items -- directory tree in a folder diff window.
//////////////////////////////////////////////////////////////////

#ifndef H_FD_ITEM_TABLE_H
#define H_FD_ITEM_TABLE_H

//////////////////////////////////////////////////////////////////

class fd_item_table
{
public:
	fd_item_table(void);
	~fd_item_table(void);

	void			setFdFd(fd_fd * pFdFd)		{ m_pFdFd = pFdFd; };
	fd_fd *			getFdFd(void)				const { return m_pFdFd; };

	fd_item *		addItem(int kItem, const wxString * pRelativePathname);
	fd_item *			findItem(const wxString & rRelativePathname) const;

	void			markAllStale(void);
	void			deleteAllStale(void);

	void			computeAllStatus(const fd_filter * pFdFilter,
									 const fd_quickmatch * pFdQuickmatch,
									 const fd_softmatch * pFdSoftmatch,
									 const rs_ruleset_table * pRsRuleSetTable,
									 bool bUsePreviousSoftmatchResult) const;

	inline long		getItemCount(void)			const { return (long)m_map.size(); };
	inline long		getStats(fd_item::Status s)	const { return m_stats[s]; };

	fd_item * 		beginIter(void ** ppvoid)	const;
	fd_item *		nextIter(void * pvoid)		const;
	void			endIter(void * pvoid)		const;

	void			rememberIgnoreMatchupCase(bool b) { m_bIgnoreMatchupCase = b; };

private:
	typedef std::map<wxString, fd_item *>		TMap;
	typedef TMap::const_iterator				TMapConstIterator;
	typedef TMap::iterator						TMapIterator;
	typedef TMap::value_type					TMapValue;

	TMap			m_map;
	fd_fd *			m_pFdFd;
	bool			m_bIgnoreMatchupCase;	// value set during last loadFolders()

private:
	friend class fd_item;

	TMapConstIterator	findIter(const wxString & rRelativePathname) const;

	void			_updateStats(fd_item::Status oldValue, fd_item::Status newValue);
	long			m_stats[fd_item::__FD_ITEM_STATUS__COUNT__];

#ifdef DEBUG
public:
	void			dump(int indent) const;
	void			dump_stats(int indent) const;
	void			print_stats(const wxString & strMsg) const;
#endif
};

//////////////////////////////////////////////////////////////////

#endif//H_FD_ITEM_TABLE_H
