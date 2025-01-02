// util_reftab.cpp
// a generic container to reference-count objects
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>

//////////////////////////////////////////////////////////////////

util_reftab::util_reftab(const wxChar * szDebugName, bool bRequireEmptyOnDestroy)
	: m_DebugName(szDebugName),
	  m_bRequireEmptyOnDestroy(bRequireEmptyOnDestroy)
{
}

util_reftab::~util_reftab(void)
{
	// require table to be empty

#ifdef DEBUG
	size_t size = m_map.size();
	if (size > 0)
	{
		wxLogTrace(TRACE_UTIL_DUMP,
				   _T("%s[%p]::~%s: [size %d]"),
				   m_DebugName.wc_str(),
				   this,
				   m_DebugName.wc_str(),
				   (int)size);
		dump(0);
		wxASSERT_MSG( (0), _T("util_reftab::~util_reftab: table not empty.") );
	}
#endif
}

//////////////////////////////////////////////////////////////////

int util_reftab::addRef(void * pObject)
{
	int refCnt;

	TMapIterator it = m_map.find(pObject);
	if (it == m_map.end())
	{
		refCnt = 1;
		m_map.insert( TMapValue(pObject,refCnt) );
	}
	else
	{
		it->second++;
		refCnt = it->second;
	}
	
	//wxLogTrace(TRACE_UTIL_DUMP, _T("%s[%p]::addRef: [%p,%d]"), m_DebugName.wc_str(),this,pObject,refCnt);

	return refCnt;
}

int util_reftab::unRef(void * pObject)
{
	// release a reference to the object.  if refCnt goes to zero, **caller** must delete it.
	// we didn't allocate it, so it's not right for us to delete it.  (besides we only have
	// a "void *", so we don't know how to delete it.)

	TMapIterator it = m_map.find(pObject);
	if (it == m_map.end())
	{
		wxASSERT_MSG( (0), wxString::Format(_T("%s[%p]::unRef: not found [%p]"), m_DebugName.wc_str(),this,pObject));
		return -1;
	}

	it->second--;
	int refCnt = it->second;
	
	//wxLogTrace(TRACE_UTIL_DUMP, _T("%s[%p]::unRef: [%p,%d]"), m_DebugName.wc_str(),this,pObject,refCnt);

	if (refCnt == 0)
	{
		m_map.erase(it);
	}

	return refCnt;
}

//////////////////////////////////////////////////////////////////

void * util_reftab::apply(util_reftab_apply_cb pfn, void * pParam1, void * pParam2, void * pObjectFirst)
{
	// apply the given function to each object in the map (in order).
	// if the function returns true, we stop looping and return the
	// object.  if the function returns false, we keep looping.
	//
	// if pObjectFirst is given, we start there rather than at begin().
	// if pObjectFirst is given and not in the map, we return null.
	
	TMapIterator it;

	if (pObjectFirst)
		it = m_map.find(pObjectFirst);
	else
		it = m_map.begin();
	
	for (/*it*/; (it != m_map.end()); it++)
	{
		void * pObject = it->first;
		bool bResult = (*pfn)(pParam1,pParam2,pObject);
		if (bResult)
			return pObject;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////

#ifdef DEBUG
void util_reftab::dump(int indent) const
{
	wxLogTrace(TRACE_UTIL_DUMP,
			   _T("%*c%s[%p]::dump [size %d]"),
			   indent,' ',m_DebugName.wc_str(),
			   this,
			   (int)m_map.size());
	int k = 0;
	for (TMapConstIterator it=m_map.begin(); (it != m_map.end()); k++, it++)
	{
		void * pObject = it->first;
		int refCnt     = it->second;

		wxLogTrace(TRACE_UTIL_DUMP,
				   _T("%*c[%d] [%p][refCnt %d]"),
				   indent,' ',k,
				   pObject,
				   refCnt);
	}
}
#endif
