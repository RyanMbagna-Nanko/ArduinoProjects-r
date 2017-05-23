
//********************************************************************INCLUDES****************************************************************
#include <LaserSensor.h>
#include <PingSensor.h>
#include <ZumoBuzzer.h>
#include <ZumoMotors.h>
#include <Ping.h>
#include <Wire.h>
#include <LIDARLite.h>
#include <avr/pgmspace.h>

LIDARLite myLidarLite;
ZumoMotors motors;
ZumoBuzzer buzzer;
Ping ping = Ping(4);
int count = 0;

//********************************************************************SETUP****************************************************************
void setup() {
  Serial.begin(115200);

  LaserSensor.SETUP();
  PingSensor.SETUP();

}

//********************************************************************LOOP****************************************************************
void loop() {

  if (count == 0) {
    PingSensor.RUN(false);
    else if ((count == 1) || (PingSensor.RUN(true))) {
      LaserSensor.RUN(false);
    }
    else {
      digitalWrite(13, LOW);

      motors.setLeftSpeed(4000);
      motors.setRightSpeed(4000);

    }
  }
}





