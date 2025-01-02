## Config/Config.Windows.inc -- GNU Makefile configuration parameters
## to be included by makefiles in peer directories
##################################################################

##################################################################
## MODE either "Debug" or "Release"
##
## should be set by outer Makefile
##
##MODE=Debug

##################################################################
## ARCH and ARCH_PACKAGE should be set by outer Makefile.

# see ConfigExecutable.Windows.inc for BINDIR

ifeq ($(strip $(BUILD_BUILDLABEL)),)
DOT_BUILDLABEL:=
else
DOT_BUILDLABEL:=.$(BUILD_BUILDLABEL)
endif

MY_DIR:=$(OS_NAME)/$(ARCH)/$(MODE)/tmp/$(BUILD_MAJOR).$(BUILD_MINOR).$(BUILD_REVISION).$(BUILD_BUILDNUM)$(DOT_BUILDLABEL)
## requires conversion to Windows based paths
LIBDIR:=$(BUILD_DOS)/$(MY_DIR)/obj
## this requires the WSL based path
OBJDIR:=$(BUILD_CYG)/$(MY_DIR)/obj

##################################################################

CONFIG_HDRS=	../Config/ConfigDcl.h \
		../Config/ConfigFeatures.template \
		../Config/BuildNumber.template \
		../Config/ConfigMacros.h \
		../Config/ConfigPch.h \
		../Config/wx_proportional_splitter.h

##################################################################

EXE_SFX=	.exe
OBJ_SFX=	.o
LIB_SFX=	.a
LIB_PFX=	lib

##################################################################

MY_OBJDEPS+=	../Config/Config.$(OS_NAME).inc $(CONFIG_HDRS)
MY_LIBDEPS+=	../Config/Config.$(OS_NAME).inc
MY_EXEDEPS+=	../Config/Config.$(OS_NAME).inc
