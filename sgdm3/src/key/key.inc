## key.inc -- GNU Makefile for sgdm3/key for Linux
## file declarations for dependency checking by users of this library.
##################################################################

KEY_HDRS=	../key/sg_uuid.h \
		../key/sghash.h \
		../key/sghash__private.h \
		../key/sghash__sha1.h \
		../key/key_license.h

KEY_SRCS=	../key/sg_uuid.cpp \
		../key/sghash.cpp \
		../key/sghash__sha1.cpp \
		../key/key_license.cpp

KEY_LIB=	sgdm_key

##################################################################

MY_CXX_FLAGS+=	-I../key
MY_LD_LIBS+=	-l$(KEY_LIB)
MY_OBJDEPS+=	../key/key.inc $(KEY_HDRS)
MY_LIBDEPS+=	../key/key.inc
MY_EXEDEPS+=	../key/key.inc $(LIBDIR)/$(LIB_PFX)$(KEY_LIB)$(LIB_SFX)
