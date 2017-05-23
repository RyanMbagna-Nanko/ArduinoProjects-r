#include "tfsm.h"
#include <Arduino.h>

using namespace tfsm;

TracksFSM::TracksFSM()
: state(State::STARTUP) {
}

int16_t TracksFSM::tween(int16_t current, int16_t target, int16_t max_increment) {
  int16_t step = target - current;
  step = constrain(step, -max_increment, max_increment);
  return current + step;
}

void TracksFSM::update() {
  if(state == State::STARTUP) {
    left_current = 0;
    right_current = 0;
    left_target = 0;
    right_target = 0;
    left_start = 0;
    left_start = 0;
    digitalWrite(TRACK1_SPEED, LOW);
    digitalWrite(TRACK2_SPEED, LOW);
    digitalWrite(TRACK1_DIRECTION, LOW);
    digitalWrite(TRACK2_DIRECTION, LOW);
    pinMode(TRACK1_SPEED, OUTPUT);
    pinMode(TRACK2_SPEED, OUTPUT);
    pinMode(TRACK1_DIRECTION, OUTPUT);
    pinMode(TRACK2_DIRECTION, OUTPUT);
    state = State::HOLDING;
  }

  if(state == State::ACCELERATING) {
    unsigned long dt = millis() - accelStart;
    int16_t max_increment = dt / MS_OVER_DELTASPEED;
    write(tween(right_start, right_target, max_increment),
          tween(left_start, left_target, max_increment));
    if(left_current == left_target && right_current == right_target) {
      state = State::HOLDING;
    }
  }
}

void TracksFSM::target(int16_t right, int16_t left) {
  if(state == State::STARTUP) return;

  left_start = left_current;
  right_start = right_current;
  left_target = left;
  right_target = right;

  accelStart = millis();
  state = State::ACCELERATING;
}

void TracksFSM::write(int16_t right, int16_t left) {
  left_current = left;
  right_current = right;

  if(right < 0) {
    right = -right;
    digitalWrite(TRACK1_DIRECTION, HIGH);
  } else {
    digitalWrite(TRACK1_DIRECTION, LOW);
  }

  if(left < 0) {
    left = -left;
    digitalWrite(TRACK2_DIRECTION, HIGH);
  } else {
    digitalWrite(TRACK2_DIRECTION, LOW);
  }

  if(left != 0) left = map(left, 0, 255, MIN_VIABLE_SPEED, MAX_VIABLE_SPEED);
  if(right != 0) right = map(right, 0, 255, MIN_VIABLE_SPEED, MAX_VIABLE_SPEED);

  analogWrite(TRACK1_SPEED, right);
  analogWrite(TRACK2_SPEED, left);
}
