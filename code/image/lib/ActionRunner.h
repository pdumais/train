#ifndef ACTIONRUNNER_H
#define ACTIONRUNNER_H

#include "splitterannotation.h"

#include <QObject>
#include <QVector>
#include <actions/Action.h>

class RailroadLogicService;

class ActionRunner : public QObject
{
    Q_OBJECT
public:
    explicit ActionRunner(QObject *parent = nullptr);

    void setRailroadService(RailroadLogicService* rrls);
    void setConfiguration(Configuration* conf);

    void addAction(Action* action);
    QVector<Action*> getActions(bool includeCurrent=true);

    void onEnterTurnout(SplitterAnnotation* sa);
    void onLeaveTurnout(SplitterAnnotation* sa);
    void onTrainMoved(Train* train);

    void run();
    void abort();

private:
    RailroadLogicService* railroadService;
    Configuration* configuration;
    QVector<Action*> actions;

    Action* currentAction;

    void runNextAction();
};

#endif // ACTIONRUNNER_H
