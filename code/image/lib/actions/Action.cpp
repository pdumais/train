#include "Action.h"

Action::Action()
{

}

void Action::execute(RailroadLogicService* val, QVector<Annotation*> annotationsInRange, std::function<void()>& cb)
{
    this->railroadLogicService = val;
    this->callBack = cb;
    this->start(annotationsInRange);
}

void Action::onEnterTurnout(SplitterAnnotation*)
{
}

void Action::onLeaveTurnout(SplitterAnnotation*)
{
}

void Action::onTrainMoved(Train*)
{
}

void Action::start(QVector<Annotation*> annotationsInRange)
{
}

void Action::done()
{
    this->callBack();
}

QString Action::toString()
{
    return "ERROR: virtual action";
}
