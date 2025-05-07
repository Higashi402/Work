QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    autocontrastdialog.cpp \
    bmp_handler.cpp \
    brushtool.cpp \
    filemenuhandler.cpp \
    main.cpp \
    maintools.cpp \
    mainwindow.cpp \
    medianfilterdialog.cpp \
    q_image.cpp

HEADERS += \
    autocontrastdialog.h \
    bmp_handler.h \
    brushtool.h \
    filemenuhandler.h \
    maintools.h \
    mainwindow.h \
    medianfilterdialog.h \
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
