## Config/ConfigCompile.inc -- GNU Makefile snippets for compiling C++ code.
##################################################################

MY_OBJDEPS+=	../Config/ConfigCompile.inc

##################################################################

SRCDIR:=	$(shell pwd)

vpath		%.cpp	$(SRCDIR)

##################################################################

ifeq ($(OS_NAME),Apple)

XXX1:=$(SRCS:.cpp=$(OBJ_SFX))
XXX2:=$(XXX1:.mm=$(MM_OBJ_SFX))
XXX3:=$(XXX2:.c=$(C_OBJ_SFX))
OBJS:=$(addprefix $(OBJDIR)/,$(notdir $(XXX3)))

$(filter %$(OBJ_SFX),$(OBJS)): $(OBJDIR)/%$(OBJ_SFX): %.cpp $(MY_OBJDEPS) $(OBJDIR)/GEN_BuildNumber.h $(PCHDIR_GCH)
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
	@echo Compiling C++ $<
	@$(WX_CXX) $(WX_CXX_FLAGS) $(MY_CXX_FLAGS) -o $@ -c $<

$(filter %$(MM_OBJ_SFX),$(OBJS)): $(OBJDIR)/%$(MM_OBJ_SFX): %.mm $(MY_OBJDEPS) $(OBJDIR)/GEN_BuildNumber.h $(PCHDIR_GCH)
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
	@echo Compiling Objective-C $<
	@$(WX_CXX) $(WX_CXX_FLAGS) $(MY_CXX_FLAGS) -o $@ -c $<

$(filter %$(C_OBJ_SFX),$(OBJS)): $(OBJDIR)/%$(C_OBJ_SFX): %.c $(MY_OBJDEPS) $(OBJDIR)/GEN_BuildNumber.h $(PCHDIR_GCH)
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
	@echo Compiling C $<
	@$(WX_CC) $(WX_CC_FLAGS) $(MY_CXX_FLAGS) -o $@ -c $<

else

OBJS=		$(addprefix $(OBJDIR)/,$(notdir $(SRCS:.cpp=$(OBJ_SFX))))

$(OBJS):	$(OBJDIR)/%$(OBJ_SFX): %.cpp $(MY_OBJDEPS) $(OBJDIR)/GEN_BuildNumber.h $(PCHDIR_GCH)
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
	@echo Compiling $<
	@$(WX_CXX) $(WX_CXX_FLAGS) $(MY_CXX_FLAGS) -o $@ -c $<

endif

##################################################################

ifdef PCHDIR_GCH
$(PCHDIR_GCH): ../Config/ConfigPch.h
	@[ -d $(PCHDIR) ] || mkdir -p $(PCHDIR)
	@echo Compiling Precompiled Header $<
	@$(WX_CXX) $(WX_CXX_FLAGS) $(MY_CXX_FLAGS) -o $@ -c $<
endif

##################################################################

include ../Config/ConfigBuildNumber.inc
