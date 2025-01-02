## Config/ConfigExecutable.Linux.inc -- GNU Makefile snippets for
## building an executable.
##################################################################

ifeq ($(strip $(BUILD_BUILDLABEL)),)
DOT_BUILDLABEL:=
else
DOT_BUILDLABEL:=.$(BUILD_BUILDLABEL)
endif

## on Linux, the package is called "diffmerge".
## the executable is called "diffmerge".
## TODO should we call the package sourcegear_diffmerge or something?
## TODO should we call the exe sgdm?  (i didn't find any conflicts with
## TODO common debian packages, but i didn't look that hard....)

PACKAGE:=	diffmerge
TARGET_EXE:=	diffmerge

VER3:=		$(BUILD_MAJOR).$(BUILD_MINOR).$(BUILD_REVISION)$(DOT_BUILDLABEL)
VER4:=		$(BUILD_MAJOR).$(BUILD_MINOR).$(BUILD_REVISION).$(BUILD_BUILDNUM)$(DOT_BUILDLABEL)
BINDIR:=	$(BUILD)/$(OS_NAME)/$(ARCH)/$(MODE)/tmp/$(VER4)/bin
TARGET:=	$(BINDIR)/$(TARGET_EXE)$(EXE_SFX)

##################################################################

include ../Config/ConfigExecutable.$(OS_NAME).$(FLAVOR).inc

##################################################################

MY_EXEDEPS+=	../Config/ConfigExecutable.$(OS_NAME).inc

