#-------------------------------------------------
#
# Project created by QtCreator 2018-03-15T16:54:54
#
#-------------------------------------------------

QT       += core gui widgets sql
TARGET = f-crm
TEMPLATE = app
CONFIG += c++14

QMAKE_TARGET_COMPANY = The Last Viking LTD
QMAKE_TARGET_PRODUCT = f-crm
QMAKE_TARGET_DESCRIPTION = CRM for Independent Contractors and Freelancers
QMAKE_TARGET_COPYRIGHT = Copyright (c) 2018, 2019 by Jarle (jgaa) Aase

macx {
    ICON = res/icons/f-crm.icns
} win32 {
    RC_ICONS = res/icons/f_crm.ico
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
    src/contactsmodel.cpp \
    src/channelsmodel.cpp \
    src/channeldialog.cpp \
    src/channel.cpp \
    src/persondialog.cpp \
    src/contact.cpp \
    src/intent.cpp \
    src/intentsmodel.cpp \
    src/intentdialog.cpp \
    src/action.cpp \
    src/actiondialog.cpp \
    src/actionsmodel.cpp \
    src/utility.cpp \
    src/actionexecutedialog.cpp \
    src/document.cpp \
    src/documentsmodel.cpp \
    src/documentdialog.cpp \
    src/tableviewwithdrop.cpp \
    src/documentproxymodel.cpp \
    src/contactproxymodel.cpp \
    src/channelproxymodel.cpp \
    src/intentproxymodel.cpp \
    src/actionproxymodel.cpp \
    src/settingsdialog.cpp \
    src/journalmodel.cpp \
    src/journalproxymodel.cpp \
    src/favoritesdialog.cpp \
    src/upcomingmodel.cpp \
    src/aboutdialog.cpp

HEADERS += \
    src/mainwindow.h \
    src/database.h \
    src/logging.h \
    src/version.h \
    src/contactsmodel.h \
    src/strategy.h \
    src/release.h \
    src/channelsmodel.h \
    src/channeldialog.h \
    src/channel.h \
    src/persondialog.h \
    src/contact.h \
    src/intent.h \
    src/intentsmodel.h \
    src/intentdialog.h \
    src/action.h \
    src/actiondialog.h \
    src/actionsmodel.h \
    src/utility.h \
    src/actionexecutedialog.h \
    src/document.h \
    src/documentsmodel.h \
    src/documentdialog.h \
    src/tableviewwithdrop.h \
    src/documentproxymodel.h \
    src/contactproxymodel.h \
    src/channelproxymodel.h \
    src/intentproxymodel.h \
    src/actionproxymodel.h \
    src/settingsdialog.h \
    src/journalmodel.h \
    src/journalproxymodel.h \
    src/favoritesdialog.h \
    src/upcomingmodel.h \
    src/aboutdialog.h

FORMS += \
        ui/mainwindow.ui \
    ui/channeldialog.ui \
    ui/persondialog.ui \
    ui/intentdialog.ui \
    ui/actiondialog.ui \
    ui/actionexecutedialog.ui \
    ui/documentdialog.ui \
    ui/settingsdialog.ui \
    ui/favoritesdialog.ui \
    ui/aboutdialog.ui

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
    res/icons/f-crm.svg \
    res/icons/f_crm.ico \
    README.md


