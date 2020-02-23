#include "Action.h"

Action::Action()
{

}

void Action::execute(RailroadLogicService* val, std::function<void()> cb)
{
    this->railroadLogicService = val;
    this->callBack = cb;
    this->start();
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

void Action::start()
{
}

void Action::done()
{
    this->callBack();
}
