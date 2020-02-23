#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QUuid>
#include <QInputDialog>
#include <QMediaPlayer>
#include <QSignalTransition>
#include <QTextStream>
#include <QGraphicsVideoItem>
#include "constants.h"
#include "crossroadannotation.h"
#include "splitterannotation.h"

MainWindow::MainWindow(TrainController *controller, 
        Configuration *configuration, 
        DisplayService* vision, 
        VisionService* decoder, 
        RailroadLogicService* railroadService, 
        TrackLearningService* trackLearningService, 
        LightService* lightService, 
        AudioService* audioService,
        QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Setup dependencies
    this->configuration = configuration;
    this->trackLearningService = trackLearningService;
    this->railroadService = railroadService;
    this->vision = decoder;
    this->lightService = lightService;
    this->audioService = audioService;

    this->display = vision;
    this->display->init(this->ui->graphicsView);
    this->display->start();
    this->display->setProbe(this->vision->probe());
    this->display->createLocomotiveItem("locomotive", DisplayService::ViewType::All);

    //this->controller = controller;
    connect(controller, &TrainController::speedChanged, this, &MainWindow::on_speed_changed);

    // Setup status bar
    this->speedLabel = new QLabel();
    this->viewLabel = new QLabel();
    this->ui->statusbar->addPermanentWidget(speedLabel);
    this->ui->statusbar->addWidget(viewLabel);

    this->ui->graphicsView->show();

    this->ui->lightSlider->setValue(this->configuration->geLightLevel());
    this->ui->volumeSlider->setValue(this->configuration->getSoundLevel());

    this->setupFSM();

    this->updateTrackList();
    this->railroadService->updateTracks();
    this->railroadService->updateAnnotations();

}

MainWindow::~MainWindow()
{
    delete this->ui;
}

/**********************************************************************
 **********************************************************************
 *                    FSM control
 **********************************************************************
 **********************************************************************/


void MainWindow::setupFSM()
{
    QState* learningInactiveState = new QState();
    QState* learningActiveState = new QState();

    QState* operateView = new QState();
    QStateMachine *learnView = new QStateMachine();

    learnView->addState(learningInactiveState);
    learnView->addState(learningActiveState);
    learnView->setInitialState(learningInactiveState);

    connect(operateView, &QState::entered, this, &MainWindow::operateView_entry);
    connect(operateView, &QState::exited, this, &MainWindow::operateView_exit);
    connect(learningActiveState, &QState::entered, this, &MainWindow::learningActive_entry);
    connect(learningActiveState, &QState::exited, this, &MainWindow::learningActive_exit);
    connect(learningInactiveState, &QState::entered, this, &MainWindow::learningInactive_entry);
    connect(learnView, &QState::entered, this, &MainWindow::learnView_entry);


    this->views.addState(operateView);
    this->views.addState(learnView);
    this->views.setInitialState(learnView);

    learningInactiveState->addTransition(this->ui->learntrack_button, SIGNAL(clicked(bool)), learningActiveState);
    learningActiveState->addTransition(this->ui->learntrack_button, SIGNAL(clicked(bool)), learningInactiveState);
    learningActiveState->addTransition(this->trackLearningService, SIGNAL(learningStopped(QString)), learningInactiveState);

    operateView->addTransition(this, SIGNAL(viewLearn()), learnView);
    learnView->addTransition(this, SIGNAL(viewOperate()), operateView);

    this->views.start();
}

void MainWindow::learningInactive_entry()
{
    this->ui->learntrack_button->setText("Learn");
    this->ui->deletetrack_button->setEnabled(true);
    viewLabel->setText("Learn view (inactive)");
}

//TODO: this logic should be done in railroadservice
void MainWindow::learningActive_exit()
{
    this->trackLearningService->stop();
    QPolygon* poly = this->trackLearningService->getLearnedPolygon();
    QString trackName = this->trackLearningService->getName();

    for (Track* t : configuration->getTracks())
    {
        if (t->getName() == trackName)
        {
            t->setPolygon(*poly);
            break;
        }
    }

    this->railroadService->updateTracks();

}

void MainWindow::learningActive_entry()
{
    //TODO: must cancel operation if the parent transitions to something else. We must exit this state and call the exit code
    this->ui->learntrack_button->setText("Stop");
    this->ui->deletetrack_button->setEnabled(false);
    QVariant v = this->ui->track_list->currentItem()->data(Qt::UserRole);
    Track *t = v.value<Track*>();
    viewLabel->setText("Learn view (active for track '"+t->getName()+"')");

    //TODO: this logic should be done in railroadservice
    this->trackLearningService->start(t->getName(), this->display->track("LearningTrack"));

}

void MainWindow::operateView_entry()
{
    this->display->setViewType(DisplayService::ViewType::Operation);
    viewLabel->setText("Operate view");
}

void MainWindow::operateView_exit()
{
    this->railroadService->stopTrain();
}

void MainWindow::learnView_entry()
{
    this->display->setViewType(DisplayService::ViewType::Learning);
    viewLabel->setText("Learn view");
}

/**********************************************************************
 **********************************************************************
 *                    Widget slots
 **********************************************************************
 **********************************************************************/

void MainWindow::on_speed_changed(int val)
{
    int speed = val;
    QString str;
    if (speed)
    {
        QTextStream(&str) << "Speed: " << speed << "%";
    }
    else
    {
        str = "Speed: Stopped";
    }
    this->speedLabel->setText(str);
}

// Panic button
void MainWindow::on_pushButton_2_clicked()
{
    this->railroadService->stopTrain();
    //TODO: should abort current state if learning
}


void MainWindow::on_pushButton_5_clicked()
{
    bool ok;
    QString text = QInputDialog::getText(this, "Add new track", "Track name: ", QLineEdit::Normal,"", &ok);
    if (!ok || text.isEmpty()) return;

    Track *t = new Track(text);
    this->configuration->addTrack(t);
    this->updateTrackList();

}


void MainWindow::on_track_list_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (current)
    {
        this->ui->deletetrack_button->setEnabled(true);
        this->ui->learntrack_button->setEnabled(true);
    }
    else
    {
        this->ui->deletetrack_button->setEnabled(false);
        this->ui->learntrack_button->setEnabled(false);
    }
}

void MainWindow::on_deletetrack_button_clicked()
{
    QVariant v = this->ui->track_list->currentItem()->data(Qt::UserRole);
    Track *t = v.value<Track*>();
    this->display->removeItem(t->getName());
    this->configuration->removeTrack(t);
    this->updateTrackList();
}


void MainWindow::on_tabWidget_currentChanged(int index)
{
    switch (index) {
    case 0:
        emit this->viewOperate();
        break;
    case 1:
        emit this->viewLearn();
        break;
    }
}

void MainWindow::on_save_button_clicked()
{
    this->configuration->save();
    this->ui->statusbar->showMessage("Configuration saved", 5000);
}



void MainWindow::updateTrackList()
{
    this->ui->track_list->clear();

    for (Track* t : this->configuration->getTracks())
    {
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(t->getName());
        QVariant v;
        v.setValue(t);
        item->setData(Qt::UserRole,v);
        this->ui->track_list->addItem(item);
    }
    this->railroadService->updateTracks();

}

void MainWindow::on_pushButton_4_clicked()
{
    this->railroadService->startTrainForward();
}

void MainWindow::on_pushButton_3_clicked()
{
    this->railroadService->stopAll();
}

void MainWindow::on_pushButton_6_clicked()
{
    this->railroadService->startTrainReverse();
}

void MainWindow::on_pushButton_clicked()
{
    this->configuration->save();
    this->ui->statusbar->showMessage("Configuration saved", 5000);
}

void MainWindow::on_volumeSlider_valueChanged(int value)
{
    this->configuration->setSoundLevel(value);
    this->audioService->setLevel(value);
}

void MainWindow::on_lightSlider_valueChanged(int value)
{
    this->configuration->setLightLevel(value);
    this->lightService->setLevel(value);
}

void MainWindow::on_goWaypoint_button_clicked()
{
    this->railroadService->gotoWaypoint();
}
