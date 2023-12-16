#ifndef PARAM_H
#define PARAM_H

struct Param {
public:

    // Wifi definitions.
    const char* ssid     = "Harris Lab Drop-Gen";
    const char* password = "";
    

    // Drop generator pin definitions
    const int DIR_A      = 4;          // Piezo PWN pin 1
    const int DIR_B      = 3;          // Piezo PWN pin 2
    const int trigger    = 5;          // Camera trigger
    const int led        = 21;          // BUILTIN_LED

    // Drop generator parameters
    int delayTime        = 15;         // Camera delay
    int pulsewidth       = 1000;       // Piezo pulse width
    int dropletDelay     = 500;        // Time between droplets
    int dropNum          = 100;        // Number of droplets
    int counterr         = dropNum;    // Counts droplet numbers
    bool camera          = false;      // Camera on/off
    bool staterr         = false;      // Droplet generator on/off

    // Motor pin definitions
    const int resDIAG    = 10;         // Reservoir motor diagnostic pin
    const int genDIAG    = 7;          // Generator motor diagnostic pin
    const int resDIR     = 9;          // Reservoir motor direction pin
    const int resSTEP    = 8;          // Reservoir motor step pin
    const int genDIR     = 1;          // Generator motor direction pin
    const int genSTEP    = 2;          // Generator motor step pin

    // Motor variables
    int reservMotPos     = 0;          // Reservoir motor position
    int genMotPos        = 0;          // Generator motor position
    int speed            = 1000;       // Motor speed
    int accel            = 500;        // Motor acceleration
    int travelLength     = 1;          // Motor travel length
    int roundSize        = 640;        // Motor round size
    int DIAG_THRESHOLD   = 50;         // Auto home over-current threshold

    // Utils
    bool changed = false;              // Server data update check
};
extern Param param;

#endif
