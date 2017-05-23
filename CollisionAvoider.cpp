/**
 *
 * CollisionAvoider.h
 *
 * Using FRED's two LV-MaxSonar-EZ sensors, we will try to avoid running into
 * things.  We'll let FRED decide when to check for obstructions (e.g., the
 * collision avoidance system can be disabled when FRED is still and/or is
 * performing a scan).
 *
 */

#include "CollisionAvoider.h"
#include "scanfsm.h"
#include "messages.h"

using namespace collAvoider;


// Constructor
CollisionAvoider::CollisionAvoider()
{
  leftSensor.setReadPin(A0, LvMaxSonarSensor::ANALOG_MODE);
  rightSensor.setReadPin(A1, LvMaxSonarSensor::ANALOG_MODE);
  leftSensor.enableReadPin();

  state = State::IDLE;
  condition = NO_OBSTRUCTION;
}

void CollisionAvoider::StateString() 
{ 
   switch(state) {
      case State::IDLE: Serial.println("IDLE"); break;
      case State::ACTIVE: Serial.println("ACTIVE"); break;
      case State::STARTUP: Serial.println("STARTUP"); break;
      default: Serial.print(" ");
  }
}

  
void CollisionAvoider::setEnabled(boolean enabled)
{

  if(enabled)
  {
    leftSensor.enableReadPin();
    rightSensor.enableReadPin();  // already tied to left sensor
    leftSensor.disableReadPin();
    state = State::ACTIVE;
    condition = NO_OBSTRUCTION;  // until we see something...

  }
  else
  {
    leftSensor.disableReadPin();
    rightSensor.disableReadPin();  // already tied to left sensor
    state = State::IDLE;
    condition = NO_OBSTRUCTION;  // we're probably not driving right now
  }
}


void CollisionAvoider::update() 
{
  if(State::STARTUP == state)  //for calibration
  {
    auto previousmillis = millis();

   while (millis() < (previousmillis + 2000) ) {  //according to datasheet, calibration takes 450 ms)
      condition = NO_OBSTRUCTION; 
      leftCm = leftSensor.getDistanceCm();
      rightCm = rightSensor.getDistanceCm();
    }
    state = State::IDLE;
    condition = NO_OBSTRUCTION; 
  }

  if(State::IDLE == state) 
  {
    condition = NO_OBSTRUCTION;
  }

  // if we're active, then let's see if anything is in our way
  if(state == State::ACTIVE)
  {
    condition = NO_OBSTRUCTION;  // ever the optimist!
    leftSensor.disableReadPin(); //**
    leftSensor.enableReadPin();  //**
    leftCm = leftSensor.getDistanceCm();
    rightCm = rightSensor.getDistanceCm();
    delayMicroseconds(30);
    leftSensor.disableReadPin();

    Serial.print("Left: ");
    Serial.print(leftCm);
Serial.print( "Right: ");
Serial.print(rightCm);
Serial.println();

    // let's see what condition my condition is in...
    if(leftCm <= WARNING_DISTANCE_CM)
    {
      condition |= OBSTRUCTION_LEFT;

            Serial.println("OBSTRUCTION_LEFT");
      Serial.print(leftCm);
      Serial.println("OBSTRUCTION_LEFT");
      Serial.print(leftCm);
      Serial.println("OBSTRUCTION_LEFT");
      Serial.print(leftCm);

   }
  if(rightCm <= WARNING_DISTANCE_CM)  {
      condition |= OBSTRUCTION_RIGHT;
         Serial.println("OBSTRUCTION_RIGHT");
      Serial.print(rightCm);
         Serial.println("OBSTRUCTION_RIGHT");
      Serial.print(rightCm);
         Serial.println("OBSTRUCTION_RIGHT");
      Serial.print(rightCm);
  } 
    Serial.print(leftCm);
    Serial.print(", ");
    Serial.print(rightCm);
    Serial.print(", state = ");
    Serial.println(condition);
 
}
}



