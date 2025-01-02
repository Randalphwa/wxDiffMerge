Summary: Visually compare and merge files.
Name: @PACKAGE@
Version: @MAJOR@.@MINOR@.@REVISION@.@BUILDNUM@@DOTLABEL@
Release: @RPMVER@
License: SourceGear
Group: Development/Tools
URL: www.sourcegear.com/diffmerge/index.html
Source0: %{name}-%{version}-%{release}.tar.gz
BuildRoot: @RPMBUILD@


%description
SourceGear DiffMerge is a free utility to visually compare and merge
files and folders.

%prep
%setup -q

%build

%define _unpackaged_files_terminate_build 0

%install
rm -rf $RPM_BUILD_ROOT/usr
mkdir -p $RPM_BUILD_ROOT/usr/bin
mkdir -p $RPM_BUILD_ROOT/usr/share/man/man1
mkdir -p $RPM_BUILD_ROOT/usr/share/doc/@PACKAGE@-@VER4@
mkdir -p $RPM_BUILD_ROOT/usr/share/pixmaps
mkdir -p $RPM_BUILD_ROOT/usr/share/applications

cp -pr $RPM_BUILD_DIR/@PACKAGE@-@VER4@/usr/bin/@PACKAGE@ $RPM_BUILD_ROOT/usr/bin/@PACKAGE@
cp -pr $RPM_BUILD_DIR/@PACKAGE@-@VER4@/usr/share $RPM_BUILD_ROOT/usr

rm -rf $RPM_BUILD_DIR/@PACKAGE@-@VER4@/usr

desktop-file-validate $RPM_BUILD_ROOT/usr/share/applications/sourcegear.com-@PACKAGE@.desktop

%clean

%pre
#echo This is PRE for %{version}-%{release}: arg=$1

%post
#echo This is POST for %{version}-%{release}: arg=$1

%preun
#echo This is PREUN for %{version}-%{release}: arg=$1

%postun
#echo This is POSTUN for %{version}-%{release}: arg=$1


%files
%defattr(-,root,root,-)

%dir /usr/share/doc/@PACKAGE@-@VER4@
%doc /usr/share/doc/@PACKAGE@-@VER4@/DiffMergeManual.pdf
%doc /usr/share/doc/@PACKAGE@-@VER4@/copyright
%doc /usr/share/doc/@PACKAGE@-@VER4@/Readme.txt

/usr/bin/@PACKAGE@
/usr/share/man/man1/@PACKAGE@.1.gz
/usr/share/pixmaps/sourcegearcom-@PACKAGE@.png
/usr/share/applications/sourcegear.com-@PACKAGE@.desktop

%changelog
