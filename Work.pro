QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    bmp_handler.cpp \
    main.cpp \
    mainwindow.cpp \
    q_image.cpp

HEADERS += \
    bmp_handler.h \
    mainwindow.h \
    q_image.h

FORMS += \
    mainwindow.ui

INCLUDEPATH += C:\Users\Higashi\Desktop\Photon\tiff-4.7.0\build\libtiff
INCLUDEPATH += C:\Users\Higashi\Desktop\Photon\tiff-4.7.0\libtiff

LIBS += -LC:\Users\Higashi\Desktop\Photon\tiff-4.7.0\build\libtiff\Release -ltiff


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
