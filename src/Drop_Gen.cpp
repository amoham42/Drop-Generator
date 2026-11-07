#include <Arduino.h>
#include "soc/gpio_reg.h"
#include "Drop_Gen.h"
#include "param.h"

#define PIEZO_A (1UL << param.DIR_A)
#define PIEZO_B (1UL << param.DIR_B)
#define TRIGGER (1UL << param.TRIG)

void Generator::dgSetup() {
  pinMode(param.LED, OUTPUT);
  pinMode(param.DIR_A, OUTPUT);
  pinMode(param.DIR_B, OUTPUT);
  pinMode(param.TRIG, OUTPUT);
  digitalWrite(param.TRIG, LOW);
  digitalWrite(param.LED, HIGH);
}

void Generator::startRun() {
  param.dropsLeft = param.dropNum;
  param.drop = true;
  param.stateChanged = true;
}

void Generator::stopRun() {
  param.drop = false;
  param.dropsLeft = 0;
  param.stateChanged = true;
}

// Runs forever on its own core. Interrupts are off only for the microseconds of the pulse itself,
// so its width doesn't jitter with Wi-Fi traffic.
void Generator::dgStart() {
  if (param.shakeRequested) {
    param.shakeRequested = false;
    shake();
  }

  if (!param.drop || param.dropsLeft <= 0) {
    if (param.drop) stopRun();  // run finished
    delay(5);
    return;
  }

  const uint32_t start = millis();
  noInterrupts();
  generate();
  interrupts();
  if (param.camera) triggerCamera();

  param.dropsLeft = param.dropsLeft - 1;
  if (param.dropsLeft <= 0) {
    stopRun();
    return;
  }
  param.stateChanged = true;

  // keep the droplet period exact whether or not the camera delay was spent above
  const uint32_t elapsed = millis() - start;
  if (elapsed < (uint32_t)param.dropDelay) delay(param.dropDelay - elapsed);
}

void Generator::triggerCamera() {
  delay(param.delayTime);
  REG_WRITE(GPIO_OUT_W1TS_REG, TRIGGER);
  delayMicroseconds(6);
  REG_WRITE(GPIO_OUT_W1TC_REG, TRIGGER);
}

// Buzzes the piezo for a few seconds to shake bubbles out of the nozzle.
void Generator::shake() {
  const uint32_t start = millis();
  while (millis() - start < (uint32_t)param.SHAKE_TIME) {
    noInterrupts();
    generate();
    interrupts();
    delay(param.SHAKE_SPEED);
  }
}

void IRAM_ATTR Generator::generate() {
  REG_WRITE(GPIO_OUT_W1TS_REG, PIEZO_A);  // A high
  REG_WRITE(GPIO_OUT_W1TC_REG, PIEZO_B);  // B low
  delayMicroseconds(param.pulseWidth);
  REG_WRITE(GPIO_OUT_W1TC_REG, PIEZO_A);  // A low
  REG_WRITE(GPIO_OUT_W1TS_REG, PIEZO_B);  // B high: hold the opposite polarity until the next droplet
}
