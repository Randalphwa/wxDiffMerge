## fim.inc -- GNU Makefile for sgdm3/fim for Linux
## file declarations for dependency checking by users of this library.
##################################################################

FIM_HDRS=	../fim/fim.h \
		../fim/fim_buf.h \
		../fim/fim_buf_table.h \
		../fim/fim_crec.h \
		../fim/fim_crecvec.h \
		../fim/fim_dcl.h \
		../fim/fim_frag.h \
		../fim/fim_patchset.h \
		../fim/fim_ptable.h \
		../fim/fim_ptable_table.h

FIM_SRCS=	../fim/fim_buf.cpp \
		../fim/fim_buf_table.cpp \
		../fim/fim_crec.cpp \
		../fim/fim_crecvec.cpp \
		../fim/fim_frag.cpp \
		../fim/fim_patchset.cpp \
		../fim/fim_ptable.cpp \
		../fim/fim_ptable_table.cpp

FIM_LIB=	sgdm_fim

##################################################################

MY_CXX_FLAGS+=	
MY_LD_LIBS+=	-l$(FIM_LIB)
MY_OBJDEPS+=	../fim/fim.inc $(FIM_HDRS)
MY_LIBDEPS+=	../fim/fim.inc
MY_EXEDEPS+=	../fim/fim.inc $(LIBDIR)/$(LIB_PFX)$(FIM_LIB)$(LIB_SFX)
