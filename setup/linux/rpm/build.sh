#!/bin/bash

. ../common/variables.sh

ARCHITECTURE="amd64"

BUILD_DIR="$HOME/rpmbuild"

PACKAGE_NAME="mmex-$MMEX_VERSION"

mkdir $BUILD_DIR
mkdir "$BUILD_DIR/SPECS"
mkdir "$BUILD_DIR/SOURCES"

#Create the spec file
cp mmex.spec "$BUILD_DIR/SPECS"

sed -i "s/MMEX_VERSION/$MMEX_VERSION/g" "$BUILD_DIR/SPECS/mmex.spec"
sed -i "s/MMEX_SUMMARY/$MMEX_SUMMARY/g" "$BUILD_DIR/SPECS/mmex.spec"
sed -i "s|MMEX_HOMEPAGE|$MMEX_HOMEPAGE|g" "$BUILD_DIR/SPECS/mmex.spec"
#TODO: Something in the description needs escaping
sed -i "s/MMEX_DESCRIPTION/$MMEX_DESCRIPTION/g" "$BUILD_DIR/SPECS/mmex.spec"

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
