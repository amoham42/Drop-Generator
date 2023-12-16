#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include "Wifi_Cont.h"
#include "Motor_Cont.h"
#include "index_html.h"
#include "param.h"

WebServer server(80);                                 
WebSocketsServer webSocket = WebSocketsServer(81);  
IPAddress local_IP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);                                                        // Access point IP
IPAddress subnet(255, 255, 255, 0);
char Track = NULL;

// Main page for server and captures values sent by server
void wclass::wfRun(mclass motor) {

  server.handleClient();                         
  webSocket.loop();
  if (param.changed) { 
    char char_array[64];
    int index = 0;
    char_array[index++] = param.staterr ? '1' : '0';
    char_array[index++] = param.camera ? ' 1' : ' 0';
    index += sprintf(&char_array[index], " %d", param.pulsewidth);
    index += sprintf(&char_array[index], " %d", param.delayTime);
    index += sprintf(&char_array[index], " %d", param.dropletDelay);
    index += sprintf(&char_array[index], " %d", param.counterr);
    index += sprintf(&char_array[index], " %d", param.travelLength);
    char_array[index] = '\0';
    webSocket.broadcastTXT(char_array);
    if(Track != NULL){controller(Track, motor);}
    char Track = NULL;
    param.changed = false;                          
  }
}

void wclass::controller(char &payload, mclass motor){

  if(payload == 't'){motor.reservMotor(param.roundSize *param.travelLength);}              //Reservoir up
  if(payload == 'y'){motor.reservMotor(param.roundSize * -param.travelLength);}            //Reservoir down
  if(payload == 'g'){motor.genMotor(param.roundSize * param.travelLength);}                //Drop-gen up
  if(payload == 'h'){motor.genMotor(param.roundSize * -param.travelLength);}               //Drop-gen down
  if(payload == 'b'){motor.genMotor(param.roundSize * param.travelLength);                 //Both up
                     motor.reservMotor(param.roundSize *param.travelLength);}       
  if(payload == 'o'){motor.genMotor(param.roundSize * -param.travelLength);                //Both down
                     motor.reservMotor(param.roundSize * -param.travelLength);}
  if(payload == 'z'){motor.calibration();}                                                 //calibration
}

void webSocketEvent(byte num, WStype_t type, uint8_t * payload, size_t length) {    

  switch (type) {                                    
    case WStype_DISCONNECTED:                         
      Serial.println("Client " + String(num) + " disconnected");
      break;
    case WStype_CONNECTED:                           
      Serial.println("Client " + String(num) + " connected");
      break;
    case WStype_TEXT:           
      if (length > 0) {
        param.changed = true;
        if(payload[0] == 'p'){
          char* token = nullptr;

          token = strtok((char*)payload, "p");                                              // Pulse width token
          param.pulsewidth = atoi(token);
          token = strtok(NULL, "p");                                                       // Camera delay token
          param.delayTime = atoi(token);
          token = strtok(NULL, "p");                                                       // Drop delay token
          param.dropletDelay = atoi(token);
          token = strtok(NULL, "p");                                                       // Drop number token
          param.dropNum = atoi(token);
          param.counterr = param.dropNum;
          token = strtok(NULL, "p");                                                       // Motor travel length token
          param.travelLength = atoi(token);
        } else {Track = (char)payload[0];}           
      } 
    break;
  }
}

// This function sets up the IP, name, and password
void wclass::wfSetup() {

  WiFi.softAP(param.ssid, param.password);
  delay(100);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  server.on("/", []() {server.send(200, "text/html", main_page);});
  server.on("/dropgen", []() {param.staterr = !param.staterr; server.send(200, "text/plain", "");});
  server.on("/camera", []() {param.camera = !param.camera; server.send(200, "text/plain", "");});
  server.begin();                                    
  webSocket.begin();                                 
  webSocket.onEvent(webSocketEvent); 
}
