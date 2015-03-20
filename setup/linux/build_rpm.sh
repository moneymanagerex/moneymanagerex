#!/bin/bash

. common/variables.sh

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
Source:         http://www.codelathe.com/mmex/%{name}-%{version}.tar.gz
Summary:        $MMEX_SUMMARY
Group:          Applications/Productivity
License:        GPLv2+
Icon:           mmex.xpm
URL:            $MMEX_HOMEPAGE
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
#Requires:    	$MMEX_RPM_DEPENDS

%description
$MMEX_DESCRIPTION


%prep
%setup -q
./bootstrap


%build
%configure
make %{?_smp_mflags}


%install
rm -rf %{buildroot}
make install DESTDIR=%{buildroot}


%clean
rm -rf %{buildroot}

%find_lang %{name}

%files -f %{name}.lang
%defattr(-,root,root,-)
%{_bindir}/*
%{_datadir}/*
/usr/share/icons/hicolor/scalable/apps
/usr/share/applications
%docdir /usr/share/doc/mmex
/usr/share/doc/mmex/*
%{_mandir}/man1/*


%changelog" > "$BUILD_DIR/SPECS/mmex.spec"

cd ../..

#Copy icon
cp resources/mmex.xpm "$BUILD_DIR/SOURCES"

#Copy source
cd ..
cp -r moneymanagerex "$BUILD_DIR/SOURCES/$PACKAGE_NAME"
#No need to copy the git stuff over
rm -rf "$BUILD_DIR/SOURCES/$PACKAGE_NAME/.git"

cd "$BUILD_DIR/SOURCES"

#Copy the manpage in to place and modify
cp "$PACKAGE_NAME/setup/linux/common/mmex.1" "$PACKAGE_NAME/doc/mmex.1"
sed -i "s/MMEX_VERSION/$MMEX_VERSION/g" "$PACKAGE_NAME/doc/mmex.1"
sed -i "s/MMEX_RELEASE_DATE/$MMEX_RELEASE_DATE/g" "$PACKAGE_NAME/doc/mmex.1"
gzip -9 -f "$PACKAGE_NAME/doc/mmex.1"
chmod 644 "$PACKAGE_NAME/doc/mmex.1.gz"

#Compress the source
tar -zcvf "$PACKAGE_NAME.tar.gz" $PACKAGE_NAME

#Build the package
cd "$BUILD_DIR/SPECS"
rpmbuild -bb mmex.spec

#Check for any packaging problems
#TODO: Run rpmlint
