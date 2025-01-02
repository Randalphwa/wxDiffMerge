## de.inc -- GNU Makefile for sgdm3/de for Linux
## file declarations for dependency checking by users of this library.
##################################################################

DE_HDRS=	../de/de.h \
		../de/de_css.h \
		../de/de_dcl.h \
		../de/de_de.h \
		../de/de_mark.h \
		../de/de_patch.h \
		../de/de_sync.h \
		../de/de_sync_line_omitted.h \
		../de/de_sync_list.h

DE_SRCS=	../de/de_css.cpp \
		../de/de_de.cpp \
		../de/de_de__batchoutput_html_sxs.cpp \
		../de/de_de__batchoutput_html_traditional.cpp \
		../de/de_de__batchoutput_html_unified.cpp \
		../de/de_de__batchoutput_text_traditional.cpp \
		../de/de_de__batchoutput_text_unified.cpp \
		../de/de_de__build_displaylist.cpp \
		../de/de_de__build_vecline.cpp \
		../de/de_de__fixup_nextprev.cpp \
		../de/de_de__insert_omitted.cpp \
		../de/de_de__intraline.cpp \
		../de/de_de__patch.cpp \
		../de/de_line.cpp \
		../de/de_mark.cpp \
		../de/de_sync.cpp \
		../de/de_sync_line_omitted.cpp \
		../de/de_sync_list.cpp

DE_LIB=		sgdm_de

##################################################################

MY_CXX_FLAGS+=	
MY_LD_LIBS+=	-l$(DE_LIB)
MY_OBJDEPS+=	../de/de.inc $(DE_HDRS)
MY_LIBDEPS+=	../de/de.inc
MY_EXEDEPS+=	../de/de.inc $(LIBDIR)/$(LIB_PFX)$(DE_LIB)$(LIB_SFX)
