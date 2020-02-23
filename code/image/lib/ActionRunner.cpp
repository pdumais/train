#include "ActionRunner.h"
#include <QTimer>

ActionRunner::ActionRunner(QObject *parent) : QObject(parent)
{
    this->currentAction = nullptr;
    this->railroadService = nullptr;
}

void ActionRunner::setRailroadService(RailroadLogicService* rrls)
{
    this->railroadService = rrls;
}

void ActionRunner::addAction(Action* action)
{
    this->actions.append(action);
}

QVector<Action*> ActionRunner::getActions(bool includeCurrent/*=true*/)
{
    QVector<Action*> ret = this->actions;
    if (includeCurrent && this->currentAction) ret.prepend(this->currentAction);
    return ret;
}

void ActionRunner::onEnterTurnout(SplitterAnnotation* sa)
{
    if (!this->currentAction) return;

    this->currentAction->onEnterTurnout(sa);
}

void ActionRunner::onLeaveTurnout(SplitterAnnotation* sa)
{
    if (!this->currentAction) return;

    this->currentAction->onLeaveTurnout(sa);
}

void ActionRunner::onTrainMoved(Train* train)
{
    if (!this->currentAction) return;

    this->currentAction->onTrainMoved(train);
}

void ActionRunner::abort()
{
    if (this->currentAction)
    {
        delete this->currentAction;
        this->currentAction = nullptr;
    }

    for (Action *a : this->actions)
    {
        delete a;
    }

    this->actions.clear();
}

void ActionRunner::run()
{
    //TODO: if running again before the othr run ended, it will skip the first action in the list
    this->runNextAction();
}

void ActionRunner::runNextAction()
{
    if (this->currentAction)
    {
        delete this->currentAction;
        this->currentAction = nullptr;
    }
    if (this->actions.size() < 1) return;

    this->currentAction = this->actions.takeFirst();
    qDebug() << "Running next Action " << this->currentAction;
    this->currentAction->execute(this->railroadService,[=](){
            //QTimer::singleShot(1000,[=](){
                this->runNextAction();
            //});
            //delete this->currentAction;
            //this->currentAction = nullptr;
    });
}

