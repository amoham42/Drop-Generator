#include <AccelStepper.h>
#include "Motor_Cont.h"
#include "param.h"

// Initializing the motors
AccelStepper resStep(AccelStepper::DRIVER, param.RES_STEP, param.RES_DIR);
AccelStepper genStep(AccelStepper::DRIVER, param.GEN_STEP, param.GEN_DIR);

volatile bool stopRes = false;
volatile bool stopGen = false;

void IRAM_ATTR resStalled(){
  if(digitalRead(param.RES_DIAG) > param.DIAG_THRESHOLD){
    resStep.setAcceleration(200000);
    resStep.moveTo(param.resMotPos);
    resStep.setAcceleration(param.ACCEL);
    stopRes = true;
  }
}

void IRAM_ATTR genStalled(){
  if(digitalRead(param.GEN_DIAG) > param.DIAG_THRESHOLD){
    genStep.setAcceleration(200000);
    genStep.moveTo(param.genMotPos);
    genStep.setAcceleration(param.ACCEL);
    stopGen = true;
  }
}

void Motor::stop(){
  if(resStep.isRunning()){
    param.resMotPos -= resStep.distanceToGo();
    resStep.stop();
  } else if (genStep.isRunning()){
    param.genMotPos -= genStep.distanceToGo();
    genStep.stop();
  }
}

// This function sets up the motor acceleration, position, and speed
void Motor::motorSetup(){

  resStep.setMaxSpeed(param.SPEED);
  resStep.setAcceleration(param.ACCEL);
  resStep.setCurrentPosition(param.resMotPos);
  attachInterrupt(digitalPinToInterrupt(param.RES_DIAG),
                                          resStalled, RISING);
  
  genStep.setMaxSpeed(param.SPEED);
  genStep.setAcceleration(param.ACCEL);
  genStep.setCurrentPosition(param.genMotPos);
  pinMode(param.GEN_DIAG, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(param.GEN_DIAG),
                                          genStalled, RISING);
}

// This function runs the motors
void Motor::runMotor(){
  if(param.calibrate) calibrate();
  resStep.runToPosition();
  genStep.runToPosition();
}

void Motor::move(int dir, int motor){
  switch(motor) {
    case 0: // Generator Motor
      calculateDist(param.genMotPos, genStep, dir);
      break;
    
    case 1: // Reservoir Motor
      calculateDist(param.resMotPos, resStep, dir);
      break;
    
    case 2: // Both Motors
      calculateDist(param.genMotPos, genStep, dir);
      calculateDist(param.resMotPos, resStep, dir);
      break;

    default:
      Serial.println("Invalid motor identifier");
      break;
    }
}

void Motor::calculateDist(int& motorPos, AccelStepper& stepper, int dir) {
  if(!stepper.isRunning()){
    motorPos += param.travelLength * dir;
    stepper.moveTo(motorPos * param.LEAD_SIZE);
  }
}

void Motor::moveToAbsolute(){
  if(!resStep.isRunning()) resStep.moveTo(param.resMotPos * param.LEAD_SIZE);
  if(!genStep.isRunning()) genStep.moveTo(param.genMotPos * param.LEAD_SIZE);
}

// Calibration function for the motors
void Motor::calibrate(){

  if(!stopRes){
    param.resMotPos -= param.LEAD_SIZE / 8;
    resStep.moveTo(param.resMotPos);
  }
  if(!stopGen){
    param.genMotPos -= param.LEAD_SIZE / 8;
    genStep.moveTo(param.genMotPos);
  }
  
  if(stopRes && stopGen){
    param.genMotPos = 0;
    param.resMotPos = 0;
    param.calibrate = false;
    stopRes = false;
    stopGen = false;
  }
}
