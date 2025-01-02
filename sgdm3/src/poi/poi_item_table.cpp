// poi_item_table.cpp
// a "pathname of interest" table -- a container of poi_items.
// there should only be one global instance of this table.
// items are added to this table when a pathname is used for
// something.
//
// WE NEVER DELETE ITEMS FROM THIS TABLE BECAUSE WE USE THEM AS
// HANDLES TO PATHNAMES.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>

//////////////////////////////////////////////////////////////////

poi_item_table::poi_item_table(void)
{
}

void poi_item_table::_free_items_on_list(void)
{
	wxCriticalSectionLocker( this->m_lock );

	for (TMapIterator it = m_map.begin(); (it != m_map.end()); it++)
	{
		poi_item * pPoiItem = it->second;
		delete pPoiItem;
	}

	// TODO 2013/08/12 should we truncate the list while we have the lock ?
}

poi_item_table::~poi_item_table(void)
{
	// map value has pointers rather than references, so we're responsible
	// for deleteing the values before the map gets destroyed..

	// I don't know if this interlude is absolutely necessary,
	// but I want to hold the lock while freeing the
	// items and release the lock before the CS is destroyed.
	_free_items_on_list();
}

//////////////////////////////////////////////////////////////////

void poi_item_table::OnInit(void)
{
}

//////////////////////////////////////////////////////////////////

poi_item * poi_item_table::addItem(const wxString & dirpathname, const wxString & filename)
{
	// always used normalized pathnames to do lookups

	wxFileName f(dirpathname,filename);
	int flagsNorm = wxPATH_NORM_ALL;	// the default (everything except case)
#if defined(__WXMSW__)
	flagsNorm &= ~wxPATH_NORM_ENV_VARS;	// turn off envvar expansion (see item:13183)
	flagsNorm &= ~wxPATH_NORM_SHORTCUT; // turn off .lnk expansion (see W6778)
#endif
	f.Normalize(flagsNorm);

//	wxLogTrace(TRACE_POI_DUMP, _T("wxFileName::Normalize: [in %s / %s][out %s]"), dirpathname.wc_str(), filename.wc_str(), f.GetFullPath().wc_str());

	return _addItem(f);
}

poi_item * poi_item_table::addItem(const wxString & pathname)
{
	// always used normalized pathnames to do lookups

	wxFileName f(pathname);
	int flagsNorm = wxPATH_NORM_ALL;	// the default (everything except case)
#if defined(__WXMSW__)
	flagsNorm &= ~wxPATH_NORM_ENV_VARS;	// turn off envvar expansion (see item:13183)
	flagsNorm &= ~wxPATH_NORM_SHORTCUT; // turn off .lnk expansion (see W6778)
#endif
	f.Normalize(flagsNorm);

//	wxLogTrace(TRACE_POI_DUMP, _T("wxFileName::Normalize: [in %s][out %s]"), pathname.wc_str(), f.GetFullPath().wc_str());

	return _addItem(f);
}

poi_item * poi_item_table::_addItem(const wxFileName & normalizedFilename)
{
	// always used normalized pathnames to do lookups.
	// we do not fold case during pathname normalizaion on win32
	// because we want the actual case when we print strings.
	// we fold only for the map key.

	wxString normPathname = normalizedFilename.GetFullPath();
	wxString strKey(normPathname);
#if defined(__WXMSW__)
	strKey.MakeLower();
#endif

	poi_item * pPoiItem = NULL;

	wxCriticalSectionLocker( this->m_lock );

	TMapIterator it = m_map.find(strKey);
	if (it != m_map.end())
	{
		pPoiItem = it->second;		// we already have one with this key.
	}
	else
	{
		pPoiItem = new poi_item();
		util_error err = pPoiItem->setFileName(normalizedFilename);

		// TODO decide if/how we should complain if we get an error here.

//		wxLogTrace(TRACE_POI_DUMP, _T("poi_item_table::addItem: [%p][%s]"),pPoiItem,normPathname.wc_str());
		m_map.insert( TMapValue(strKey,pPoiItem) );
	}

	return pPoiItem;
}

//////////////////////////////////////////////////////////////////

poi_item * poi_item_table::findItem(const wxString & pathname) /*const -- not const because of lock*/
{
	// always used normalized pathnames to do lookups.
	// we do not fold case during pathname normalizaion on win32
	// because we want the actual case when we print strings.
	// we fold only for the map key.

	wxFileName f(pathname);
	int flagsNorm = wxPATH_NORM_ALL;	// the default (everything except case)
#if defined(__WXMSW__)
	flagsNorm &= ~wxPATH_NORM_ENV_VARS;	// turn off envvar expansion (see item:13183)
	flagsNorm &= ~wxPATH_NORM_SHORTCUT; // turn off .lnk expansion (see W6778)
#endif
	f.Normalize(flagsNorm);
	wxString normPathname = f.GetFullPath();
	wxString strKey(normPathname);
#if defined(__WXMSW__)
	strKey.MakeLower();
#endif

	wxCriticalSectionLocker( this->m_lock );

	TMapConstIterator it = m_map.find(strKey);
	if (it == m_map.end())
		return NULL;

	return it->second;
}

//////////////////////////////////////////////////////////////////

#ifdef DEBUG
void poi_item_table::dump(int indent) /*const -- not const because of lock*/
{
	wxCriticalSectionLocker( this->m_lock );

	wxLogTrace(TRACE_POI_DUMP, _T("%*cPOI_ITEM_TABLE: [nr %d]"),indent,' ',(int)m_map.size());

	int k = 0;
	for (TMapConstIterator it=m_map.begin(); (it != m_map.end()); k++, it++)
	{
		wxString pathname = it->first;
		const poi_item * pPoiItem = it->second;
		wxLogTrace(TRACE_POI_DUMP, _T("%*c[%d] [%p][%s]:"),indent,' ',k,pPoiItem,pathname.wc_str());

		pPoiItem->dump(indent+5);
	}
}
#endif
