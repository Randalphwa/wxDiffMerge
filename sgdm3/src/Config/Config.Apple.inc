## Config/Config.Apple.inc -- GNU Makefile configuration parameters
## to be included by makefiles in peer directories
##################################################################

##################################################################
## Should be set by outer Makefile.
##
####WXROOT
####WXCFGARG

##################################################################
## set and export these environment variables so that we reference
## and link with our private copy of wxWidgets.

LD_LIBRARY_PATH:=	$(WXROOT)/lib
export LD_LIBRARY_PATH

####PATH:=			$(WXROOT)/bin:$(PATH)
####export PATH

##################################################################
## MODE either "Debug" or "Release"
##
## should be set by outer Makefile
##
##MODE=Debug

##################################################################
## ARCH should be set by outer Makefile.

ifeq ($(ARCH),arm64)
ARCH_FLAGS:=-arch arm64
USE_PCH:=no
else ifeq ($(ARCH),x86_64)
ARCH_FLAGS:=-arch x86_64
USE_PCH:=yes
else
$(error stop unknown architecture. ARCH is $(ARCH).  Specify 'arm64' or 'x86_64' as an environment variable.)
endif

##################################################################
## WXCFG maps to the inplace version of wx-config in the build
## directory of the wxWidgets distribution.

WXCFG:=		$(WXROOT)/wx-config $(WXCFGARGS)

WX_CXX:=	$(shell $(WXCFG) --cxx)
WX_CXX_FLAGS:=	$(shell $(WXCFG) --cxxflags)
WX_CXX_LIBS:=	$(shell $(WXCFG) --libs)

WX_CC:=		$(shell $(WXCFG) --cc)
WX_CC_FLAGS:=	$(shell $(WXCFG) --cflags)

##################################################################
## BUILD gives the root of where our output will go.
##
## Should be set by outer Makefile.
##
####BUILD

# see ConfigExecutable.Apple.inc for BINDIR

ifeq ($(strip $(BUILD_BUILDLABEL)),)
DOT_BUILDLABEL:=
else
DOT_BUILDLABEL:=.$(BUILD_BUILDLABEL)
endif

LIBDIR:=	$(BUILD)/$(OS_NAME)/$(ARCH)/$(MODE)/tmp/$(BUILD_MAJOR).$(BUILD_MINOR).$(BUILD_REVISION).$(BUILD_BUILDNUM)$(DOT_BUILDLABEL)/lib
OBJDIR:=	$(BUILD)/$(OS_NAME)/$(ARCH)/$(MODE)/tmp/$(BUILD_MAJOR).$(BUILD_MINOR).$(BUILD_REVISION).$(BUILD_BUILDNUM)$(DOT_BUILDLABEL)/obj

ifeq ($(USE_PCH),yes)
PCHDIR:=	$(BUILD)/$(OS_NAME)/$(ARCH)/$(MODE)/tmp/$(BUILD_MAJOR).$(BUILD_MINOR).$(BUILD_REVISION).$(BUILD_BUILDNUM)$(DOT_BUILDLABEL)/pch
DASH_I_PCHDIR:= -I$(PCHDIR)
PCHDIR_GCH:=	$(PCHDIR)/ConfigPch.h.gch
CONFIGPCH_H:=	../Config/ConfigPch.h
endif

##################################################################

ifeq ($(MODE),Debug)
CONFIG_CXX_FLAGS=	-DDEBUG -D_DEBUG -ggdb -Wall
CONFIG_LD_FLAGS=	-g
else
MODE=			Release
CONFIG_CXX_FLAGS=	-UDEBUG -U_DEBUG
CONFIG_LD_FLAGS=
endif

CONFIG_CXX_FLAGS+=	-DUNICODE -DMAC
CONFIG_CXX_FLAGS+=	$(ARCH_FLAGS)

CONFIG_LD_FLAGS+=	-L$(LIBDIR) $(ARCH_FLAGS)

##################################################################

CONFIG_HDRS=	../Config/ConfigDcl.h \
		../Config/ConfigFeatures.template \
		../Config/BuildNumber.template \
		../Config/ConfigMacros.h \
		$(CONFIGPCH_H) \
		../Config/wx_proportional_splitter.h

##################################################################

EXE_SFX=
OBJ_SFX=	.cpp_o
MM_OBJ_SFX=	.mm_o
C_OBJ_SFX=	.c_o
LIB_SFX=	.a
LIB_PFX=	lib

##################################################################

MY_CXX_FLAGS+=	$(CONFIG_CXX_FLAGS) $(DASH_I_PCHDIR) -I$(OBJDIR) -I../Config -I../util -I ../rs -I../poi -I../fd -I../fim -I../fs -I../fl -I../de -I../xt -I../gui
MY_LD_FLAGS+=	$(CONFIG_LD_FLAGS)
MY_LD_LIBS+=	$(CONFIG_LD_LIBS)
MY_OBJDEPS+=	../Config/Config.$(OS_NAME).inc $(CONFIG_HDRS)
MY_LIBDEPS+=	../Config/Config.$(OS_NAME).inc
MY_EXEDEPS+=	../Config/Config.$(OS_NAME).inc
