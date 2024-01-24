#ifndef Motor_Cont_H
#define Motor_Cont_H

#include <AccelStepper.h>
class Motor{
  public:

  // Functions that are used by other controllers
    void motorSetup();
    void calibrate();
    void runMotor();
    void stop();
    void move(int, int);
    void moveToAbsolute();

  private:
    void calculateDist(double&, AccelStepper&, int);
};

#endif
