// fim_ptable_table.h
// a container of fim_ptable.
// there should only be one global instance of this table.
//////////////////////////////////////////////////////////////////

#ifndef H_FIM_PTABLE_TABLE_H
#define H_FIM_PTABLE_TABLE_H

//////////////////////////////////////////////////////////////////

class fim_ptable_table
{
public:
	fim_ptable_table(void);				// TODO make this private and have friend which creates all global classes.
	~fim_ptable_table(void);			// TODO make this private and have friend which creates all global classes.

	util_error				create(const wxString & path,	// create new ptable and load file (implicit addRef())
								   bool bSniffEncodingBOM,
								   util_encoding enc,
								   fim_ptable ** ppPTable);

	util_error				create(poi_item * pPoiItem,		// create new ptable and load file (implicit addRef())
								   bool bSniffEncodingBOM,
								   util_encoding enc,
								   fim_ptable ** ppPTable);

	util_error				create(poi_item * pPoiItem, bool bSniffEncodingBOM,
								   int nrEnc, util_encoding aEnc[],
								   fim_ptable ** ppPTable);

	fim_ptable *			createClone(const fim_ptable * pPTableSrc, fr_prop propMask, poi_item * pPoiClone);	// create new ptable by cloning one in memory.

	void					addRef(fim_ptable * pPTable);	// add reference to existing ptable
	bool					unRef(fim_ptable * pPTable);	// release reference to ptable and delete if zero; return true if deleted.

private:
	fim_ptable *			_create(void);					// create new ptable for a new,blank document (implicit addRef())

	util_reftab				m_reftab;

#ifdef DEBUG
public:
	void					dump(int indent) const			{ m_reftab.dump(indent); };
#endif
};

//////////////////////////////////////////////////////////////////

#endif//H_FIM_PTABLE_TABLE_H
