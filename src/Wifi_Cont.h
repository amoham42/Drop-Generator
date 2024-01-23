#ifndef Wifi_Cont_H
#define Wifi_Cont_H

#include "Drop_Gen.h"
#include "Motor_Cont.h"

class WifiClass{
  public:
  
  // Functions that are used by other controllers
    void update(bool);
    void wfSetup(Motor&);
    void wfRun(Motor&);

  private:
    void setupServer(Motor&);
    void handleFile(char*, char*);
};

#endif