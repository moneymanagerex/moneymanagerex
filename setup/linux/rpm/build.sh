#!/bin/bash

. ../common/variables.sh

ARCHITECTURE="amd64"

BUILD_DIR="$HOME/rpmbuild"

PACKAGE_NAME="mmex-$MMEX_VERSION"

cp mmex.spec "$BUILD_DIR/SPECS"

cd ../../../..

cp mmex "$BUILD_DIR/SOURCES"

tar -zcvf "$BUILD_DIR/SOURCES/$PACKAGE_NAME.tar.gz" "$BUILD_DIR/mmex"

cd "$BUILD_DIR/SPECS"
rpmbuild -ba mmex.spec
