## fd.inc -- GNU Makefile for sgdm3/fd for Linux
## file declarations for dependency checking by users of this library.
##################################################################

FD_HDRS=	../fd/fd.h \
		../fd/fd_dcl.h \
		../fd/fd_fd.h \
		../fd/fd_fd_table.h \
		../fd/fd_filter.h \
		../fd/fd_item.h \
		../fd/fd_item_table.h \
		../fd/fd_quickmatch.h \
		../fd/fd_softmatch.h

FD_SRCS=	../fd/fd_fd.cpp \
		../fd/fd_fd__export.cpp \
		../fd/fd_fd__export_csv.cpp \
		../fd/fd_fd__export_html.cpp \
		../fd/fd_fd__export_rq.cpp \
		../fd/fd_fd_table.cpp \
		../fd/fd_filter.cpp \
		../fd/fd_item.cpp \
		../fd/fd_item_table.cpp \
		../fd/fd_quickmatch.cpp \
		../fd/fd_softmatch.cpp \
		../fd/fd_softmatch__ruleset_mode_compare.cpp \
		../fd/fd_softmatch__simple_mode_compare.cpp

FD_LIB=		sgdm_fd

##################################################################

MY_CXX_FLAGS+=	
MY_LD_LIBS+=	-l$(FD_LIB)
MY_OBJDEPS+=	../fd/fd.inc $(FD_HDRS)
MY_LIBDEPS+=	../fd/fd.inc
MY_EXEDEPS+=	../fd/fd.inc $(LIBDIR)/$(LIB_PFX)$(FD_LIB)$(LIB_SFX)
