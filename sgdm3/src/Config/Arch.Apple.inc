## Arch.Apple.inc -- A makefile snippet to figure out how to set ARCH
## and ARCH_PACKAGE on Apple.
##
## This makefile does not depend upon the rest of the src/Config
## makefile stuff, but it *USED* by it.
##################################################################

FLAVOR:=mac

## Set this on the command line to override.
#ARCH:=arm64

ifeq ($(ARCH),arm64)
ARCH_PACKAGE:=arm64

else ifeq ($(ARCH),x86_64)
ARCH_PACKAGE:=x64

else
$(error stop unknown architecture. ARCH is $(ARCH).  Specify 'arm64' or 'x86_64' as an environment variable.)
endif

FLAVOR_ARCH:=$(FLAVOR).$(ARCH_PACKAGE)
