#ifndef Motor_Cont_H
#define Motor_Cont_H

class mclass{
  public:

  // Functions that are used by other controllers
    void motorSetup();
    void reservMotor(int);
    void genMotor(int);
    void calibration();
    void runMotor();
};

#endif
