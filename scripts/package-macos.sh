#!/bin/bash

# Compile and prepare a signed .dmg package for distribution
# Assimes the environment-variable QTDIR to point to the
# Qt installation
#
# Example:
#     Jarles-Mac-mini:scripts jgaa$ QTDIR=/Users/jgaa/Qt/5.10.0/clang_64 ./package-macos.sh

if [ -z "$WHID_VERSION" ]; then
    WHID_VERSION="2.0.0"
    echo "Warning: Missing WHID_VERSION variable!"
fi

if [ -z ${DIST_DIR:-} ];
then
    DIST_DIR=`pwd`/dist/macos
fi

if [ -z ${SIGN_CERT:-} ];
then
    SIGN_CERT="Developer ID Application"
fi

if [ -z ${BUILD_DIR:-} ]; then
    BUILD_DIR=`pwd`/build
fi

if [ -z ${SRC_DIR:-} ];
then
# Just assume we are run from the scipts directory
    SRC_DIR=`pwd`/..
fi

echo "Building whid for macos into ${DIST_DIR} from ${SRC_DIR}"

rm -rf $DIST_DIR $BUILD_DIR

mkdir -p $DIST_DIR && cd $DIST_DIR
mkdir -p $BUILD_DIR

pushd $BUILD_DIR

$QTDIR/bin/qmake \
    -spec macx-clang \
    "CONFIG += release x86_64" \
    $SRC_DIR/whid.pro

make -j4

popd

pushd $DIST_DIR

mv $BUILD_DIR/whid.app $BUILD_DIR/whid-${WHID_VERSION}.app

echo "Making dmg package with $QTDIR/bin/macdeployqt"
$QTDIR/bin/macdeployqt $BUILD_DIR/whid-${WHID_VERSION}.app -dmg -appstore-compliant -codesign="$SIGN_CERT"

mv  $BUILD_DIR/whid-${WHID_VERSION}.dmg .

popd
