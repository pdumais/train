#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "traincontroller.h"
#include "configuration.h"
#include "displayservice.h"
#include "visionservice.h"
#include "tracklearningservice.h"
#include "crossroadannotation.h"
#include "splitterannotation.h"
#include "LightService.h"
#include "RailroadLogicService.h"
#include "AudioService.h"

#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QStateMachine>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(TrainController *controller, 
            Configuration *configuration, 
            DisplayService* display, 
            VisionService* vision, 
            RailroadLogicService* railroadService, 
            TrackLearningService* trackLearningService, 
            LightService* lightService, 
            AudioService* audioService,
            QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void viewOperate();
    void viewLearn();
    void viewDebug();

private slots:
    void on_pushButton_2_clicked();

    void on_pushButton_5_clicked();

    void on_track_list_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_deletetrack_button_clicked();

    void on_speed_changed(int val);

    void on_tabWidget_currentChanged(int index);

    void on_save_button_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_clicked();

    void on_volumeSlider_valueChanged(int value);

    void on_lightSlider_valueChanged(int value);

    void on_goWaypoint_button_clicked();

    void on_debugImages_currentIndexChanged(const QString &arg1);

    void on_debug_image(QImage);

private:
    Ui::MainWindow *ui;
    bool learning;
    QStateMachine views;
    QLabel *speedLabel;
    QLabel *viewLabel;

    Configuration *configuration;
    DisplayService* display;
    VisionService* vision;
    RailroadLogicService* railroadService;
    LightService* lightService;
    TrackLearningService* trackLearningService;
    AudioService* audioService;

    void setupFSM();
    void updateTrackList();
    void operateView_entry();
    void operateView_exit();
    void debugView_entry();
    void debugView_exit();
    void learnView_entry();
    void learningInactive_entry();
    void learningActive_entry();
    void learningActive_exit();
};
#endif // MAINWINDOW_H
