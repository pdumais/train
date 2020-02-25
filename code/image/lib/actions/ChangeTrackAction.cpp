#include "ActivateTurnoutAction.h"

ActivateTurnoutAction::ActivateTurnoutAction(SplitterAnnotation* sa, bool activate)
{
    this->sa = sa;
    this->activate = activate;
}


SplitterAnnotation* ActivateTurnoutAction::getSplitterAnnotation()
{
    return this->sa;
}

bool ActivateTurnoutAction::getActivate()
{
    return this->activate;
}

