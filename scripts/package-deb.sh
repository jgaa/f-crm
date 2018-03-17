#!/bin/bash

# Compile and prepare a .deb package for distribution
# Uses the QT libraries from the lilnux ditribution
#
# Example:
#   F_CRM_VERSION="2.0.1" DIST_DIR=`pwd`/dist BUILD_DIR=`pwd`/build SRC_DIR=`pwd`/f-crm  ./f-crm/scripts/package-deb.sh

if [ -z "$F_CRM_VERSION" ]; then
    F_CRM_VERSION="0.0.1"
    echo "Warning: Missing F_CRM_VERSION variable!"
fi

if [ -z ${DIST_DIR:-} ]; then
    DIST_DIR=`pwd`/dist/linux
fi

if [ -z ${BUILD_DIR:-} ]; then
    BUILD_DIR=`pwd`/build
fi

if [ -z ${SRC_DIR:-} ]; then
# Just assume we are run from the scipts directory
    SRC_DIR=`pwd`/..
fi

echo "Building f-crm for linux into ${DIST_DIR} from ${SRC_DIR}"

rm -rf $DIST_DIR $BUILD_DIR

mkdir -p $DIST_DIR &&\
pushd $DIST_DIR &&\
mkdir -p $BUILD_DIR &&\
pushd $BUILD_DIR &&\
qmake $SRC_DIR/f-crm.pro &&\
make && make install &&\
popd &&\
fpm --input-type dir \
    --output-type deb \
    --force \
    --name f-crm \
    --version ${F_CRM_VERSION} \
    --vendor "The Last Viking LTD" \
    --description "Time Tracking for Freelancers and Independenet Contractors" \
    --depends qt5-default --depends libsqlite3-0 \
    --chdir ${DIST_DIR}/root/ \
    --package ${DIST_NAME}f-crm-VERSION_ARCH.deb &&\
echo "Debian package is available in $PWD" &&\
popd
