#include <AccelStepper.h>
#include "Motor_Cont.h"
#include "param.h"

static AccelStepper resStep(AccelStepper::DRIVER, param.RES_STEP, param.RES_DIR);
static AccelStepper genStep(AccelStepper::DRIVER, param.GEN_STEP, param.GEN_DIR);

// Set by the stall interrupts, handled in runMotor(). The ISRs only raise a flag: AccelStepper
// does floating-point maths, and using the FPU inside an ESP32 interrupt crashes the core.
static volatile bool resStallFlag = false;
static volatile bool genStallFlag = false;
static bool homing = false;
static bool resHomed = false, genHomed = false;

static void IRAM_ATTR resStalled() { resStallFlag = true; }
static void IRAM_ATTR genStalled() { genStallFlag = true; }

static long toSteps(float mm) { return lroundf(mm * param.LEAD_SIZE); }
static float toMm(long steps) { return (float)steps / param.LEAD_SIZE; }

void Motor::motorSetup() {
  for (AccelStepper* s : {&resStep, &genStep}) {
    s->setMaxSpeed(param.SPEED);
    s->setAcceleration(param.ACCEL);
  }
  resStep.setCurrentPosition(toSteps(param.resMotPos));
  genStep.setCurrentPosition(toSteps(param.genMotPos));

  pinMode(param.RES_DIAG, INPUT_PULLUP);
  pinMode(param.GEN_DIAG, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(param.RES_DIAG), resStalled, RISING);
  attachInterrupt(digitalPinToInterrupt(param.GEN_DIAG), genStalled, RISING);
}

// A stall halts the motor where it is: during homing that's the end stop, otherwise something
// is in the way.
static void handleStall(AccelStepper& stepper, volatile bool& flag, float& target, bool& homed) {
  if (!flag) return;
  flag = false;
  if (!stepper.isRunning()) return;  // noise on the line while idle
  if (homing) {
    stepper.setCurrentPosition(0);
    target = 0.0f;
    homed = true;
  } else {
    stepper.setCurrentPosition(stepper.currentPosition());  // stops immediately
    target = toMm(stepper.currentPosition());
  }
  param.stateChanged = true;
}

void Motor::runMotor() {
  handleStall(resStep, resStallFlag, param.resMotPos, resHomed);
  handleStall(genStep, genStallFlag, param.genMotPos, genHomed);

  if (homing) {
    // a motor that ran its full homing travel without stalling is treated as home too
    if (!resHomed && !resStep.isRunning()) { resStep.setCurrentPosition(0); param.resMotPos = 0; resHomed = true; }
    if (!genHomed && !genStep.isRunning()) { genStep.setCurrentPosition(0); param.genMotPos = 0; genHomed = true; }
    if (resHomed && genHomed) {
      homing = false;
      param.save(pref);
      param.stateChanged = true;
    }
  }

  resStep.run();
  genStep.run();
}

void Motor::calibrate() {
  homing = true;
  resHomed = genHomed = false;
  resStallFlag = genStallFlag = false;
  resStep.setCurrentPosition(0);
  genStep.setCurrentPosition(0);
  resStep.moveTo(-toSteps(param.HOMING_TRAVEL_MM));
  genStep.moveTo(-toSteps(param.HOMING_TRAVEL_MM));
}

static void jog(AccelStepper& stepper, float& target, int dir) {
  if (stepper.isRunning()) return;
  target += param.travelLength * dir;
  stepper.moveTo(toSteps(target));
}

void Motor::move(int dir, int motor) {
  if (homing) return;
  if (motor == 0 || motor == 2) jog(genStep, param.genMotPos, dir);
  if (motor == 1 || motor == 2) jog(resStep, param.resMotPos, dir);
}

void Motor::moveToAbsolute() {
  if (homing) return;
  resStep.moveTo(toSteps(param.resMotPos));
  genStep.moveTo(toSteps(param.genMotPos));
}

// Decelerate both motors; the targets become wherever they come to rest.
void Motor::stop() {
  homing = false;
  resStep.stop();
  genStep.stop();
  param.resMotPos = toMm(resStep.targetPosition());
  param.genMotPos = toMm(genStep.targetPosition());
}

void Motor::reset() {
  homing = false;
  resStep.setCurrentPosition(0);  // also cancels any move
  genStep.setCurrentPosition(0);
  param.resMotPos = 0.0f;
  param.genMotPos = 0.0f;
  param.save(pref);
}

bool Motor::isMoving() { return resStep.isRunning() || genStep.isRunning(); }
float Motor::resPosition() { return toMm(resStep.currentPosition()); }
float Motor::genPosition() { return toMm(genStep.currentPosition()); }
