QT       += core gui gamepad
QT       +=network
QT      +=serialport
QT       += charts
QT += multimedia multimediawidgets
QT += location positioning
QT += qml quickwidgets
QT +=gamepad
QT += concurrent
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
LIBS += -lopencv_core -lopencv_imgproc -lopencv_calib3d
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    VirtualKeyboard.cpp \
    main.cpp \
    mavlinkclass.cpp \
    tracking.cpp \
    traversal.cpp \
    udpclass.cpp \
    udpvideo.cpp \
    widget.cpp

HEADERS += \
    VirtualKeyboard.h \
    mavlinkclass.h \
    tracking.h \
    traversal.h \
    udpclass.h \
    udpvideo.h \
    widget.h

FORMS += \
    widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += $$system(pkg-config --cflags-only-I opencv4 | sed 's/-I//g')
LIBS += $$system(pkg-config --libs opencv4)

RESOURCES += \
    qrc.qrc
