# MMEX specification file for rpmbuild
# Copyright (C) 2009 VaDiM

Name:           mmex
Version:        MMEX_VERSION
Release:        1%{?dist}
Source:         http://www.codelathe.com/mmex/%{name}-%{version}.tar.gz
# Patch0:       mmex-x.x.x.x-bugfix1.patch
# Patch1:       mmex-x.x.x.x-bugfix2.patch
Summary:        MMEX_SUMMARY
Group:          Applications/Productivity
License:        GPL2 or any later version
Icon:           mmex.xpm
URL:            MMEX_HOMEPAGE
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Prefix:         /usr
#Requires:    	libgtk-x11 >= 2.0

%description
MMEX_DESCRIPTION


%prep
%setup -q


%build
%configure
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
/usr/bin/mmex
/usr/share/mmex
/usr/share/icons/hicolor/scalable/apps
/usr/share/applications
%docdir /usr/share/doc/mmex
/usr/share/doc/mmex
/usr/share/man/man1


%changelog
