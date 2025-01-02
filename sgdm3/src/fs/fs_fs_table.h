// fs_fs_table.h
// a container of fs_fs.
// there should only be one global instance of this table.
//////////////////////////////////////////////////////////////////

#ifndef H_FS_FS_TABLE_H
#define H_FS_FS_TABLE_H

//////////////////////////////////////////////////////////////////

class fs_fs_table
{
public:
	fs_fs_table(void);			// TODO make this private and have friend which creates all global classes.
	~fs_fs_table(void);			// TODO make this private and have friend which creates all global classes.

	// create new fileset on these 2 paths (implicit addRef())
	fs_fs *			create(const wxString & path0, const wxString & path1, const cl_args * pArgs);

	// find existing fileset on these paths (no addRef())
	fs_fs *			find(  const wxString & path0, const wxString & path1);
	fs_fs *			find(        poi_item * pPoi0,       poi_item * pPoi1);

	// create new fileset on these 3 paths (implicit addRef())
	fs_fs *			create(const wxString & path0, const wxString & path1, const wxString & path2, const cl_args * pArgs);

	// find existing fileset on these paths (no addRef())
	fs_fs *			find(  const wxString & path0, const wxString & path1, const wxString & path2);
	fs_fs *			find(        poi_item * pPoi0,       poi_item * pPoi1,       poi_item * pPoi2);

	inline void		addRef(fs_fs * pFsFs)	{ m_reftab.addRef(pFsFs); };						// add reference to existing fileset object
	inline void		unRef(fs_fs * pFsFs)	{ if (m_reftab.unRef(pFsFs)==0) delete pFsFs;  };	// release reference to fileset -- delete if zero

	void			applyNewRulesetTable(const rs_ruleset_table * pRST_New);

	void			rebindPoi(poi_item * pPoiOld, poi_item * pPoiNew);

private:
	util_reftab		m_reftab;

#ifdef DEBUG
public:
	void			dump(int indent) const	{ m_reftab.dump(indent); };
#endif
};

//////////////////////////////////////////////////////////////////

#endif//H_FS_FS_TABLE_H
