#include <WiFi.h>
#include <sstream>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include "SPIFFS.h"
#include "Wifi_Cont.h"
#include "Motor_Cont.h"
#include "param.h"

WebServer server(80);                                 
WebSocketsServer webSocket = WebSocketsServer(81);  
IPAddress local_IP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);   // Access point IP
IPAddress subnet(255, 255, 255, 0);
bool sendData = false;

// Main page for server and captures values sent by server
void WifiClass::wfRun(Motor& motor) {

  server.handleClient();                         
  webSocket.loop();
  if (param.changed) update(true);
  if(sendData) update(true);
  motor.runMotor();
}

void webSocketEvent(byte num, WStype_t type, uint8_t* payload, size_t length) {    

  switch (type) {
    case WStype_DISCONNECTED:
      Serial.println("Client " + String(num) + " disconnected");
      break;
    case WStype_CONNECTED:
      Serial.println("Client " + String(num) + " connected");
      break;
    case WStype_TEXT:
      if (length > 0) {
        if(payload[0] == 'p'){
          char* token = nullptr;

          // Pulse width token
          if (token = strtok(nullptr, "p")) param.pulsewidth = atoi(token);
          
          // Camera delay token
          if (token = strtok(nullptr, "p")) param.delayTime = atoi(token);
          
          // Drop delay token
          if (token = strtok(nullptr, "p")) param.dropDelay = atoi(token);
          

          // Drop number token
          if (token = strtok(nullptr, "p")) {
            param.dropNum = atoi(token);
            param.counter = param.dropNum;
          }

          // Motor travel length token
          if (token = strtok(nullptr, "p")) param.travelLength = atoi(token);
          
          // Head tilt degree
          if (token = strtok(nullptr, "p")) param.tiltDegree = atoi(token);
          
          // Reservoir motor position
          if (token = strtok(nullptr, "p")) param.resMotPos = atoi(token);
          
          // Generator motor position
          if (token = strtok(nullptr, "p")) param.genMotPos = atoi(token);
        }
        sendData = true;
      }
      break;
  }
}

// This function sets up the IP, name, and password
void WifiClass::wfSetup(Motor& motor) {

  if(!SPIFFS.begin(true)) return;
  WiFi.softAP(param.ssid, param.password);
  delay(100);           // WiFi Setup Delay
  WiFi.softAPConfig(local_IP, gateway, subnet);
  setupServer(motor);
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void WifiClass::update(bool textUpdate){

  if(textUpdate){
    std::ostringstream oss;
    oss << (param.drop ? "1" : "0")
        << (param.camera ? " 1" : " 0")
        << ' ' << param.pulsewidth
        << ' ' << param.delayTime
        << ' ' << param.dropDelay
        << ' ' << param.counter
        << ' ' << param.travelLength
        << ' ' << param.tiltDegree
        << ' ' << param.resMotPos
        << ' ' << param.genMotPos;
    webSocket.broadcastTXT(oss.str().c_str());
    param.changed = false;
  } else server.send(200, "text/plain", "");
  sendData = false;
}

// Sets up the links for requested functions and files 
void WifiClass::setupServer(Motor& motor){

  // Files
  server.on("/",            HTTP_GET, [this]() {handleFile("/index.html", "text/html");});
  server.on("/camera.png",  HTTP_GET, [this]() {handleFile("/camera.png", "image/png");});
  server.on("/hlogo.png",   HTTP_GET, [this]() {handleFile("/hlogo.png", "image/png");});
  server.on("/config.css",  HTTP_GET, [this]() {handleFile("/config.css", "text/css");});
  server.on("/database.js", HTTP_GET, [this]() {handleFile("/database.js", "text/javascript");});

  // Functions
  server.on("/genUP",    [&motor, this]() {motor.move( 1, 0); update(false);});
  server.on("/resUP",    [&motor, this]() {motor.move( 1, 1); update(false);});
  server.on("/mainUP",   [&motor, this]() {motor.move( 1, 2); update(false);});
  server.on("/genDOWN",  [&motor, this]() {motor.move(-1, 0); update(false);});
  server.on("/resDOWN",  [&motor, this]() {motor.move(-1, 1); update(false);});
  server.on("/mainDOWN", [&motor, this]() {motor.move(-1, 2); update(false);});
  server.on("/calib",    [&motor, this]() {param.calibrate = true; update(false);});
  server.on("/dropgen",  [&motor, this]() {param.drop = !param.drop; update(true);});
  server.on("/camera",   [&motor, this]() {param.camera = !param.camera; update(true);});
}

// Send necessary extension files saved in SPIFFS memory to server
void WifiClass::handleFile(char* name, char* type){

  File file = SPIFFS.open(name, "r");
  if (file) {
    server.send(200, type, file.readString());
    file.close();
  } else {
    server.send(404, "text/plain", "File not found");
  }
}