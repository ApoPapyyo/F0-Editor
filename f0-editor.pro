QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++23

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    f0.cpp \
    main.cpp \
    mainwindow.cpp \
    midi.cpp \
    midieditor.cpp \
    piano.cpp \
    pitcheditor.cpp

HEADERS += \
    f0.h \
    mainwindow.h \
    midi.h \
    midieditor.h \
    piano.h \
    pitcheditor.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    i18n/f0-editor_en.ts
CONFIG += lrelease
CONFIG += embed_translations

LIBS += -lm

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
