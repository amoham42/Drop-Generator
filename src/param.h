#ifndef PARAM_H
#define PARAM_H
#include <Preferences.h>

struct Param {

  public:
    
    // Wifi definitions.
    const bool WIFI_STATION = true;    // True for external WiFi connection
    const char* SSID        = "Brown-Guest";
    const char* PASSWORD    = "";
    const char* DNS_NAME    = "dropgen";
    
    // Drop generator pin definitions
    const int DIR_A      = 5;          // Piezo PWN pin 1
    const int DIR_B      = 4;          // Piezo PWN pin 2
    const int TRIG       = 6;          // Camera trigger
    const int LED        = 21;         // BUILTIN_LED

    // Motor pin definitions
    const int RES_DIAG    = 9;         // Reservoir motor diagnostic pin
    const int GEN_DIAG    = 44;        // Generator motor diagnostic pin
    const int RES_STEP    = 7;         // Reservoir motor step pin
    const int RES_DIR     = 8;         // Reservoir motor direction pin
    const int GEN_STEP    = 3;         // Generator motor step pin
    const int GEN_DIR     = 2;         // Generator motor direction pin

    // Drop generator parameters
    int delayTime        = 15;         // Camera delay
    int pulseWidth       = 1000;       // Piezo pulse width
    int dropDelay        = 500;        // Time between droplets
    int dropNum          = 100;        // Number of droplets
    volatile bool camera = false;      // Camera on/off
    volatile bool drop   = false;      // Droplet generator on/off
    int SHAKE_TIME       = 3000;       // Time length to shake the piezo
    int SHAKE_SPEED      = 20;         // Piezo vibration delay
    
    // Motor variables
    bool calibrate       = false;      // Calibrate the motors
    int tiltDeg          = 0;
    double resMotPos     = 0.0;        // Reservoir motor position
    double genMotPos     = 0.0;        // Generator motor position
    double travelLength  = 1;          // Motor travel length (mm)
    int SPEED            = 3200;       // Motor speed
    int ACCEL            = 3200;       // Motor acceleration
    int LEAD_SIZE        = 1280;       // Motor round size (steps / mm)
    int DIAG_THRESHOLD   = 50;         // Auto home over-current threshold

    void loadParameters(Preferences &pref){
        delayTime        = pref.getInt("DT", delayTime);
        pulseWidth       = pref.getInt("PW", pulseWidth);
        dropDelay        = pref.getInt("DD", dropDelay);
        dropNum          = pref.getInt("DN", dropNum);
        resMotPos        = pref.getInt("RMP", resMotPos);
        genMotPos        = pref.getInt("GMP", genMotPos);
        travelLength     = pref.getInt("TL", travelLength);
    }
};
extern Param param;

#endif
