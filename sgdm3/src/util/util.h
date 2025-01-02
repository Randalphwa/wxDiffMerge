// util.h
//////////////////////////////////////////////////////////////////

#ifndef H_UTIL_H
#define H_UTIL_H

//////////////////////////////////////////////////////////////////

#include <util_bitmap_button_validator.h>
#include <util_cbl.h>
#include <util_conditional_nonblank_text_validator.h>
#include <util_dont_show_again_msgbox.h>
#include <util_enc.h>
#include <util_error.h>
#include <util_help_dlg.h>
#include <util_log.h>
#include <util_GlobalProps.h>
#include <util_nonblank_text_validator.h>
#include <util_reftab.h>
#include <util_regexp_text_validator.h>
#include <util_treebook.h>
#include <util_perf.h>
#include <util_background_thread_helper.h>
#include <util_convert.h>
#include <jsonval.h>
#include <jsonreader.h>
#include <jsonwriter.h>

#if defined(__WXMSW__)
#include <util_shell_lnk.h>
#endif

//////////////////////////////////////////////////////////////////

#define TRACE_UTIL_DUMP			wxT("util_dump")
#define TRACE_GLOBALPROPS		wxT("global_props")
#define TRACE_IDLE_PROC			wxT("idle_proc")
#define TRACE_PERF				wxT("perf")
#define TRACE_UTIL_ENC			wxT("util_enc")
#define TRACE_BATCH				wxT("batch")
#define TRACE_NETWORK			wxT("network")
#define TRACE_LNK				wxT("lnk")

//////////////////////////////////////////////////////////////////

extern GlobalProps *	gpGlobalProps;

//////////////////////////////////////////////////////////////////

// this enum will allow different types of URLs to be generated
typedef enum
{
	T__URL__LICENSE,
	T__URL__UPDATE,
	T__URL__VISIT,
	T__URL__WEBHELP
} TYPE_URL;


//////////////////////////////////////////////////////////////////

int util_string_find_last(const wxString & strBuffer, int limit, const wxChar * szPattern);

wxString util_printable_s(const wxString & s);
wxString util_printable_cl(const wxChar * pBuf, size_t len);
void util_print_buffer_cl(const wxChar * szTrace, const wxChar * pBuf, size_t len);

void util_parse_font_spec(wxString strInputSpec, long * pRequestedPointSize, long * pRequestedFamily, wxString * pRequestedFaceName);

wxFont * util_font_create_normal_font(long requestedPointSize, long requestedFamily, wxString requestedFaceName);
wxFont * util_font_create_bold_font(long requestedPointSize, long requestedFamily, wxString requestedFaceName);
wxFont * util_font_create_font(long requestedPointSize, long requestedFamily, wxString requestedFaceName, wxFontWeight requestedFontWeight);

wxFont * util_font_create_normal_font_from_spec(const wxString strInputSpec);
wxFont * util_font_create_bold_font_from_spec(const wxString strInputSpec);

wxString util_font_create_spec_from_font(const wxFont * pFont);

int util_file_open(const wxChar *szFileName, wxFile::OpenMode mode = wxFile::read, int access = wxS_DEFAULT);
int util_file_open(util_error & err, const wxChar *szFileName, wxFile::OpenMode mode = wxFile::read, int accessMode = wxS_DEFAULT);

#if defined(__WXMSW__)
util_error util_file__is_shell_lnk(const wxString & pathname,
								   bool * pbIsLnk);
#endif

#if defined(__WXMAC__)
wxString	util__mac__get_system_search_text(void);
void		util__mac__set_system_search_text(const wxString & str);
#endif

wxString util_string__format_unified_date_from_dtm(const wxDateTime & dtm,
												   bool bIncludeNanoseconds=true,
												   bool bIncludeTZ=true);

bool util_string__parse_hex_char(const char c,
								 unsigned int * pv);

wxString util_my_branch(void);
wxString util_my_package(void);
wxString util_my_version(void);
wxString util_my_dpf_stats(void);
wxString util_create_query_url(const wxString & strUrlPath);
wxString util_make_url(TYPE_URL t, bool bSendQueryFields);

wxString util_create_temp_dir(const wxString& prefix);

wxLongLong_t util_copy_stream(wxInputStream &fsIn, wxOutputStream &fsOut, wxLongLong_t bytesToCopy /*= -1*/);

bool util_is_vault_installed(void);
bool util_have_license(bool bExplicitLicenseOnly = false);

wxUint32 util_calc_crc32(const wxByte* pBuf, size_t len);

//////////////////////////////////////////////////////////////////

#endif//H_UTIL_H
