QT       += core gui
TEMPLATE = lib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport multimedia multimediawidgets

CONFIG += c++17

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

RESOURCES      = train.qrc

SOURCES += \
    annotation.cpp \
    configuration.cpp \
    crossroadannotation.cpp \
    cvobject.cpp \
    displayservice.cpp \
    LightService.cpp \
    mainwindow.cpp \
    qdragdropgraphicsscene.cpp \
    RailroadLogicService.cpp \
    splitterannotation.cpp \
    track.cpp \
    tracklearningservice.cpp \
    trackobjectlabel.cpp \
    traincontroller.cpp \
    visionservice.cpp \
    AudioService.cpp \
    Functions.cpp \
    ActionRunner.cpp \
    actions/Action.cpp \
    actions/MoveToAction.cpp \
    actions/ActivateTurnoutAction.cpp \
    Train.cpp \
    CollisionMatrix.cpp \
    PerformanceMonitor.cpp
    visionservice.cpp \


HEADERS += \
    annotation.h \
    configuration.h \
    constants.h \
    crossroadannotation.h \
    cvobject.h \
    displayservice.h \
    LightService.h \
    mainwindow.h \
    qdragdropgraphicsscene.h \
    RailroadLogicService.h \
    splitterannotation.h \
    track.h \
    tracklearningservice.h \
    trackobjectlabel.h \
    traincontroller.h \
    visionservice.h \
    AudioService.h \
    Functions.h \
    ITrainController.h \
    IVisionService.h \
    IDisplayService.h \
    ActionRunner.h \
    actions/Action.h \
    actions/MoveToAction.h \
    actions/ActivateTurnoutAction.h \
    Train.h \
    CollisionMatrix.h \
    PerformanceMonitor.h
    visionservice.h \


FORMS += \
    mainwindow.ui

INCLUDEPATH += /usr/include/opencv /usr/include/leptonica

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    ../../../todo \
    ../../../README
