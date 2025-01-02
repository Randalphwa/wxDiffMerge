// de_de__batchoutput_html_unified.cpp -- diff engine -- parts of de_de related
// to generating batch output (such as gnu-diff-like output) but formatted
// in HTML rather than raw text.  This of this as something more similar
// to GITHUB's diff view rather than gnu-diff.
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

static const wchar_t * gsz_u2d_h_x = (		// part of unified header: path+date
	L"<tr class=\"u2d_h_x\">\n"
	L"  <td class=\"u2d_h_x_prefix\">%s</td>\n"
	L"  <td class=\"u2d_h_x_path\">%s</td>\n"
	L"  <td class=\"u2d_h_x_dtm\">%s</td>\n"
	L"</tr>\n"
	);

static const wchar_t * gsz_u2d_h_stats = (	// part of unified header: stats
	L"<tr class=\"u2d_h_stats\">\n"
	L"  <td colspan=\"3\">%s</td>\n"			// de_stats2
	L"</tr>\n"
	);

static const wchar_t * gsz_u2d_h = (			// overall unified header
	L"<table class=\"u2d_h\">\n"
	L"%s"			// left path/label
	L"%s"			// right path/label
	L"%s"			// stats in header
	L"</table>\n"
	);

static const wchar_t * gsz_u2d_f = (			// overall unified footer
	L"<table class=\"u2d_f\">\n"
	L"  <tr class=\"u2d_f_dops\">\n"
	L"    <td>%s</td>\n"						// display_ops
	L"  </tr>\n"
	L"</table>\n"
	);

static const wchar_t * gsz_u2d_x_atat = (		// a row of code in unified diff
	L"<tr class=\"u2d_x_atat\">\n"
	L"  <td class=\"u2d_ln u2d_x_atat_ln\">...</td>\n"			// left line number
	L"  <td class=\"u2d_ln u2d_x_atat_ln\">...</td>\n"			// right line number
	L"  <td class=\"u2d_x_atat_data\" colspan=\"2\">%s</td>\n"	// @@ data
	L"</tr>\n"
	);

static const wchar_t * gsz_u2d_x = (		// a row of code in unified diff
	L"<tr class=\"u2d_x_eq\">\n"
	L"  <td class=\"u2d_ln u2d_x_eq_ln\">%ld</td>\n"		// left line number
	L"  <td class=\"u2d_ln u2d_x_eq_ln\">%ld</td>\n"		// right line number
	L"  <td class=\"u2d_char\"></td>\n"						// column for unified '+' or '-'
	L"  <td class=\"u2d_code\">%s</td>\n"					// a line of code
	L"</tr>\n"
	);

static const wchar_t * gsz_u2d_x1[2] = {		// a row of code in unified diff
	(
		L"<tr class=\"u2d_x_del\">\n"
		L"  <td class=\"u2d_ln u2d_x_del_ln\">%ld</td>\n"		// left line number
		L"  <td class=\"u2d_ln u2d_x_del_ln\"></td>\n"
		L"  <td class=\"u2d_char\">-</td>\n"		// column for unified '+' or '-'
		L"  <td class=\"u2d_code\">%s</td>\n"		// a line of code
		L"</tr>\n"
		),
	(
		L"<tr class=\"u2d_x_add\">\n"
		L"  <td class=\"u2d_ln u2d_x_add_ln\"></td>\n"		// left line number
		L"  <td class=\"u2d_ln u2d_x_add_ln\">%ld</td>\n"		// right line number
		L"  <td class=\"u2d_char\">+</td>\n"		// column for unified '+' or '-'
		L"  <td class=\"u2d_code\">%s</td>\n"		// a line of code
		L"</tr>\n"
		)
};

static const wchar_t * gsz_u2d_x1_unimp_il[2] = {			// intraline highlight
	(L"<span class=\"u2d_x_unimp_il\">%s</span>"),	// DO NOT ADD TRAILING \n
	(L"<span class=\"u2d_x_unimp_il\">%s</span>")	// DO NOT ADD TRAILING \n
};
	
static const wchar_t * gsz_u2d_x1_il[2] = {			// intraline highlight
	(L"<span class=\"u2d_x_del_il\">%s</span>"),		// DO NOT ADD TRAILING \n
	(L"<span class=\"u2d_x_add_il\">%s</span>")		// DO NOT ADD TRAILING \n
};

static const wchar_t * gsz_u2d_x_eq_unimp_il = 		// intraline highlight
	(L"<span class=\"u2d_x_unimp_il\">%s</span>");		// DO NOT ADD TRAILING \n

	
static const wchar_t * gsz_u2d_x10_omitted = 		// an omitted line 
	(
		L"<tr class=\"u2d_x_omit\">\n"
		L"  <td class=\"u2d_ln u2d_x_omit_ln\">%ld</td>\n"		// left line number
		L"  <td class=\"u2d_ln u2d_x_omit_ln\"></td>\n"
		L"  <td class=\"u2d_char\">x</td>\n"		// column for unified '+' or '-'
		L"  <td class=\"u2d_code\">%s</td>\n"		// a line of code
		L"</tr>\n"
		);
static const wchar_t * gsz_u2d_x01_omitted = 		// an omitted line 
	(
		L"<tr class=\"u2d_x_omit\">\n"
		L"  <td class=\"u2d_ln u2d_x_omit_ln\"></td>\n"		// left line number
		L"  <td class=\"u2d_ln u2d_x_omit_ln\">%ld</td>\n"
		L"  <td class=\"u2d_char\">x</td>\n"		// column for unified '+' or '-'
		L"  <td class=\"u2d_code\">%s</td>\n"		// a line of code
		L"</tr>\n"
		);
static const wchar_t * gsz_u2d_x11_omitted = 		// an omitted line 
	(
		L"<tr class=\"u2d_x_omit\">\n"
		L"  <td class=\"u2d_ln u2d_x_omit_ln\">%ld</td>\n"		// left line number
		L"  <td class=\"u2d_ln u2d_x_omit_ln\">%ld</td>\n"
		L"  <td class=\"u2d_char\">x</td>\n"		// column for unified '+' or '-'
		L"  <td class=\"u2d_code\">%s</td>\n"		// a line of code
		L"</tr>\n"
		);

static const wchar_t * gsz_u2d_x1_unimp[2] = { 		// an unimportant line 
	(
		L"<tr class=\"u2d_x_unimp\">\n"
		L"  <td class=\"u2d_ln u2d_x_unimp_ln\">%ld</td>\n"		// left line number
		L"  <td class=\"u2d_ln u2d_x_unimp_ln\"></td>\n"
		L"  <td class=\"u2d_char\">u</td>\n"		// column for unified '+' or '-'
		L"  <td class=\"u2d_code\">%s</td>\n"		// a line of code
		L"</tr>\n"
		),
	(
		L"<tr class=\"u2d_x_unimp\">\n"
		L"  <td class=\"u2d_ln u2d_x_unimp_ln\"></td>\n"		// left line number
		L"  <td class=\"u2d_ln u2d_x_unimp_ln\">%ld</td>\n"
		L"  <td class=\"u2d_char\">u</td>\n"		// column for unified '+' or '-'
		L"  <td class=\"u2d_code\">%s</td>\n"		// a line of code
		L"</tr>\n"
		)
};

static const wchar_t * gsz_u2d = (			// overall unified diff
	L"<div class=\"u2d\">\n"
	L"  <table class=\"u2d_t\">\n"
	L"    <thead class=\"u2d_thead\">\n"
	L"      <tr>\n"
	L"        <td class=\"u2d_thead_td\" colspan=\"4\">\n"
	L"%s"									// u2d_h
	L"        </td>\n"
	L"      </tr>\n"
	L"    </thead>\n"
	L"    <tbody class=\"u2d_tbody\">\n"
	L"%s"									// array of u2d_x
	L"    </tbody>\n"
	L"    <tfoot class=\"u2d_tfoot\">\n"
	L"      <tr>\n"
	L"        <td class=\"u2d_tfoot_td\" colspan=\"4\">\n"
	L"%s"									// u2d_f
	L"        </td>\n"
	L"      </tr>\n"
	L"    </tfoot>\n"
	L"  </table>\n"
	L"</div>\n"
	);

// css-3 is in the process of defining a tab-size: property
// http://www.w3.org/TR/css3-text/#tab-size1
// which i'm going to try to use here.  so it may not work
// on all browsers.

#define TOK__CSS_TABSIZE		L"%TABSIZE%"
#define TOK__CSS_PATH_FONTSIZE	L"%PATHFONTSIZE%"
#define TOK__CSS_STAT_FONTSIZE	L"%STATFONTSIZE%"
#define TOK__CSS_CODE_FONTSIZE	L"%CODEFONTSIZE%"

static const wchar_t * gsz_u2d_css = (		// css info for u2d
	L"  .u2d_h { width: 100%; border-spacing: 0px; text-shadow: 0px 1px 0px rgb(255,255,255); background-color: rgb(234,234,234); background-image: linear-gradient(rgb(250,250,250), rgb(234,234,234)); border-bottom: 1px solid rgb(216,216,216); } \n"
	L"  .u2d_h_x { }\n"
	L"  .u2d_h_x_prefix { width: 1px; padding-right: 8px; font-weight: bold; font-size: "  TOK__CSS_PATH_FONTSIZE  L"pt; }\n"
	L"  .u2d_h_x_path { font-weight: bold; font-size: "  TOK__CSS_PATH_FONTSIZE  L"pt; }\n"
	L"  .u2d_h_x_dtm { font-size: "  TOK__CSS_STAT_FONTSIZE  L"pt; text-align: right }\n"
	L"  .u2d_h_stats { font-size: "  TOK__CSS_STAT_FONTSIZE  L"pt; text-align: right }\n"

	L"  .u2d_f { width: 100%; border-spacing: 0px; text-shadow: 0px 1px 0px rgb(255,255,255); background-color: rgb(234,234,234); background-image: linear-gradient(rgb(250,250,250), rgb(234,234,234)); border-top: 1px solid rgb(216,216,216); }\n"
	L"  .u2d_f_dops { font-size: "  TOK__CSS_STAT_FONTSIZE  L"pt; text-align: right }\n"

	L"  .u2d_ln   { width: 1px; padding-left: 8px; padding-right: 8px; text-align: right; vertical-align: text-top; }\n"
	L"  .u2d_char { width: 1px; vertical-align: text-top; }\n"
	L"  .u2d_code { vertical-align: text-top;\n"
	L"              white-space: pre-wrap;\n"
	L"              tab-size:      "  TOK__CSS_TABSIZE L";\n"
	L"              -moz-tab-size: "  TOK__CSS_TABSIZE L";\n"
	L"              -o-tab-size:   "  TOK__CSS_TABSIZE L";\n"
	L"            }\n"


	L"  .u2d_x_atat      { background-color: rgb(240,240,255);   border-top: 1px solid rgb(183,183,216); color: rgb(168,168,168); }\n"
	L"  .u2d_x_atat_ln   { background-color: rgb(216,216,255); border-right: 1px solid rgb(183,183,216); text-align: center; }\n"
	L"  .u2d_x_atat_data { }\n"

	L"  .u2d_x_eq      { }\n"
	L"  .u2d_x_eq_ln   { border-right: 1px solid rgb(216,216,216); }\n"

	L"  .u2d_x_add      { background-color: rgb(190,233,190); }\n"
	L"  .u2d_x_add_ln   { background-color: rgb(170,216,170); border-right: 1px solid rgb(161,204,161); }\n"

	L"  .u2d_x_del      { background-color: rgb(233,190,190); }\n"
	L"  .u2d_x_del_ln   { background-color: rgb(216,170,170); border-right: 1px solid rgb(204,161,161); }\n"

	L"  .u2d_x_omit      { background-color: rgb(240,240,240); color: rgb(128,128,128); }\n"
	L"  .u2d_x_omit_ln   { background-color: rgb(216,216,216); border-right: 1px solid rgb(183,183,183); }\n"

	L"  .u2d_x_unimp      { background-color: rgb(240,240,240); color: rgb(128,128,128); }\n"
	L"  .u2d_x_unimp_ln   { background-color: rgb(216,216,216); border-right: 1px solid rgb(183,183,183); }\n"

	L"  .u2d_x_del_il   { background-color: rgb(255,170,170); font-weight: bold; }\n"
	L"  .u2d_x_add_il   { background-color: rgb(170,255,170); font-weight: bold; }\n"
	L"  .u2d_x_unimp_il { color: rgb(150,150,150); }\n"

	L"  .u2d { }\n"
	L"  .u2d_t { width: 100%; border: 1px solid rgb(204,204,204); border-spacing: 0px; border-collapse: collapse; font-family: Courier New,monospace; font-size: "  TOK__CSS_CODE_FONTSIZE  L"pt; }\n"
	L"  .u2d_thead { }\n"
	L"  .u2d_thead_td { padding: 0px; }\n"
	L"  .u2d_tbody { }\n"
	L"  .u2d_tfoot { }\n"
	L"  .u2d_tfoot_td { padding: 0px; }\n"

	);

static const wchar_t * gsz_u2 = (
	L"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
	L"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
	L"  <head>\n"
	L"    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n"
	L"    <style type=\"text/css\">\n"
	L"%s"									// u2d_css
	L"    </style>\n"
	L"    <title>\n"
	L"      SourceGear DiffMerge Unified Diff\n"
	L"    </title>\n"
	L"  </head>\n"
	L"  <body>\n"
	L"    <div>\n"
	L"%s"			// u2d containing complete unified diff, headers, and footers
	L"    </div>\n"
	L"  </body>\n"
	L"</html>\n"
	);

//////////////////////////////////////////////////////////////////

static wxString _u2d_h_x(const wxChar * szPrefix,
					   poi_item * pPoiItem,
					   wxString * pStrLabel=NULL)
{
	// NOTE we assume that there are no '<', '>', or '&' in the
	// given prefix and in the generated DTM strings.

	wxDateTime dtm;
	pPoiItem->getDateTimeModified(&dtm);
	wxString strDTM = util_string__format_unified_date_from_dtm(dtm, false, false);

	wxString strPath;
	if (pStrLabel)
		strPath = *pStrLabel;
	else
		strPath = pPoiItem->getFullPath();
	wxString strPathEsc = html_diff_escape_string(strPath);

	return wxString::Format( gsz_u2d_h_x,
							 szPrefix, strPathEsc.wc_str(), strDTM.wc_str() );
}

static wxString _u2d_h_stats(const de_stats2 * pStats)
{
	wxString strStats = pStats->formatStatsString();
	wxString strStatsEsc = html_diff_escape_string(strStats);

	return wxString::Format( gsz_u2d_h_stats, strStatsEsc.wc_str() );

}

static wxString _u2d_h(const de_de * pDeDe, long kSync,
					 wxString * pStrLabelA, wxString * pStrLabelB)
{
	wxString str0( _u2d_h_x( L"---", pDeDe->getFsFs()->getPoi(kSync,PANEL_T0), pStrLabelA) );
	wxString str1( _u2d_h_x( L"+++", pDeDe->getFsFs()->getPoi(kSync,PANEL_T1), pStrLabelB) );
	wxString strS( _u2d_h_stats( pDeDe->getStats2(kSync)) );

	return wxString::Format( gsz_u2d_h,
							 str0.wc_str(), str1.wc_str(), strS.wc_str() );
}

static wxString _u2d_f(const de_de * pDeDe, long kSync, int colTabWidth)
{
	wxString strModeString = pDeDe->getDisplayModeString(kSync, colTabWidth);
	wxString strModeStringEsc = html_diff_escape_string(strModeString);

	return wxString::Format( gsz_u2d_f,
							 strModeStringEsc );
}


static wxString _u2d_x_content(const de_row & rDeRowK, PanelIndex kPanel,
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
						szTemplate = gsz_u2d_x1_unimp_il[kPanel];
					else
						szTemplate = gsz_u2d_x1_il[kPanel];

					wxString strThisSpan;
					strThisSpan.Printf( szTemplate, strContentEsc );
					strThisLineEsc += strThisSpan;
				}
				else if (DE_ATTR__IS_TYPE(attr_il, DE_ATTR_DIF_2EQ))
				{
					if (bIgnoreUnimportant && DE_ATTR__IS_SET(attr_il, DE_ATTR_UNIMPORTANT))
					{
						szTemplate = gsz_u2d_x_eq_unimp_il;
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

static wxString _u2d_array(de_de * pDeDe, long kSync)
{
	wxString strResult;		// to accumulate detail lines

	de_display_ops dops = pDeDe->getDisplayOps(kSync);
	bool bIgnoreUnimportant = DE_DOP__IS_SET_IGN_UNIMPORTANT(dops);
	bool bGlobalRespectEOL = RS_ATTRS_RespectEOL( pDeDe->getFsFs()->getRuleSet()->getMatchStripAttrs() );

	TVector_Display * pDis = pDeDe->getDisplayList(kSync);	// forces run() if necessary
	long nrRows = (long)pDis->size() - 1;
	long kRowStart = 0;
	while (kRowStart < nrRows)
	{
		long kRowEnd = -1;
		if (!unified_diff__compute_row_end_of_chunk(pDis, nrRows, kRowStart, &kRowEnd))
			break;
			
		struct _unified_diff__line_nr_mapping lnm;
		unified_diff__map_rows_to_line_numbers(pDeDe, pDis, kSync, kRowStart, kRowEnd, &lnm);

		//////////////////////////////////////////////////////////////////
		// write hunk header -- we always write length [startLine,nrLines "%d,%d"]
		// rather than just the start when the length is 1.

		wxString strAtAt;
		strAtAt.Printf( _T("@@ -%ld,%ld +%ld,%ld @@\n"),
						lnm.lineNrFirst[0], lnm.len[0],
						lnm.lineNrFirst[1], lnm.len[1] );
		wxString strAtAtDetail;
		strAtAtDetail.Printf( gsz_u2d_x_atat, strAtAt );
		strResult += strAtAtDetail;
		
		//////////////////////////////////////////////////////////////////
		// write diffs
		//
		// because we have context, we have EQ sync nodes and NEQ sync nodes.
		// as we walk the display list rows, we output the EQ ones as soon as
		// we see them.  we accumulate the NEQ ones until we reach either an EQ
		// node or the end of the chunk -- so that we can display all of the
		// deletions and then all the additions -- rather than interleave them
		// one by one.

		wxString strLines[2];
		long lineNr[2];
		lineNr[0] = lnm.lineNrFirst[0];
		lineNr[1] = lnm.lineNrFirst[1];

		for (long kRow=kRowStart; (kRow<kRowEnd); kRow++)
		{
			const de_row & rDeRowK = (*pDis)[kRow];
			const de_sync * pSyncK = rDeRowK.getSync();

//			wxLogTrace(wxTRACE_Messages, _T("u2d:array: [kRow %d][syncType %04x][unimp %d][line %d %d]"),
//					   (int)kRow, (int)pSyncK->getAttr(), (int)pSyncK->isUnimportant(),
//					   (int)lineNr[0], (int)lineNr[1]);

			bool bTreatAsUnimportant = (bIgnoreUnimportant && pSyncK->isUnimportant());

			if (pSyncK->isSameType(DE_ATTR_DIF_2EQ)
				|| pSyncK->isSameType(DE_ATTR_OMITTED)
				|| bTreatAsUnimportant)
			{
				// 2-way equal or omitted or an ignored unimportant change.
				// 
				// when we have an omitted (from analysis) line, we
				// have to draw these in a unified diff so that the
				// range numbers in the @@ header is well-defined.
				//
				// flush any cached lines.

				if (strLines[0].Length() > 0)
				{
					strResult += strLines[0];
					strLines[0].Empty();
				}
				if (strLines[1].Length() > 0)
				{
					strResult += strLines[1];
					strLines[1].Empty();
				}
			}

			if (pSyncK->isSameType(DE_ATTR_DIF_2EQ))
			{
				// 2-way equal.  write this line immediately.
				// since they are equal, no intra-line highlight
				// should be necessary, but we still try so that
				// unimportant contexts are grayed.
				//
				// choose content from either panel since they
				// are the same.
					
				const de_line * pDeLine = rDeRowK.getPanelLine(PANEL_T0);
				wxASSERT_MSG( (pDeLine), _T("Coding Error") );

				wxString strContentEsc( _u2d_x_content(rDeRowK, PANEL_T0,
													   bIgnoreUnimportant, bGlobalRespectEOL) );

				wxString strThisLineDetail;
				strThisLineDetail.Printf( gsz_u2d_x, lineNr[0], lineNr[1], strContentEsc );
				strResult += strThisLineDetail;

				// TODO 2013/09/03 think about whether to do something with this
				// if ((strThisEol.Length() == 0) || (!strThisEol.EndsWith(_T("\n"))))
				//     strOut += _T("\n\\ No newline at end of file\n");

				lineNr[0]++;
				lineNr[1]++;
			}
			else if (bTreatAsUnimportant)
			{
				// an unimportant change that is adjacent to an important change.
				// normally we would exclude an isolated unimportant change (because
				// we're only showing context diffs here), but when an unimportant
				// change is adjacent to an important change, we need to draw part
				// of it as the "context" to keep the range numbers in the @@ header
				// in sync.
				//
				// the question is, should we display both sides or just pick one.
				// they've already said they don't care about it.  for example, if
				// it is an indentation change in the code, showing both sides makes
				// as much noise as just showing the '-' and '+' lines for important
				// changes.
				//
				// for now, i think i'll duplicate what i did for omitted lines.
				// this avoids the ambiguity in the display and shouldn't cause
				// too much noise since we're only talking about adjacent changes.
				//
				// I'm not going to bother with intra-line highlight on these lines
				// because the whole line is unimportant and already gray.

				for (int kPanel=PANEL_T0; (kPanel<PANEL_T2); kPanel++)
				{
					const de_line * pDeLine = rDeRowK.getPanelLine((PanelIndex)kPanel);
					if (pDeLine)
					{
						wxString strContentEsc( _u2d_x_content(rDeRowK, (PanelIndex)kPanel,
															   bIgnoreUnimportant, bGlobalRespectEOL) );

						wxString strThisLineDetail;
						strThisLineDetail.Printf( gsz_u2d_x1_unimp[kPanel],
												  lineNr[kPanel], strContentEsc );

						strResult += strThisLineDetail;

						// TODO 2013/09/03 think about whether to do something with this
						// if ((strThisEol.Length() == 0) || (!strThisEol.EndsWith(_T("\n"))))
						//     strLines[kPanel] += _T("\n\\ No newline at end of file\n");

						lineNr[kPanel]++;
					}
				}
			}
			else if (pSyncK->isSameType(DE_ATTR_DIF_0EQ))
			{
				// accumulate changed lines as 2 sets of "+" and "-" lines
				// in the line cache. so that we can cluster them.

				for (int kPanel=PANEL_T0; (kPanel<PANEL_T2); kPanel++)
				{
					const de_line * pDeLine = rDeRowK.getPanelLine((PanelIndex)kPanel);
					if (pDeLine)
					{
						wxString strContentEsc( _u2d_x_content(rDeRowK, (PanelIndex)kPanel,
															   bIgnoreUnimportant, bGlobalRespectEOL) );
						
						wxString strThisLineDetail;
						strThisLineDetail.Printf( gsz_u2d_x1[kPanel],
												  lineNr[kPanel], strContentEsc );
						strLines[kPanel] += strThisLineDetail;

						// TODO 2013/09/03 think about whether to do something with this
						// if ((strThisEol.Length() == 0) || (!strThisEol.EndsWith(_T("\n"))))
						//     strLines[kPanel] += _T("\n\\ No newline at end of file\n");

						lineNr[kPanel]++;
					}
				}
			}
			else if (pSyncK->isSameType(DE_ATTR_OMITTED))
			{
				// write this line immediately.
				// no need for intra-line highlight here since we didn't
				// actually use the line in the analysis.

				wxString strContentEsc[2];
				bool bPresent[2];
					
				for (int kPanel=PANEL_T0; (kPanel<PANEL_T2); kPanel++)
				{
					const de_line * pDeLine = rDeRowK.getPanelLine((PanelIndex)kPanel);
					if (pDeLine)
					{
						bPresent[kPanel] = true;
						strContentEsc[kPanel] = _u2d_x_content(rDeRowK, (PanelIndex)kPanel,
															   bIgnoreUnimportant, bGlobalRespectEOL);

						// TODO 2013/09/03 think about whether to do something with this
						// if ((strThisEol.Length() == 0) || (!strThisEol.EndsWith(_T("\n"))))
						//     strLines[kPanel] += _T("\n\\ No newline at end of file\n");

					}
					else
					{
						bPresent[kPanel] = false;
					}
				}

				if (bPresent[0] && bPresent[1] && strContentEsc[0].IsSameAs(strContentEsc[1]))
				{
					// if both sides have an omitted line and they are equal,
					// just write a single line in the output with both line numbers.
					wxString strThisLineDetail;
					strThisLineDetail.Printf( gsz_u2d_x11_omitted,
											  lineNr[0], lineNr[1], strContentEsc[0] );
					strResult += strThisLineDetail;

					lineNr[0]++;
					lineNr[1]++;
				}
				else
				{
					// one side only or both sides are different.
					// write '-' then '+' rows like we do for changes.
					if (bPresent[0])
					{
						wxString strThisLineDetail;
						strThisLineDetail.Printf( gsz_u2d_x10_omitted,
												  lineNr[0], strContentEsc[0] );
						strResult += strThisLineDetail;

						lineNr[0]++;
					}
					if (bPresent[1])
					{
						wxString strThisLineDetail;
						strThisLineDetail.Printf( gsz_u2d_x01_omitted,
												  lineNr[1], strContentEsc[1] );
						strResult += strThisLineDetail;

						lineNr[1]++;
					}
				}
			}
			else
			{
				wxASSERT_MSG( (0), _T("Coding Error") );
			}
		}

		// reached end of chunk, see if we have any cached lines (possible when
		// no EQ context following NEQ lines.
			
		if (strLines[0].Length() > 0)
		{
			strResult += strLines[0];
			strLines[0].Empty();
		}
		if (strLines[1].Length() > 0)
		{
			strResult += strLines[1];
			strLines[1].Empty();
		}

		// set up for start of next chunk

		kRowStart = kRowEnd;
	}

	return strResult;
}

wxString de_de::batchoutput_html_unified_diff2__div(long kSync,
													bool * pbHadChanges,
													int colTabWidth,
													wxString * pStrLabelA,
													wxString * pStrLabelB)
{
	wxString strDiv;

	de_display_ops dopsOld = m_dops[kSync];

	// unified diffs are only really defined as context diff.
	// force CTX mode and rebuild the display list if necessary.
	// we pick up the ignore-unimportant.
	// We don't bother with the hide-omitted, since they won't
	// show up in a context diff anyway.
	de_display_ops dopsNew = DE_DOP_CTX;
	if (DE_DOP__IS_SET_IGN_UNIMPORTANT(dopsOld))
		dopsNew |= DE_DOP_IGN_UNIMPORTANT;
	setDisplayOps(kSync, dopsNew);
	run();
	
	const de_stats2 * pStats = getStats2(kSync);
	long cChanges = pStats->m_cImportantChanges;	// + pStats->m_cUnimportantChanges;

	*pbHadChanges = (cChanges > 0);
	
	wxString str_u2d_h( _u2d_h(this, kSync, pStrLabelA, pStrLabelB) );
	wxString str_u2d_f( _u2d_f(this, kSync, colTabWidth) );
	wxString str_u2d_array( _u2d_array(this, kSync) );

	strDiv = wxString::Format( gsz_u2d,
							   str_u2d_h.wc_str(),
							   str_u2d_array.wc_str(),
							   str_u2d_f.wc_str() );

	wxLogTrace(wxTRACE_Messages, _T("u2d:div: [%s]\n"), strDiv.wc_str());

	// Restore users display options.  Really only needed if interactive.
	setDisplayOps(kSync, dopsOld);
	run();

	return strDiv;
}

/*static*/ wxString de_de::batchoutput_html_unified_diff2__css(int colTabWidth)
{
	wxString strCSS(gsz_u2d_css);

	strCSS.Replace( TOK__CSS_TABSIZE,       wxString::Format("%d", colTabWidth), true);

	// TODO 2013/09/03 Think about using a global prop for the font sizes.
	strCSS.Replace( TOK__CSS_PATH_FONTSIZE, wxString::Format("%d", 10), true);
	strCSS.Replace( TOK__CSS_STAT_FONTSIZE, wxString::Format("%d", 8), true);
	strCSS.Replace( TOK__CSS_CODE_FONTSIZE, wxString::Format("%d", 10), true);

	wxLogTrace(wxTRACE_Messages, _T("u2d:css: [%s]\n"), strCSS.wc_str());

	return strCSS;
}

void de_de::batchoutput_html_unified_diff2(long kSync,
										   wxString & strOut,
										   bool * pbHadChanges,
										   int colTabWidth,
										   wxString * pStrLabelA,
										   wxString * pStrLabelB)
{
	wxString str_css( batchoutput_html_unified_diff2__css(colTabWidth) );
	wxString str_u2d( batchoutput_html_unified_diff2__div(kSync, pbHadChanges, colTabWidth,
														  pStrLabelA, pStrLabelB) );
	
	strOut.Printf( gsz_u2, str_css, str_u2d );
}
