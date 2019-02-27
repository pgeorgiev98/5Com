QT += core gui widgets serialport network

TARGET = 5Com
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11 static

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    hexview.cpp \
    asciitable.cpp \
    bytereceivetimesdialog.cpp \
    sendfiledialog.cpp \
    plaintextview.cpp \
    escapecodesdialog.cpp \
    latestreleasechecker.cpp \
    changelogdialog.cpp

HEADERS += \
    mainwindow.h \
    hexview.h \
    asciitable.h \
    bytereceivetimesdialog.h \
    sendfiledialog.h \
    plaintextview.h \
    common.h \
    escapecodesdialog.h \
    latestreleasechecker.h \
    changelogdialog.h

RESOURCES += resources.qrc

win32:QMAKE_LFLAGS_RELEASE += -static-libstdc++ -static-libgcc -static -lpthread
win32:RC_ICONS = res/5Com.ico

unix:!macx: include(unixconf.pri)
