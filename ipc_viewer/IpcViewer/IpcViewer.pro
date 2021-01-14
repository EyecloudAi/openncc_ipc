#-------------------------------------------------
#
# Project created by QtCreator 2020-12-01T12:03:28
#
#-------------------------------------------------

QT       += core gui sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = IpcViewer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += $$PWD/../depends/cliSdk/inc

INCLUDEPATH += $$PWD/../3rd/ffmpeg4.1/linux/include

DEPENDPATH += $$PWD/../depends/cliSdk/inc


CONFIG += c++11 console

SOURCES += \
    aiutil.cpp \
    choosepkg.cpp \
    deviceagent.cpp \
    ipclog.cpp \
        main.cpp \
        mainwindow.cpp \
    playwidget.cpp \
    videodecodethread.cpp

HEADERS += \
    aiutil.h \
    choosepkg.h \
    deviceagent.h \
    ipclog.h \
        mainwindow.h \
        cli_sdk/cliSdk.h \
    playwidget.h \
    videodecodethread.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


unix:{

    CONFIG += link_pkgconfig
    PKGCONFIG += opencv
    PKGCONFIG += libusb-1.0

     LIBS += \
        -L$$PWD/../depends/cliSdk -lClientSdk \
        -L$$PWD/../3rd/ffmpeg4.1/linux/lib -lavutil -lavformat -lavcodec -lswresample -lswscale

}

RESOURCES += \
    lot_css.qrc
