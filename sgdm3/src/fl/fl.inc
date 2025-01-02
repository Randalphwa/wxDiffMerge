## fl.inc -- GNU Makefile for sgdm3/fl for Linux
## file declarations for dependency checking by users of this library.
##################################################################

FL_HDRS=	../fl/fl.h \
		../fl/fl_dcl.h \
		../fl/fl_fl.h \
		../fl/fl_line.h \
		../fl/fl_run.h \
		../fl/fl_run_list.h \
		../fl/fl_run_list_endpoints.h

FL_SRCS=	../fl/fl_fl.cpp \
		../fl/fl_line.cpp \
		../fl/fl_run.cpp \
		../fl/fl_run_list.cpp \
		../fl/fl_run_list_endpoints.cpp

FL_LIB=		sgdm_fl

##################################################################

MY_CXX_FLAGS+=	
MY_LD_LIBS+=	-l$(FL_LIB)
MY_OBJDEPS+=	../fl/fl.inc $(FL_HDRS)
MY_LIBDEPS+=	../fl/fl.inc
MY_EXEDEPS+=	../fl/fl.inc $(LIBDIR)/$(LIB_PFX)$(FL_LIB)$(LIB_SFX)
