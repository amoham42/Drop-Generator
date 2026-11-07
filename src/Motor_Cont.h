#ifndef Motor_Cont_H
#define Motor_Cont_H

// Two lead-screw stepper motors: the reservoir (sets the fluid level, so the pressure at the
// nozzle) and the generator head. Positions are kept in millimetres from home everywhere;
// conversion to steps happens only at the AccelStepper boundary.
class Motor {
  public:
    void motorSetup();
    void runMotor();  // call as often as possible from the motor task

    void move(int dir, int motor);  // jog by param.travelLength: motor 0 = generator, 1 = reservoir, 2 = both
    void moveToAbsolute();          // go to param.resMotPos / param.genMotPos
    void stop();
    void reset();                   // declare the current position zero
    void calibrate();               // home both motors against their end stops

    bool isMoving();
    float resPosition();  // live position, mm
    float genPosition();
};

#endif
