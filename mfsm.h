#ifndef MFSM_H
#define MFSM_H

#include "common.h"
#include "tfsm.h"

namespace mfsm {

    // using namespace std;

  class MotorCompensation {
    public:
      virtual const Vector<int>& center() const = 0;
  };

  class ComplexCompensation : public MotorCompensation {
    public:
      Vector<int> forward, reverse, right, left, stopped;
      tfsm::TracksFSM& tfsm;

      ComplexCompensation(const Vector<int>& forward, const Vector<int>& reverse, const Vector<int>& right, const Vector<int>& left, tfsm::TracksFSM& fsm);

      virtual const Vector<int>& center() const;
  };

  class SimpleCompensation : public MotorCompensation {
    public:
      Vector<int> _center;

      SimpleCompensation(const Vector<int>& _center);

      virtual const Vector<int>& center() const;
  };

  enum class State {
    STARTUP,
    CONFIGURE,
    REQUESTING,
    RECEIVING,
  };



  class MagFSM {

    public:
    
    unsigned long delay_end;
    double accumulator, hdg;

    State state;
    int8_t address;

    public:
    int16_t x, y, z;
    uint8_t updated_data : 1;
    uint8_t first_measurement : 1;

    MagFSM(uint8_t address);

    void ackData();

    long pendingDelay();

    bool delayComplete();

    void update();

    double heading();

    double filteredHeading();
  };
}

#endif
