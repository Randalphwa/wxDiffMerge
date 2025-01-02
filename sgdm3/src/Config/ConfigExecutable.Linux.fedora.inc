## Config/ConfigExecutable.Linux.fedora.inc -- GNU Makefile snippets for
## building an executable and packaging as an RPM.
##################################################################

RPMVER:=	1

FEDORA_RELEASE:=$(shell uname -r)

##################################################################

ifeq ($(MODE),Release)
default:	rpm
else
default:	$(TARGET)
endif

##################################################################

include ../Config/ConfigCompile.inc

##################################################################

MY_EXEDEPS+=	../Config/ConfigExecutable.$(OS_NAME).$(FLAVOR).inc

## 2011/12/19 Fedora 16 requires -lX11 for some reason. I thought it
##            was getting pulled in by the stuff in $(WX_CXX_LIBS)
##            but I guess not.

$(TARGET):	$(OBJS) $(MY_EXEDEPS)
	@[ -d $(BINDIR) ] || mkdir -p $(BINDIR)
	@echo Linking $@
	@$(WX_CXX) -o $(TARGET) $(OBJS) $(MY_LD_FLAGS) $(MY_LD_LIBS) $(WX_CXX_LIBS) -lX11 -lcurl -luuid
ifeq ($(MODE),Release)
	@strip --strip-all --remove-section=.comment --remove-section=.note $(TARGET)
endif

##################################################################

TARBALL_DIR:=	$(BUILD)/$(OS_NAME)/$(ARCH)/$(MODE)/tmp/$(VER4)/tarball_dir
TARBALL_ROOT:=	$(TARBALL_DIR)/$(PACKAGE)-$(VER4)
RPMBUILD:=	$(BUILD)/$(OS_NAME)/$(ARCH)/$(MODE)/tmp/$(VER4)/rpm_build

rpm:	$(TARGET)
	@# we've already done everything that the rpm.spec %build (compile) target should do.
	@# so now we fake it for the rpm step.
	@# since the .spec file requires at least one SOURCE file, we construct a tarball
	@# containing the target exe and branded/versioned accessory files.  this gets placed
	@# in the $RPMBUILD/SOURCES directory.
	@echo Starting to create RPM.  Building initial TARBALL.
	@rm -rf $(TARBALL_ROOT)
	@mkdir -p $(TARBALL_ROOT)
	@mkdir -p $(TARBALL_ROOT)/usr/bin
	@mkdir -p $(TARBALL_ROOT)/usr/share/man/man1
	@mkdir -p $(TARBALL_ROOT)/usr/share/doc/$(PACKAGE)-$(VER4)
	@mkdir -p $(TARBALL_ROOT)/usr/share/pixmaps
	@mkdir -p $(TARBALL_ROOT)/usr/share/applications
	@cp $(TARGET) 	$(TARBALL_ROOT)/usr/bin
	@chmod 755	$(TARBALL_ROOT)/usr/bin/$(PACKAGE)
	@cp ../Installers/DiffMergeManual.pdf	$(TARBALL_ROOT)/usr/share/doc/$(PACKAGE)-$(VER4)
	@chmod 644	$(TARBALL_ROOT)/usr/share/doc/$(PACKAGE)-$(VER4)/DiffMergeManual.pdf
	@@cat ../Installers/$(OS_NAME).fedora.man1		\
	| sed 's|@MAJOR@|$(BUILD_MAJOR)|g'			\
	| sed 's|@MINOR@|$(BUILD_MINOR)|g'			\
	| sed 's|@REVISION@|$(BUILD_REVISION)|g'		\
	| sed 's|@BUILDNUM@|$(BUILD_BUILDNUM)|g'		\
	| sed 's|@COPYRIGHTDATE@|$(BUILD_COPYRIGHTDATE)|g'	\
	| sed 's|@BUILDLABEL@|$(BUILD_BUILDLABEL)|g'		\
	| sed 's|@CSID@|$(BUILD_CSID)|g'			\
	| sed 's|@DOTLABEL@|$(DOT_BUILDLABEL)|g'		\
	| sed 's|@PACKAGE@|$(PACKAGE)|g'			\
	| sed 's|@RPMVER@|$(RPMVER)|g'				\
	| sed 's|@ARCH@|$(ARCH)|g'				\
	| sed 's|@ARCH_PACKAGE@|$(ARCH_PACKAGE)|g'		\
	| sed 's|@EXE@|/usr/bin/$(TARGET_EXE)|g'		\
	| sed 's|@VER4@|$(VER4)|g'				\
	| gzip --stdout --best 					\
	> $(TARBALL_ROOT)/usr/share/man/man1/$(PACKAGE).1.gz
	@chmod 644 $(TARBALL_ROOT)/usr/share/man/man1/$(PACKAGE).1.gz
	@cat ../Installers/$(OS_NAME).fedora.copyright		\
	| sed 's|@MAJOR@|$(BUILD_MAJOR)|g'			\
	| sed 's|@MINOR@|$(BUILD_MINOR)|g'			\
	| sed 's|@REVISION@|$(BUILD_REVISION)|g'		\
	| sed 's|@BUILDNUM@|$(BUILD_BUILDNUM)|g'		\
	| sed 's|@COPYRIGHTDATE@|$(BUILD_COPYRIGHTDATE)|g'	\
	| sed 's|@BUILDLABEL@|$(BUILD_BUILDLABEL)|g'		\
	| sed 's|@CSID@|$(BUILD_CSID)|g'			\
	| sed 's|@DOTLABEL@|$(DOT_BUILDLABEL)|g'		\
	| sed 's|@PACKAGE@|$(PACKAGE)|g'			\
	| sed 's|@RPMVER@|$(RPMVER)|g'				\
	| sed 's|@ARCH@|$(ARCH)|g'				\
	| sed 's|@ARCH_PACKAGE@|$(ARCH_PACKAGE)|g'		\
	| sed 's|@EXE@|/usr/bin/$(TARGET_EXE)|g'		\
	| sed 's|@RPMBUILD@|$(RPMBUILD)|g'			\
	| sed 's|@VER4@|$(VER4)|g'				\
	> $(TARBALL_ROOT)/usr/share/doc/$(PACKAGE)-$(VER4)/copyright
	@chmod 644 $(TARBALL_ROOT)/usr/share/doc/$(PACKAGE)-$(VER4)/copyright
	@cat ../Installers/$(OS_NAME).fedora.Readme.txt		\
	| sed 's|@MAJOR@|$(BUILD_MAJOR)|g'			\
	| sed 's|@MINOR@|$(BUILD_MINOR)|g'			\
	| sed 's|@REVISION@|$(BUILD_REVISION)|g'		\
	| sed 's|@BUILDNUM@|$(BUILD_BUILDNUM)|g'		\
	| sed 's|@COPYRIGHTDATE@|$(BUILD_COPYRIGHTDATE)|g'	\
	| sed 's|@BUILDLABEL@|$(BUILD_BUILDLABEL)|g'		\
	| sed 's|@CSID@|$(BUILD_CSID)|g'			\
	| sed 's|@DOTLABEL@|$(DOT_BUILDLABEL)|g'		\
	| sed 's|@PACKAGE@|$(PACKAGE)|g'			\
	| sed 's|@RPMVER@|$(RPMVER)|g'				\
	| sed 's|@ARCH@|$(ARCH)|g'				\
	| sed 's|@ARCH_PACKAGE@|$(ARCH_PACKAGE)|g'		\
	| sed 's|@EXE@|/usr/bin/$(TARGET_EXE)|g'		\
	| sed 's|@RPMBUILD@|$(RPMBUILD)|g'			\
	| sed 's|@VER4@|$(VER4)|g'				\
	| sed 's|@FEDORA_RELEASE@|$(FEDORA_RELEASE)|g'		\
	> $(TARBALL_ROOT)/usr/share/doc/$(PACKAGE)-$(VER4)/Readme.txt
	@chmod 644 $(TARBALL_ROOT)/usr/share/doc/$(PACKAGE)-$(VER4)/Readme.txt
	@cat ../Installers/$(OS_NAME).fedora.desktop		\
	| sed 's|@MAJOR@|$(BUILD_MAJOR)|g'			\
	| sed 's|@MINOR@|$(BUILD_MINOR)|g'			\
	| sed 's|@REVISION@|$(BUILD_REVISION)|g'		\
	| sed 's|@BUILDNUM@|$(BUILD_BUILDNUM)|g'		\
	| sed 's|@COPYRIGHTDATE@|$(BUILD_COPYRIGHTDATE)|g'	\
	| sed 's|@BUILDLABEL@|$(BUILD_BUILDLABEL)|g'		\
	| sed 's|@CSID@|$(BUILD_CSID)|g'			\
	| sed 's|@DOTLABEL@|$(DOT_BUILDLABEL)|g'		\
	| sed 's|@PACKAGE@|$(PACKAGE)|g'			\
	| sed 's|@RPMVER@|$(RPMVER)|g'				\
	| sed 's|@ARCH@|$(ARCH)|g'				\
	| sed 's|@ARCH_PACKAGE@|$(ARCH_PACKAGE)|g'		\
	| sed 's|@EXE@|/usr/bin/$(TARGET_EXE)|g'		\
	| sed 's|@RPMBUILD@|$(RPMBUILD)|g'			\
	| sed 's|@VER4@|$(VER4)|g'				\
	> $(TARBALL_ROOT)/usr/share/applications/sourcegear.com-$(PACKAGE).desktop
	@chmod 644 $(TARBALL_ROOT)/usr/share/applications/sourcegear.com-$(PACKAGE).desktop
	@cp ../Installers/$(OS_NAME).fedora.DiffMergeIcon48.png $(TARBALL_ROOT)/usr/share/pixmaps/sourcegearcom-$(PACKAGE).png
	@chmod 644 $(TARBALL_ROOT)/usr/share/pixmaps/sourcegearcom-$(PACKAGE).png
	@rm -rf $(RPMBUILD)
	@mkdir -p $(RPMBUILD)
	@mkdir -p $(RPMBUILD)/RPMS
	@mkdir -p $(RPMBUILD)/SOURCES
	@mkdir -p $(RPMBUILD)/SPECS
	@mkdir -p $(RPMBUILD)/SRPMS
	@mkdir -p $(RPMBUILD)/BUILD
	@(cd $(TARBALL_DIR); tar -czvf $(RPMBUILD)/SOURCES/$(PACKAGE)-$(VER4)-$(RPMVER).tar.gz $(PACKAGE)-$(VER4))
	@echo Creating RPMBUILD in $(RPMBUILD)
	@# brand and version the .SPEC file
	@cat ../Installers/$(OS_NAME).fedora.spec		\
	| sed 's|@MAJOR@|$(BUILD_MAJOR)|g'			\
	| sed 's|@MINOR@|$(BUILD_MINOR)|g'			\
	| sed 's|@REVISION@|$(BUILD_REVISION)|g'		\
	| sed 's|@BUILDNUM@|$(BUILD_BUILDNUM)|g'		\
	| sed 's|@COPYRIGHTDATE@|$(BUILD_COPYRIGHTDATE)|g'	\
	| sed 's|@BUILDLABEL@|$(BUILD_BUILDLABEL)|g'		\
	| sed 's|@CSID@|$(BUILD_CSID)|g'			\
	| sed 's|@DOTLABEL@|$(DOT_BUILDLABEL)|g'		\
	| sed 's|@PACKAGE@|$(PACKAGE)|g'			\
	| sed 's|@RPMVER@|$(RPMVER)|g'				\
	| sed 's|@ARCH@|$(ARCH)|g'				\
	| sed 's|@ARCH_PACKAGE@|$(ARCH_PACKAGE)|g'		\
	| sed 's|@EXE@|/usr/bin/$(TARGET_EXE)|g'		\
	| sed 's|@RPMBUILD@|$(RPMBUILD)|g'			\
	| sed 's|@VER4@|$(VER4)|g'				\
	> $(RPMBUILD)/SPECS/$(PACKAGE)-$(VER4)-$(RPMVER).spec
	@# brand and version ~/rpmmacros - these are secretly used by rpmbuild
	@cat ../Installers/$(OS_NAME).fedora.rpmmacros		\
	| sed 's|@MAJOR@|$(BUILD_MAJOR)|g'			\
	| sed 's|@MINOR@|$(BUILD_MINOR)|g'			\
	| sed 's|@REVISION@|$(BUILD_REVISION)|g'		\
	| sed 's|@BUILDNUM@|$(BUILD_BUILDNUM)|g'		\
	| sed 's|@COPYRIGHTDATE@|$(BUILD_COPYRIGHTDATE)|g'	\
	| sed 's|@BUILDLABEL@|$(BUILD_BUILDLABEL)|g'		\
	| sed 's|@CSID@|$(BUILD_CSID)|g'			\
	| sed 's|@DOTLABEL@|$(DOT_BUILDLABEL)|g'		\
	| sed 's|@PACKAGE@|$(PACKAGE)|g'			\
	| sed 's|@RPMVER@|$(RPMVER)|g'				\
	| sed 's|@ARCH@|$(ARCH)|g'				\
	| sed 's|@ARCH_PACKAGE@|$(ARCH_PACKAGE)|g'		\
	| sed 's|@EXE@|/usr/bin/$(TARGET_EXE)|g'		\
	| sed 's|@RPMBUILD@|$(RPMBUILD)|g'			\
	> ~/.rpmmacros
	@# let rpmbuild build the .rpm.  it will unzip the tarball in $RPMBUILD/SOURCES into
	@# $RPMBUILD/BUILD/diffmerge-3.1.0.1000/usr/...
	@# the %install target inside the .SPEC file just copies the entire tree into 
	@# $RPMBUILD/usr/...  (as if it did an /bin/install) and then packages up everything
	@# in $RPMBUILD/usr  (from our %files targets) into the .rpm.
	@# the resulting .rpm is in $RPMBUILD/RPMS/$ARCH_PACKAGE -- we copy this to OUR output directory.
	@echo Running rpmbuild...
	@(cd $(RPMBUILD)/SPECS; rpmbuild --target $(ARCH_PACKAGE) -bb $(PACKAGE)-$(VER4)-$(RPMVER).spec)
	@cp $(RPMBUILD)/RPMS/$(ARCH_PACKAGE)/$(PACKAGE)-$(VER4)-$(RPMVER).$(ARCH_PACKAGE).rpm $(BUILD)/$(OS_NAME)/$(ARCH)/$(MODE)/$(PACKAGE)-$(VER4)-$(RPMVER).$(ARCH_PACKAGE).rpm

##################################################################

clean:
	@rm -f $(OBJS)

##################################################################

.PHONY:	clean run rpm

##################################################################
## describe the outputs

report_outputs:
	@echo $(BUILD)/$(OS_NAME)/$(ARCH)/$(MODE)/$(PACKAGE)-$(VER4)-$(RPMVER).$(ARCH_PACKAGE).rpm

