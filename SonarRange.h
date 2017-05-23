/*
  SonarRange.h - Library for using the MaxSonar-EZ1
  Created by Jesse Hess, October 2013
  Modified BSD license
*/

#ifndef SonarRange_h
#define SonarRange_h

#include "Arduino.h"
#include <SoftwareSerial.h>
#include <ctype.h>

enum RangeUnit { RangeUnit_IN = 1, RangeUnit_CM = 2 };
enum RangeOutput { RangeOutput_Serial = 100, RangeOutput_Analog = 200, RangeOutput_Pulse = 300 };

/*
  SonarRange - read Serial, Analog, or Pulse Width data from the MaxSonar sensor
  Example usage:
  #include <SoftwareSerial.h>
  #include <SonarRange.h>
    SonarRange sonarRange = SonarRange();
    void setup() {
      sonarRange.setUnit(RangeUnit_IN);
      sonarRange.setPins(PIN_D7);
      sonarRange.setOutput(RangeOutput_Serial);
      sonarRange.begin();
    }
    void loop() {
      Serial.println(sonarRange.read());
      delay(50);
    }
*/
class SonarRange {
  public:
    SonarRange(enum RangeUnit rangeUnit = RangeUnit_IN, enum RangeOutput rangeOutput = RangeOutput_Pulse);
    ~SonarRange();
    void setPins(int readPin, int chainPin = -1);
    void setUnit(enum RangeUnit rangeUnit = RangeUnit_IN);
    void setOutput(enum RangeOutput rangeOutput = RangeOutput_Pulse);
    void begin();
    long read();
    void printSetup();
  private:
    long readSerial();
    long readAnalog();
    long readPulse();
    long convertUnit(long rangeSample);
    int rxPin;
    int txPin;
    int bwPin;
    int anPin;
    int pwPin;
    enum RangeUnit unit;
    enum RangeOutput output;
    SoftwareSerial* serialComms;
    char serialChars[4];
    int serialCounter;
    long serialSample;
};

#endif
