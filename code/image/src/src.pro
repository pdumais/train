QT       += core gui
TEMPLATE = app

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport multimedia multimediawidgets

CONFIG += c++17

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += main.cpp


HEADERS +=


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

QMAKE_LFLAGS += -Wl,-rpath,"'\$$ORIGIN'"

LIBS += -L$$OUT_PWD/../lib/ -llib
LIBS += -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_highgui -lopencv_text
PRE_TARGETDEPS += $$OUT_PWD/../lib/liblib.so

INCLUDEPATH += $$PWD/../lib
DEPENDPATH += $$PWD/../lib
