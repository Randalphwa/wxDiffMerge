// de_de__batchoutput_html_sxs.cpp -- diff engine -- parts of de_de related
// to generating batch output (such as gnu-diff-like output) but formatted
// in HTML rather than raw text.  This version does a side-by-side view.
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <poi.h>
#include <fim.h>
#include <fs.h>
#include <fl.h>
#include <rs.h>
#include <de.h>

#if defined(__WXMSW__)
#	include <sys/types.h>
#	include <sys/timeb.h>
#elif defined(__WXMAC__)

#elif defined(__WXGTK__)

#endif

//////////////////////////////////////////////////////////////////

static const wchar_t * gsz_s2d_h_x = (		// part of header: path+date
	L"<tr class=\"s2d_h_x\">\n"
	L"  <td class=\"s2d_h_x_path\">%s</td>\n"
	L"  <td class=\"s2d_h_x_dtm\">%s</td>\n"
	L"</tr>\n"
	);

static const wchar_t * gsz_s2d_h_stats = (	// part of header: stats
	L"<tr class=\"s2d_h_stats\">\n"
	L"  <td colspan=\"2\">%s</td>\n"			// de_stats2
	L"</tr>\n"
	);

static const wchar_t * gsz_s2d_h = (			// overall header
	L"<table class=\"s2d_h\">\n"
	L"%s"			// left path/label
	L"%s"			// right path/label
	L"%s"			// stats in header
	L"</table>\n"
	);

static const wchar_t * gsz_s2d_f = (			// overall footer
	L"<table class=\"s2d_f\">\n"
	L"  <tr class=\"s2d_f_dops\">\n"
	L"    <td>%s</td>\n"						// display_ops
	L"  </tr>\n"
	L"</table>\n"
	);


static const wchar_t * gsz_s2d_x_eq = (		// a row of code in diff
	L"<tr class=\"s2d_x_eq\">\n"
	L"  <td class=\"s2d_ln s2d_x_eq_ln\">%ld</td>\n"		// left line number
	L"  <td class=\"s2d_code s2d_x_eq_code\">%s</td>\n"		// a line of code
	L"  <td class=\"s2d_ln s2d_x_eq_ln\">%ld</td>\n"		// right line number
	L"  <td class=\"s2d_code s2d_x_eq_code\">%s</td>\n"		// a line of code
	L"</tr>\n"
	);

static const wchar_t * gsz_s2d_x_neq = (
	L"<tr class=\"s2d_x_neq\">\n"
	L"  <td class=\"s2d_ln s2d_x_neq_ln\">%ld</td>\n"		// left line number
	L"  <td class=\"s2d_code s2d_x_neq_code\">%s</td>\n"		// a line of code
	L"  <td class=\"s2d_ln s2d_x_neq_ln\">%ld</td>\n"		// right line number
	L"  <td class=\"s2d_code s2d_x_neq_code\">%s</td>\n"		// a line of code
	L"</tr>\n"
	);

static const wchar_t * gsz_s2d_x_del = (
	L"<tr class=\"s2d_x_neq\">\n"
	L"  <td class=\"s2d_ln s2d_x_neq_ln\">%ld</td>\n"		// left line number
	L"  <td class=\"s2d_code s2d_x_neq_code\">%s</td>\n"		// a line of code
	L"  <td class=\"s2d_ln s2d_void\"></td>\n"		// right line number
	L"  <td class=\"s2d_code s2d_void\"></td>\n"		// a line of code
	L"</tr>\n"
	);

static const wchar_t * gsz_s2d_x_add = (
	L"<tr class=\"s2d_x_neq\">\n"
	L"  <td class=\"s2d_ln s2d_void\"></td>\n"		// left line number
	L"  <td class=\"s2d_code s2d_void\"></td>\n"		// a line of code
	L"  <td class=\"s2d_ln s2d_x_neq_ln\">%ld</td>\n"		// right line number
	L"  <td class=\"s2d_code s2d_x_neq_code\">%s</td>\n"		// a line of code
	L"</tr>\n"
	);


static const wchar_t * gsz_s2d_x_eq_unimp_il = (		// intraline highlight
	L"<span class=\"s2d_x_eq_unimp_il\">%s</span>"		// DO NOT ADD TRAILING \n
	);
	
	
static const wchar_t * gsz_s2d_x_neq_unimp_il = (		// intraline highlight
	L"<span class=\"s2d_x_neq_unimp_il\">%s</span>"		// DO NOT ADD TRAILING \n
	);
	
static const wchar_t * gsz_s2d_x_neq_il = (			// intraline highlight
	L"<span class=\"s2d_x_neq_il\">%s</span>"		// DO NOT ADD TRAILING \n
	);

	
static const wchar_t * gsz_s2d_x10_omitted = 		// an omitted line 
	(
		L"<tr class=\"s2d_x_omit\">\n"
		L"  <td class=\"s2d_ln s2d_x_omit_ln\">%ld</td>\n"		// left line number
		L"  <td class=\"s2d_code s2d_x_omit_code\">%s</td>\n"
		L"  <td class=\"s2d_ln s2d_void\"></td>\n"
		L"  <td class=\"s2d_code s2d_void\"></td>\n"
		L"</tr>\n"
		);
static const wchar_t * gsz_s2d_x01_omitted = 		// an omitted line 
	(
		L"<tr class=\"s2d_x_omit\">\n"
		L"  <td class=\"s2d_ln s2d_void\"></td>\n"		// left line number
		L"  <td class=\"s2d_code s2d_void\"></td>\n"
		L"  <td class=\"s2d_ln s2d_x_omit_ln\">%ld</td>\n"
		L"  <td class=\"s2d_code s2d_x_omit_code\">%s</td>\n"		// a line of code
		L"</tr>\n"
		);
static const wchar_t * gsz_s2d_x11_omitted = 		// an omitted line 
	(
		L"<tr class=\"s2d_x_omit\">\n"
		L"  <td class=\"s2d_ln s2d_x_omit_ln\">%ld</td>\n"		// left line number
		L"  <td class=\"s2d_code s2d_x_omit_code\">%s</td>\n"
		L"  <td class=\"s2d_ln s2d_x_omit_ln\">%ld</td>\n"
		L"  <td class=\"s2d_code s2d_x_omit_code\">%s</td>\n"		// a line of code
		L"</tr>\n"
		);

	
static const wchar_t * gsz_s2d_x10_unimp =
	(
		L"<tr class=\"s2d_x_unimp\">\n"
		L"  <td class=\"s2d_ln s2d_x_unimp_ln\">%ld</td>\n"		// left line number
		L"  <td class=\"s2d_code s2d_x_unimp_code\">%s</td>\n"
		L"  <td class=\"s2d_ln s2d_void\"></td>\n"
		L"  <td class=\"s2d_code s2d_void\"></td>\n"
		L"</tr>\n"
		);
static const wchar_t * gsz_s2d_x01_unimp =
	(
		L"<tr class=\"s2d_x_unimp\">\n"
		L"  <td class=\"s2d_ln s2d_void\"></td>\n"		// left line number
		L"  <td class=\"s2d_code s2d_void\"></td>\n"
		L"  <td class=\"s2d_ln s2d_x_unimp_ln\">%ld</td>\n"
		L"  <td class=\"s2d_code s2d_x_unimp_code\">%s</td>\n"		// a line of code
		L"</tr>\n"
		);
static const wchar_t * gsz_s2d_x11_unimp =
	(
		L"<tr class=\"s2d_x_unimp\">\n"
		L"  <td class=\"s2d_ln s2d_x_unimp_ln\">%ld</td>\n"		// left line number
		L"  <td class=\"s2d_code s2d_x_unimp_code\">%s</td>\n"
		L"  <td class=\"s2d_ln s2d_x_unimp_ln\">%ld</td>\n"
		L"  <td class=\"s2d_code s2d_x_unimp_code\">%s</td>\n"		// a line of code
		L"</tr>\n"
		);


static const wchar_t * gsz_s2d_gap =
	(
		L"<tr class=\"s2d_x_gap_tr\">\n"
		L"   <td class=\"s2d_x_gap_td\" colspan=\"4\">&nbsp;</td>\n"
		L"</tr>\n"
		);


static const wchar_t * gsz_s2d = (			// overall diff
	L"<div class=\"s2d\">\n"
	L"  <table class=\"s2d_t\">\n"
	L"    <thead class=\"s2d_thead\">\n"
	L"      <tr>\n"
	L"        <td class=\"s2d_thead_td\" colspan=\"4\">\n"
	L"%s"									// s2d_h
	L"        </td>\n"
	L"      </tr>\n"
	L"    </thead>\n"
	L"    <tbody class=\"s2d_tbody\">\n"
	L"%s"									// array of s2d_x
	L"    </tbody>\n"
	L"    <tfoot class=\"s2d_tfoot\">\n"
	L"      <tr>\n"
	L"        <td class=\"s2d_tfoot_td\" colspan=\"4\">\n"
	L"%s"									// s2d_f
	L"        </td>\n"
	L"      </tr>\n"
	L"    </tfoot>\n"
	L"  </table>\n"
	L"</div>\n"
	);

// css-3 is in the process of defining a tab-size: property
// http://www.w3.org/TR/css3-text/#tab-size1
// which i'm going to try to use here.  so it may not work
// on all browsers.  there are some non-standard fall-backs
// that each browser seems to support, so i'll include those
// too.
// https://developer.mozilla.org/en-US/docs/Web/CSS/tab-size

#define TOK__CSS_TABSIZE		L"%TABSIZE%"
#define TOK__CSS_PATH_FONTSIZE	L"%PATHFONTSIZE%"
#define TOK__CSS_STAT_FONTSIZE	L"%STATFONTSIZE%"
#define TOK__CSS_CODE_FONTSIZE	L"%CODEFONTSIZE%"

static const wchar_t * gsz_s2d_css = (		// css info for s2d
	L"  .s2d_h { width: 100%; border-spacing: 0px; text-shadow: 0px 1px 0px rgb(255,255,255); background-color: rgb(234,234,234); background-image: linear-gradient(rgb(250,250,250), rgb(234,234,234)); border-bottom: 1px solid rgb(216,216,216); } \n"
	L"  .s2d_h_x { }\n"
	L"  .s2d_h_x_path { font-weight: bold; font-size: "  TOK__CSS_PATH_FONTSIZE  L"pt; }\n"
	L"  .s2d_h_x_dtm  {                    font-size: "  TOK__CSS_STAT_FONTSIZE  L"pt; text-align: right }\n"
	L"  .s2d_h_stats  {                    font-size: "  TOK__CSS_STAT_FONTSIZE  L"pt; text-align: right }\n"

	L"  .s2d_f { width: 100%; border-spacing: 0px; text-shadow: 0px 1px 0px rgb(255,255,255); background-color: rgb(234,234,234); background-image: linear-gradient(rgb(250,250,250), rgb(234,234,234)); border-top: 1px solid rgb(216,216,216); }\n"
	L"  .s2d_f_dops { font-size: "  TOK__CSS_STAT_FONTSIZE  L"pt; text-align: right }\n"

	L"  .s2d_ln   { width: 1px; padding-left: 8px; padding-right: 8px; text-align: right; vertical-align: text-top; }\n"
	L"  .s2d_code { vertical-align: text-top;\n"
	L"              white-space: pre-wrap;\n"
	L"              tab-size:      "  TOK__CSS_TABSIZE L";\n"
	L"              -moz-tab-size: "  TOK__CSS_TABSIZE L";\n"
	L"              -o-tab-size:   "  TOK__CSS_TABSIZE L";\n"
	L"            }\n"

	L"    .s2d_x_eq      { }\n"
	L"    .s2d_x_eq_ln   { border-right: 1px solid rgb(216,216,216); }\n"
	L"  td.s2d_x_eq_code { border-right: 1px solid rgb(216,216,216); }\n"

	L"    .s2d_x_neq      { background-color: rgb(253,196,196); }\n"
	L"    .s2d_x_neq_ln   { background-color: rgb(216,170,170); right; border-right: 1px solid rgb(204,161,161); }\n"
	L"  td.s2d_x_neq_code { border-right: 1px solid rgb(204,161,161); }\n"

	L"    .s2d_x_omit      { background-color: rgb(240,240,240); color: rgb(128,128,128); }\n"
	L"    .s2d_x_omit_ln   { background-color: rgb(216,216,216); border-right: 1px solid rgb(183,183,183); }\n"
	L"  td.s2d_x_omit_code { border-right: 1px solid rgb(183,183,183); }\n"

	L"    .s2d_x_unimp      { }\n"
	L"    .s2d_x_unimp_ln   { border-right: 1px solid rgb(216,216,216); }\n"
	L"  td.s2d_x_unimp_code { border-right: 1px solid rgb(216,216,216); }\n"


	L"  .s2d_x_neq_il       { background-color: rgb(255,170,170); font-weight: bold; }\n"
	L"  .s2d_x_neq_unimp_il { color: rgb(150,150,150); }\n"
	L"  .s2d_x_eq_unimp_il  { }\n"


	L"  .s2d_void { border-right: 1px solid rgb(216,216,216);"
	L"              background-color: rgb(234,234,234);"
	L"              background-image: linear-gradient(to right, rgb(234,234,234), rgb(250,250,250));"
	L"            }\n"


	L"  .s2d_x_gap_tr { background-color: rgb(255,255,255); }\n"
	L"  .s2d_x_gap_td { border-top: 1px solid rgb(216,216,216); border-bottom: 1px solid rgb(216,216,216); }\n"


	L"  .s2d { }\n"
	L"  .s2d_t { width: 100%; border: 1px solid rgb(204,204,204); border-spacing: 0px; border-collapse: collapse; font-family: Courier New,monospace; font-size: "  TOK__CSS_CODE_FONTSIZE  L"pt; }\n"
	L"  .s2d_thead { }\n"
	L"  .s2d_thead_td { padding: 0px; }\n"
	L"  .s2d_tbody { }\n"
	L"  .s2d_tfoot { }\n"
	L"  .s2d_tfoot_td { padding: 0px; }\n"

	);

static const wchar_t * gsz_s2 = (
	L"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
	L"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
	L"  <head>\n"
	L"    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n"
	L"    <style type=\"text/css\">\n"
	L"%s"									// s2d_css
	L"    </style>\n"
	L"    <title>\n"
	L"      SourceGear DiffMerge Diff\n"
	L"    </title>\n"
	L"  </head>\n"
	L"  <body>\n"
	L"    <div>\n"
	L"%s"			// s2d containing complete diff, headers, and footers
	L"    </div>\n"
	L"  </body>\n"
	L"</html>\n"
	);

//////////////////////////////////////////////////////////////////

static wxString _s2d_h_x(poi_item * pPoiItem,
						 wxString * pStrLabel=NULL)
{
	// NOTE we assume that there are no '<', '>', or '&' in the
	// generated DTM strings.

	wxDateTime dtm;
	pPoiItem->getDateTimeModified(&dtm);
	wxString strDTM = util_string__format_unified_date_from_dtm(dtm, false, false);

	wxString strPath;
	if (pStrLabel)
		strPath = *pStrLabel;
	else
		strPath = pPoiItem->getFullPath();
	wxString strPathEsc = html_diff_escape_string(strPath);

	return wxString::Format( gsz_s2d_h_x,
							 strPathEsc.wc_str(), strDTM.wc_str() );
}

static wxString _s2d_h_stats(const de_stats2 * pStats)
{
	wxString strStats = pStats->formatStatsString();
	wxString strStatsEsc = html_diff_escape_string(strStats);

	return wxString::Format( gsz_s2d_h_stats, strStatsEsc.wc_str() );

}

static wxString _s2d_h(const de_de * pDeDe, long kSync,
					 wxString * pStrLabelA, wxString * pStrLabelB)
{
	wxString str0( _s2d_h_x( pDeDe->getFsFs()->getPoi(kSync,PANEL_T0), pStrLabelA) );
	wxString str1( _s2d_h_x( pDeDe->getFsFs()->getPoi(kSync,PANEL_T1), pStrLabelB) );
	wxString strS( _s2d_h_stats( pDeDe->getStats2(kSync)) );

	return wxString::Format( gsz_s2d_h,
							 str0.wc_str(), str1.wc_str(), strS.wc_str() );
}

static wxString _s2d_f(const de_de * pDeDe, long kSync, int colTabWidth)
{
	wxString strModeString = pDeDe->getDisplayModeString(kSync, colTabWidth);
	wxString strModeStringEsc = html_diff_escape_string(strModeString);

	return wxString::Format( gsz_s2d_f,
							 strModeStringEsc );
}

static wxString _s2d_gap(void)
{
	// we are drawing row-by-row using the display list -- not the line list
	// in the document.  this allows us to show diffs-only or diffs-with-context.
	// if the row is marked "gapped", then the diff-engine hid some content
	// immediately prior to the current row.  in printouts and in the GUI we
	// draw a line in the leading above the characters.  here in html we do it
	// a little differently.

	return wxString(gsz_s2d_gap);
}

static wxString _s2d_x_content(const de_row & rDeRowK, PanelIndex kPanel,
							   bool bIgnoreUnimportant,
							   bool bGlobalRespectEOL)
{
	wxString strThisLineEsc;
	const de_line * pDeLine = rDeRowK.getPanelLine(kPanel);

	// caller only calls us when this side has content (!eof, !void)
	wxASSERT_MSG( (pDeLine), _T("Coding Error") );
	if (!pDeLine)
		return strThisLineEsc;

	const de_sync * pDeSync = rDeRowK.getSync();
	if (pDeSync->haveIntraLineSyncInfo())
	{
		long                 offsetInSync  = rDeRowK.getOffsetInSync();
		const de_sync_list * pDeSyncListIL = pDeSync->getIntraLineSyncList(offsetInSync);
		const de_sync *      pDeSyncIL;
					
		wxString strLine( pDeLine->getStrLine() );
		if (bGlobalRespectEOL)					// if we included EOLs in the match, then there will be
			strLine += pDeLine->getStrEOL();	// intra-line sync nodes indexing past the end of the regular line.

		for (pDeSyncIL=pDeSyncListIL->getHead(); (pDeSyncIL && !pDeSyncIL->isEND()); pDeSyncIL=pDeSyncIL->getNext())
		{
			long len = pDeSyncIL->getLen(kPanel);
			if (len > 0)
			{
				de_attr attr_il = pDeSyncIL->getAttr();
				wxString strContent(strLine.wc_str() + pDeSyncIL->getNdx((PanelIndex)kPanel), len);
				wxString strContentEsc( html_diff_escape_string(strContent, bGlobalRespectEOL) );

				const wchar_t * szTemplate = NULL;
				if (DE_ATTR__IS_TYPE(attr_il, DE_ATTR_DIF_0EQ))
				{
					if (bIgnoreUnimportant && DE_ATTR__IS_SET(attr_il, DE_ATTR_UNIMPORTANT))
						szTemplate = gsz_s2d_x_neq_unimp_il;
					else
						szTemplate = gsz_s2d_x_neq_il;

					wxString strThisSpan;
					strThisSpan.Printf( szTemplate, strContentEsc );
					strThisLineEsc += strThisSpan;
				}
				else if (DE_ATTR__IS_TYPE(attr_il, DE_ATTR_DIF_2EQ))
				{
					if (bIgnoreUnimportant && DE_ATTR__IS_SET(attr_il, DE_ATTR_UNIMPORTANT))
					{
						szTemplate = gsz_s2d_x_eq_unimp_il;
						wxString strThisSpan;
						strThisSpan.Printf( szTemplate, strContentEsc );
						strThisLineEsc += strThisSpan;
					}
					else
					{
						// Don't waste a <span> on normal content
						strThisLineEsc += strContentEsc;
					}
				}
			}
		}
	}
	else
	{
		wxString strThisLine;
		const fl_line * pFlLine = pDeLine->getFlLine();
		pFlLine->buildStringsFromRuns(bGlobalRespectEOL, &strThisLine  /*,&strThisEol*/);
		strThisLineEsc = html_diff_escape_string(strThisLine, bGlobalRespectEOL);
	}

	// TODO 2013/09/03 think about whether to do something with this
	// if ((strThisEol.Length() == 0) || (!strThisEol.EndsWith(_T("\n"))))
	//     strLines[kPanel] += _T("\n\\ No newline at end of file\n");

	return strThisLineEsc;
}

static wxString _s2d_array(de_de * pDeDe, long kSync)
{
	wxString strResult;		// to accumulate detail lines

	de_display_ops dops = pDeDe->getDisplayOps(kSync);
	bool bIgnoreUnimportant = DE_DOP__IS_SET_IGN_UNIMPORTANT(dops);
	bool bGlobalRespectEOL = RS_ATTRS_RespectEOL( pDeDe->getFsFs()->getRuleSet()->getMatchStripAttrs() );

	TVector_Display * pDis = pDeDe->getDisplayList(kSync);	// forces run() if necessary
	long nrRows = (long)pDis->size() - 1;
	long kRow;
	for (kRow=0; kRow<nrRows; kRow++)
	{
		const de_row & rDeRowK = (*pDis)[kRow];
		if (rDeRowK.haveGap())
			strResult += _s2d_gap();
			
		const de_sync * pSyncK = rDeRowK.getSync();
		const de_line * pDeLine0 = rDeRowK.getPanelLine(PANEL_T0);
		const de_line * pDeLine1 = rDeRowK.getPanelLine(PANEL_T1);
		const fl_line * pFlLine0 = (pDeLine0) ? pDeLine0->getFlLine() : NULL;
		const fl_line * pFlLine1 = (pDeLine1) ? pDeLine1->getFlLine() : NULL;
		bool bTreatAsUnimportant = (bIgnoreUnimportant && pSyncK->isUnimportant());
		long lineNr0 = ((pFlLine0) ? (long)pFlLine0->getLineNr()+1 : -1);
		long lineNr1 = ((pFlLine1) ? (long)pFlLine1->getLineNr()+1 : -1);

		wxLogTrace(wxTRACE_Messages, _T("s2d:array: [kRow %d][syncType %04x][unimp %d][line %ld %ld]"),
				   (int)kRow, (int)pSyncK->getAttr(), (int)pSyncK->isUnimportant(),
				   lineNr0, lineNr1);

		if (pDeLine0 && pDeLine1)
		{
			wxString strContentEsc0( _s2d_x_content(rDeRowK, PANEL_T0, bIgnoreUnimportant, bGlobalRespectEOL) );
			wxString strContentEsc1( _s2d_x_content(rDeRowK, PANEL_T1, bIgnoreUnimportant, bGlobalRespectEOL) );

			const wchar_t * szTemplate = NULL;
			if (bTreatAsUnimportant)
				szTemplate = gsz_s2d_x11_unimp;
			else if (pSyncK->isSameType(DE_ATTR_DIF_2EQ))
				szTemplate = gsz_s2d_x_eq;
			else if (pSyncK->isSameType(DE_ATTR_DIF_0EQ))
				szTemplate = gsz_s2d_x_neq;
			else if (pSyncK->isSameType(DE_ATTR_OMITTED))
				szTemplate = gsz_s2d_x11_omitted;
			else
				wxLogTrace(wxTRACE_Messages, _T("s2d:array: 11"));

			if (szTemplate)
			{
				wxString strThisLineDetail;
				strThisLineDetail.Printf( szTemplate, lineNr0, strContentEsc0, lineNr1, strContentEsc1 );
				strResult += strThisLineDetail;
			}
		}
		else if (pDeLine0)
		{
			wxString strContentEsc0( _s2d_x_content(rDeRowK, PANEL_T0, bIgnoreUnimportant, bGlobalRespectEOL) );

			const wchar_t * szTemplate = NULL;
			if (bTreatAsUnimportant)
				szTemplate = gsz_s2d_x10_unimp;
			else if (pSyncK->isSameType(DE_ATTR_DIF_0EQ))
				szTemplate = gsz_s2d_x_del;
			else if (pSyncK->isSameType(DE_ATTR_OMITTED))
				szTemplate = gsz_s2d_x10_omitted;
			else
				wxLogTrace(wxTRACE_Messages, _T("s2d:array: 10"));

			if (szTemplate)
			{
				wxString strThisLineDetail;
				strThisLineDetail.Printf( szTemplate, lineNr0, strContentEsc0 );
				strResult += strThisLineDetail;
			}
		}
		else if (pDeLine1)
		{
			wxString strContentEsc1( _s2d_x_content(rDeRowK, PANEL_T1, bIgnoreUnimportant, bGlobalRespectEOL) );

			const wchar_t * szTemplate = NULL;
			if (bTreatAsUnimportant)
				szTemplate = gsz_s2d_x01_unimp;
			else if (pSyncK->isSameType(DE_ATTR_DIF_0EQ))
				szTemplate = gsz_s2d_x_add;
			else if (pSyncK->isSameType(DE_ATTR_OMITTED))
				szTemplate = gsz_s2d_x01_omitted;
			else
				wxLogTrace(wxTRACE_Messages, _T("s2d:array: 01"));

			if (szTemplate)
			{
				wxString strThisLineDetail;
				strThisLineDetail.Printf( szTemplate, lineNr1, strContentEsc1 );
				strResult += strThisLineDetail;
			}
		}
		else
		{
			wxASSERT_MSG( (0), _T("Coding Error") );
		}
	}

	return strResult;
}

wxString de_de::batchoutput_html_sxs_diff2__div(long kSync,
												bool * pbHadChanges,
												int colTabWidth,
												wxString * pStrLabelA,
												wxString * pStrLabelB)
{
	wxString strDiv;

	// since this is a side-by-side diff, we can respect the
	// current display-op, ignore-unimportant, and hide-omitted
	// settings.

	const de_stats2 * pStats = getStats2(kSync);
	long cChanges = pStats->m_cImportantChanges;	// + pStats->m_cUnimportantChanges;

	*pbHadChanges = (cChanges > 0);
	
	wxString str_s2d_h( _s2d_h(this, kSync, pStrLabelA, pStrLabelB) );
	wxString str_s2d_f( _s2d_f(this, kSync, colTabWidth) );
	wxString str_s2d_array( _s2d_array(this, kSync) );

	strDiv = wxString::Format( gsz_s2d,
							   str_s2d_h.wc_str(),
							   str_s2d_array.wc_str(),
							   str_s2d_f.wc_str() );

	wxLogTrace(wxTRACE_Messages, _T("s2d:div: [%s]\n"), strDiv.wc_str());

	return strDiv;
}

/*static*/ wxString de_de::batchoutput_html_sxs_diff2__css(int colTabWidth)
{
	wxString strCSS(gsz_s2d_css);

	strCSS.Replace( TOK__CSS_TABSIZE,       wxString::Format("%d", colTabWidth), true);

	// TODO 2013/09/03 Think about using a global prop for the font sizes.
	strCSS.Replace( TOK__CSS_PATH_FONTSIZE, wxString::Format("%d", 10), true);
	strCSS.Replace( TOK__CSS_STAT_FONTSIZE, wxString::Format("%d", 8), true);
	strCSS.Replace( TOK__CSS_CODE_FONTSIZE, wxString::Format("%d", 10), true);

	wxLogTrace(wxTRACE_Messages, _T("s2d:css: [%s]\n"), strCSS.wc_str());

	return strCSS;
}

void de_de::batchoutput_html_sxs_diff2(long kSync,
									   wxString & strOut,
									   bool * pbHadChanges,
									   int colTabWidth,
									   wxString * pStrLabelA,
									   wxString * pStrLabelB)
{
	wxString str_css( batchoutput_html_sxs_diff2__css(colTabWidth) );
	wxString str_s2d( batchoutput_html_sxs_diff2__div(kSync, pbHadChanges, colTabWidth,
													  pStrLabelA, pStrLabelB) );
	
	strOut.Printf( gsz_s2, str_css, str_s2d );
}
