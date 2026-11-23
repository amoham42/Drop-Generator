#ifndef Wifi_Cont_H
#define Wifi_Cont_H

#include "Drop_Gen.h"
#include "Motor_Cont.h"

// Wi-Fi, the HTTP server for the web UI (files in SPIFFS, buttons as endpoints) and a WebSocket
// that pushes the device state to every open browser.
class WifiClass {
  public:
    void wfSetup(Motor& motor, Generator& generator);
    void wfRun(Motor& motor);  // one iteration of the web/motor task

  private:
    void connect();
    void setupServer(Motor& motor, Generator& generator);
    void broadcast(Motor& motor);
};

#endif
