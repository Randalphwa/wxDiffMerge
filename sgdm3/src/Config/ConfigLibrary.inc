## Config/ConfigLibrary.inc -- GNU Makefile snippets for building 
## a static library.
##################################################################

TARGET=		$(LIBDIR)/$(LIB_PFX)$(TARGET_LIB)$(LIB_SFX)

default:	$(TARGET)

##################################################################

include ../Config/ConfigCompile.inc

##################################################################
## on Apple, we ranlib a little differently due to universal binaries.

MY_LIBDEPS+=	../Config/ConfigLibrary.inc

$(TARGET):	$(OBJS) $(MY_LIBDEPS)
	@[ -d $(LIBDIR) ] || mkdir -p $(LIBDIR)
	@echo Archiving $@
ifeq ($(OS_NAME),Apple)
	@libtool -static -o $@ -s $(OBJS)
else
	@$(AR) -rcs $@ $(OBJS)
endif

##################################################################

clean:
	@rm -f $(OBJS)
	@rm -f $(TARGET)

##################################################################

.PHONY:	clean

xxxx:
	echo TARGET is $(TARGET)
	echo SRCS is $(SRCS)
	echo OBJS is $(OBJS)
	exit 1
