QT += testlib widgets serialport multimedia multimediawidgets
QT -= gui

CONFIG += c++17
CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

SOURCES +=  tst_tests.cpp \
    MockTrainController.cpp \
    MockVisionService.cpp \
    MockDisplayService.cpp

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../lib/release/ -llib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../lib/debug/ -llib
else:unix: LIBS += -L$$OUT_PWD/../lib/ -llib

INCLUDEPATH += $$PWD/../lib
DEPENDPATH += $$PWD/../lib

LIBS += -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_highgui -lopencv_text -lopencv_video
PRE_TARGETDEPS += $$OUT_PWD/../lib/liblib.so

HEADERS += \
    MockTrainController.h \
    MockVisionService.h \
    MockDisplayService.h
