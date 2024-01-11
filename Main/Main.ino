#include "Drop_Gen.h"
#include "Wifi_Cont.h"
#include "param.h"

// Initializing two cores on ESP32
static const BaseType_t app_cpu = 0;
static const BaseType_t pro_cpu = 1;

// Initializing each controller
Generator DropGen;        // Drop generator Controller
Motor MotorCont;          // Motor Controller
WifiClass  WiFiCont;           // Wifi & server Controller 
Param param;              // Parameters

// Functions run by each core
void codeTask1(void* parameter){while(1){DropGen.dgStart();}}
void codeTask2(void* parameter){while(1){WiFiCont.wfRun(MotorCont);}}

// set up all the controllers initial values and making each core function
void setup(){
  Serial.begin(115200);
  DropGen.dgSetup();
  MotorCont.motorSetup();
  WiFiCont.wfSetup(MotorCont);
  xTaskCreatePinnedToCore(codeTask1, "DropGen", 10240, NULL, 1, NULL, pro_cpu);
  xTaskCreatePinnedToCore(codeTask2, "Controls", 10240, NULL, 1, NULL, app_cpu);
}
void loop(){vTaskDelay(portMAX_DELAY);}
