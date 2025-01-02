## util.inc -- GNU Makefile for sgdm3/util for Linux
## file declarations for dependency checking by users of this library.
##################################################################

UTIL_HDRS=	../util/util.h \
		../util/util_background_thread_helper.h \
		../util/util_bitmap_button_validator.h \
		../util/util_cbl.h \
		../util/util_conditional_nonblank_text_validator.h \
		../util/util_convert.h \
		../util/util_dcl.h \
		../util/util_dont_show_again_msgbox.h \
		../util/util_enc.h \
		../util/util_error.h ../util/util_error__defs.h \
		../util/util_GlobalProps.h ../util/util_GlobalProps__defs.h \
		../util/util_help_dlg.h \
		../util/util_log.h \
		../util/util_nonblank_text_validator.h \
		../util/util_perf.h \
		../util/util_reftab.h \
		../util/util_regexp_text_validator.h \
		../util/util_shell_lnk.h \
		../util/util_treebook.h \
		../util/json_defs.h \
		../util/jsonval.h \
		../util/jsonreader.h \
		../util/jsonwriter.h

UTIL_SRCS=	../util/util_GlobalProps.cpp \
		../util/util_background_thread_helper.cpp \
		../util/util_bitmap_button_validator.cpp \
		../util/util_cbl.cpp \
		../util/util_conditional_nonblank_text_validator.cpp \
		../util/util_convert.cpp \
		../util/util_dont_show_again_msgbox.cpp \
		../util/util_enc.cpp \
		../util/util_error.cpp \
		../util/util_file.cpp \
		../util/util_help_dlg.cpp \
		../util/util_log.cpp \
		../util/util_misc.cpp \
		../util/util_nonblank_text_validator.cpp \
		../util/util_perf.cpp \
		../util/util_reftab.cpp \
		../util/util_regexp_text_validator.cpp \
		../util/util_shell_lnk.cpp \
		../util/util_string.cpp \
		../util/util_treebook.cpp \
		../util/jsonval.cpp \
		../util/jsonreader.cpp \
		../util/jsonwriter.cpp

ifeq ($(OS_NAME),Apple)
UTIL_HDRS+=
UTIL_SRCS+=	../util/util__mac__pasteboard.mm
endif
UTIL_LIB=	sgdm_util

##################################################################

MY_CXX_FLAGS+=	-I../util
MY_LD_LIBS+=	-l$(UTIL_LIB)
MY_OBJDEPS+=	../util/util.inc $(UTIL_HDRS)
MY_LIBDEPS+=	../util/util.inc
MY_EXEDEPS+=	../util/util.inc $(LIBDIR)/$(LIB_PFX)$(UTIL_LIB)$(LIB_SFX)
