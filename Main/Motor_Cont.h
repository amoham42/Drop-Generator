#ifndef Motor_Cont_H
#define Motor_Cont_H

#include <AccelStepper.h>
class Motor{
  public:

  // Functions that are used by other controllers
    void motorSetup();
    void calibration();
    void runMotor();
    void move(int, int);

  private:
    void IRAM_ATTR resStalled();
    void IRAM_ATTR genStalled();
    void calculateDist(int&, AccelStepper&, int);
};

#endif
