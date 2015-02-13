#!/bin/bash

. ../common/variables.sh

ARCHITECTURE="amd64"

BUILD_DIR="$HOME/rpmbuild"

PACKAGE_NAME="mmex-$MMEX_VERSION"

mkdir $BUILD_DIR
mkdir "$BUILD_DIR/SPECS"
mkdir "$BUILD_DIR/SOURCES"

cp mmex.spec "$BUILD_DIR/SPECS"

sed -i "s/MMEX_VERSION/$MMEX_VERSION/g" "$BUILD_DIR/SPECS/mmex.spec"
sed -i "s/MMEX_SUMMARY/$MMEX_SUMMARY/g" "$BUILD_DIR/SPECS/mmex.spec"
#TODO: Need to escape the url
sed -i "s/MMEX_HOMEPAGE/$MMEX_HOMEPAGE/g" "$BUILD_DIR/SPECS/mmex.spec"
#TODO: Something in the description needs escaping
sed -i "s/MMEX_DESCRIPTION/$MMEX_DESCRIPTION/g" "$BUILD_DIR/SPECS/mmex.spec"

cd ../../..

cp resources/mmex.xpm "$BUILD_DIR/SOURCES"

./bootstrap
if [ $? -gt 0 ]; then
    echo "ERROR!"
    exit 1
fi

./configure --prefix="$BUILD_DIR/SOURCES/$PACKAGE_NAME/usr"

if [ $? -gt 0 ]; then
    echo "ERROR!"
    exit 1
fi
make && make install
if [ $? -gt 0 ]; then
    echo "ERROR!"
    exit 1
fi

cd "$BUILD_DIR/SOURCES"

tar -zcvf "$PACKAGE_NAME.tar.gz" $PACKAGE_NAME

cd "$BUILD_DIR/SPECS"
rpmbuild -ba mmex.spec
