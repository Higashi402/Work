QT += core gui
QT += testlib


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
CONFIG += testcase

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dialogs/autocontrastdialog.cpp \
    bmp/bmp_handler.cpp \
    #tests/tst_medianfilter.cpp \
    dialogs/sobeldialog.cpp \
    tools/brushtool.cpp \
    filemenuhandler.cpp \
    main.cpp \
    tools/maintools.cpp \
    mainwindow.cpp \
    dialogs/medianfilterdialog.cpp \
    q_image.cpp

HEADERS += \
    dialogs/autocontrastdialog.h \
    bmp/bmp_handler.h \
    dialogs/sobeldialog.h \
    tools/brushtool.h \
    filemenuhandler.h \
    tools/maintools.h \
    mainwindow.h \
    dialogs/medianfilterdialog.h \
    q_image.h

FORMS += \
    mainwindow.ui

INCLUDEPATH += C:\Users\Higashi\Desktop\Photon\tiff-4.7.0\build\libtiff
INCLUDEPATH += C:\Users\Higashi\Desktop\Photon\tiff-4.7.0\libtiff

LIBS += -LC:\Users\Higashi\Desktop\Photon\tiff-4.7.0\build\libtiff\Release -ltiff


qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
