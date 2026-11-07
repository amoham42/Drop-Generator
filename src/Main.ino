// DropGen: droplet-on-demand generator for fluid-mechanics experiments (Harris Lab, Brown University).
//
// Core 1 runs the drop generator alone, so droplet timing never waits on Wi-Fi. Core 0 runs the
// web server, the WebSocket and the stepper motors.

#include "Drop_Gen.h"
#include "Motor_Cont.h"
#include "Wifi_Cont.h"
#include "param.h"
#include <Preferences.h>

Param param;
Preferences pref;
Generator DropGen;
Motor MotorCont;
WifiClass WiFiCont;

static const BaseType_t WEB_CORE = 0;
static const BaseType_t DROP_CORE = 1;

void dropTask(void*) { for (;;) DropGen.dgStart(); }
void controlTask(void*) { for (;;) WiFiCont.wfRun(MotorCont); }

void setup() {
  Serial.begin(115200);
  disableCore0WDT();  // the control task never blocks long enough to feed the idle task
  pref.begin("param", false);
  param.load(pref);
  DropGen.dgSetup();
  MotorCont.motorSetup();
  WiFiCont.wfSetup(MotorCont, DropGen);
  xTaskCreatePinnedToCore(dropTask, "DropGen", 4096, nullptr, 2, nullptr, DROP_CORE);
  xTaskCreatePinnedToCore(controlTask, "Controls", 10240, nullptr, 1, nullptr, WEB_CORE);
}

void loop() { vTaskDelay(portMAX_DELAY); }
