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

void ActionRunner::setConfiguration(Configuration* conf)
{
    this->configuration = conf;
}

void ActionRunner::addAction(Action* action)
{
    qDebug() << "ActionRunner::addAction: " << action->toString();
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

bool ActionRunner::isRunningAction()
{
    return (this->currentAction != nullptr);
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
        // TODO: use QObject::deleteLater() instead
        delete this->currentAction;
        this->currentAction = nullptr;
    }
    if (this->actions.size() < 1) return;

    this->currentAction = this->actions.takeFirst();
    qDebug() << "Running next Action " << this->currentAction;

    QVector<Annotation*> inRange;
    for (auto it : this->configuration->getAnnotations())
    {
        if (it->getInRange()) inRange.append(it);
    }

    std::function<void()> f = [this](){
        this->runNextAction();
    };
    this->currentAction->execute(this->railroadService, inRange,f);
}

