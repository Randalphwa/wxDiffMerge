## Arch.Windows.inc -- A makefile snippet to figure out
## and set ARCH and ARCH_PACKAGE based upon the currently selected
## compiler.
##
## When executing a 64-bit build, the outer makefile should have
## set the TARGET_PLATFORM to x86 or x64
##

FLAVOR:=windows

ifeq ($(TARGET_PLATFORM),x86)
ARCH=i686
ARCH_PACKAGE=x86
FLAVOR_ARCH:=$(FLAVOR).x86
else ifeq ($(TARGET_PLATFORM),x64)
ARCH=amd64
ARCH_PACKAGE=x64
FLAVOR_ARCH:=$(FLAVOR).x64
else
$(error stop unknown platform. TARGET_PLATFORM is "$(TARGET_PLATFORM)")
endif
