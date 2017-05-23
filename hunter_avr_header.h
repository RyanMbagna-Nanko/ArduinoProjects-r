#include <Servo.h>
#include <limits.h>
#include <espduino.h>
#include "custom_rest.h"
#include <ArduinoJson.h>
#include <Wire.h>

#include "messages.h"
#include "pfsm.h"
#include "gpsfsm.h"
#include "mfsm.h"
#include "CollisionAvoider.h"
#include "scanfsm.h"
#include "tfsm.h"
#include "pid.h"

#include "common.h"
#include "swarm_config.h"
#include "ApplicationMonitor.h"

#define VBATTERY A5


// comment out to test collision avoidance code
#define DISABLE_COLLISION_AVOIDANCE

namespace hunter_avr {


pfsm::ProtocolFSM protocolFSM{Serial1, "swarmiest", "swarmiest", "192.168.168.100", 8080};

GPSFSM gpsfsm(Serial3, &Serial);

tfsm::TracksFSM trackFSM;
mfsm::MagFSM magFSM{0x1E}; // I2C 7bit address of HMC5883

collAvoider::CollisionAvoider collisionAvoider;
SCANfsm::ScanFSM scanningfsm;

  
   using namespace std;
  // FRED --> Free-Roving Exploration Device ;-)
   enum class State {
     // STARTUP,
      IDLE,
      COMMAND_COMPLETE,
      COMMAND_FAILED,
      START_SET_HEADING,
      RUNNING_SET_HEADING,
      COLLISION_REDIRECT,
      START_DRIVE,
      RUNNING_DRIVE,
      COLLISION,
      START_SCANNING,
      SCANNING,
      MAX
  };


long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
  ADMUX = _BV(MUX5) | _BV(MUX0);
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
  ADMUX = _BV(MUX3) | _BV(MUX2);
#else
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#endif

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA, ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH; // unlocks both

  long result = (high << 8) | low;

  result = 1.1 * 1023 * 1000 / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
  // for Vin, the measurement that should always return ~5V.

}

class FRED {
  

  public:
  PID hdgPid;
 
  State state;

    double hdgSetPoint, hdgInput, hdgOutput;
    double hdgLongTerm;  // we have a desired heading, but we might have to deviate temporarily
    unsigned long delay_end;
    unsigned long delay_start;
    unsigned long react_collision;
    unsigned long command_timeout;
    uint8_t drive_speed;

    void setDelay(long ms) {
      delay_start = millis();
      delay_end = delay_start + ms;
    }
    //millis() is your time now
    bool delayExpired() {
      return millis() >= delay_end;
    }

    unsigned long delayElapsed() {
      return millis() - delay_start;
    }
    
    void update(mfsm::MagFSM &magFSM, tfsm::TracksFSM &trackFSM);

    void setCollisionDelay(long ms) {
      react_collision = millis() + ms;
    }

    bool collisionDelayExpired() {
      return millis() >= react_collision;
    }

    // PID values (2, 5, 1) are probably good... may need to investigate I (integral) term
    FRED()
      : hdgPid(&hdgInput, &hdgOutput, &hdgSetPoint, 2, 5, 1, PID::DIRECT), delay_end(0), command_timeout(5000) {

      hdgInput = 0;
      hdgSetPoint = 0;
      hdgLongTerm = 0;

    }


};
}

