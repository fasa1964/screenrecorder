#-------------------------------------------------
#
# Project created by QtCreator 2020-10-06T14:42:45
#
#-------------------------------------------------

QT      += core gui
QT      += multimedia
QT      += network


win32:{
    QT      += winextras
}

linux:{
    QT      += dbus
}


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FScreenRecorder
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

RC_ICONS = FScreenRecorder.ico

SOURCES += \
    formmessagewidget.cpp \
        main.cpp \
        mainwindow.cpp \
    dialogvideolist.cpp \
    dialoginfo.cpp \
    slidertwohandle.cpp

HEADERS += \
    formmessagewidget.h \
        mainwindow.h \
    dialogvideolist.h \
    dialoginfo.h \
    slidertwohandle.h

FORMS += \
    formmessagewidget.ui \
        mainwindow.ui \
    dialogvideolist.ui \
    dialoginfo.ui

RESOURCES += \
    images.qrc

DISTFILES += \
    GPLV3.txt \
    FFMEGInfo.txt \
    ToDo.txt
