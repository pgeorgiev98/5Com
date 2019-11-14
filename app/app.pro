QT += core gui widgets serialport network

TARGET = 5Com
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++17 static

SOURCES += \
    checkforupdatesdialog.cpp \
    common.cpp \
    continuoussendwindow.cpp \
    exportdialog.cpp \
    favoriteslistobject.cpp \
    favoritesselectionlist.cpp \
    inputfield.cpp \
    inputwithfavorites.cpp \
    keyboardshortcutsdialog.cpp \
    keyboardshortcutswindow.cpp \
    keysequenceinputwidget.cpp \
    line.cpp \
    main.cpp \
    mainwindow.cpp \
    hexview.cpp \
    asciitable.cpp \
    bytereceivetimesdialog.cpp \
    sendfiledialog.cpp \
    plaintextview.cpp \
    escapecodesdialog.cpp \
    latestreleasechecker.cpp \
    changelogdialog.cpp \
    sendsequencewindow.cpp \
    serialport.cpp \
    settingspage.cpp

HEADERS += \
    byteselection.h \
    checkforupdatesdialog.h \
    config.h \
    continuoussendwindow.h \
    exportdialog.h \
    favoriteslistobject.h \
    favoritesselectionlist.h \
    inputfield.h \
    inputwithfavorites.h \
    keyboardshortcutsdialog.h \
    keyboardshortcutswindow.h \
    keysequenceinputwidget.h \
    line.h \
    mainwindow.h \
    hexview.h \
    asciitable.h \
    bytereceivetimesdialog.h \
    sendfiledialog.h \
    plaintextview.h \
    common.h \
    escapecodesdialog.h \
    latestreleasechecker.h \
    changelogdialog.h \
    sendsequencewindow.h \
    serialport.h \
    settingspage.h

RESOURCES += resources.qrc

win32: QT += gui-private
win32: SOURCES += autoupdatedialog.cpp
win32: HEADERS += autoupdatedialog.h

win32:QMAKE_LFLAGS_RELEASE += -static-libstdc++ -static-libgcc -static -lpthread
win32:RC_ICONS = res/5Com.ico

unix:!macx: include(unixconf.pri)
