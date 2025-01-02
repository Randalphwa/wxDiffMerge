// util_cbl.cpp
// a generic callback list
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>

//////////////////////////////////////////////////////////////////

util_cbl::util_cbl(void)
{
}

util_cbl::~util_cbl(void)
{
	size_t nrHoles = 0;
	size_t nrValid = 0;
	for (TVecIterator it=m_vec.begin(); it<m_vec.end(); it++)
	{
		_item * pItem = (*it);
		if ( (pItem->m_pfn==NULL) && (pItem->m_pData==NULL) )
			nrHoles++;
		else
			nrValid++;
	}

	wxASSERT_MSG( (nrValid == 0), _T("Coding Error: ubil_cbl::~util_cbl: CB list not empty") );
	wxASSERT_MSG( (nrHoles == m_vec.size()), _T("Coding Error: ubil_cbl::~util_cbl: CB list not empty") );

	for (TVecIterator it=m_vec.begin(); it<m_vec.end(); it++)
		delete (*it);
}

//////////////////////////////////////////////////////////////////

void util_cbl::addCB(util_cbl_fn pfn, void * pData)
{
	// TODO should we add a refCnt so we can do multiple adds for a (pfn,pData) pair ?

	// see if we can reuse a hole before appending a new item

	for (TVecIterator it=m_vec.begin(); it<m_vec.end(); it++)
	{
		_item * pItem = (*it);
		if ( (pItem->m_pfn==NULL) && (pItem->m_pData==NULL) )
		{
			pItem->m_pfn = pfn;
			pItem->m_pData = pData;
			return;
		}
	}
	
	_item * pItem = new _item(pfn,pData);
	m_vec.push_back(pItem);
}

void util_cbl::delCB(util_cbl_fn pfn, void * pData)
{
	for (TVecIterator it=m_vec.begin(); it<m_vec.end(); it++)
	{
		_item * pItem = (*it);
		if ( (pItem->m_pfn==pfn) && (pItem->m_pData==pData) )
		{
			// leave hole in vec so we don't invalidate iterators
			pItem->m_pfn = NULL;
			pItem->m_pData = NULL;
			return;
		}
	}
	wxASSERT_MSG( 0, _T("Coding Error: util_cbl::delCB: item not in list") );
}

//////////////////////////////////////////////////////////////////

void util_cbl::callAll(const util_cbl_arg & arg)
{
	for (TVecIterator it=m_vec.begin(); it<m_vec.end(); it++)
	{
		_item * pItem = (*it);
		if (pItem->m_pfn)
			(*pItem->m_pfn)(pItem->m_pData,arg);
	}
}

//////////////////////////////////////////////////////////////////

long util_cbl::count(void) const
{
	size_t nrHoles = 0;
	size_t nrValid = 0;
	for (TVecConstIterator cit=m_vec.begin(); cit<m_vec.end(); cit++)
	{
		_item * pItem = (*cit);
		if ( (pItem->m_pfn==NULL) && (pItem->m_pData==NULL) )
			nrHoles++;
		else
			nrValid++;
	}

	return (long)nrValid;
}

//////////////////////////////////////////////////////////////////

#ifdef DEBUG
void util_cbl::dump(int indent) const
{
	for (TVecConstIterator cit=m_vec.begin(); cit<m_vec.end(); cit++)
		wxLogTrace(TRACE_UTIL_DUMP, _T("%*cUTIL_CBL: [%p][%p]"),indent,' ',(*cit)->m_pfn,(*cit)->m_pData);
}
#endif
