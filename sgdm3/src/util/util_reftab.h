// util_reftab.h
// a generic container to reference-count objects
//
// we make this a generic map of void * for simplicity -- as
// opposed to making it a template.
//////////////////////////////////////////////////////////////////

#ifndef H_UTIL_REFTAB_H
#define H_UTIL_REFTAB_H

//////////////////////////////////////////////////////////////////
// a callback onto the owner of the reftab to let it operation on
// an object in our table.  returns true if apply() should stop
// and return this object.
//
// useful for "find" for example.

typedef bool (*util_reftab_apply_cb)(void * pParam1, void * pParam2, void * pObject);

//////////////////////////////////////////////////////////////////

class util_reftab
{
public:
	util_reftab(const wxChar * szDebugName, bool bRequireEmptyOnDestroy=true);
	~util_reftab(void);

	int					addRef(void * pObject);
	int					unRef(void * pObject); // caller must delete object if this returns zero.

	void *				apply(util_reftab_apply_cb pfn, void * pParam1=NULL, void * pParam2=NULL, void * pObjectFirst=NULL);
	
private:
	typedef std::map<void *, int>				TMap;
	typedef TMap::const_iterator				TMapConstIterator;
	typedef TMap::iterator						TMapIterator;
	typedef TMap::value_type					TMapValue;

	wxString			m_DebugName;
	bool				m_bRequireEmptyOnDestroy;

	TMap				m_map;

#ifdef DEBUG
public:
	void				dump(int indent) const;
#endif
	
};

//////////////////////////////////////////////////////////////////

#endif//H_UTIL_REFTAB_H
