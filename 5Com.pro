#-------------------------------------------------
#
# Project created by QtCreator 2018-08-17T20:05:27
#
#-------------------------------------------------

QT       += core gui widgets serialport

TARGET = 5Com
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

CONFIG += c++11 static

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    hexview.cpp \
    asciitable.cpp \
    bytereceivetimesdialog.cpp \
    sendfiledialog.cpp \
    plaintextview.cpp

HEADERS += \
        mainwindow.h \
    hexview.h \
    asciitable.h \
    bytereceivetimesdialog.h \
    sendfiledialog.h \
    plaintextview.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32: QMAKE_LFLAGS_RELEASE += -static-libstdc++ -static-libgcc -static -lpthread

RESOURCES += \
    resources.qrc

win32:RC_ICONS = res/5Com.ico

unix:!macx: include(unixconf.pri)
