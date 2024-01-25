#include <WiFi.h>
#include <sstream>
#include <ESPmDNS.h>
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
  motor.runMotor();
  server.handleClient();                         
  webSocket.loop();
  if(sendData) update(true);
}

void webSocketEvent(byte num, WStype_t type, uint8_t* payload, size_t length) {    

  switch (type) {
    case WStype_DISCONNECTED:
      Serial.println("Client " + String(num) + " disconnected");
      break;
    case WStype_CONNECTED:
      Serial.println("Client " + String(num) + " connected");
      sendData = true;
      break;
    case WStype_TEXT:
      if (length > 0) {
        if(payload[0] == 'p'){
          char* token = nullptr;

          token = strtok((char*)payload, "p");             // Pulse width 
          param.pulseWidth = atoi(token);
          token = strtok(NULL, "p");                       // Camera delay
          param.delayTime = atoi(token);
          token = strtok(NULL, "p");                       // Drop delay
          param.dropDelay = atoi(token);
          token = strtok(NULL, "p");                       // Drop number
          param.dropNum = atoi(token);
          token = strtok(NULL, "p");                       // Motor travel length
          param.travelLength = atof(token);
          token = strtok(NULL, "p");                       // Head tilt degree
          param.tiltDeg = atoi(token);
          token = strtok(NULL, "p");                       // Reservoir motor position
          param.resMotPos = atof(token);
          token = strtok(NULL, "p");                       // Generator motor position
          param.genMotPos = atof(token);
        }
        sendData = true;
      }
      break;
  }
}

// This function sets up the IP, name, and password
void WifiClass::wfSetup(Motor& motor, Generator& generator) {

  if(!SPIFFS.begin(true)) return;
  if(param.WIFI_STATION){
    WiFi.mode(WIFI_STA);
    WiFi.begin(param.SSID, param.PASSWORD);
    while(WiFi.status() != WL_CONNECTED){delay(1000); Serial.println("Connecting...");}
    Serial.print("Ip Address: ");
    Serial.println(WiFi.localIP());
  } else {
    WiFi.softAP(param.SSID, param.PASSWORD);
    delay(100);           // WiFi Setup Delay
    WiFi.softAPConfig(local_IP, gateway, subnet);
  }
  while(!MDNS.begin(param.DNS_NAME)) {Serial.println("No MDNS");}
  setupServer(motor,generator);
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("http", "udp", 80);
}

void WifiClass::update(bool textUpdate){

  if(textUpdate){
    std::ostringstream oss;
    oss << (param.drop ? "1" : "0")
        << (param.camera ? " 1" : " 0")
        << ' ' << param.pulseWidth
        << ' ' << param.delayTime
        << ' ' << param.dropDelay
        << ' ' << param.dropNum
        << ' ' << param.travelLength
        << ' ' << param.tiltDeg
        << ' ' << param.resMotPos
        << ' ' << param.genMotPos;
    webSocket.broadcastTXT(oss.str().c_str());
    server.send(200, "text/plain", "");
  } else server.send(200, "text/plain", "");
  sendData = false;
}

// Sets up the links for requested functions and files 
void WifiClass::setupServer(Motor& motor, Generator& generator){

  // Files
  server.onNotFound([this]() {handleFile((char*)"/index.html", (char*)"text/html");});
  server.on("/",            HTTP_GET, [this]() {handleFile((char*)"/index.html", (char*)"text/html");});
  server.on("/camera.png",  HTTP_GET, [this]() {handleFile((char*)"/camera.png", (char*)"image/png");});
  server.on("/hlogo.png",   HTTP_GET, [this]() {handleFile((char*)"/hlogo.png", (char*)"image/png");});
  server.on("/config.css",  HTTP_GET, [this]() {handleFile((char*)"/config.css", (char*)"text/css");});
  server.on("/database.js", HTTP_GET, [this]() {handleFile((char*)"/database.js", (char*)"text/javascript");});

  // Functions
  server.on("/genUP",    [&motor, this]() {motor.move( 1, 0); update(true);});
  server.on("/resUP",    [&motor, this]() {motor.move( 1, 1); update(true);});
  server.on("/mainUP",   [&motor, this]() {motor.move( 1, 2); update(true);});
  server.on("/genDOWN",  [&motor, this]() {motor.move(-1, 0); update(true);});
  server.on("/resDOWN",  [&motor, this]() {motor.move(-1, 1); update(true);});
  server.on("/mainDOWN", [&motor, this]() {motor.move(-1, 2); update(true);});
  server.on("/stop",     [&motor, this]() {motor.stop(); update(true);});
  server.on("/shake",    [&generator]() {generator.shake();});
  server.on("/go",       [&motor, this]() {motor.moveToAbsolute(); update(true);});
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