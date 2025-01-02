// poi_item_table.h
// "pathname of interest" item table -- a container of poi_item.
// there should only be one global instance of this table.
//////////////////////////////////////////////////////////////////

#ifndef H_POI_ITEM_TABLE_H
#define H_POI_ITEM_TABLE_H

//////////////////////////////////////////////////////////////////

class poi_item_table
{
public:
	poi_item_table(void);	// TODO make this private and have friend which creates all global classes.
	~poi_item_table(void);	// TODO make this private and have friend which deletes all global classes.

	void			OnInit(void);

	// pathname can be relative or absolute, we will normalize it.

	poi_item *		addItem(const wxString & pathname);
	poi_item *		addItem(const wxString & dirpathname, const wxString & filename);
	poi_item *		findItem(const wxString & pathname);

private:
	poi_item *		_addItem(const wxFileName & normalizedFilename);
	void			_free_items_on_list(void);

private:
	typedef std::map<wxString, poi_item *>		TMap;
	typedef TMap::const_iterator				TMapConstIterator;
	typedef TMap::iterator						TMapIterator;
	typedef TMap::value_type					TMapValue;

	TMap			m_map;

	wxCriticalSection m_lock;

#ifdef DEBUG
public:
	void			dump(int indent);
#endif
};

//////////////////////////////////////////////////////////////////

#endif//H_POI_ITEM_TABLE_H
