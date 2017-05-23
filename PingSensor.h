

#ifndef PingSensor_h
#define PingSensor_h
#include <Ping.h>
#include <ZumoMotors.h>



class Pclass {
  public:
    Pclass();
    void SETUP();
    void RUN(bool crash);

};

extern Pclass PingSensor;

#endif



