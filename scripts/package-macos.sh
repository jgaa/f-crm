#!/bin/bash

# Compile and prepare a signed .dmg package for distribution
# Assimes the environment-variable QTDIR to point to the
# Qt installation
#
# Example:
#     Jarles-Mac-mini:scripts jgaa$ QTDIR=/Users/jgaa/Qt/5.10.0/clang_64 ./package-macos.sh

APP="f-crm"

if [ -z "$F_CRM_VERSION" ]; then
    F_CRM_VERSION="2.0.0"
    echo "Warning: Missing F_CRM_VERSION variable!"
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

echo "Building ${APP} for macos into ${DIST_DIR} from ${SRC_DIR}"

rm -rf $DIST_DIR $BUILD_DIR

mkdir -p $DIST_DIR && cd $DIST_DIR
mkdir -p $BUILD_DIR

pushd $BUILD_DIR

$QTDIR/bin/qmake \
    -spec macx-clang \
    "CONFIG += release x86_64" \
    $SRC_DIR/${APP}.pro

make -j8

popd

pushd $DIST_DIR

mv $BUILD_DIR/${APP}.app $BUILD_DIR/${APP}-${F_CRM_VERSION}.app

echo "Making dmg package with $QTDIR/bin/macdeployqt"
#$QTDIR/bin/macdeployqt $BUILD_DIR/${APP}-${F_CRM_VERSION}.app -dmg -appstore-compliant -codesign="$SIGN_CERT"

$QTDIR/bin/macdeployqt $BUILD_DIR/${APP}-${F_CRM_VERSION}.app -dmg -appstore-compliant

rm $BUILD_DIR/${APP}-${F_CRM_VERSION}.app/Contents/PlugIns/sqldrivers/libqsqlmysql.dylib

$QTDIR/bin/macdeployqt $BUILD_DIR/${APP}-${F_CRM_VERSION}.app -dmg -always-overwrite codesign="$SIGN_CERT"


mv  $BUILD_DIR/${APP}-${F_CRM_VERSION}.dmg .

popd
