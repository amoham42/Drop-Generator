#ifndef Wifi_Cont_H
#define Wifi_Cont_H

#include "Drop_Gen.h"
#include "Motor_Cont.h"

class wclass{
  public:
  
  // Functions that are used by other controllers
    void wfSetup();
    void wfRun(mclass);
    void controller(char&, mclass);
};

#endif
