QT       += core gui sql charts printsupport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GCentreFormation
TEMPLATE = app

CONFIG += c++17 utf8_source

DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += src

SOURCES += \
    src/main.cpp \
    src/gcentreformation.cpp \
    src/connection.cpp \
    src/formateur.cpp \
    src/cours.cpp \
    src/authentification.cpp \
    src/login.cpp

HEADERS += \
    src/gcentreformation.h \
    src/connection.h \
    src/formateur.h \
    src/cours.h \
    src/authentification.h \
    src/login.h

FORMS += \
    ui/gcentreformation.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
