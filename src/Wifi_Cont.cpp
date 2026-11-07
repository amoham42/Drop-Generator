#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include "SPIFFS.h"
#include "Wifi_Cont.h"
#include "param.h"

static WebServer server(80);
static WebSocketsServer webSocket(81);
static const IPAddress apIP(192, 168, 4, 1), apMask(255, 255, 255, 0);
static uint32_t lastBroadcast = 0;

// "p<pulse>p<camDelay>p<dropDelay>p<dropNum>p<travel>p<tilt>p<resPos>p<genPos>" from the Submit
// and GO buttons. Every field must parse; anything missing or malformed rejects the whole message.
static bool parseSettings(char* msg) {
  float v[8];
  char* save = nullptr;
  char* token = strtok_r(msg, "p", &save);
  for (int i = 0; i < 8; i++) {
    if (!token) return false;
    char* end = nullptr;
    v[i] = strtof(token, &end);
    if (end == token) return false;
    token = strtok_r(nullptr, "p", &save);
  }
  param.pulseWidth   = (int)v[0];
  param.delayTime    = (int)v[1];
  param.dropDelay    = (int)v[2];
  param.dropNum      = (int)v[3];
  param.travelLength = v[4];
  param.tiltDeg      = (int)v[5];
  param.resMotPos    = v[6];
  param.genMotPos    = v[7];
  param.clampSettings();
  param.save(pref);
  return true;
}

static void onSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      Serial.printf("client %u connected\n", num);
      param.stateChanged = true;
      break;
    case WStype_DISCONNECTED:
      Serial.printf("client %u disconnected\n", num);
      break;
    case WStype_TEXT:
      if (length > 0 && payload[0] == 'p' && !parseSettings(reinterpret_cast<char*>(payload)))
        Serial.println("rejected malformed settings message");
      param.stateChanged = true;
      break;
    default:
      break;
  }
}

// Joins the lab network, or falls back to an access point of its own.
void WifiClass::connect() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(param.SSID, param.PASSWORD);
  Serial.printf("joining %s", param.SSID);
  const uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < param.WIFI_TIMEOUT_MS) {
    delay(500);
    Serial.print('.');
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\nconnected, open http://%s.local or http://%s\n", param.DNS_NAME, WiFi.localIP().toString().c_str());
    return;
  }
  WiFi.disconnect(true);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, apMask);
  WiFi.softAP(param.AP_SSID, strlen(param.AP_PASSWORD) >= 8 ? param.AP_PASSWORD : nullptr);
  Serial.printf("\nno network; join Wi-Fi \"%s\" and open http://%s\n", param.AP_SSID, apIP.toString().c_str());
}

void WifiClass::wfSetup(Motor& motor, Generator& generator) {
  if (!SPIFFS.begin(true)) Serial.println("SPIFFS mount failed; did you run `pio run -t uploadfs`?");
  connect();
  if (MDNS.begin(param.DNS_NAME)) MDNS.addService("http", "tcp", 80);
  setupServer(motor, generator);
  server.begin();
  webSocket.begin();
  webSocket.onEvent(onSocketEvent);
}

void WifiClass::wfRun(Motor& motor) {
  motor.runMotor();
  server.handleClient();
  webSocket.loop();
  // push state on every change, and ~5x a second while motors move so positions stay live
  const uint32_t now = millis();
  if (param.stateChanged || (motor.isMoving() && now - lastBroadcast > 200)) broadcast(motor);
}

// "drop camera pulse camDelay dropDelay dropNum travel tilt resPos genPos dropsLeft moving"
void WifiClass::broadcast(Motor& motor) {
  param.stateChanged = false;
  lastBroadcast = millis();
  char msg[160];
  snprintf(msg, sizeof(msg), "%d %d %d %d %d %d %.2f %d %.2f %.2f %d %d", param.drop ? 1 : 0, param.camera ? 1 : 0,
           param.pulseWidth, param.delayTime, param.dropDelay, param.dropNum, param.travelLength, param.tiltDeg,
           motor.resPosition(), motor.genPosition(), (int)param.dropsLeft, motor.isMoving() ? 1 : 0);
  webSocket.broadcastTXT(msg);
}

static void serveFile(const char* path, const char* type) {
  File file = SPIFFS.open(path, "r");
  if (!file) {
    server.send(404, "text/plain", "File not found");
    return;
  }
  server.streamFile(file, type);  // binary-safe, unlike reading into a String
  file.close();
}

void WifiClass::setupServer(Motor& motor, Generator& generator) {
  // UI files
  server.on("/", HTTP_GET, [] { serveFile("/index.html", "text/html"); });
  server.on("/camera.png", HTTP_GET, [] { serveFile("/camera.png", "image/png"); });
  server.on("/hlogo.png", HTTP_GET, [] { serveFile("/hlogo.png", "image/png"); });
  server.on("/config.css", HTTP_GET, [] { serveFile("/config.css", "text/css"); });
  server.on("/database.js", HTTP_GET, [] { serveFile("/database.js", "text/javascript"); });
  server.onNotFound([] { serveFile("/index.html", "text/html"); });

  // buttons: act, reply at once, and let the WebSocket carry the new state
  auto action = [](std::function<void()> fn) {
    return [fn] {
      fn();
      param.stateChanged = true;
      server.send(204);
    };
  };
  server.on("/genUP", action([&motor] { motor.move(1, 0); }));
  server.on("/resUP", action([&motor] { motor.move(1, 1); }));
  server.on("/mainUP", action([&motor] { motor.move(1, 2); }));
  server.on("/genDOWN", action([&motor] { motor.move(-1, 0); }));
  server.on("/resDOWN", action([&motor] { motor.move(-1, 1); }));
  server.on("/mainDOWN", action([&motor] { motor.move(-1, 2); }));
  server.on("/go", action([&motor] { motor.moveToAbsolute(); }));
  server.on("/stop", action([&motor] { motor.stop(); }));
  server.on("/reset", action([&motor] { motor.reset(); }));
  server.on("/calib", action([&motor] { motor.calibrate(); }));
  server.on("/dropgen", action([&generator] { param.drop ? generator.stopRun() : generator.startRun(); }));
  server.on("/camera", action([] { param.camera = !param.camera; }));
  server.on("/shake", action([] { param.shakeRequested = true; }));  // done by the generator task
}
