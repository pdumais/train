#include "mainwindow.h"

#include <QApplication>
#include "traincontroller.h"
#include "configuration.h"
#include "displayservice.h"
#include "visionservice.h"
#include "tracklearningservice.h"
#include "LightService.h"
#include "RailroadLogicService.h"
#include "AudioService.h"

#define CONFIG_FILE "/home/pat/projects/train/data/config.json"
#define TRACK_DATA "/home/pat/projects/train/data/tracks.dat"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Configuration conf(TRACK_DATA, CONFIG_FILE);
    AudioService audioService(conf.getSoundLevel());
    VisionService visionService(1920,1080);
    DisplayService displayService;
    TrainController trainController(&conf);
    LightService lightService(&trainController, conf.geLightLevel());
    TrackLearningService trackLearningService(&trainController, &visionService, &conf);
    RailroadLogicService railroadLogicService(&trainController, &visionService, &displayService, &conf, &audioService);

/*    RailroadLogicService railroadLogicService(&trainController, 0, 0, &conf, 0);
    railroadLogicService.currentPosition = QPoint(668,600);
    railroadLogicService.findTurnouts();
    railroadLogicService.setWaypoint(QPoint(1000,500));
    railroadLogicService.gotoWaypoint();*/


    MainWindow w(&trainController, 
            &conf, 
            &displayService, 
            &visionService, 
            &railroadLogicService, 
            &trackLearningService, 
            &lightService,
            &audioService);
    w.show();
    return a.exec();
}
