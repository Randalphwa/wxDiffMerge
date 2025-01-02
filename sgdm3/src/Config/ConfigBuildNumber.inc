## Config/ConfigBuildNumber.inc -- GNU Makefile snippet
##################################################################
## put BUILDNUM and associated variables into header file so that
## we get the #define symbols without using -D compiler flags (which
## don't work on Windows when using a .sln).

$(OBJDIR)/GEN_BuildNumber.h:
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
	@cat ../Config/BuildNumber.template 			\
	| sed 's|@MAJOR@|$(BUILD_MAJOR)|g'			\
	| sed 's|@MINOR@|$(BUILD_MINOR)|g'			\
	| sed 's|@REVISION@|$(BUILD_REVISION)|g'		\
	| sed 's|@BUILDNUM@|$(BUILD_BUILDNUM)|g'		\
	| sed 's|@COPYRIGHTDATE@|$(BUILD_COPYRIGHTDATE)|g'	\
	| sed 's|@BUILDLABEL@|$(BUILD_BUILDLABEL)|g'		\
	| sed 's|@CSID@|$(BUILD_CSID)|g'			\
	| sed 's|@ARCH@|$(ARCH)|g'				\
	| sed 's|@ARCH_PACKAGE@|$(ARCH_PACKAGE)|g'		\
	| sed 's|@FLAVOR@|$(FLAVOR)|g'				\
	| sed 's|@FLAVOR_ARCH@|$(FLAVOR_ARCH)|g'		\
	> $@
	@#
	@# Also create a file with just the number and a special name.
	@#
	@rm -f $(OBJDIR)/GEN_BuildNumber.*_*_*_*.h
	@echo $(BUILD_MAJOR).$(BUILD_MINOR).$(BUILD_REVISION).$(BUILD_BUILDNUM) \
	> $(OBJDIR)/GEN_BuildNumber.$(BUILD_MAJOR)_$(BUILD_MINOR)_$(BUILD_REVISION)_$(BUILD_BUILDNUM).h
	@echo Created $@
