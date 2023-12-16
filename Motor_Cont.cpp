#include <AccelStepper.h>
#include "Motor_Cont.h"
#include "param.h"

// Initializing the motors
AccelStepper reservStep(AccelStepper::DRIVER, param.resSTEP, param.resDIR);
AccelStepper genStep(AccelStepper::DRIVER, param.genSTEP, param.genDIR);

// This function sets up the motor acceleration, position, and speed
void mclass::motorSetup(){

  reservStep.setMaxSpeed(param.speed);
  reservStep.setAcceleration(param.accel);
  reservStep.setCurrentPosition(param.reservMotPos);
  pinMode(param.resDIAG, INPUT_PULLUP);

  genStep.setMaxSpeed(param.speed);
  genStep.setAcceleration(param.accel);
  genStep.setCurrentPosition(param.genMotPos);
  pinMode(param.genDIAG, INPUT_PULLUP);
}

// This function runs the motors
void mclass::runMotor(){
  reservStep.run();
  genStep.run();
}

// Reservoir motor
void mclass::reservMotor(int value){
  param.reservMotPos += value;
  reservStep.moveTo(param.reservMotPos);
}

// Drop generator motor
void mclass::genMotor(int value){
  param.genMotPos += value;
  genStep.moveTo(param.genMotPos);
}

// Calibration function for the motors
void mclass::calibration(){
  while(digitalRead(param.resDIAG) > param.DIAG_THRESHOLD || 
        digitalRead(param.genDIAG) > param.DIAG_THRESHOLD){

    if(digitalRead(param.resDIAG) > param.DIAG_THRESHOLD){
      reservStep.moveTo(param.reservMotPos + 1);
      reservStep.run();
    }
    if(digitalRead(param.genDIAG) > param.DIAG_THRESHOLD){
      genStep.moveTo(param.genMotPos + 1);
      genStep.run();
    }
  }
  param.genMotPos = 0;
  param.reservMotPos = 0;
}
