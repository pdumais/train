#include <QtTest>

#include <actions/MoveToSplitterAction.h>
#include <actions/MoveToAction.h>
#include <actions/ChangeTrackAction.h>

#include "MockDisplayService.h"
#include "MockTrainController.h"
#include "MockVisionService.h"
#include "RailroadLogicService.h"
#include "configuration.h"
#include "ActionRunner.h"
#include "constants.h"
#include "Functions.h"
#include <any>
#include <utility>

#define CONFIG_FILE "/home/pat/projects/train/code/image/tests/testdata/config.json"
#define TRACK_DATA "/home/pat/projects/train/code/image/tests/testdata/tracks.dat"


class tests : public QObject
{
    Q_OBJECT

public:
    tests();
    ~tests();

private slots:
    void init();
    void cleanup();
    void checkTrackAssignment();
    void validateGraph();
    void goFromLoopToInner();
    void goFromInnerToLoop();
    void goFromInnerToOutter();
    void goFromLoopEndToOutter();
    void goFromLoopEndToMainAfterOutter();
    void goFromOutterToMainAfterOutter();
    void goFromOutterToLoop();
    void goFromOutterToFarEnd();
    void validateTrainObject();

private:
    Configuration config;
    SplitterAnnotation* outterSplitter;
    SplitterAnnotation* innerSplitter;
    SplitterAnnotation* loopTopSplitter;
    SplitterAnnotation* loopBottomSplitter;

    RailroadLogicService* railroadLogicService;
    MockTrainController* controller;
    MockVisionService* vision;
    MockDisplayService* display;

    void validateSplitterSwitching(SplitterAnnotation *sa, int activatedTrack, bool after);
    void setTrainPosition(QPoint p, int w=4, QVector<CVObject> wagons = QVector<CVObject>());
    CVObject buildCVObject(QPoint p, int width);
    SplitterAnnotation* getSAFromAny(std::any a);
};

tests::tests()
{
    this->config.load(TRACK_DATA, CONFIG_FILE);
}

tests::~tests()
{
}

void tests::init()
{
    // Override the sleep timer so that tests dont hang on those
    std::function<void(int)> fn = [](int ms) {};
    Functions::overrideSleepFunction(fn);

    controller = new MockTrainController();
    vision = new MockVisionService();
    display = new MockDisplayService(); 
    railroadLogicService = new RailroadLogicService(controller, vision, display, &config, 0);
    railroadLogicService->updateTurnouts();

    loopBottomSplitter = nullptr;
    loopTopSplitter = nullptr;
    auto splitters = config.getSplitterAnnotations();
    for (auto sa : splitters)
    {
        if (sa->getTrack1() == "main" && sa->getTrack2() == "loop")
        {
            //TODO: make this more deterministic. If we change map, this might become wrong
            if (!loopTopSplitter) loopTopSplitter = sa; else loopBottomSplitter = sa;
        }
        if (sa->getTrack1() == "main" && sa->getTrack2() == "inner") innerSplitter = sa;
        if (sa->getTrack1() == "outter" && sa->getTrack2() == "main") outterSplitter = sa;
    }

}


void tests::cleanup()
{
    delete railroadLogicService;
    delete display;
    delete vision;
    delete controller;
}

CVObject tests::buildCVObject(QPoint p, int width)
{
    CVObject cv;
    cv.setCenter(p);
    int w = width;
    int h = LOCO_HEIGHT;
    QRect r(p.x()-(w/2),p.y()-(h/2),w,h);
    cv.setPolygon(QPolygon(r));
    cv.setLine(QLine(QPoint(p.x()-(w/2),p.y()),QPoint(p.x()+(w/2),p.y())));
    return cv;
}

void tests::setTrainPosition(QPoint p, int width, QVector<CVObject> wagons)
{

    CVObject cv = buildCVObject(p,width);
    this->vision->loco = cv;

    railroadLogicService->on_frame_processed(cv, wagons);
}

void tests::validateSplitterSwitching(SplitterAnnotation *sa, int activatedTrack, bool after)
{
    qDebug() << "validateSplitterSwitching("<< sa << ", " << activatedTrack << ")";
    controller->triggerCount[sa->getRelay(0)] = 0;
    controller->triggerCount[sa->getRelay(1)] = 0;
    setTrainPosition(sa->getPosition());
    if (after)
    {
        // If we want to move the train after the zone, just move it anywhere outside the zone
        setTrainPosition(QPoint(1,1));
    }
    if (activatedTrack == 0)
    {
        QCOMPARE(controller->triggerCount[sa->getRelay(0)], 0);
        QCOMPARE(controller->triggerCount[sa->getRelay(1)], 0);
    }
    else if (activatedTrack == 1)
    {
        QCOMPARE(controller->triggerCount[sa->getRelay(0)], 1);
        QCOMPARE(controller->triggerCount[sa->getRelay(1)], 0);
    }
    else if (activatedTrack == 2)
    {
        QCOMPARE(controller->triggerCount[sa->getRelay(0)], 0);
        QCOMPARE(controller->triggerCount[sa->getRelay(1)], 1);
    }
}

SplitterAnnotation* tests::getSAFromAny(std::any a)
{
    if (!a.has_value()) return nullptr;
    return anycast(a);
}

void tests::validateGraph()
{
    Railroad* rr = this->railroadLogicService->getRailroad();

    QCOMPARE(rr->getGraph()->nodeCount(), 6);
    QCOMPARE(rr->getGraph()->edgeCount(), 7);

    TrainPosition source = this->railroadLogicService->findClosestPoint(QPoint(853,816));
    TrainPosition dest = this->railroadLogicService->findClosestPoint(QPoint(119,186));
    auto path = rr->findShortestPath(source.point, dest.point);
    QCOMPARE(path.size(), 3);
    QCOMPARE(getSAFromAny(path[0].node1Data), nullptr);
    QCOMPARE(getSAFromAny(path[0].node2Data), loopTopSplitter);
    QCOMPARE(getSAFromAny(path[1].node1Data), outterSplitter);
    QCOMPARE(getSAFromAny(path[1].node2Data), loopTopSplitter);
    QCOMPARE(getSAFromAny(path[2].node1Data), nullptr);
    QCOMPARE(getSAFromAny(path[2].node2Data), outterSplitter);
}

void tests::checkTrackAssignment()
{
    QCOMPARE(innerSplitter->getTrack1(), QString("main"));
    QCOMPARE(innerSplitter->getTrack2(), QString("inner"));
    QCOMPARE(innerSplitter->getInputTrack(), QString("main"));
    QCOMPARE(loopTopSplitter->getTrack1(), QString("main"));
    QCOMPARE(loopTopSplitter->getTrack2(), QString("loop"));
    QCOMPARE(loopTopSplitter->getInputTrack(), QString("main"));
    QCOMPARE(loopBottomSplitter->getTrack1(), QString("main"));
    QCOMPARE(loopBottomSplitter->getTrack2(), QString("loop"));
    QCOMPARE(loopBottomSplitter->getInputTrack(), QString("main"));
    QCOMPARE(outterSplitter->getTrack1(), QString("outter"));
    QCOMPARE(outterSplitter->getTrack2(), QString("main"));
    QCOMPARE(outterSplitter->getInputTrack(), QString("main"));

    QVERIFY(loopTopSplitter->getPosition().y() > loopBottomSplitter->getPosition().y());
}

void tests::goFromLoopToInner()
{
    setTrainPosition(QPoint(668,600));
    QPoint waypoint(1000,500);
    railroadLogicService->setWaypoint(waypoint);
    railroadLogicService->gotoWaypoint();

    auto actionList = railroadLogicService->getActionRunner()->getActions();
    QCOMPARE(actionList.size(),6);

    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[0])->getCurrentTrack(), "loop");
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[0])->getReverse(), true);
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[0])->getTargetTrack(), "main");

    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[1])->getSplitterAnnotation(), loopBottomSplitter);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[1])->getCommingFromT0(), false);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[1])->getTargetTrack(), "main");

    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[2])->getCurrentTrack(), "main");
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[2])->getReverse(), true);
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[2])->getTargetTrack(), "inner");

    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[3])->getSplitterAnnotation(), innerSplitter);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[3])->getCommingFromT0(), false);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[3])->getTargetTrack(), "inner");

    QCOMPARE(dynamic_cast<MoveToAction*>(actionList[4])->getTargetPosition(), railroadLogicService->getTrackWaypoint());
    QCOMPARE(dynamic_cast<MoveToAction*>(actionList[4])->getReverse(), false);
    QCOMPARE(dynamic_cast<MoveToAction*>(actionList[4])->getCurrentTrack(), "inner");


    // Moving towards loop bottom
    validateSplitterSwitching(loopBottomSplitter, 1, false);   // This one will trigger an action change
    validateSplitterSwitching(loopBottomSplitter, 0, true);   // This one will trigger an action change

    // Moving into inner
    validateSplitterSwitching(innerSplitter, 2, false);

    // moving pass inner
    validateSplitterSwitching(innerSplitter, 1, true);
    
    // Now we move forward into the splitter, it should fork
    validateSplitterSwitching(innerSplitter, 1, false);
}


void tests::goFromInnerToLoop()
{
    setTrainPosition(QPoint(1000,500));

    QPoint waypoint(668,600);
    railroadLogicService->setWaypoint(waypoint);
    railroadLogicService->gotoWaypoint();

    auto actionList = railroadLogicService->getActionRunner()->getActions();
    QCOMPARE(actionList.size(),6);

    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[0])->getCurrentTrack(), "inner");
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[0])->getReverse(), true);
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[0])->getTargetTrack(), "main");

    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[1])->getSplitterAnnotation(), innerSplitter);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[1])->getCommingFromT0(), false);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[1])->getTargetTrack(), "main");

    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[2])->getCurrentTrack(), "main");
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[2])->getReverse(), false);
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[2])->getTargetTrack(), "loop");

    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[3])->getSplitterAnnotation(), loopBottomSplitter);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[3])->getCommingFromT0(), true);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[3])->getTargetTrack(), "loop");

    QCOMPARE(dynamic_cast<MoveToAction*>(actionList[4])->getTargetPosition(), railroadLogicService->getTrackWaypoint());
    QCOMPARE(dynamic_cast<MoveToAction*>(actionList[4])->getReverse(), false);
    QCOMPARE(dynamic_cast<MoveToAction*>(actionList[4])->getCurrentTrack(), "loop");


    validateSplitterSwitching(innerSplitter, 1, false);  // This one will trigger an action change

    // We will then be on main track, the target, so nothing will get activated
    validateSplitterSwitching(innerSplitter, 0, true);

    // We're stepping back in, the main track should get activated
    validateSplitterSwitching(innerSplitter, 2, false);
    validateSplitterSwitching(innerSplitter, 0, true);

    // Moving towards inner (after, so we can change dir)
    validateSplitterSwitching(loopBottomSplitter, 1, false);

}


void tests::goFromInnerToOutter()
{
    setTrainPosition(QPoint(1000,500));

    QPoint waypoint(119,186);
    railroadLogicService->setWaypoint(waypoint);
    railroadLogicService->gotoWaypoint();

    auto actionList = railroadLogicService->getActionRunner()->getActions();
    QCOMPARE(actionList.size(),10);

    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[0])->getCurrentTrack(), "inner");
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[0])->getReverse(), true);
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[0])->getTargetTrack(), "main");

    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[1])->getSplitterAnnotation(), innerSplitter);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[1])->getCommingFromT0(), false);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[1])->getTargetTrack(), "main");

    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[2])->getCurrentTrack(), "main");
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[2])->getReverse(), false);
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[2])->getTargetTrack(), "loop");

    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[3])->getSplitterAnnotation(), loopBottomSplitter);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[3])->getCommingFromT0(), true);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[3])->getTargetTrack(), "loop");

    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[4])->getCurrentTrack(), "loop");
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[4])->getReverse(), false);
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[4])->getTargetTrack(), "main");

    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[5])->getSplitterAnnotation(), loopTopSplitter);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[5])->getCommingFromT0(), false);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[5])->getTargetTrack(), "main");

    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[6])->getCurrentTrack(), "main");
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[6])->getReverse(), true);
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[6])->getTargetTrack(), "outter");

    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[7])->getSplitterAnnotation(), outterSplitter);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[7])->getCommingFromT0(), true);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[7])->getTargetTrack(), "outter");


    QCOMPARE(dynamic_cast<MoveToAction*>(actionList[8])->getTargetPosition(), railroadLogicService->getTrackWaypoint());
    QCOMPARE(dynamic_cast<MoveToAction*>(actionList[8])->getReverse(), true);
    QCOMPARE(dynamic_cast<MoveToAction*>(actionList[8])->getCurrentTrack(), "outter");

    
    validateSplitterSwitching(innerSplitter, 1, false);  
    validateSplitterSwitching(innerSplitter, 0, true);   // This one will trigger an action change
    validateSplitterSwitching(innerSplitter, 2, false);   // Go back in: splitter must now make traffic go to main
    validateSplitterSwitching(innerSplitter, 0, true);   // This one will trigger an action change

    validateSplitterSwitching(loopBottomSplitter, 1, false);
    validateSplitterSwitching(loopBottomSplitter, 0, true);

    validateSplitterSwitching(loopTopSplitter, 1, false);
    validateSplitterSwitching(loopTopSplitter, 0, true);
    validateSplitterSwitching(loopTopSplitter, 2, false);
    validateSplitterSwitching(loopTopSplitter, 0, true);

    validateSplitterSwitching(outterSplitter, 2, false);  // reach splitter, it must activate fork
    validateSplitterSwitching(outterSplitter, 0, true);  // go after splitter, nothing would change. But we will change action


}

void tests::goFromLoopEndToOutter()
{
    setTrainPosition(QPoint(853,816));

    QPoint waypoint(119,186);
    railroadLogicService->setWaypoint(waypoint);
    railroadLogicService->gotoWaypoint();

    auto actionList = railroadLogicService->getActionRunner()->getActions();
    QCOMPARE(actionList.size(),6);

    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[0])->getCurrentTrack(), "loop");
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[0])->getReverse(), false);
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[0])->getTargetTrack(), "main");

    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[1])->getSplitterAnnotation(), loopTopSplitter);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[1])->getCommingFromT0(), false);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[1])->getTargetTrack(), "main");

    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[2])->getCurrentTrack(), "main");
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[2])->getReverse(), true);
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[2])->getTargetTrack(), "outter");

    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[3])->getSplitterAnnotation(), outterSplitter);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[3])->getCommingFromT0(), true);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[3])->getTargetTrack(), "outter");

    QCOMPARE(dynamic_cast<MoveToAction*>(actionList[4])->getTargetPosition(), railroadLogicService->getTrackWaypoint());
    QCOMPARE(dynamic_cast<MoveToAction*>(actionList[4])->getReverse(), true);
    QCOMPARE(dynamic_cast<MoveToAction*>(actionList[4])->getCurrentTrack(), "outter");

    validateSplitterSwitching(loopTopSplitter, 1, false);  
    validateSplitterSwitching(loopTopSplitter, 0, true);
    validateSplitterSwitching(loopTopSplitter, 2, false);
    validateSplitterSwitching(loopTopSplitter, 0, true);

    validateSplitterSwitching(outterSplitter, 2, false);  
    validateSplitterSwitching(outterSplitter, 0, true);  

}


void tests::goFromLoopEndToMainAfterOutter()
{
    setTrainPosition(QPoint(761,850));

    QPoint waypoint(291,753);
    railroadLogicService->setWaypoint(waypoint);
    railroadLogicService->gotoWaypoint();

    auto actionList = railroadLogicService->getActionRunner()->getActions();
    QCOMPARE(actionList.size(),6);

    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[0])->getCurrentTrack(), "loop");
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[0])->getReverse(), false);
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[0])->getTargetTrack(), "main");

    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[1])->getSplitterAnnotation(), loopTopSplitter);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[1])->getCommingFromT0(), false);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[1])->getTargetTrack(), "main");

    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[2])->getCurrentTrack(), "main");
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[2])->getReverse(), true);
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[2])->getTargetTrack(), "main");

    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[3])->getSplitterAnnotation(), outterSplitter);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[3])->getCommingFromT0(), true);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[3])->getTargetTrack(), "main");

    QCOMPARE(dynamic_cast<MoveToAction*>(actionList[4])->getTargetPosition(), railroadLogicService->getTrackWaypoint());
    QCOMPARE(dynamic_cast<MoveToAction*>(actionList[4])->getReverse(), true);
    QCOMPARE(dynamic_cast<MoveToAction*>(actionList[4])->getCurrentTrack(), "main");

    
    validateSplitterSwitching(loopTopSplitter, 1, false);
    validateSplitterSwitching(loopTopSplitter, 0, true);
    validateSplitterSwitching(loopTopSplitter, 2, false);
    validateSplitterSwitching(loopTopSplitter, 0, true);

    validateSplitterSwitching(outterSplitter, 1, false);
    validateSplitterSwitching(outterSplitter, 0, true);
}

void tests::goFromOutterToMainAfterOutter()
{
    setTrainPosition(QPoint(119,186));

    QPoint waypoint(291,753);
    railroadLogicService->setWaypoint(waypoint);
    railroadLogicService->gotoWaypoint();

    auto actionList = railroadLogicService->getActionRunner()->getActions();
    QCOMPARE(actionList.size(),4);

    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[0])->getCurrentTrack(), "outter");
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[0])->getReverse(), false);
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[0])->getTargetTrack(), "main");

    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[1])->getSplitterAnnotation(), outterSplitter);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[1])->getCommingFromT0(), false);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[1])->getTargetTrack(), "main");

    QCOMPARE(dynamic_cast<MoveToAction*>(actionList[2])->getTargetPosition(), railroadLogicService->getTrackWaypoint());
    QCOMPARE(dynamic_cast<MoveToAction*>(actionList[2])->getReverse(), true);
    QCOMPARE(dynamic_cast<MoveToAction*>(actionList[2])->getCurrentTrack(), "main");


    validateSplitterSwitching(outterSplitter, 2, false);
    validateSplitterSwitching(outterSplitter, 0, true);
    validateSplitterSwitching(outterSplitter, 1, false);
    validateSplitterSwitching(outterSplitter, 0, true);
}


void tests::goFromOutterToLoop()
{
    setTrainPosition(QPoint(119,186));

    QPoint waypoint(853,916);
    railroadLogicService->setWaypoint(waypoint);
    railroadLogicService->gotoWaypoint();

    auto actionList = railroadLogicService->getActionRunner()->getActions();
    QCOMPARE(actionList.size(),6);

    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[0])->getCurrentTrack(), "outter");
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[0])->getReverse(), false);
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[0])->getTargetTrack(), "main");

    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[1])->getSplitterAnnotation(), outterSplitter);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[1])->getCommingFromT0(), false);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[1])->getTargetTrack(), "main");

    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[2])->getCurrentTrack(), "main");
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[2])->getReverse(), false);
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[2])->getTargetTrack(), "loop");

    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[3])->getSplitterAnnotation(), loopTopSplitter);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[3])->getCommingFromT0(), false);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[3])->getTargetTrack(), "loop");

    QCOMPARE(dynamic_cast<MoveToAction*>(actionList[4])->getTargetPosition(), railroadLogicService->getTrackWaypoint());
    QCOMPARE(dynamic_cast<MoveToAction*>(actionList[4])->getReverse(), true);
    QCOMPARE(dynamic_cast<MoveToAction*>(actionList[4])->getCurrentTrack(), "loop");

    validateSplitterSwitching(outterSplitter, 2, false);
    validateSplitterSwitching(outterSplitter, 0, true);
    validateSplitterSwitching(loopTopSplitter, 2, false);
    validateSplitterSwitching(loopTopSplitter, 1, true);
    validateSplitterSwitching(loopTopSplitter, 1, false);
    validateSplitterSwitching(loopTopSplitter, 0, true);

}


void tests::goFromOutterToFarEnd()
{
    setTrainPosition(QPoint(119,186));

    QPoint waypoint(1825,495);
    railroadLogicService->setWaypoint(waypoint);
    railroadLogicService->gotoWaypoint();

    SplitterAnnotation *firstSplitter = outterSplitter;

    auto actionList = railroadLogicService->getActionRunner()->getActions();
    QCOMPARE(actionList.size(),6);

    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[0])->getCurrentTrack(), "outter");
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[0])->getReverse(), false);
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[0])->getTargetTrack(), "main");

    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[1])->getSplitterAnnotation(), outterSplitter);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[1])->getCommingFromT0(), false);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[1])->getTargetTrack(), "main");

    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[2])->getCurrentTrack(), "main");
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[2])->getReverse(), false);
    QCOMPARE(dynamic_cast<MoveToSplitterAction*>(actionList[2])->getTargetTrack(), "main");

    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[3])->getSplitterAnnotation(), loopTopSplitter);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[3])->getCommingFromT0(), false);
    QCOMPARE(dynamic_cast<ChangeTrackAction*>(actionList[3])->getTargetTrack(), "main");

    QCOMPARE(dynamic_cast<MoveToAction*>(actionList[4])->getTargetPosition(), railroadLogicService->getTrackWaypoint());
    QCOMPARE(dynamic_cast<MoveToAction*>(actionList[4])->getReverse(), false);
    QCOMPARE(dynamic_cast<MoveToAction*>(actionList[4])->getCurrentTrack(), "main");

    validateSplitterSwitching(outterSplitter, 2, false);
    validateSplitterSwitching(outterSplitter, 0, true);
    validateSplitterSwitching(loopTopSplitter, 2, false);
    validateSplitterSwitching(loopTopSplitter, 0, true);

}


void tests::validateTrainObject()
{
    int w = 100;

    QVector<CVObject> wagonsList;
    wagonsList.append(buildCVObject(QPoint(1000-w-MAXIMUM_WAGON_CONNECTION_SIZE+1, 500), w));
    wagonsList.append(buildCVObject(QPoint(1000-(2*w)-MAXIMUM_WAGON_CONNECTION_SIZE+1, 500), w));
    wagonsList.append(buildCVObject(QPoint(1000-w-MAXIMUM_WAGON_CONNECTION_SIZE+1, 700), w));
    wagonsList.append(buildCVObject(outterSplitter->getPosition(), w));

    setTrainPosition(QPoint(1000,500), w, wagonsList);

    QColor wcol0 = this->display->polygonItem("wagon0")->brush().color();
    QColor wcol1 = this->display->polygonItem("wagon1")->brush().color();
    QColor wcol2 = this->display->polygonItem("wagon2")->brush().color();
    QColor wcol3 = this->display->polygonItem("wagon3")->brush().color();
    
    // 2 first wagons should be the same color since they are being detected as part of the train
    QVERIFY(wcol0 == wcol1);

    // 3rd wagon should be a different color since it is not part of the train
    QVERIFY(wcol2 != wcol1);
    QVERIFY(wcol3 != wcol1);
   
    // an unlinked wagon should not trigger an annotation to be in range 
    QVERIFY(outterSplitter->getInRange() == false);

}

QTEST_APPLESS_MAIN(tests)

#include "tst_tests.moc"


