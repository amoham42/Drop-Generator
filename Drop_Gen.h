#ifndef Drop_Gen_H
#define Drop_Gen_H

class dclass{

  public:
    // Functions that are used by other controllers
    void dgSetup();
    void dgStart();

  private:
    void camera();
    void generate();
};

#endif
