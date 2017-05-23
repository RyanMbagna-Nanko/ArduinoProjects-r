/********************************************************************INCLUDES*****************************************************************/
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
int left = 150;
int right = 80;
int center = 120;
int count = 0;
int cm = ping.centimeters();
int mseconds = ping.microseconds();
int space;
int nack = Wire.endTransmission();
const char fugue[] PROGMEM =
  "! O5 L16 agafaea dac+adaea fa<aa<bac#a dac#adaea f"
  "O6 dcd<b-d<ad<g d<f+d<gd<ad<b- d<dd<ed<f+d<g d<f+d<gd<ad"
  "L8 MS <b-d<b-d MLe-<ge-<g MSc<ac<a ML d<fd<f O5 MS b-gb-g"
  "ML >c#e>c#e MS afaf ML gc#gc# MS fdfd ML e<b-e<b-"
  "O6 L16ragafaea dac#adaea fa<aa<bac#a dac#adaea faeadaca"
  "<b-acadg<b-g egdgcg<b-g <ag<b-gcf<af dfcf<b-f<af"
  "<gf<af<b-e<ge c#e<b-e<ae<ge <fe<ge<ad<fd"
  "O5 e>ee>ef>df>d b->c#b->c#a>df>d e>ee>ef>df>d"
  "e>d>c#>db>d>c#b >c#agaegfe f O6 dc#dfdc#<b c#4";

//********************************************************************SETUP****************************************************************
void setup() {
  Serial.begin(115200);
  myLidarLite.configure(0); // Change this number to try out alternate configurations
  pinMode(13, OUTPUT);
}

//********************************************************************LOOP****************************************************************
void loop() {

  digitalWrite(13, HIGH);
  motors.setLeftSpeed(4000);
  motors.setRightSpeed(4000);

  if (count == 0) {
    for (int i = 0; i < 5; i++) {
      int space = ping.inches();  // where is everything?
      ping.fire();

      Serial.print(ping.inches());  // print on the serial monitor
      Serial.print(" , ");
      Serial.print(cm);
      Serial.print(" , ");
      Serial.println(mseconds);

      digitalWrite(13, LOW);
      motors.setRightSpeed(4000);  // full speed ahead
      motors.setLeftSpeed(4000);

      delay(100);

      if ((space > 1) && (space <= 10)) { // if the FuzzBot gets too close...
        Serial.println("PingSpace is <= 10inches !!!!!!!!!!");
        buzzer.playFromProgramSpace(fugue);
        buzzer.play("! V10 cdefgab>cbagfedc");
        digitalWrite(13, HIGH);  // turn the other way
        motors.setLeftSpeed(-1000);
        motors.setRightSpeed(-1000);
        delay(100);
        digitalWrite(13, HIGH);  // turn the other way
        motors.setLeftSpeed(4000);
        motors.setRightSpeed(-4000);
        delay(100);
        digitalWrite(13, LOW);  // turn the other way
        motors.setLeftSpeed(4000);
        motors.setRightSpeed(4000);
        buzzer.stopPlaying();
        count = 0;
      }

      else if ((space <= 1) || (space == 123.74)) {
        Serial.println("PingSensor CRASHED!!!!!!.........");
        digitalWrite(13, LOW);
        buzzer.playFromProgramSpace(fugue);
        buzzer.play("! V10 cdefgab>cbagfedc");
        motors.setRightSpeed(0000);  // full speed ahead
        motors.setLeftSpeed(0000);
        delay(100);
        buzzer.stopPlaying();
        Serial.println("Switching to LaserSensor!");
        Serial.println("...........1.........2..............3!!!!");
        count = 1;
      }
      else {
        digitalWrite(13, LOW);

        motors.setLeftSpeed(4000);
        motors.setRightSpeed(4000);
        count = 0;
      }
    }
  }

  if (count == 1) {
    for (int i = 0; i < 5; i++) {
      int space = myLidarLite.distance();

      myLidarLite.begin(0, true); // Set configuration to default and I2C to 400 kHz
      Serial.println(myLidarLite.distance());

      for (int i = 0; i < 99; i++)
      {
        Serial.println(myLidarLite.distance(false));
      }
      digitalWrite(13, LOW);
      motors.setRightSpeed(4000);  // full speed ahead
      motors.setLeftSpeed(4000);

      if ((space > 12.7) && (space <= 25)) { // if the FuzzBot gets too close (25cm ~ 10inch)...
        Serial.println("LaserSpace is <= 10inches!!!!!!!!!!");
        buzzer.playFromProgramSpace(fugue);
        buzzer.play("! V10 cdefgab>cbagfedc");
        digitalWrite(13, HIGH);  // turn the other way
        motors.setLeftSpeed(-1000);
        motors.setRightSpeed(-1000);
        delay(100);
        digitalWrite(13, HIGH);  // turn the other way
        motors.setLeftSpeed(4000);
        motors.setRightSpeed(-4000);
        delay(100);
        digitalWrite(13, LOW);  // turn the other way
        motors.setLeftSpeed(4000);
        motors.setRightSpeed(4000);
        buzzer.stopPlaying();
        count = 1;
      }

      else if ((space <= 5) || (nack != 0))
      {
        Serial.println("LaserSensor CRASHED!!!!!!.........");
        digitalWrite(13, LOW);
        buzzer.playFromProgramSpace(fugue);
        buzzer.play("! V10 cdefgab>cbagfedc");
        motors.setRightSpeed(0000);  // stops
        motors.setLeftSpeed(0000);
        delay(100);
        buzzer.stopPlaying();
        Serial.println("Switching to PingSensor");
        Serial.println("...........1.........2..............3!!!!");
        count = 0;
      }
      else {
        digitalWrite(13, LOW);
        motors.setLeftSpeed(4000);
        motors.setRightSpeed(4000); 
        count = 1;

      }
    }
  }

  else {
    digitalWrite(13, LOW);
    motors.setLeftSpeed(4000);
    motors.setRightSpeed(4000);
    //count = 1;
  }
}


//------------------------------------------------------other options------------------------------------------------------------------------------------------------------
/*
  int nackCatcher = Wire.endTransmission();
  if(nackCatcher != 0)
  {
    Serial.println("> nack");
  }

*/
