/*
  SonarRange.cpp - Library for using the MaxSonar-EZ1
  Created by Jesse Hess, October 2013
  Modified BSD license
*/

#include "SonarRange.h"

const long PW_IN = 147;
const long IN2CM = 2.54;

SonarRange::SonarRange(enum RangeUnit rangeUnit, enum RangeOutput rangeOutput) {
  unit = rangeUnit;
  output = rangeOutput;
}
SonarRange::~SonarRange() {
  if (serialComms) {
    free(serialComms);
  }
}
void SonarRange::setPins(int readPin, int chainPin) {
  anPin = readPin;
  pwPin = readPin;
  rxPin = readPin;
  bwPin = chainPin;
}
void SonarRange::setUnit(enum RangeUnit rangeUnit) {
  unit = rangeUnit;
}
void SonarRange::setOutput(enum RangeOutput rangeOutput) {
  output = rangeOutput;
}
// setup for Pulse Width and Serial data
void SonarRange::begin() {
  if (output == RangeOutput_Pulse) {
    pinMode(pwPin, INPUT);
  } else if (RangeOutput_Serial) {
    serialComms = new SoftwareSerial(rxPin, -1, true);
    serialComms->begin(9600);
    while (!serialComms->isListening()) { ; }
    serialCounter = -1;
    serialSample = -1;
  }
}
// read a range sample
long SonarRange::read() {
  long result = 0;
  if (output == RangeOutput_Serial) {
    result = readSerial();
  } else if (output == RangeOutput_Analog) {
    result = readAnalog();
  } else if (output == RangeOutput_Pulse) {
    result = readPulse();
  }
  return result;
}
// read Serial range in inches of the format 'Rxxx' (0 to Vcc, 9600Baud, 81N)
// read the latest values from the SoftwareSerial stream into serialSample 
long SonarRange::readSerial() {
  long result = -1;
  while (serialComms->available()) {
    char c = serialComms->read();
    if (c == 'R') {
      serialChars[0] = 0x00; serialChars[1] = 0x00; serialChars[2] = 0x00;
      serialCounter = 0;
    } else if (serialCounter == 3) {
      serialChars[3] = 0x00;
      serialSample = (long)atoi(serialChars);
      serialCounter++;
    } else if (isdigit(c) && serialCounter >= 0 && serialCounter < 3) {
      serialChars[serialCounter] = c;
      serialCounter++;
    } else {
      Serial.print(F("readSerial c="));Serial.print(c);Serial.print(F(" counter="));Serial.println(serialCounter);
    }
  }
  result = serialSample;
  if (result != -1) {
    result = convertUnit(result);
  }
  return result;
}
// read Analog range, Scaling factor is (Vcc/512) per inch
long SonarRange::readAnalog() {
  int rangeSample = (long)(analogRead(anPin) / 2);
  return convertUnit(rangeSample);
}
// read Pulse Width in inches from in microseconds
long SonarRange::readPulse() {
  long rangeSample = pulseIn(pwPin, HIGH) / PW_IN;
  return convertUnit(rangeSample);
}
// convert the rangeSample into Inches or Centimetres
long SonarRange::convertUnit(long rangeSample) {
  long result = rangeSample;
  if (unit == RangeUnit_CM) {
    result = rangeSample * IN2CM;
  }
  return result;
}
// print internal values for debugging
void SonarRange::printSetup() {
  Serial.print(F("SonarRange settings"));
  Serial.print(F(" RX:"));Serial.print(rxPin);
  Serial.print(F(" PW:"));Serial.print(pwPin);
  Serial.print(F(" AN:"));Serial.print(anPin);
  Serial.print(F(" BW:"));Serial.print(bwPin);
  Serial.print(F(" Unit "));Serial.print(unit);
  Serial.print(F(" Output "));Serial.println(output);
}
