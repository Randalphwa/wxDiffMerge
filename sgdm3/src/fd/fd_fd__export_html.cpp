// fd_fd__export_html.cpp
//////////////////////////////////////////////////////////////////

#include <ConfigPch.h>
#include <ConfigDcl.h>
#include <util.h>
#include <fim.h>
#include <poi.h>
#include <fd.h>
#include <rs.h>

//////////////////////////////////////////////////////////////////

static const wchar_t * gsz_f2d_h_x = (		// part of unified header: path+date
	L"<tr class=\"f2d_h_x\">\n"
	L"  <td class=\"f2d_h_x_prefix\">%s</td>\n"
	L"  <td class=\"f2d_h_x_path\">%s</td>\n"
	L"  <td class=\"f2d_h_x_dtm\">%s</td>\n"
	L"</tr>\n"
	);

static const wchar_t * gsz_f2d_h_stats = (	// part of unified header: stats
	L"<tr class=\"f2d_h_stats\">\n"
	L"  <td colspan=\"3\">%s</td>\n"			// de_stats2
	L"</tr>\n"
	);

static const wchar_t * gsz_f2d_h = (			// overall unified header
	L"<table class=\"f2d_h\">\n"
	L"%s"			// left path/label
	L"%s"			// right path/label
	L"%s"			// stats in header
	L"</table>\n"
	);


static const wchar_t * gsz_f2d_f = (			// overall unified footer
	L"<table class=\"f2d_f\">\n"
	L"  <tr class=\"f2d_f_stats\">\n"
	L"    <td> %s </td>\n"						// display_ops
	L"  </tr>\n"
	L"</table>\n"
	);


static const wchar_t * gsz_f2d_x_eq = (
	L"<tr class=\"f2d_x_eq\">\n"
	L"  <td class=\"f2d_col_char f2d_x_eq_char\">=</td>\n"
	L"  <td class=\"f2d_col_path f2d_x_eq_path\">%s</td>\n"
	L"  <td class=\"f2d_col_size0 f2d_x_eq_size0\">%s</td>\n"
	L"  <td class=\"f2d_col_size1 f2d_x_eq_size1\">%s</td>\n"
	L"</tr>\n"
	);

static const wchar_t * gsz_f2d_x_equiv = (
	L"<tr class=\"f2d_x_equiv\">\n"
	L"  <td class=\"f2d_col_char f2d_x_equiv_char\">~</td>\n"
	L"  <td class=\"f2d_col_path f2d_x_equiv_path\">%s</td>\n"
	L"  <td class=\"f2d_col_size0 f2d_x_equiv_size0\">%s</td>\n"
	L"  <td class=\"f2d_col_size1 f2d_x_equiv_size1\">%s</td>\n"
	L"</tr>\n"
	);

static const wchar_t * gsz_f2d_x_neq = (
	L"<tr class=\"f2d_x_neq\">\n"
	L"  <td class=\"f2d_col_char f2d_x_neq_char\">!=</td>\n"
	L"  <td class=\"f2d_col_path f2d_x_neq_path\">%s</td>\n"
	L"  <td class=\"f2d_col_size0 f2d_x_neq_size0\">%s</td>\n"
	L"  <td class=\"f2d_col_size1 f2d_x_neq_size1\">%s</td>\n"
	L"</tr>\n"
	);

static const wchar_t * gsz_f2d_x_dir = (
	L"<tr class=\"f2d_x_dir\">\n"
	L"  <td class=\"f2d_col_char f2d_x_dir_char\">dir</td>\n"
	L"  <td class=\"f2d_col_path f2d_x_dir_path\">%s</td>\n"
	L"  <td class=\"f2d_col_size0 f2d_x_dir_size0\">%s</td>\n"
	L"  <td class=\"f2d_col_size1 f2d_x_dir_size1\">%s</td>\n"
	L"</tr>\n"
	);

static const wchar_t * gsz_f2d_x_del = (
	L"<tr class=\"f2d_x_del\">\n"
	L"  <td class=\"f2d_col_char f2d_x_del_char\">-</td>\n"
	L"  <td class=\"f2d_col_path f2d_x_del_path\">%s</td>\n"
	L"  <td class=\"f2d_col_size0 f2d_x_del_size0\">%s</td>\n"
	L"  <td class=\"f2d_col_size1 f2d_x_del_size1\">%s</td>\n"
	L"</tr>\n"
	);

static const wchar_t * gsz_f2d_x_add = (
	L"<tr class=\"f2d_x_add\">\n"
	L"  <td class=\"f2d_col_char f2d_x_add_char\">+</td>\n"
	L"  <td class=\"f2d_col_path f2d_x_add_path\">%s</td>\n"
	L"  <td class=\"f2d_col_size0 f2d_x_add_size0\">%s</td>\n"
	L"  <td class=\"f2d_col_size1 f2d_x_add_size1\">%s</td>\n"
	L"</tr>\n"
	);

static const wchar_t * gsz_f2d_x_err = (
	L"<tr class=\"f2d_x_err\">\n"
	L"  <td class=\"f2d_col_char f2d_x_err_char\">err</td>\n"
	L"  <td class=\"f2d_col_path f2d_x_err_path\">%s</td>\n"
	L"  <td class=\"f2d_col_size0 f2d_x_err_size0\">%s</td>\n"
	L"  <td class=\"f2d_col_size1 f2d_x_err_size1\">%s</td>\n"
	L"</tr>\n"
	);

static const wchar_t * gsz_f2d = (			// overall unified diff
	L"<div class=\"f2d\">\n"
	L"  <table class=\"f2d_t\">\n"
	L"    <thead class=\"f2d_thead\">\n"
	L"      <tr>\n"
	L"        <td class=\"f2d_thead_td\" colspan=\"4\">\n"
	L"%s"									// f2d_h
	L"        </td>\n"
	L"      </tr>\n"
	L"    </thead>\n"
	L"    <tbody class=\"f2d_tbody\">\n"
	L"%s"									// array of f2d_x
	L"    </tbody>\n"
	L"    <tfoot class=\"f2d_tfoot\">\n"
	L"      <tr>\n"
	L"        <td class=\"f2d_tfoot_td\" colspan=\"4\">\n"
	L"%s"									// f2d_f
	L"        </td>\n"
	L"      </tr>\n"
	L"    </tfoot>\n"
	L"  </table>\n"
	L"</div>\n"
	);

#define TOK__CSS_PATH_FONTSIZE	L"%PATHFONTSIZE%"
#define TOK__CSS_STAT_FONTSIZE	L"%STATFONTSIZE%"
#define TOK__CSS_CODE_FONTSIZE	L"%CODEFONTSIZE%"

static const wchar_t * gsz_f2d_css = (		// css info for f2d
	L"  .f2d_h { width: 100%; border-spacing: 0px; text-shadow: 0px 1px 0px rgb(255,255,255); background-color: rgb(234,234,234); background-image: linear-gradient(rgb(250,250,250), rgb(234,234,234)); border-bottom: 1px solid rgb(216,216,216); } \n"
	L"  .f2d_h_x { }\n"
	L"  .f2d_h_x_prefix { width: 1%; padding-right: 8px; font-weight: bold; font-size: "  TOK__CSS_PATH_FONTSIZE  L"pt; }\n"
	L"  .f2d_h_x_path {                                  font-weight: bold; font-size: "  TOK__CSS_PATH_FONTSIZE  L"pt; }\n"
	L"  .f2d_h_x_dtm { font-size: "  TOK__CSS_STAT_FONTSIZE  L"pt; text-align: right }\n"
	L"  .f2d_h_stats { font-size: "  TOK__CSS_STAT_FONTSIZE  L"pt; text-align: right }\n"

	L"  .f2d_f { width: 100%; border-spacing: 0px; text-shadow: 0px 1px 0px rgb(255,255,255); background-color: rgb(234,234,234); background-image: linear-gradient(rgb(250,250,250), rgb(234,234,234)); border-top: 1px solid rgb(216,216,216); }\n"
	L"  .f2d_f_stats { font-size: "  TOK__CSS_STAT_FONTSIZE  L"pt; text-align: right }\n"

	// I'm using an arbitrarily low pixel width on the various columns
	// to try to force them to be as small as possible (and the pathname
	// as large as possible).  I tried 1%, but that didn't work in Safari
	// and Chrome on Mac.  On Window IE10, the 1px caused "!=" to be split
	// until I also added white-space: pre (or nowrap).

	L"  .f2d_col_char  { white-space: nowrap;  width: 1px; padding-left: 8px; padding-right: 8px; text-align: right; vertical-align: text-top; }\n"
	L"  .f2d_col_path  { white-space: pre-wrap; }\n"
	L"  .f2d_col_size0 { width: 1px; padding-left: 8px; padding-right: 8px; text-align: right; vertical-align: text-top; }\n"
	L"  .f2d_col_size1 { width: 1px; padding-left: 8px; padding-right: 8px; text-align: right; vertical-align: text-top; }\n"

	L"  .f2d_x_eq       { }\n"
	L"  .f2d_x_eq_char  { border-right: 1px solid rgb(216,216,216); }\n"
	L"  .f2d_x_eq_path  { border-right: 1px solid rgb(216,216,216); }\n"
	L"  .f2d_x_eq_size0 { border-right: 1px solid rgb(216,216,216); }\n"
	L"  .f2d_x_eq_size1 { border-right: 1px solid rgb(216,216,216); }\n"

	L"  .f2d_x_equiv       { background-color: rgb(240,240,240); }\n"
	L"  .f2d_x_equiv_char  { border-right: 1px solid rgb(183,183,183); }\n"
	L"  .f2d_x_equiv_path  { border-right: 1px solid rgb(183,183,183); }\n"
	L"  .f2d_x_equiv_size0 { border-right: 1px solid rgb(183,183,183); }\n"
	L"  .f2d_x_equiv_size1 { border-right: 1px solid rgb(183,183,183); }\n"

	L"  .f2d_x_neq       { background-color: rgb(240,240,255); }\n"
	L"  .f2d_x_neq_char  { background-color: rgb(216,216,255); border-right: 1px solid rgb(183,183,216); }\n"
	L"  .f2d_x_neq_path  { border-right: 1px solid rgb(183,183,216); }\n"
	L"  .f2d_x_neq_size0 { border-right: 1px solid rgb(183,183,216); }\n"
	L"  .f2d_x_neq_size1 { border-right: 1px solid rgb(183,183,216); }\n"

	L"  .f2d_x_dir       { }\n"
	L"  .f2d_x_dir_char  { border-right: 1px solid rgb(216,216,216); }\n"
	L"  .f2d_x_dir_path  { border-right: 1px solid rgb(216,216,216); }\n"
	L"  .f2d_x_dir_size0 { border-right: 1px solid rgb(216,216,216); }\n"
	L"  .f2d_x_dir_size1 { border-right: 1px solid rgb(216,216,216); }\n"

	L"  .f2d_x_add       { background-color: rgb(190,233,190); }\n"
	L"  .f2d_x_add_char  { background-color: rgb(170,216,170); border-right: 1px solid rgb(161,204,161); }\n"
	L"  .f2d_x_add_path  { border-right: 1px solid rgb(161,204,161); }\n"
	L"  .f2d_x_add_size0 { border-right: 1px solid rgb(161,204,161); }\n"
	L"  .f2d_x_add_size1 { border-right: 1px solid rgb(161,204,161); }\n"

	L"  .f2d_x_del       { background-color: rgb(233,190,190); }\n"
	L"  .f2d_x_del_char  { background-color: rgb(216,170,170); border-right: 1px solid rgb(204,161,161); }\n"
	L"  .f2d_x_del_path  { border-right: 1px solid rgb(204,161,161); }\n"
	L"  .f2d_x_del_size0 { border-right: 1px solid rgb(204,161,161); }\n"
	L"  .f2d_x_del_size1 { border-right: 1px solid rgb(204,161,161); }\n"

	L"  .f2d_x_err       { background-color: rgb(255,255,208); }\n"
	L"  .f2d_x_err_char  { background-color: rgb(240,240,196); border-right: 1px solid rgb(226,226,185); }\n"
	L"  .f2d_x_err_path  { border-right: 1px solid rgb(226,226,185); }\n"
	L"  .f2d_x_err_size0 { border-right: 1px solid rgb(226,226,185); }\n"
	L"  .f2d_x_err_size1 { border-right: 1px solid rgb(226,226,185); }\n"

	L"  .f2d { }\n"
	L"  .f2d_t { width: 100%; border: 1px solid rgb(204,204,204); border-spacing: 0px; border-collapse: collapse; font-family: Courier New,monospace; font-size: "  TOK__CSS_CODE_FONTSIZE  L"pt; }\n"
	L"  .f2d_thead { }\n"
	L"  .f2d_thead_td { padding: 0px; }\n"
	L"  .f2d_tbody { }\n"
	L"  .f2d_tfoot { }\n"
	L"  .f2d_tfoot_td { padding: 0px; }\n"

	);

static const wchar_t * gsz_f2 = (
	L"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
	L"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
	L"  <head>\n"
	L"    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n"
	L"    <style type=\"text/css\">\n"
	L"%s"									// f2d_css
	L"    </style>\n"
	L"    <title>\n"
	L"      SourceGear DiffMerge Folder Comparison\n"
	L"    </title>\n"
	L"  </head>\n"
	L"  <body>\n"
	L"    <div>\n"
	L"%s"			// f2d containing complete unified diff, headers, and footers
	L"    </div>\n"
	L"  </body>\n"
	L"</html>\n"
	);

static const wchar_t * gasz[] = {
	gsz_f2d_x_err,		// FD_ITEM_STATUS_UNKNOWN=0,	// unknown, not yet computed
	gsz_f2d_x_err,		// FD_ITEM_STATUS_ERROR,		// error, see m_err
	gsz_f2d_x_err,		// FD_ITEM_STATUS_MISMATCH,		// mismatch
	gsz_f2d_x_err,		// FD_ITEM_STATUS_BOTH_NULL,	// neither set (probably error)

	gsz_f2d_x_eq,		// FD_ITEM_STATUS_SAME_FILE,	// they are the same (physical) file
	gsz_f2d_x_eq,		// FD_ITEM_STATUS_IDENTICAL,	// files are equal
	gsz_f2d_x_equiv,	// FD_ITEM_STATUS_EQUIVALENT,	// soft-match equivalent
	gsz_f2d_x_equiv,	// FD_ITEM_STATUS_QUICKMATCH,	// quick-match equivalent

	gsz_f2d_x_neq,		// FD_ITEM_STATUS_DIFFERENT,	// files are different

	gsz_f2d_x_dir,		// FD_ITEM_STATUS_FOLDERS,		// both are folders

	gsz_f2d_x_del,		// FD_ITEM_STATUS_FILE_NULL,	// left side only has file; right side null
	gsz_f2d_x_add,		// FD_ITEM_STATUS_NULL_FILE,	// left side null, right side has file

	gsz_f2d_x_del,		// FD_ITEM_STATUS_FOLDER_NULL,	// left side only has folder; right side null
	gsz_f2d_x_add,		// FD_ITEM_STATUS_NULL_FOLDER,	// left side null, right side has folder

	gsz_f2d_x_del,		// FD_ITEM_STATUS_SHORTCUT_NULL,	// left side only has .lnk ; right side null
	gsz_f2d_x_add,		// FD_ITEM_STATUS_NULL_SHORTCUT,	// left side null, right side has .lnk

	gsz_f2d_x_eq,		// FD_ITEM_STATUS_SHORTCUTS_SAME,	// the same file
	gsz_f2d_x_eq,		// FD_ITEM_STATUS_SHORTCUTS_EQ,		// both are .lnk shortcuts and are equal/equivalent
	gsz_f2d_x_neq,		// FD_ITEM_STATUS_SHORTCUTS_NEQ,	// both are .lnk shortcuts and are different

	gsz_f2d_x_del,		// FD_ITEM_STATUS_SYMLINK_NULL,		// left side only has symlink; right side null
	gsz_f2d_x_add,		// FD_ITEM_STATUS_NULL_SYMLINK,		// left side null, right side has symlink
	gsz_f2d_x_eq,		// FD_ITEM_STATUS_SYMLINKS_SAME,	// the same file
	gsz_f2d_x_eq,		// FD_ITEM_STATUS_SYMLINKS_EQ,		// both are symlinks and the text of their targets are identical
	gsz_f2d_x_neq,		// FD_ITEM_STATUS_SYMLINKS_NEQ,		// both are symlinks and are different
};

//////////////////////////////////////////////////////////////////

static wxString _esc(const wxString & strInput)
{
	wxString s(strInput);

	s.Replace(_T("&"), _T("&amp;"), true);
	s.Replace(_T("<"), _T("&lt;"),  true);
	s.Replace(_T(">"), _T("&gt;"),  true);

	return s;
}

//////////////////////////////////////////////////////////////////

static wxString _f2d_h_x(const wxChar * szPrefix,
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
	wxString strPathEsc = _esc(strPath);

	return wxString::Format( gsz_f2d_h_x,
							 szPrefix, strPathEsc.wc_str(), strDTM.wc_str() );
}

wxString fd_fd::_f2d_h_stats(void) const
{
	wxString strStats = formatStatsString();
	wxString strStatsEsc = _esc(strStats);

	return wxString::Format( gsz_f2d_h_stats, strStatsEsc.wc_str() );
}

wxString fd_fd::_f2d_h(wxString * pStrLabelA, wxString * pStrLabelB) const
{
	wxString str0( _f2d_h_x( L"---", getRootPoi(0), pStrLabelA) );
	wxString str1( _f2d_h_x( L"+++", getRootPoi(1), pStrLabelB) );
	wxString strS( _f2d_h_stats() );

	return wxString::Format( gsz_f2d_h,
							 str0.wc_str(), str1.wc_str(), strS.wc_str() );
}

wxString fd_fd::_f2d_f(void) const
{
	wxString strStats = formatStatsString();
	wxString strStatsEsc = _esc(strStats);

	return wxString::Format( gsz_f2d_f,
							 strStatsEsc );
}

//////////////////////////////////////////////////////////////////

wxString fd_fd::_f2d_array(void) const
{
	wxString strResult;		// to accumulate detail lines
	bool bShowSizes = true;	// TODO get from global props maybe

	for (TVecConstIterator it=m_vec.begin(); it!=m_vec.end(); it++)
	{
		fd_item * pFdItem = *it;

		poi_item * pPoiItem0 = pFdItem->getPoiItem(0);
		poi_item * pPoiItem1 = pFdItem->getPoiItem(1);

		wxString strName;
		if (pPoiItem0)
			strName = pFdItem->getRelativePathname(0);
		else if (pPoiItem1)
			strName = pFdItem->getRelativePathname(1);

		wxString strSize0;
		wxString strSize1;
		if (bShowSizes)
		{
			// get file size.  ignore directories as the result
			// doesn't make much sense (and is always -1 on windows).

			if (pPoiItem0 && (pPoiItem0->getPoiType() != POI_T_DIR))
				strSize0 = pPoiItem0->getFileSizeAsString();
			if (pPoiItem1 && (pPoiItem1->getPoiType() != POI_T_DIR))
				strSize1 = pPoiItem1->getFileSizeAsString();
		}

		int st = (int)pFdItem->getStatus();

		wxString strThisLine;
		strThisLine.Printf( gasz[st], strName, strSize0, strSize1 );

		strResult += strThisLine;
	}
	
	return strResult;
}

//////////////////////////////////////////////////////////////////
	
/*static*/ wxString batchoutput_html__css(void)
{
	wxString strCSS(gsz_f2d_css);

	// TODO 2013/09/03 Think about using a global prop for the font sizes.
	strCSS.Replace( TOK__CSS_PATH_FONTSIZE, wxString::Format("%d", 10), true);
	strCSS.Replace( TOK__CSS_STAT_FONTSIZE, wxString::Format("%d", 8), true);
	strCSS.Replace( TOK__CSS_CODE_FONTSIZE, wxString::Format("%d", 10), true);

	//wxLogTrace(wxTRACE_Messages, _T("f2d:css: [%s]\n"), strCSS.wc_str());

	return strCSS;
}

wxString fd_fd::batchoutput_html__div(wxString * pStrLabelA,
									  wxString * pStrLabelB) const
{
	wxString strDiv;

	wxString str_f2d_h( _f2d_h(pStrLabelA, pStrLabelB) );
	wxString str_f2d_f( _f2d_f() );
	wxString str_f2d_array( _f2d_array() );

	strDiv = wxString::Format( gsz_f2d,
							   str_f2d_h.wc_str(),
							   str_f2d_array.wc_str(),
							   str_f2d_f.wc_str() );

	//wxLogTrace(wxTRACE_Messages, _T("f2d:div: [%s]\n"), strDiv.wc_str());

	return strDiv;
}

void fd_fd::batchoutput_html(wxString & strOut,
							 wxString * pStrLabelA,
							 wxString * pStrLabelB) const
{
	wxString str_css( batchoutput_html__css() );
	wxString str_f2d( batchoutput_html__div(pStrLabelA, pStrLabelB) );
	
	strOut.Printf( gsz_f2, str_css, str_f2d );
}

//////////////////////////////////////////////////////////////////

/**
 * Generate an HTML view of the diffs between these two folders.
 * This should be pretty much stand-alone web content suitable
 * for view in a browser or sharing.  (So no external dependencies
 * like images or style sheets.)
 *
 * we respect the various show/hide bits currently set
 * in fd_fd::m_ShowHideFlags (and don't force a reload).
 *
 */
util_error fd_fd::_exportContents__html(wxString & strResult) const
{
	util_error ue;		// TODO decide if we need this

	// TODO 2013/09/10 Eventually I'd like to be able to use the
	// TODO            -title1=<x> -title2=<y> strings given on
	// TODO            the command line if this is the initial
	// TODO            window, but not today.

	batchoutput_html(strResult, NULL, NULL);

	return ue;
}

