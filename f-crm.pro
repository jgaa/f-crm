#-------------------------------------------------
#
# Project created by QtCreator 2018-03-15T16:54:54
#
#-------------------------------------------------

QT       += core gui widgets sql

TARGET = f-crm
TEMPLATE = app

QMAKE_TARGET_COMPANY = The Last Viking LTD
QMAKE_TARGET_PRODUCT = f-crm
QMAKE_TARGET_DESCRIPTION = CRM for Independent Contractors and Freelancers
QMAKE_TARGET_COPYRIGHT = Copyright (c) 2018 by Jarle (jgaa) Aase

macx {
    ICON = res/icons/f-crm.icns
} win32 {
    RC_ICONS = res/icons/f-crm.ico
    VERSION = $$(F_CRM_VERSION).$$(BUILD_NUMBER)
} unix:!macx {
    ICON = res/icons/f-crm.svg
}

linux {
    DIST_DIR = $$(DIST_DIR)
    isEmpty(DIST_DIR) {
        DIST_DIR = $$_PRO_FILE_PWD_/../dist/desktop-linux
    }
    target.path = $${DIST_DIR}/root/usr/bin
    INSTALLS += target
}


# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/database.cpp \
    src/logging.cpp \
    src/contactsmodel.cpp

HEADERS += \
    src/mainwindow.h \
    src/database.h \
    src/logging.h \
    src/version.h \
    src/contactsmodel.h \
    src/strategy.h \
    src/release.h

FORMS += \
        ui/mainwindow.ui

RESOURCES += \
    resources.qrc

DISTFILES += \
    ci/jenkins/Dockefile.debian-stretch \
    ci/jenkins/Dockefile.debian-testing \
    ci/jenkins/Dockefile.ubuntu-xenial \
    ci/jenkins/Jenkinsfile.groovy \
    scripts/package-deb.sh \
    scripts/package-macos.sh \
    scripts/package-windows.bat \
    res/icons/f-crm.icns \
    res/icons/f-crm.ico

