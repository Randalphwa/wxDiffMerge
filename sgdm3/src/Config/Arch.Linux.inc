## Arch.Linux.inc -- A makefile snippet to figure out the mess
## between the CPU architecture and the .deb/.rpm architecture
## and what wxWidgets wants to call it.
##
## This makefile does not depend upon the rest of the src/Config
## makefile stuff, but it *USED* by it.
##################################################################

ifeq ($(wildcard /etc/redhat-release*),/etc/redhat-release)
FLAVOR:=fedora
else
FLAVOR:=ubuntu
endif

##################################################################

ARCH:=$(shell uname -p)

ifeq ($(ARCH),unknown)
ARCH:=$(shell uname -m)
endif

##################################################################
## Fedora calls 32-bit packages i686 and 64-bit packages x64_64.

ifeq ($(FLAVOR),fedora)

ifeq ($(ARCH),i386)

ARCH:=i686
ARCH_PACKAGE:=i686
FLAVOR_ARCH:=$(FLAVOR).x86

else ifeq ($(ARCH),i686)

ARCH:=i686
ARCH_PACKAGE:=i686
FLAVOR_ARCH:=$(FLAVOR).x86

else ifeq ($(ARCH),x86_64)

ARCH:=x86_64
ARCH_PACKAGE:=x86_64
FLAVOR_ARCH:=$(FLAVOR).x64

else ifeq ($(ARCH),amd64)

ARCH:=x86_64
ARCH_PACKAGE:=x86_64
FLAVOR_ARCH:=$(FLAVOR).x64

else
$(error stop unknown architecture. ARCH is $(ARCH))
endif

endif

##################################################################
## Ubuntu calls 32-bit i386 and 64-bit amd64.

ifeq ($(FLAVOR),ubuntu)

ifeq ($(ARCH),i386)

ARCH:=i686
ARCH_PACKAGE:=i386
FLAVOR_ARCH:=$(FLAVOR).x86

else ifeq ($(ARCH),i686)

ARCH:=i686
ARCH_PACKAGE:=i386
FLAVOR_ARCH:=$(FLAVOR).x86

else ifeq ($(ARCH),x86_64)

ARCH:=x86_64
ARCH_PACKAGE:=amd64
FLAVOR_ARCH:=$(FLAVOR).x64

else ifeq ($(ARCH),amd64)

ARCH:=x86_64
ARCH_PACKAGE:=amd64
FLAVOR_ARCH:=$(FLAVOR).x64

else
$(error stop unknown architecture. ARCH is $(ARCH))
endif

endif

