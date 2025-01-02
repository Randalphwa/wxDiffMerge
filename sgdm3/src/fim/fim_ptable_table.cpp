// fim_ptable_table.cpp
// a container of fim_ptable.
// there should only be one global instance of this table.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <poi.h>
#include <fim.h>

//////////////////////////////////////////////////////////////////

fim_ptable_table::fim_ptable_table(void)
	: m_reftab( _T("fim_ptable_table"), true)
{
}

fim_ptable_table::~fim_ptable_table(void)
{
	// map keys are pointers that we allocated, so we're responsible for
	// deleteing them before the map gets destroyed.
	//
	// but this table should be empty before we get destroyed since all
	// windows should have been closed before the app cleans up globals,
	// so we set 'bRequireEmptyOnDestroy' in util_reftab.
}

//////////////////////////////////////////////////////////////////

#if 0 // not needed ?
util_error fim_ptable_table::create(const wxString & path, bool bSniffEncodingBOM, util_encoding enc, fim_ptable ** ppPTable)
{
	poi_item * pPoiItem = gpPoiItemTable->addItem(path); // use filename (rather than directory-name) version

	return create(pPoiItem, bSniffEncodingBOM, enc, ppPTable);
}
#endif

//////////////////////////////////////////////////////////////////

util_error fim_ptable_table::create(poi_item * pPoiItem, bool bSniffEncodingBOM,
									int nrEnc, util_encoding * aEnc,
									fim_ptable ** ppPTable)
{
	util_error ue;
	int k;

	for (k=0; k<nrEnc; k++)
	{
		ue.clear();
		ue = create(pPoiItem, bSniffEncodingBOM, aEnc[k], ppPTable);
		if (ue.isOK())
			return ue;
	}

	return ue;
}

util_error fim_ptable_table::create(poi_item * pPoiItem, bool bSniffEncodingBOM, util_encoding enc, fim_ptable ** ppPTable)
{
	// return piecetable for this pathname.  return existing (shared)
	// piecetable if possible.  create a new one and load file from
	// disk if necessary.

	util_error ue;

	fim_ptable * pPTable = pPoiItem->getPTable();
#if 1
	wxASSERT_MSG( (!pPTable), _T("Coding Error") );
#else
	// with changes to fs_fs::loadFiles(), we don't need this
	if (pPTable)				// existing ptable already active for this poi
	{
		int refCnt = m_reftab.addRef(pPTable);
		//wxLogTrace(TRACE_PTABLE_DUMP,
		//		   _T("fim_ptable_table::create: ref existing [%p,%d]==[%s]"),
		//		   pPTable,refCnt,pPoiItem->getFullPath().wc_str());
		*ppPTable = pPTable;
		return ue;
	}
#endif

	// create a new one and try to load it from disk.

	pPTable = _create();

	ue = pPTable->loadFile(pPoiItem,bSniffEncodingBOM,enc);
	if (ue.isErr())						// could not load file from disk, bail.
	{
//		wxLogTrace(TRACE_PTABLE_DUMP,
//				   _T("fim_ptable_table::create: [%s] failed [%s][%s]"),
//				   util_printable_s(pPoiItem->getFullPath()).wc_str(),
//				   util_printable_s(ue.getMessage()).wc_str(),
//				   util_printable_s(ue.getExtraInfo()).wc_str());

		unRef(pPTable);

		*ppPTable = NULL;
		return ue;
	}

	//wxLogTrace(TRACE_PTABLE_DUMP,
	//		   _T("fim_ptable_table::create: successful [%p]==[%s]"),
	//		   pPTable,
	//		   pPoiItem->getFullPath().wc_str());

	*ppPTable = pPTable;
	return ue;
}

//////////////////////////////////////////////////////////////////

fim_ptable * fim_ptable_table::_create(void)
{
	// create a new ptable for ptable representing a new,blank document.

	fim_ptable * pPTable = new fim_ptable();
	m_reftab.addRef(pPTable);

	return pPTable;
}

//////////////////////////////////////////////////////////////////

fim_ptable * fim_ptable_table::createClone(const fim_ptable * pPTableSrc, fr_prop propMask, poi_item * pPoiClone)
{
	wxASSERT_MSG( (pPTableSrc), _T("Coding Error") );

	// create a new piecetable by cloning the content of the given one.
	// we do not remember any association between these 2 piecetables.
	// we do not associate the src's POI (if it has one) with the clone.

	// TODO for now, we allocate a new growbuf and let the ptable do a
	// TODO frag-by-frag copy.  later, we should consider sharing
	// TODO the growbuf and have the per-frag insert stuff reference
	// TODO the existing buffer rather than allocating new buffer space.
	// TODO we should be able to share a growbuf -- provided we don't
	// TODO truncate the buffer when we abandon a redo path (see TODO
	// TODO in fim_crecvec::_push_crec())

	fim_ptable * pPTableClone = _create();			// allocate and addRef

	// associate the new piece-table with the new (given) poi-item.

	wxASSERT_MSG( (pPoiClone->getPTable()==NULL), _T("Coding Error") );
	pPTableClone->_setPoiItem(pPoiClone);
	pPoiClone->setPTable(pPTableClone);

	// copy the content of the source document.

	pPTableClone->clone(pPTableSrc,propMask);
	
	return pPTableClone;
}

//////////////////////////////////////////////////////////////////

void fim_ptable_table::addRef(fim_ptable * pPTable)
{
	// add reference to existing ptable

	wxASSERT_MSG( (pPTable != NULL), _T("Coding Error") );

	m_reftab.addRef(pPTable);
}

bool fim_ptable_table::unRef(fim_ptable * pPTable)
{
	// release reference to ptable and delete if zero; return true if deleted.

	wxASSERT_MSG( (pPTable != NULL), _T("Coding Error") );

	bool bDeleted = (m_reftab.unRef(pPTable)==0);
	if (bDeleted)
		delete pPTable;

	return bDeleted;
}
