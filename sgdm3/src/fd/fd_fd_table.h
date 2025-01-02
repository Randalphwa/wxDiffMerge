// fd_fd_table.h
// a container of fd_fd.
// there should only be one global instance of this table.
//////////////////////////////////////////////////////////////////

#ifndef H_FD_FD_TABLE_H
#define H_FD_FD_TABLE_H

//////////////////////////////////////////////////////////////////

class fd_fd_table
{
public:
	fd_fd_table(void);			// TODO make this private and have friend which creates all global classes.
	~fd_fd_table(void);			// TODO make this private and have friend which creates all global classes.

	fd_fd *			create(const wxString & path0, const wxString & path1);		// find/create new folderdiff on these 2 paths (implicit addRef())
	fd_fd *			find(  const wxString & path0, const wxString & path1);		// find existing folderdiff on these paths (no addRef())
	fd_fd *			find(        poi_item * pPoi0,       poi_item * pPoi1);		// find existing folderdiff on these paths (no addRef())

	inline void		addRef(fd_fd * pFdFd)	{ m_reftab.addRef(pFdFd); };						// add reference to existing folderdiff object
	inline void		unRef(fd_fd * pFdFd)	{ if (m_reftab.unRef(pFdFd)==0) delete pFdFd;  };	// release reference to folderdiff -- delete if zero
	
private:
	util_reftab		m_reftab;

#ifdef DEBUG
public:
	void			dump(int indent) const	{ m_reftab.dump(indent); };
#endif
};

//////////////////////////////////////////////////////////////////

#endif//H_FD_FD_TABLE_H
