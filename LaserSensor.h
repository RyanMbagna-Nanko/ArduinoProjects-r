

#ifndef LaserSensor_h
#define LaserSensor_h
#include <Wire.h>
#include <LIDARLite.h>
#include <ZumoMotors.h>


class Lclass
{
  public:
    Lclass();
    void SETUP();
    void RUN(bool crash);

};
extern Lclass LaserSensor;

#endif

