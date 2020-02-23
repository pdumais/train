#ifndef ACTION_H
#define ACTION_H

#include <functional>
#include <RailroadLogicService.h>
#include <splitterannotation.h>



class Action
{
public:
    Action();
    virtual ~Action() {}

    virtual void execute(RailroadLogicService* railroadLogicService, std::function<void()> callBack);

    virtual void onEnterTurnout(SplitterAnnotation* sa);
    virtual void onLeaveTurnout(SplitterAnnotation* sa);
    virtual void onTrainMoved(Train* train);

protected:
    RailroadLogicService* railroadLogicService;
    void done();
    virtual void start();

private:
    std::function<void()> callBack;
};

#endif // ACTION_H
