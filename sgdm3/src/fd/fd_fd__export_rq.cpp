// fd_fd__export_rq.cpp
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fd.h>
#include <rs.h>

//////////////////////////////////////////////////////////////////

/**
 * generate a brief summary of the diffs between these
 * two folders.  this is something like:
 *      /usr/bin/diff -r -q F1 F2
 *
 * we respect the various show/hide bits currently set
 * in fd_fd::m_ShowHideFlags (and don't force a reload).
 *
 */
util_error fd_fd::_exportContents__rq(wxString & strResult)
{
	util_error ue;
	wxString strLine, strName0, strName1;
	static const wxChar * apszType[] = { L"(null)", L"file", L"folder", L"shortcut", L"(other)" };
	const wxChar * pszType0;
	const wxChar * pszType1;

#if defined(__WXMSW__)
#define MY_EOL L"\r\n"
#else
#define MY_EOL L"\n"
#endif

	strLine.Printf( (L"=== Left:  %s" MY_EOL), m_pPoiItemRoot[0]->getFullPath().wc_str());
	strResult += strLine;
	strLine.Empty();

	strLine.Printf( (L"=== Right: %s" MY_EOL), m_pPoiItemRoot[1]->getFullPath().wc_str());
	strResult += strLine;
	strLine.Empty();

	strLine.Printf( (L"=== Summary: %s" MY_EOL), formatStatsString());
	strResult += strLine;
	strLine.Empty();
	
	strLine.Printf( (L"=== Showing Equal: %s" MY_EOL),
					((m_ShowHideFlags & FD_SHOW_HIDE_FLAGS__EQUAL)      ? L"yes" : L"no"));
	strResult += strLine;
	strLine.Empty();

	strLine.Printf( (L"=== Showing Equivalent: %s" MY_EOL),
					((m_ShowHideFlags & FD_SHOW_HIDE_FLAGS__EQUIVALENT) ? L"yes" : L"no"));
	strResult += strLine;
	strLine.Empty();

	strLine.Printf( (L"=== Showing QuickMatch: %s" MY_EOL),
					((m_ShowHideFlags & FD_SHOW_HIDE_FLAGS__QUICKMATCH) ? L"yes" : L"no"));
	strResult += strLine;
	strLine.Empty();

	strLine.Printf( (L"=== Showing Peerless: %s" MY_EOL),
					((m_ShowHideFlags & FD_SHOW_HIDE_FLAGS__SINGLES)    ? L"yes" : L"no"));
	strResult += strLine;
	strLine.Empty();

	strLine.Printf( (L"=== Showing Folders: %s" MY_EOL),
					((m_ShowHideFlags & FD_SHOW_HIDE_FLAGS__FOLDERS)    ? L"yes" : L"no"));
	strResult += strLine;
	strLine.Empty();

	strLine.Printf( (L"=== Showing Errors: %s" MY_EOL),
					((m_ShowHideFlags & FD_SHOW_HIDE_FLAGS__ERRORS)     ? L"yes" : L"no"));
	strResult += strLine;
	strLine.Empty();
	
	strLine.Printf( (L"=== ================================================================" MY_EOL));
	strResult += strLine;
	strLine.Empty();

	for (TVecConstIterator it=m_vec.begin(); it!=m_vec.end(); it++)
	{
		fd_item * pFdItem = *it;

		poi_item * pPoiItem0 = pFdItem->getPoiItem(0);
		poi_item * pPoiItem1 = pFdItem->getPoiItem(1);

		strName0 = ((pPoiItem0) ? pFdItem->getRelativePathname(0) : _T(""));
		strName1 = ((pPoiItem1) ? pFdItem->getRelativePathname(1) : _T(""));

		switch (pFdItem->getStatus())
		{
		default:
		case fd_item::FD_ITEM_STATUS_UNKNOWN:
			strLine.Printf( (L"Unknown status for '%s' and '%s'" MY_EOL), strName0.wc_str(), strName1.wc_str());
			break;

		case fd_item::FD_ITEM_STATUS_ERROR:
			strLine.Printf( (L"Error reading '%s' or '%s'" MY_EOL), strName0.wc_str(), strName1.wc_str());
			break;

		case fd_item::FD_ITEM_STATUS_MISMATCH:
			pszType0 = apszType[pPoiItem0->getPoiType()];
			pszType1 = apszType[pPoiItem1->getPoiType()];
			strLine.Printf( (L"Not comparable: %s '%s' and %s '%s'" MY_EOL),
							pszType0, strName0.wc_str(), pszType1, strName1.wc_str());
			break;

		case fd_item::FD_ITEM_STATUS_BOTH_NULL:
			strLine.Printf( (L"Both null" MY_EOL) );	// can't happen
			break;

		case fd_item::FD_ITEM_STATUS_SAME_FILE:
		case fd_item::FD_ITEM_STATUS_IDENTICAL:
			strLine.Printf( (L"Files '%s' and '%s' are identical" MY_EOL), strName0.wc_str(), strName1.wc_str());
			break;
			
		case fd_item::FD_ITEM_STATUS_EQUIVALENT:
			strLine.Printf( (L"Files '%s' and '%s' are equivalent" MY_EOL), strName0.wc_str(), strName1.wc_str());
			break;

		case fd_item::FD_ITEM_STATUS_QUICKMATCH:
			strLine.Printf( (L"Files '%s' and '%s' quick-matched" MY_EOL), strName0.wc_str(), strName1.wc_str());
			break;

		case fd_item::FD_ITEM_STATUS_DIFFERENT:
			strLine.Printf( (L"Files '%s' and '%s' are different" MY_EOL), strName0.wc_str(), strName1.wc_str());
			break;

		case fd_item::FD_ITEM_STATUS_FOLDERS:
			strLine.Printf( (L"Folders '%s' and '%s' exist in both" MY_EOL), strName0.wc_str(), strName1.wc_str());
			break;

		case fd_item::FD_ITEM_STATUS_FILE_NULL:
			strLine.Printf( (L"File only in left: '%s'" MY_EOL), strName0.wc_str());
			break;
		case fd_item::FD_ITEM_STATUS_NULL_FILE:
			strLine.Printf( (L"File only in right: '%s'" MY_EOL), strName1.wc_str());
			break;

		case fd_item::FD_ITEM_STATUS_FOLDER_NULL:
			strLine.Printf( (L"Folder only in left: '%s'" MY_EOL), strName0.wc_str());
			break;
		case fd_item::FD_ITEM_STATUS_NULL_FOLDER:
			strLine.Printf( (L"Folder only in right: '%s'" MY_EOL), strName1.wc_str());
			break;
			
		case fd_item::FD_ITEM_STATUS_SHORTCUT_NULL:
			strLine.Printf( (L"Shortcut only in left: '%s'" MY_EOL), strName0.wc_str());
			break;
		case fd_item::FD_ITEM_STATUS_NULL_SHORTCUT:
			strLine.Printf( (L"Shortcut only in right: '%s'" MY_EOL), strName1.wc_str());
			break;

		case fd_item::FD_ITEM_STATUS_SHORTCUTS_SAME:
		case fd_item::FD_ITEM_STATUS_SHORTCUTS_EQ:
			strLine.Printf( (L"Shortcuts '%s' and '%s' are identical" MY_EOL), strName0.wc_str(), strName1.wc_str());
			break;

		case fd_item::FD_ITEM_STATUS_SHORTCUTS_NEQ:
			strLine.Printf( (L"Shortcuts '%s' and '%s' are different" MY_EOL), strName0.wc_str(), strName1.wc_str());
			break;
			
		case fd_item::FD_ITEM_STATUS_SYMLINK_NULL:
			strLine.Printf( (L"Symlink only in left: '%s'" MY_EOL), strName0.wc_str());
			break;
		case fd_item::FD_ITEM_STATUS_NULL_SYMLINK:
			strLine.Printf( (L"Symlink only in right: '%s'" MY_EOL), strName1.wc_str());
			break;

		case fd_item::FD_ITEM_STATUS_SYMLINKS_SAME:
		case fd_item::FD_ITEM_STATUS_SYMLINKS_EQ:
			strLine.Printf( (L"Symlinks '%s' and '%s' are identical" MY_EOL), strName0.wc_str(), strName1.wc_str());
			break;

		case fd_item::FD_ITEM_STATUS_SYMLINKS_NEQ:
			strLine.Printf( (L"Symlinks '%s' and '%s' are different" MY_EOL), strName0.wc_str(), strName1.wc_str());
			break;
		}

		strResult += strLine;
		strLine.Empty();
	}

	return ue;

}
