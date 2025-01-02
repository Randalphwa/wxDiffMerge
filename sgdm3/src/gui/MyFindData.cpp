// MyFindData
// Singleton class to store the last search string from the
// Find Dialog and/or the last "Use Selection for Find"
// request (and if possible on the MAC the last "Use Selection
// for Find" from another application).
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fs.h>
#include <de.h>
#include <fd.h>
#include <gui.h>

//////////////////////////////////////////////////////////////////

MyFindData::MyFindData(void)
{
	// This singleton class should be instantiated
	// when the application starts up.
	//
	// DiffMerge 4.0.0 and earlier would store/load the
	// last search string in the global prefs between
	// runs.  I'm no longer going to persist the string
	// between runs.  Such a DiffMerge-only search string
	// doesn't play well with any system-defined last
	// search string (such as Command-E stuff on Mac).

#if defined(__WXMAC__)
#else
	m_str.Empty();
#endif
}

MyFindData::~MyFindData(void)
{
}

void MyFindData::set(const wxString & str)
{
#if defined(__WXMAC__)
	util__mac__set_system_search_text(str);
#else
	m_str = str;
#endif
}

wxString MyFindData::get(void) const
{
#if defined(__WXMAC__)
	return util__mac__get_system_search_text();
#else
	return m_str;
#endif
}

#if defined(__WXMAC__)
// Mac now uses NSFindPboard and I don't want to poll
// the system everytime our menu bar wants to do the
// wxUpdateUIEvent stuff for the Find Next/Prev menu
// items.
#else
bool MyFindData::haveData(void) const
{
	wxString s = get();

	return (s.Length() > 0);
}
#endif


