// fd_dcl.h -- forward declarations for this directory
//////////////////////////////////////////////////////////////////

#ifndef H_FD_DCL_H
#define H_FD_DCL_H

//////////////////////////////////////////////////////////////////

typedef enum _SoftMatchMode { FD_SOFTMATCH_MODE_EXACT=0,	// must match astrSoftMatchMode[] in OptionsDlg.cpp
							  FD_SOFTMATCH_MODE_SIMPLE=1,
							  FD_SOFTMATCH_MODE_RULESET=2,
							  __FD_SOFTMATCH_MODE__NR__
} fd_SoftMatchMode;
	
//////////////////////////////////////////////////////////////////

class fd_fd;
class fd_fd_table;
class fd_filter;
class fd_item;
class fd_item_table;
class fd_quickmatch;
class fd_softmatch;

//////////////////////////////////////////////////////////////////

#endif//H_FD_DCL_H
