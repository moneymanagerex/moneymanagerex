# MMEX specification file for rpmbuild
# Copyright (C) 2009 VaDiM

Name:           mmex
Version:        0.9.9.0
Release:        1%{?dist}
Source:         http://www.codelathe.com/mmex/%{name}-%{version}.tar.gz
# Patch0:       mmex-x.x.x.x-bugfix1.patch
# Patch1:       mmex-x.x.x.x-bugfix2.patch
Summary:        Cross-platform simple to use financial management software
Group:          Applications/Productivity
License:        GPL2 or any later version
Icon:           mmex.xpm
URL:            http://www.codelathe.com/mmex
Packager:       VaDiM <kkkvadim@sourceforge.net>
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Prefix:         /usr
#Requires:    	libgtk-x11 >= 2.0

%description
Money Manager Ex is a free, open-source, cross-platform, easy-to-use personal 
finance software. It primarily helps organize one's finances and keeps track 
of where, when and how the money goes.

MMEX includes all the basic features that 90% of users would want to see in a
personal finance application. The design goals are to concentrate on simplicity
and user-friendliness - something one can use everyday.


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
%docdir /usr/share/doc/mmex
/usr/share/doc/mmex


%changelog
