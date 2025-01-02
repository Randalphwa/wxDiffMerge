// fd_fd__export_csv.cpp
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fd.h>
#include <rs.h>

//////////////////////////////////////////////////////////////////

util_error fd_fd::_exportContents_csv(wxString & strResult)
{
	// serialize the contents of m_vec (respecting the various show/hide bits)
	// in CSV format.

	util_error ue;

	wxString strLine, strLabel, strName0, strName1, strInfo;
	//wxString strDate0, strDate1;

	strLabel = _T("Status");
	strName0 = (m_pPoiItemRoot[0]) ? m_pPoiItemRoot[0]->getFullPath() : _T("");
	strName1 = (m_pPoiItemRoot[1]) ? m_pPoiItemRoot[1]->getFullPath() : _T("");	
	strInfo  = formatStatsString();
	//strDate0 = _T("");
	//strDate1 = _T("");

	// fields in CSV are DQuoted so that the COMMAs in the pathnames
	// won't be treated like a delimiter.  DQuote characters are
	// doubled in the CSV file rather than being backslash escaped.

	strName0.Replace(_T("\""),_T("\"\""),true);
	strName1.Replace(_T("\""),_T("\"\""),true);

	// we don't use strLine.Printf() because we are still in UNICODE
	// and don't want to have to down-convert to utf8 or something to
	// use the .c_str() method with "%s".
	// TODO 2013/04/11 revisit this comment now that we are on wx2.9.4.
		
	strLine.Empty();
	strLine += _T("\"");
	strLine += strLabel;
	strLine += _T("\",\"");
	strLine += strName0;
	strLine += _T("\",\"");
	strLine += strName1;
	strLine += _T("\",\"");
	strLine += strInfo;
	//strLine += _T("\",\"");
	//strLine += strDate0;
	//strLine += _T("\",\"");
	//strLine += strDate1;
	strLine += _T("\"\r\n");

	strResult += strLine;

	for (TVecConstIterator it=m_vec.begin(); it!=m_vec.end(); it++)
	{
		fd_item * pFdItem = *it;

		strLabel = fd_item::getStatusText(pFdItem->getStatus());

		poi_item * pPoiItem0 = pFdItem->getPoiItem(0);
		poi_item * pPoiItem1 = pFdItem->getPoiItem(1);

		strName0 = ((pPoiItem0) ? pFdItem->getRelativePathname(0) : _T(""));
		strName1 = ((pPoiItem1) ? pFdItem->getRelativePathname(1) : _T(""));

		strName0.Replace(_T("\""),_T("\"\""),true);
		strName1.Replace(_T("\""),_T("\"\""),true);

		strInfo = _T("");

		//strDate0 = ((pPoiItem0 && pFdItem->getDTM(0).IsValid() ) ? pFdItem->getDTM(0).Format() : _T(""));
		//strDate1 = ((pPoiItem1 && pFdItem->getDTM(1).IsValid() ) ? pFdItem->getDTM(1).Format() : _T(""));
		
		strLine.Empty();
		strLine += _T("\"");
		strLine += strLabel;
		strLine += _T("\",\"");
		strLine += strName0;
		strLine += _T("\",\"");
		strLine += strName1;
		strLine += _T("\",\"");
		strLine += strInfo;
		//strLine += _T("\",\"");
		//strLine += strDate0;
		//strLine += _T("\",\"");
		//strLine += strDate1;
		strLine += _T("\"\r\n");

		strResult += strLine;
	}

	return ue;
}
