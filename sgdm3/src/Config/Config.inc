## Config/Config.inc -- GNU Makefile configuration parameters
## to be included by makefiles in peer directories
##################################################################

include ../Config/Config.$(OS_NAME).inc

MY_OBJDEPS+=	../Config/Config.inc
MY_LIBDEPS+=	../Config/Config.inc
MY_EXEDEPS+=	../Config/Config.inc
