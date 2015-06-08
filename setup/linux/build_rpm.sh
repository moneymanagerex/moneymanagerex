#!/bin/bash

. variables.sh

ARCHITECTURE="amd64"

BUILD_DIR="$HOME/rpmbuild"

PACKAGE_NAME="mmex-$MMEX_VERSION"

mkdir $BUILD_DIR
mkdir "$BUILD_DIR/SPECS"
mkdir "$BUILD_DIR/SOURCES"

#Create the spec file
echo "Name:           mmex
Version:        $MMEX_VERSION
Release:        1%{?dist}
Source:         https://github.com/moneymanagerex/moneymanagerex/archive/v%{version}.tar.gz
Summary:        $MMEX_SUMMARY
License:        GPLv2+
Icon:           mmex.xpm
URL:            $MMEX_HOMEPAGE
#Requires:    	$MMEX_RPM_DEPENDS

%description
$MMEX_DESCRIPTION


%prep
%autosetup
%{_builddir}/%{name}-%{version}/bootstrap.sh

%build
%configure
make %{?_smp_mflags}


%install
%make_install
desktop-file-validate %{buildroot}%{_datadir}/applications/mmex.desktop


%files
%{_bindir}/*
%{_datadir}/mmex/*
%{_datadir}/icons/hicolor/scalable/apps/mmex.svg
%{_datadir}/applications/mmex.desktop
%{_defaultdocdir}/mmex/*
%{_mandir}/man1/mmex.1.gz




%changelog" > "$BUILD_DIR/SPECS/mmex.spec"

cd ../..

#Copy icon
cp resources/mmex.xpm "$BUILD_DIR/SOURCES"

#Copy source
rm -rf "$BUILD_DIR/SOURCES/$PACKAGES_NAME"
rsync -av --progress * "$BUILD_DIR/SOURCES/$PACKAGE_NAME" --exclude .git --exclude compile

cd "$BUILD_DIR/SOURCES"

#Compress the source
tar -zcvf "v$MMEX_VERSION.tar.gz" $PACKAGE_NAME

#Build the package
cd "$BUILD_DIR/SPECS"
rpmbuild -bb mmex.spec

#Check for any packaging problems
#TODO: Run rpmlint
