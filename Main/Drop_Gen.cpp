#include <Arduino.h>
#include "Drop_Gen.h"
#include "param.h"

// setup function for L298N H-bridge and camera
void Generator::dgSetup(){
  pinMode(param.led,      OUTPUT);
  pinMode(param.DIR_A,    OUTPUT);
  pinMode(param.DIR_B,    OUTPUT);
  pinMode(param.trigger,  OUTPUT);
  digitalWrite(param.led, HIGH);                  // BUILTIN_LED LIGHT ON
}

// piezodisk trigger with camera function
void Generator::dgStart(){
  if(param.drop){
    generate();
    if(param.camera) camera();                   // Camera switch
    param.counter--;                             // Drops left
    if(param.counter <= 0){
      param.drop = false;
      param.counter = param.dropNum;
    }
    param.changed = true;
    delay(param.dropDelay);                    // Drop delay
  }
}

void Generator::camera(){
  delay(param.delayTime);
  digitalWrite(param.trigger, HIGH);
  delayMicroseconds(6);
  digitalWrite(param.trigger, LOW);
}

void Generator::generate(){
  noInterrupts();
  digitalWrite(param.DIR_A, HIGH);              // Turn on left : high low
  digitalWrite(param.DIR_B, LOW);
  delayMicroseconds(param.pulsewidth);          // Pulse width
  digitalWrite(param.DIR_A, LOW);               // Turn on right : low high
  digitalWrite(param.DIR_B, HIGH);
  interrupts();
}
