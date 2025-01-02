## fs.inc -- GNU Makefile for sgdm3/fs for Linux
## file declarations for dependency checking by users of this library.
##################################################################

FS_HDRS=	../fs/fs.h \
		../fs/fs_dcl.h \
		../fs/fs_fs.h \
		../fs/fs_fs_table.h


FS_SRCS=	../fs/fs_fs.cpp \
		../fs/fs_fs_table.cpp

FS_LIB=		sgdm_fs

##################################################################

MY_CXX_FLAGS+=	
MY_LD_LIBS+=	-l$(FS_LIB)
MY_OBJDEPS+=	../fs/fs.inc $(FS_HDRS)
MY_LIBDEPS+=	../fs/fs.inc
MY_EXEDEPS+=	../fs/fs.inc $(LIBDIR)/$(LIB_PFX)$(FS_LIB)$(LIB_SFX)
