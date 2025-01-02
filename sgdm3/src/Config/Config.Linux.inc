## Config/Config.Linux.inc -- GNU Makefile configuration parameters
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
## Should be set by outer Makefile.
##
####MODE=Debug

##################################################################
## FLAVOR, ARCH, ARCH_PACKAGE should be set by outer Makefile.

##################################################################
## WXCFG maps to wx-config in the bin directory of the wxWidgets
## distribution. since we may have installed both
## Debug and Release versions, we need to explicitly name the
## version we want.

WXCFG:=		$(WXROOT)/wx-config $(WXCFGARGS)

WX_CXX:=	$(shell $(WXCFG) --cxx)
WX_CXX_FLAGS:=	$(shell $(WXCFG) --cxxflags)
WX_CXX_LIBS:=	$(shell $(WXCFG) --libs)

##################################################################

GTK_VERSION:=	$(shell pkg-config --modversion gtk+-2.0)
GTK_CFLAGS:=	$(shell pkg-config --cflags gtk+-2.0)
GTK_LIBS:=	$(shell pkg-config --libs gtk+-2.0)

##################################################################
## BUILD gives the root of where our output will go.
##
## Should be set by outer Makefile.
##
####BUILD

# see ConfigExecutable.Linux.inc for BINDIR

ifeq ($(strip $(BUILD_BUILDLABEL)),)
DOT_BUILDLABEL:=
else
DOT_BUILDLABEL:=.$(BUILD_BUILDLABEL)
endif

LIBDIR:=	$(BUILD)/$(OS_NAME)/$(ARCH)/$(MODE)/tmp/$(BUILD_MAJOR).$(BUILD_MINOR).$(BUILD_REVISION).$(BUILD_BUILDNUM)$(DOT_BUILDLABEL)/lib
OBJDIR:=	$(BUILD)/$(OS_NAME)/$(ARCH)/$(MODE)/tmp/$(BUILD_MAJOR).$(BUILD_MINOR).$(BUILD_REVISION).$(BUILD_BUILDNUM)$(DOT_BUILDLABEL)/obj
PCHDIR:=	$(BUILD)/$(OS_NAME)/$(ARCH)/$(MODE)/tmp/$(BUILD_MAJOR).$(BUILD_MINOR).$(BUILD_REVISION).$(BUILD_BUILDNUM)$(DOT_BUILDLABEL)/pch

PCHDIR_GCH:=	$(PCHDIR)/ConfigPch.h.gch

##################################################################

ifeq ($(MODE),Debug)
####CONFIG_CXX_FLAGS=	-DDEBUG -D_DEBUG -g -pg
####CONFIG_LD_FLAGS=	-g -pg
CONFIG_CXX_FLAGS=	-DDEBUG -D_DEBUG -g
CONFIG_LD_FLAGS=	-g
else
MODE=			Release
CONFIG_CXX_FLAGS=	-UDEBUG -U_DEBUG
CONFIG_LD_FLAGS=
endif

CONFIG_CXX_FLAGS+=	$(GTK_CFLAGS)
CONFIG_CXX_FLAGS+=	-DUNICODE -DLINUX
CONFIG_CXX_FLAGS+=	$(ARCH_FLAGS)

CONFIG_LD_FLAGS+=	-L$(LIBDIR)
CONFIG_LD_LIBS+=	$(GTK_LIBS)

##################################################################

CONFIG_HDRS=	../Config/ConfigDcl.h \
		../Config/ConfigFeatures.template \
		../Config/BuildNumber.template \
		../Config/ConfigMacros.h \
		../Config/ConfigPch.h \
		../Config/wx_proportional_splitter.h

##################################################################

EXE_SFX=
OBJ_SFX=	.o
LIB_SFX=	.a
LIB_PFX=	lib

##################################################################

MY_CXX_FLAGS+=	$(CONFIG_CXX_FLAGS) -I$(PCHDIR) -I$(OBJDIR) -I../Config -I../util -I ../rs -I../poi -I../fd -I../fim -I../fs -I../fl -I../de -I../xt -I../gui
MY_LD_FLAGS+=	$(CONFIG_LD_FLAGS)
MY_LD_LIBS+=	$(CONFIG_LD_LIBS)
MY_OBJDEPS+=	../Config/Config.$(OS_NAME).inc $(CONFIG_HDRS)
MY_LIBDEPS+=	../Config/Config.$(OS_NAME).inc
MY_EXEDEPS+=	../Config/Config.$(OS_NAME).inc
