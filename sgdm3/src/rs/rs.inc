## rs.inc -- GNU Makefile for sgdm3/rs for Linux
## file declarations for dependency checking by users of this library.
##################################################################

RS_HDRS=	../rs/rs.h \
		../rs/rs_choose_dlg.h \
		../rs/rs_context.h \
		../rs/rs_context_dlg.h \
		../rs/rs_dcl.h \
		../rs/rs_lomit.h \
		../rs/rs_lomit_dlg.h \
		../rs/rs_ruleset.h \
		../rs/rs_ruleset_dlg.h \
		../rs/rs_ruleset_table.h


RS_SRCS=	../rs/rs_choose_dlg.cpp \
		../rs/rs_context.cpp \
		../rs/rs_context_dlg.cpp \
		../rs/rs_lomit.cpp \
		../rs/rs_lomit_dlg.cpp \
		../rs/rs_ruleset.cpp \
		../rs/rs_ruleset_dlg.cpp \
		../rs/rs_ruleset_table.cpp

RS_LIB=		sgdm_rs

##################################################################

MY_CXX_FLAGS+=	
MY_LD_LIBS+=	-l$(RS_LIB)
MY_OBJDEPS+=	../rs/rs.inc $(RS_HDRS)
MY_LIBDEPS+=	../rs/rs.inc
MY_EXEDEPS+=	../rs/rs.inc $(LIBDIR)/$(LIB_PFX)$(RS_LIB)$(LIB_SFX)
