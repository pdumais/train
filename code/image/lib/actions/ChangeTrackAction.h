#ifndef ACTIVATETURNOUTACTION_H
#define ACTIVATETURNOUTACTION_H

#include "Action.h"

#include <splitterannotation.h>



class ActivateTurnoutAction : public Action
{
public:
    ActivateTurnoutAction(SplitterAnnotation *sa, bool activate);

   SplitterAnnotation* getSplitterAnnotation();
   bool getActivate();

private:
    SplitterAnnotation *sa;
    bool activate;
};

#endif // ACTIVATETURNOUTACTION_H
