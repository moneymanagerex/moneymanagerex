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
License:        GPL2 or any later version
Icon:           mmex.xpm
URL:            $MMEX_HOMEPAGE
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Prefix:         /usr
Requires:    	$MMEX_RPM_DEPENDS

%description
$MMEX_DESCRIPTION


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


%changelog" > "$BUILD_DIR/SPECS"

cd ../../..

#Copy icon
cp resources/mmex.xpm "$BUILD_DIR/SOURCES"

#Copy source
cd ..
cp -r moneymanagerex "$BUILD_DIR/SOURCES/$PACKAGE_NAME"

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
