QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../library/tbc/lddecodemetadata.cpp \
    ../library/tbc/sourcevideo.cpp \
    ../library/tbc/vbidecoder.cpp \
    ../library/tbc/logging.cpp \
    datatypes/dropouts.cpp \
    detectionsources.cpp \
    detectors/clipdetector.cpp \
    detectors/diffdetector.cpp \
    main.cpp \
    mainwindow.cpp \
    tbcsource.cpp

HEADERS += \
    ../library/tbc/lddecodemetadata.h \
    ../library/tbc/sourcevideo.h \
    ../library/tbc/vbidecoder.h \
    ../library/tbc/logging.h \
    datatypes/dropouts.h \
    detectionsources.h \
    detectors/clipdetector.h \
    detectors/diffdetector.h \
    mainwindow.h \
    tbcsource.h

FORMS += \
    mainwindow.ui

# Add external includes to the include path
INCLUDEPATH += ../library/tbc

# Include git information definitions
isEmpty(BRANCH) {
    BRANCH = "unknown"
}
isEmpty(COMMIT) {
    COMMIT = "unknown"
}
DEFINES += APP_BRANCH=\"\\\"$${BRANCH}\\\"\" \
    APP_COMMIT=\"\\\"$${COMMIT}\\\"\"

# Rules for installation
isEmpty(PREFIX) {
    PREFIX = /usr/local
}
unix:!android: target.path = $$PREFIX/bin/
!isEmpty(target.path): INSTALLS += target

