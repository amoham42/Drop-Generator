#include "Drop_Gen.h"
#include "Wifi_Cont.h"
#include "param.h"

// Initializing two cores on ESP32
static const BaseType_t app_cpu = 0;
static const BaseType_t pro_cpu = 1;

// Initializing each controller
dclass DropGen;         // Drop generator Controller
wclass WiFiCont;        // Wifi & server Controller 
mclass motorCont;       // Motor Controller
Param param;            // Parameters

// Functions run by each core
void codeTask1(void* parameter){while(1){DropGen.dgStart();}}
void codeTask2(void* parameter){while(1){WiFiCont.wfRun(motorCont);}}

// set up all the controllers initial values and making each core function
void setup(){
  Serial.begin(115200);
  DropGen.dgSetup();
  WiFiCont.wfSetup();
  motorCont.motorSetup();
  xTaskCreatePinnedToCore(codeTask1, "DropGen", 10240, NULL, 1, NULL, pro_cpu);
  xTaskCreatePinnedToCore(codeTask2, "Controls", 10240, NULL, 1, NULL, app_cpu);
}
void loop(){}
