## xt.inc -- GNU Makefile for sgdm3/xt for Linux
## file declarations for dependency checking by users of this library.
##################################################################

XT_HDRS=	../xt/xt.h \
		../xt/xt_dcl.h \
		../xt/xt_tool.h \
		../xt/xt_tool_dlg.h \
		../xt/xt_tool_table.h

XT_SRCS=	../xt/xt_tool.cpp \
		../xt/xt_tool_dlg.cpp \
		../xt/xt_tool_table.cpp

XT_LIB=		sgdm_xt

##################################################################

MY_CXX_FLAGS+=	
MY_LD_LIBS+=	-l$(XT_LIB)
MY_OBJDEPS+=	../xt/xt.inc $(XT_HDRS)
MY_LIBDEPS+=	../xt/xt.inc
MY_EXEDEPS+=	../xt/xt.inc $(LIBDIR)/$(LIB_PFX)$(XT_LIB)$(LIB_SFX)
