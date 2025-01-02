// util_dcl.h
//////////////////////////////////////////////////////////////////

#ifndef H_UTIL_DCL_H
#define H_UTIL_DCL_H

//////////////////////////////////////////////////////////////////

class util_cbl;
class util_enc;
class util_error;
class util_help_dlg;
class util_logToString;
class util_GlobalProps;
class util_reftab;
	// TODO we can remove this #ifdef once we get the build system updated to 2.8.
#if wxCHECK_VERSION(2,8,0)
#else
class util_treebook;
#endif

#if defined(__WXMSW__)
class util_shell_lnk;
#endif

class BitmapButtonValidator;
class ConditionalNonBlankTextValidator;
class NonBlankTextValidator;
class RegExpTextValidator;

//////////////////////////////////////////////////////////////////

#if 0 && defined(DEBUG) && (defined(__WXMSW__) || defined(__WXMAC__))
class util_perf_item;
class util_perf;
#	define DEBUGUTILPERF
#endif

//////////////////////////////////////////////////////////////////

#endif//H_UTIL_DCL_H
