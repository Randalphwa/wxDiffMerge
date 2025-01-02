## Config/ConfigExecutable.Linux.ubuntu.inc -- GNU Makefile snippets for
## building an executable and package as a .DEB for Ubuntu.
##################################################################

DEBIAN_LOWER:=	$(BUILD)/$(OS_NAME)/$(ARCH)/$(MODE)/tmp/$(VER4)/debian
DEBIAN_PACKAGE:=$(DEBIAN_LOWER)/$(PACKAGE)
DEBIAN_UPPER:=	$(DEBIAN_LOWER)/$(PACKAGE)/DEBIAN
DOC:=		$(DEBIAN_PACKAGE)/usr/share/doc/$(PACKAGE)

DEBNAME:=	$(PACKAGE)_$(VER4)_$(ARCH_PACKAGE).deb
DEBPATH:=	$(BUILD)/$(OS_NAME)/$(ARCH)/$(MODE)/$(DEBNAME)

RFCDATE:=	$(shell /bin/date -R)

UBUNTU_RELEASE:=$(shell uname -r)

##################################################################

ifeq ($(MODE),Release)
default:	$(DEBPATH)
else
default:	$(TARGET)
endif

##################################################################

include ../Config/ConfigCompile.inc

##################################################################

MY_EXEDEPS+=	../Config/ConfigExecutable.$(OS_NAME).$(FLAVOR).inc

$(TARGET):	$(OBJS) $(MY_EXEDEPS)
	@[ -d $(BINDIR) ] || mkdir -p $(BINDIR)
	@echo Linking $@
	@# wxWidgets adds every -l it can think of to the linker line
	@# whether it needs them or not.  this is harmless, but it
	@# causes dpkg-shlibdeps to complain about useless dependencies
	@# when building the .deb package because they are actually
	@# enforced when our package is installed.  so the following
	@# hack strips them out.
	@(							\
	  X="$(MY_LD_FLAGS) $(MY_LD_LIBS) $(WX_CXX_LIBS)";	\
	  Y=`echo $$X				\
	     | sed 's|-ljpeg||g'		\
	     | sed 's|-latk-1.0||g'		\
	     | sed 's|-lrt||g'			\
	     | sed 's|-lgmodule-2.0||g'		\
	     | sed 's|-lfreetype||g'		\
	     `;	   				\
	  $(WX_CXX) -o $(TARGET) $(OBJS) $$Y -luuid;	\
	  )
ifeq ($(MODE),Release)
	@strip --strip-all --remove-section=.comment --remove-section=.note $(TARGET)
endif

##################################################################

DEB_DIRS:=	$(DEBIAN_PACKAGE) \
		$(DEBIAN_PACKAGE)/usr/bin \
		$(DEBIAN_PACKAGE)/usr/share/doc/$(PACKAGE) \
		$(DEBIAN_PACKAGE)/usr/share/doc-base \
		$(DEBIAN_PACKAGE)/usr/share/menu \
		$(DEBIAN_PACKAGE)/usr/share/man/man1 \
		$(DEBIAN_PACKAGE)/usr/share/pixmaps \
		$(DEBIAN_PACKAGE)/usr/share/icons/hicolor/48x48/apps \
		$(DEBIAN_PACKAGE)/usr/share/applications \
		$(DEBIAN_PACKAGE)/usr/share/application-registry \
		$(DEBIAN_UPPER) \

## TODO update Linux.debian.changelog


debian_image_setup:	$(TARGET)
	@echo Deleting previous debian image structure in $(DEBIAN_LOWER)
	@rm -rf $(DEBIAN_LOWER)
	@#
	@mkdir -p $(DEBIAN_LOWER)
	@#
	@echo Setting up debian image structure in $(DEBIAN_PACKAGE)
	@for dir in $(DEB_DIRS); do \
		[ -d $$dir ] || mkdir -p $$dir; \
	done
	@find $(DEBIAN_PACKAGE) -type d | xargs chmod 755
	@#
	@cp ../Installers/$(OS_NAME).debian.postinst $(DEBIAN_UPPER)/postinst
	@chmod 755 $(DEBIAN_UPPER)/postinst
	@#
	@cp ../Installers/$(OS_NAME).debian.postrm $(DEBIAN_UPPER)/postrm
	@chmod 755 $(DEBIAN_UPPER)/postrm
	@#
	@cp $(TARGET) $(DEBIAN_PACKAGE)/usr/bin
	@chmod 755  $(DEBIAN_PACKAGE)/usr/bin/$(TARGET_EXE)
	@#
	@cp ../Installers/DiffMergeManual.pdf $(DOC)
	@chmod 644 $(DOC)/DiffMergeManual.pdf
	@#
	@cat ../Installers/$(OS_NAME).debian.doc-base		\
	| sed 's|@MAJOR@|$(BUILD_MAJOR)|g'			\
	| sed 's|@MINOR@|$(BUILD_MINOR)|g'			\
	| sed 's|@REVISION@|$(BUILD_REVISION)|g'		\
	| sed 's|@BUILDNUM@|$(BUILD_BUILDNUM)|g'		\
	| sed 's|@COPYRIGHTDATE@|$(BUILD_COPYRIGHTDATE)|g'	\
	| sed 's|@BUILDLABEL@|$(BUILD_BUILDLABEL)|g'		\
	| sed 's|@CSID@|$(BUILD_CSID)|g'			\
	| sed 's|@PACKAGE@|$(PACKAGE)|g'			\
	| sed 's|@ARCH@|$(ARCH)|g'				\
	| sed 's|@ARCH_PACKAGE@|$(ARCH_PACKAGE)|g'		\
	| sed 's|@EXE@|/usr/bin/$(TARGET_EXE)|g'		\
	| sed 's|@DEBNAME@|$(DEBNAME)|g'			\
	> $(DEBIAN_PACKAGE)/usr/share/doc-base/$(PACKAGE)
	@chmod 644 $(DEBIAN_PACKAGE)/usr/share/doc-base/$(PACKAGE)
	@#
	@cat ../Installers/$(OS_NAME).debian.Readme.txt		\
	| sed 's|@MAJOR@|$(BUILD_MAJOR)|g'			\
	| sed 's|@MINOR@|$(BUILD_MINOR)|g'			\
	| sed 's|@REVISION@|$(BUILD_REVISION)|g'		\
	| sed 's|@BUILDNUM@|$(BUILD_BUILDNUM)|g'		\
	| sed 's|@COPYRIGHTDATE@|$(BUILD_COPYRIGHTDATE)|g'	\
	| sed 's|@BUILDLABEL@|$(BUILD_BUILDLABEL)|g'		\
	| sed 's|@CSID@|$(BUILD_CSID)|g'			\
	| sed 's|@PACKAGE@|$(PACKAGE)|g'			\
	| sed 's|@ARCH@|$(ARCH)|g'				\
	| sed 's|@ARCH_PACKAGE@|$(ARCH_PACKAGE)|g'		\
	| sed 's|@EXE@|/usr/bin/$(TARGET_EXE)|g'		\
	| sed 's|@DEBNAME@|$(DEBNAME)|g'			\
	| sed 's|@UBUNTU_RELEASE@|$(UBUNTU_RELEASE)|g'		\
	> $(DOC)/README.Debian
	@chmod 644 $(DOC)/README.Debian
	@#
	@cat ../Installers/$(OS_NAME).debian.copyright		\
	| sed 's|@MAJOR@|$(BUILD_MAJOR)|g'			\
	| sed 's|@MINOR@|$(BUILD_MINOR)|g'			\
	| sed 's|@REVISION@|$(BUILD_REVISION)|g'		\
	| sed 's|@BUILDNUM@|$(BUILD_BUILDNUM)|g'		\
	| sed 's|@COPYRIGHTDATE@|$(BUILD_COPYRIGHTDATE)|g'	\
	| sed 's|@BUILDLABEL@|$(BUILD_BUILDLABEL)|g'		\
	| sed 's|@CSID@|$(BUILD_CSID)|g'			\
	| sed 's|@PACKAGE@|$(PACKAGE)|g'			\
	| sed 's|@ARCH@|$(ARCH)|g'				\
	| sed 's|@ARCH_PACKAGE@|$(ARCH_PACKAGE)|g'		\
	| sed 's|@EXE@|/usr/bin/$(TARGET_EXE)|g'		\
	| sed 's|@DEBNAME@|$(DEBNAME)|g'			\
	> $(DOC)/copyright
	@chmod 644 $(DOC)/copyright
	@#
	@cat ../Installers/$(OS_NAME).debian.changelog		\
	| sed 's|@MAJOR@|$(BUILD_MAJOR)|g'			\
	| sed 's|@MINOR@|$(BUILD_MINOR)|g'			\
	| sed 's|@REVISION@|$(BUILD_REVISION)|g'		\
	| sed 's|@BUILDNUM@|$(BUILD_BUILDNUM)|g'		\
	| sed 's|@COPYRIGHTDATE@|$(BUILD_COPYRIGHTDATE)|g'	\
	| sed 's|@BUILDLABEL@|$(BUILD_BUILDLABEL)|g'		\
	| sed 's|@CSID@|$(BUILD_CSID)|g'			\
	| sed 's|@PACKAGE@|$(PACKAGE)|g'			\
	| sed 's|@ARCH@|$(ARCH)|g'				\
	| sed 's|@ARCH_PACKAGE@|$(ARCH_PACKAGE)|g'		\
	| sed 's|@EXE@|/usr/bin/$(TARGET_EXE)|g'		\
	| sed 's|@DEBNAME@|$(DEBNAME)|g'			\
	| sed 's|@RFCDATE@|$(RFCDATE)|g'			\
	| gzip --stdout --best > $(DOC)/changelog.gz
	@chmod 644 $(DOC)/changelog.gz
	@#
#### Since we are no longer using -@DEBVER@ in the package name
#### we no longer look like a downstream package and now look
#### like a native package.  So we don't need the secondary
#### debian changelog.
####	@cat ../Installers/$(OS_NAME).debian.changelog.Debian	\
####	| sed 's|@MAJOR@|$(BUILD_MAJOR)|g'			\
####	| sed 's|@MINOR@|$(BUILD_MINOR)|g'			\
####	| sed 's|@REVISION@|$(BUILD_REVISION)|g'		\
####	| sed 's|@BUILDNUM@|$(BUILD_BUILDNUM)|g'		\
####	| sed 's|@COPYRIGHTDATE@|$(BUILD_COPYRIGHTDATE)|g'	\
####	| sed 's|@BUILDLABEL@|$(BUILD_BUILDLABEL)|g'		\
####	| sed 's|@PACKAGE@|$(PACKAGE)|g'			\
####	| sed 's|@ARCH@|$(ARCH)|g'				\
####	| sed 's|@ARCH_PACKAGE@|$(ARCH_PACKAGE)|g'		\
####	| sed 's|@EXE@|/usr/bin/$(TARGET_EXE)|g'		\
####	| sed 's|@DEBNAME@|$(DEBNAME)|g'			\
####	| sed 's|@RFCDATE@|$(RFCDATE)|g'			\
####	| gzip --stdout --best > $(DOC)/changelog.Debian.gz
####	@chmod 644 $(DOC)/changelog.Debian.gz
	@#
	@cat ../Installers/$(OS_NAME).debian.menu		\
	| sed 's|@MAJOR@|$(BUILD_MAJOR)|g'			\
	| sed 's|@MINOR@|$(BUILD_MINOR)|g'			\
	| sed 's|@REVISION@|$(BUILD_REVISION)|g'		\
	| sed 's|@BUILDNUM@|$(BUILD_BUILDNUM)|g'		\
	| sed 's|@COPYRIGHTDATE@|$(BUILD_COPYRIGHTDATE)|g'	\
	| sed 's|@BUILDLABEL@|$(BUILD_BUILDLABEL)|g'		\
	| sed 's|@CSID@|$(BUILD_CSID)|g'			\
	| sed 's|@PACKAGE@|$(PACKAGE)|g'			\
	| sed 's|@ARCH@|$(ARCH)|g'				\
	| sed 's|@ARCH_PACKAGE@|$(ARCH_PACKAGE)|g'		\
	| sed 's|@EXE@|/usr/bin/$(TARGET_EXE)|g'		\
	| sed 's|@DEBNAME@|$(DEBNAME)|g'			\
	> $(DEBIAN_PACKAGE)/usr/share/menu/$(PACKAGE)
	@chmod 644 $(DEBIAN_PACKAGE)/usr/share/menu/$(PACKAGE)
	@#
## 2011/12/02 I can't figure out this Ubuntu/debian desktop/menu crap
##            and can't find any decent documentation and can't
##            tell which of the other applications on the system
##            are "new style (as of some release)" and which are
##            legacy.
##
##            For 10.04, I put the XPM in Linux.debian.menu *and*
##            in Linux.debian.desktop and all was well.
##
##            In 11.10, the XPM in .desktop doesn't do anything
##            and we get a big "question mark" icon in the new
##            launcher/dash.  
##
##            If I switch it in .desktop to a PNG it seems to 
##            work with launcher/dash in 11.10, but lintian complains
##            if I also switch the item in the .menu.  So now we have
##            both.
	@#
	@cp ../Installers/$(OS_NAME).debian.DiffMergeIcon48.png $(DEBIAN_PACKAGE)/usr/share/icons/hicolor/48x48/apps/$(PACKAGE).png
	@chmod 644 $(DEBIAN_PACKAGE)/usr/share/icons/hicolor/48x48/apps/$(PACKAGE).png
	@cp ../Installers/$(OS_NAME).debian.DiffMergeIcon32.xpm $(DEBIAN_PACKAGE)/usr/share/pixmaps/$(PACKAGE).xpm
	@chmod 644 $(DEBIAN_PACKAGE)/usr/share/pixmaps/$(PACKAGE).xpm
	@#
	@cat ../Installers/$(OS_NAME).debian.man1		\
	| sed 's|@MAJOR@|$(BUILD_MAJOR)|g'			\
	| sed 's|@MINOR@|$(BUILD_MINOR)|g'			\
	| sed 's|@REVISION@|$(BUILD_REVISION)|g'		\
	| sed 's|@BUILDNUM@|$(BUILD_BUILDNUM)|g'		\
	| sed 's|@COPYRIGHTDATE@|$(BUILD_COPYRIGHTDATE)|g'	\
	| sed 's|@BUILDLABEL@|$(BUILD_BUILDLABEL)|g'		\
	| sed 's|@CSID@|$(BUILD_CSID)|g'			\
	| sed 's|@PACKAGE@|$(PACKAGE)|g'			\
	| sed 's|@ARCH@|$(ARCH)|g'				\
	| sed 's|@ARCH_PACKAGE@|$(ARCH_PACKAGE)|g'		\
	| sed 's|@EXE@|/usr/bin/$(TARGET_EXE)|g'		\
	| sed 's|@DEBNAME@|$(DEBNAME)|g'			\
	| gzip --stdout --best 					\
	> $(DEBIAN_PACKAGE)/usr/share/man/man1/$(PACKAGE).1.gz
	@chmod 644 $(DEBIAN_PACKAGE)/usr/share/man/man1/$(PACKAGE).1.gz
	@#
	@cat ../Installers/$(OS_NAME).debian.desktop		\
	| sed 's|@MAJOR@|$(BUILD_MAJOR)|g'			\
	| sed 's|@MINOR@|$(BUILD_MINOR)|g'			\
	| sed 's|@REVISION@|$(BUILD_REVISION)|g'		\
	| sed 's|@BUILDNUM@|$(BUILD_BUILDNUM)|g'		\
	| sed 's|@COPYRIGHTDATE@|$(BUILD_COPYRIGHTDATE)|g'	\
	| sed 's|@BUILDLABEL@|$(BUILD_BUILDLABEL)|g'		\
	| sed 's|@CSID@|$(BUILD_CSID)|g'			\
	| sed 's|@PACKAGE@|$(PACKAGE)|g'			\
	| sed 's|@ARCH@|$(ARCH)|g'				\
	| sed 's|@ARCH_PACKAGE@|$(ARCH_PACKAGE)|g'		\
	| sed 's|@EXE@|/usr/bin/$(TARGET_EXE)|g'		\
	| sed 's|@DEBNAME@|$(DEBNAME)|g'			\
	> $(DEBIAN_PACKAGE)/usr/share/applications/$(PACKAGE).desktop
	@chmod 644 $(DEBIAN_PACKAGE)/usr/share/applications/$(PACKAGE).desktop
	@#
	@cat ../Installers/$(OS_NAME).debian.applications	\
	| sed 's|@MAJOR@|$(BUILD_MAJOR)|g'			\
	| sed 's|@MINOR@|$(BUILD_MINOR)|g'			\
	| sed 's|@REVISION@|$(BUILD_REVISION)|g'		\
	| sed 's|@BUILDNUM@|$(BUILD_BUILDNUM)|g'		\
	| sed 's|@COPYRIGHTDATE@|$(BUILD_COPYRIGHTDATE)|g'	\
	| sed 's|@BUILDLABEL@|$(BUILD_BUILDLABEL)|g'		\
	| sed 's|@CSID@|$(BUILD_CSID)|g'			\
	| sed 's|@PACKAGE@|$(PACKAGE)|g'			\
	| sed 's|@ARCH@|$(ARCH)|g'				\
	| sed 's|@ARCH_PACKAGE@|$(ARCH_PACKAGE)|g'		\
	| sed 's|@EXE@|/usr/bin/$(TARGET_EXE)|g'		\
	| sed 's|@DEBNAME@|$(DEBNAME)|g'			\
	> $(DEBIAN_PACKAGE)/usr/share/application-registry/$(PACKAGE).applications
	@chmod 644 $(DEBIAN_PACKAGE)/usr/share/application-registry/$(PACKAGE).applications
	@#
	@cat ../Installers/$(OS_NAME).debian.source.control	\
	| sed 's|@MAJOR@|$(BUILD_MAJOR)|g'			\
	| sed 's|@MINOR@|$(BUILD_MINOR)|g'			\
	| sed 's|@REVISION@|$(BUILD_REVISION)|g'		\
	| sed 's|@BUILDNUM@|$(BUILD_BUILDNUM)|g'		\
	| sed 's|@COPYRIGHTDATE@|$(BUILD_COPYRIGHTDATE)|g'	\
	| sed 's|@BUILDLABEL@|$(BUILD_BUILDLABEL)|g'		\
	| sed 's|@PACKAGE@|$(PACKAGE)|g'			\
	| sed 's|@ARCH@|$(ARCH)|g'				\
	| sed 's|@ARCH_PACKAGE@|$(ARCH_PACKAGE)|g'		\
	| sed 's|@EXE@|/usr/bin/$(TARGET_EXE)|g'		\
	| sed 's|@DEBNAME@|$(DEBNAME)|g'			\
	> $(DEBIAN_LOWER)/control
	@#
	@cat ../Installers/$(OS_NAME).debian.binary.control	\
	| sed 's|@MAJOR@|$(BUILD_MAJOR)|g'			\
	| sed 's|@MINOR@|$(BUILD_MINOR)|g'			\
	| sed 's|@REVISION@|$(BUILD_REVISION)|g'		\
	| sed 's|@BUILDNUM@|$(BUILD_BUILDNUM)|g'		\
	| sed 's|@COPYRIGHTDATE@|$(BUILD_COPYRIGHTDATE)|g'	\
	| sed 's|@BUILDLABEL@|$(BUILD_BUILDLABEL)|g'		\
	| sed 's|@CSID@|$(BUILD_CSID)|g'			\
	| sed 's|@PACKAGE@|$(PACKAGE)|g'			\
	| sed 's|@ARCH@|$(ARCH)|g'				\
	| sed 's|@ARCH_PACKAGE@|$(ARCH_PACKAGE)|g'		\
	| sed 's|@EXE@|/usr/bin/$(TARGET_EXE)|g'		\
	| sed 's|@DEBNAME@|$(DEBNAME)|g'			\
	> $(DEBIAN_UPPER)/control
	@chmod 644 $(DEBIAN_UPPER)/control
	@#
	@(cd $(DEBIAN_LOWER)/..; dpkg-shlibdeps -e$(DEBIAN_PACKAGE)/usr/bin/$(PACKAGE) )
	@#
	@(cd $(DEBIAN_LOWER);                                      \
	  X=`cat substvars | sed 's|shlibs:Depends=|Depends: |'`;  \
	  rm substvars;                                            \
	  mv control control.tmp;                                  \
	  cat control.tmp | sed "s|^Depends: .*|$$X|" > control;   \
	  rm control.tmp;                                          \
	  cd $(DEBIAN_UPPER);                                      \
	  mv control control.tmp;                                  \
	  cat control.tmp | sed "s|^Depends: .*|$$X|" > control;   \
	  rm control.tmp;                                          \
	  )
	@#
	@(cd $(DEBIAN_PACKAGE); find usr -type f | xargs md5sum) > $(DEBIAN_UPPER)/md5sums
	@chmod 644 $(DEBIAN_UPPER)/md5sums

$(DEBPATH):	debian_image_setup
	@echo Creating $(DEBNAME)
	@fakeroot dpkg-deb --build $(DEBIAN_PACKAGE) $(DEBPATH)
	@#
	@echo Running lintian
	@lintian -iIv $(DEBPATH)

##################################################################

clean:
	@rm -f $(OBJS)

##################################################################

.PHONY:	clean run debian_image_setup

##################################################################
## describe the outputs

report_outputs:
	@echo $(DEBPATH)


