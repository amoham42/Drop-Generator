#ifndef Drop_Gen_H
#define Drop_Gen_H

// Drives the piezo disc through an H-bridge: one short pulse of one polarity per droplet, then
// the bridge is held in the opposite polarity until the next one. Optionally fires the camera
// a fixed delay after each droplet, so every frame catches the drop at the same moment.
class Generator {
  public:
    void dgSetup();
    void dgStart();  // one iteration of the generator task; call in a loop

    void startRun();  // begin a run of param.dropNum droplets
    void stopRun();

  private:
    void shake();
    void triggerCamera();
    void generate();
};

#endif
