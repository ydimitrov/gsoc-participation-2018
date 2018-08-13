TARGET = hvac
QT = quick qml

HEADERS += \
    translator.h

SOURCES = main.cpp \
    translator.cpp

RESOURCES += \
    hvac.qrc \
    images/images.qrc

include(app.pri)

LANGUAGES = ja_JP fr_FR
include(translations.pri)
