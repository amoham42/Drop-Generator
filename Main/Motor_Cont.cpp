#include <AccelStepper.h>
#include "Motor_Cont.h"
#include "param.h"

// Initializing the motors
AccelStepper reservStep(AccelStepper::DRIVER, param.resSTEP, param.resDIR);
AccelStepper genStep(AccelStepper::DRIVER, param.genSTEP, param.genDIR);

void IRAM_ATTR Motor::resStalled(){
  if(digitalRead(param.RESERVOIR_DIAG) > param.DIAG_THRESHOLD){
    reservStep.setAcceleration(200000);
    reservStep.moveTo(param.resMotPos);
    reservStep.setAcceleration(param.accel);
    stopRes = true;
  }
}

void IRAM_ATTR Motor::genStalled(){
  if(digitalRead(param.GENERATOR_DIAG) > param.DIAG_THRESHOLD){
    genStep.setAcceleration(200000);
    genStep.moveTo(param.genMotPos);
    genStep.setAcceleration(param.accel);
    stopGen = true;
  }
}

// This function sets up the motor acceleration, position, and speed
void Motor::motorSetup(){

  reservStep.setMaxSpeed(param.speed);
  reservStep.setAcceleration(param.accel);
  reservStep.setCurrentPosition(param.resMotPos);
  pinMode(param.resDIAG, INPUT_PULLUP);

  genStep.setMaxSpeed(param.speed);
  genStep.setAcceleration(param.accel);
  genStep.setCurrentPosition(param.genMotPos);
  pinMode(param.genDIAG, INPUT_PULLUP);
}

// This function runs the motors
void Motor::runMotor(){
  reservStep.run();
  genStep.run();
}

void Motor::move(int value, int motor){
  switch(motor) {
    case 0: // Generator Motor
      calculateDist(param.genMotPos, genStep, value);
      break;
    
    case 1: // Reservoir Motor
      calculateDist(param.resMotPos, reservStep, value);
      break;
    
    case 2: // Both Motors
      calculateDist(param.genMotPos, genStep, value);
      calculateDist(param.resMotPos, reservStep, value);
      break;

    default:
      Serial.println("Invalid motor identifier");
      break;
    }
}

void Motor::calculateDist(int& motorPos, AccelStepper& stepper, int value) {
  motorPos += param.roundSize * param.travelLength * value;
  stepper.moveTo(motorPos);
}

// Calibration function for the motors
void Motor::calibration(){
  while(digitalRead(param.resDIAG) > param.DIAG_THRESHOLD || 
        digitalRead(param.genDIAG) > param.DIAG_THRESHOLD){

    if(digitalRead(param.resDIAG) > param.DIAG_THRESHOLD){
      reservStep.moveTo(param.resMotPos + 1);
      reservStep.run();
    }
    if(digitalRead(param.genDIAG) > param.DIAG_THRESHOLD){
      genStep.moveTo(param.genMotPos + 1);
      genStep.run();
    }
  }
  param.genMotPos = 0;
  param.resMotPos = 0;
}
