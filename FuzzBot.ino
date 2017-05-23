#include <Ping.h>

/* FuzzBot

  Code for the ZumoBot, with Ping Sensor on Servo to do obstacle avoidance.

  posted on Instructables

  created 15 Apr 13
  made by Quin (Qtechknow)
*/

#include <Ping.h>
#include <ZumoMotors.h>

Ping ping = Ping(4);

int left = 150;
int right = 80;
int center = 120;

int count = 0;

ZumoMotors motors;

void setup() {
  pinMode(13, OUTPUT);

  Serial.begin(9600);    // start serial communication at 9600 baud
}


void loop()
{
  if (count == 0) {
    for (int i = 0; i < 5; i++) { // do this 5 times
      ping.fire();

      int inches = ping.inches();  // where is everything?
      int cm = ping.centimeters();
      int mseconds = ping.microseconds();

      Serial.print(inches);  // print values out on serial monitor
      Serial.print(" , ");
      Serial.print(cm);
      Serial.print(" , ");
      Serial.println(mseconds);

      motors.setRightSpeed(4000);  // full speed ahead
      motors.setLeftSpeed(4000);

      delay(100);

      if (inches <= 10) {   // if the FuzzBot gets too close...

        digitalWrite(13, HIGH);  // turn the other way

        motors.setLeftSpeed(-4000);
        motors.setRightSpeed(4000);

        delay(100);
      }
      else if ((inches > 1) && (inches <= 10)) {
        Serial.println("PingSpace is <= 10!!!!!!!!!!");
        digitalWrite(13, HIGH);  // turn the other way
        motors.setLeftSpeed(-3000);
        motors.setRightSpeed(3000);

        delay(100);
        count = 0;
      }

      else if ((inches <= 1) || (inches == 123.74)) {
        Serial.println("PingSensor CRASHED!!!!!!.........");
        digitalWrite(13, LOW);

        motors.setRightSpeed(0000);  // full speed ahead
        motors.setLeftSpeed(0000);
        delay(100);

        Serial.println("Switching to LaserSensor!");
        Serial.println("...........1.........2..............3!!!!");
        count = 1;
      }
      else {
        digitalWrite(13, LOW);

        motors.setLeftSpeed(1000);
        motors.setRightSpeed(1000);
        count = 1;
      }
    }
  }

  if (count == 1) {
    for (int i = 0; i < 5; i++) { // do this 5 times

      ping.fire();

      int inches = ping.inches();   // where are we???
      int cm = ping.centimeters();
      int mseconds = ping.microseconds();

      Serial.print(inches);  // print on the serial monitor
      Serial.print(" , ");
      Serial.print(cm);
      Serial.print(" , ");
      Serial.println(mseconds);

      motors.setRightSpeed(4000);  // full speed
      motors.setLeftSpeed(4000);

      delay(100);

      if (inches <= 10) {   // if the FuzzBot is too close, turn the other way

        digitalWrite(13, HIGH);

        motors.setLeftSpeed(-4000);
        motors.setRightSpeed(4000);

        delay(300);
      } else {
        digitalWrite(13, LOW);

        motors.setLeftSpeed(4000);
        motors.setRightSpeed(4000);
      }
    }
  }
}


