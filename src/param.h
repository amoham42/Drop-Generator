#ifndef PARAM_H
#define PARAM_H

#include <Arduino.h>
#include <Preferences.h>

// Everything the controllers share. Settings the user changes from the web UI are persisted in
// flash (Preferences), so the device comes back up the way it was left.
//
// Threading: the drop generator runs on one core, the web server and motors on the other.
// Fields written by one task and read by the other are volatile; nothing here needs a lock
// because each field has a single writer.
struct Param {
  // ---- network ----
  // Joins this network first; if that fails within WIFI_TIMEOUT_MS, the board starts its own
  // access point (AP_SSID / AP_PASSWORD) so the UI is always reachable. Open http://dropgen.local
  // (or the printed IP) in a browser.
  const char* SSID            = "Brown-Guest";
  const char* PASSWORD        = "";
  const char* AP_SSID         = "DropGen";
  const char* AP_PASSWORD     = "";        // empty = open network; otherwise at least 8 characters
  const char* DNS_NAME        = "dropgen";
  const uint32_t WIFI_TIMEOUT_MS = 20000;

  // ---- pins (Seeed XIAO ESP32-S3) ----
  const int DIR_A    = 5;    // H-bridge input 1 (piezo)
  const int DIR_B    = 4;    // H-bridge input 2 (piezo)
  const int TRIG     = 6;    // camera trigger
  const int LED      = 21;   // on-board LED
  const int RES_DIAG = 9;    // reservoir stepper driver stall/diagnostic output
  const int GEN_DIAG = 44;   // generator stepper driver stall/diagnostic output
  const int RES_STEP = 7;
  const int RES_DIR  = 8;
  const int GEN_STEP = 3;
  const int GEN_DIR  = 2;

  // ---- droplet settings (persisted) ----
  int pulseWidth   = 1000;   // piezo pulse, microseconds
  int delayTime    = 15;     // camera trigger after the pulse, ms
  int dropDelay    = 500;    // time between droplets, ms
  int dropNum      = 100;    // droplets per run
  float travelLength = 1.0f; // jog step for the motors, mm
  int tiltDeg      = 0;      // recorded with the run; the head is tilted by hand

  // ---- live state ----
  volatile bool drop     = false;  // generator running
  volatile bool camera   = false;  // trigger the camera after each droplet
  volatile int dropsLeft = 0;      // counts down during a run
  volatile bool shakeRequested = false;
  volatile bool stateChanged   = false;  // tells the web task to broadcast

  // ---- motors ----
  float resMotPos = 0.0f;  // target positions, mm from home
  float genMotPos = 0.0f;
  const int SPEED     = 3200;  // steps/s
  const int ACCEL     = 3200;  // steps/s^2
  const int LEAD_SIZE = 1280;  // steps per mm of travel
  const float HOMING_TRAVEL_MM = 60.0f;  // furthest a motor may run while looking for its end stop

  // ---- shaking: buzz the piezo to clear bubbles from the nozzle ----
  const int SHAKE_TIME  = 3000;  // ms
  const int SHAKE_SPEED = 20;    // ms between pulses

  // Valid ranges. Out-of-range input from the UI is clamped, never trusted.
  void clampSettings() {
    pulseWidth   = constrain(pulseWidth, 1, 10000);  // interrupts are off during the pulse
    delayTime    = constrain(delayTime, 0, 10000);
    dropDelay    = constrain(dropDelay, 1, 60000);
    dropNum      = constrain(dropNum, 1, 100000);
    travelLength = constrain(travelLength, 0.01f, 50.0f);
    tiltDeg      = constrain(tiltDeg, -90, 90);
  }

  void load(Preferences& pref) {
    pulseWidth   = pref.getInt("PW", pulseWidth);
    delayTime    = pref.getInt("DT", delayTime);
    dropDelay    = pref.getInt("DD", dropDelay);
    dropNum      = pref.getInt("DN", dropNum);
    travelLength = pref.getFloat("TL", travelLength);
    tiltDeg      = pref.getInt("TD", tiltDeg);
    resMotPos    = pref.getFloat("RMP", resMotPos);
    genMotPos    = pref.getFloat("GMP", genMotPos);
    clampSettings();
  }

  void save(Preferences& pref) const {
    pref.putInt("PW", pulseWidth);
    pref.putInt("DT", delayTime);
    pref.putInt("DD", dropDelay);
    pref.putInt("DN", dropNum);
    pref.putFloat("TL", travelLength);
    pref.putInt("TD", tiltDeg);
    pref.putFloat("RMP", resMotPos);
    pref.putFloat("GMP", genMotPos);
  }
};

extern Param param;
extern Preferences pref;

#endif
