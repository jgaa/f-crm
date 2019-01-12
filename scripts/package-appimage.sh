#!/bin/bash

APP="f-crm"
ARCH="x86_64"

if [ -z "$F_CRM_VERSION" ]; then
    F_CRM_VERSION="2.0.0"
    echo "Warning: Missing F_CRM_VERSION variable!"
fi

if [ -z ${DIST_DIR:-} ];
then
    DIST_DIR=`pwd`/dist/linux-appimage
fi

if [ -z ${SIGN_CERT:-} ];
then
    SIGN_CERT="Developer ID Application"
fi

if [ -z ${BUILD_DIR:-} ]; then
    BUILD_DIR=`pwd`/build
fi

if [ -z ${BUILD_DIR:-} ]; then
    BUILD_DIR=`pwd`/build
fi

if [ -z ${QTDIR:-} ];
then
    QMAKE=qmake
else
    export QT_LIBRARY_PATH=$QTDIR/lib
    export QT_PLUGIN_PATH=$QTDIR/plugins/
    QMAKE=$QTDIR/bin/qmake
fi

echo "Building ${APP} for APPIMAGE into ${DIST_DIR} from ${SRC_DIR}"

rm -rf $DIST_DIR $BUILD_DIR

mkdir -p $DIST_DIR && cd $DIST_DIR
mkdir -p $BUILD_DIR

pushd $BUILD_DIR
BUILD_DIR=$(pwd)

$QMAKE \
    -spec  linux-g++  \
    "CONFIG += release x86_64" \
    $SRC_DIR/${APP}.pro

make

popd

pushd $DIST_DIR

APPIMAGE_DIR=$(pwd)/AppImage

echo "Constructing AppImage layout in: ${APPIMAGE_DIR}"

mkdir -p $APPIMAGE_DIR/usr/bin
cp -v $BUILD_DIR/${APP} $APPIMAGE_DIR/usr/bin
mkdir -p $APPIMAGE_DIR/usr/lib
mkdir -p $APPIMAGE_DIR/usr/share/applications
printf "[Desktop Entry]\nType=Application\nName=${APP}\nExec=${APP}\nIcon=${APP}\nCategories=Office;" > $APPIMAGE_DIR/usr/share/applications/${APP}.desktop
mkdir -p $APPIMAGE_DIR/usr/share/icons/default/scalable/apps
cp $SRC_DIR/res/icons/${APP}.svg $APPIMAGE_DIR/usr/share/icons/default/scalable/apps

pushd ${APPIMAGE_DIR}

linuxdeployqt \
    $APPIMAGE_DIR/usr/share/applications/${APP}.desktop \
    -qmake=$QMAKE \
    -exclude-libs=libqsqlmysql,libqsqlpsql \
    -extra-plugins=iconengines,imageformats \
    -appimage

mv ${APP}-${ARCH}.AppImage $DIST_DIR/${APP}-${ARCH}-${F_CRM_VERSION}.AppImage

popd
popd
