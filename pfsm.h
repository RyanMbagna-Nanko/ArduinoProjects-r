#ifndef PFSM_H
#define PFSM_H

#include "messages.h"
#include "common.h"
#include <espduino.h>
#include "custom_rest.h"
#include <ArduinoJson.h>

#define RESET_PIN 2


namespace pfsm {

  // using namespace std;
  enum class State {
    POWER_ON,
    POWER_ON_RESET,
    STARTUP,
    ENABLE,
    RESET,
    DISCONNECTED_WIFI,
    CONNECTING_WIFI,

    DISCONNECTED_REST,
    CONNECTED_REST,

    IDLE,
    
    FETCH_COMMAND,
    AWAITING_COMMAND,

    ACK_COMMAND,
    AWAITING_ACK,

    SENDING_STATUS,
    SENDING_SCAN_RESULTS,
  };

//  const char* StateStr(State);

  class ProtocolFSM {

    private:
      const char* ssid;
      const char* password;
      const char* server;
      uint16_t port;

      unsigned long delay_end;
      unsigned long ready_check;
      unsigned long reset_time;
      unsigned long command_check;

    public:

      State state;
       
      Stream& serial;
      ESP esp;
      CREST rest;
      SensorStatus status;
      ScanResults scanResults;
      DriveCommand command;

      uint8_t wifi_connected : 1;
      uint8_t status_pending : 1;
      uint8_t scanResults_pending : 1;
      uint8_t command_valid    : 1;
      uint8_t command_complete : 1;
      float heading; //??

      ProtocolFSM(Stream& serial, const char* ssid, const char* password, const char* server, uint16_t port);

      void wifiCallback(void* response);
      
      uint32_t dataLoad() const;

      long pendingDelay();

      bool delayComplete();

      bool readyCheckTime();

      bool isResetTime();

      bool commandCheck();

      void resetResetTime();

      void resetReadyCheck();

      void StateString();

      void update();

      void sendStatus(const SensorStatus& _status);

      void sendScanResults(const ScanResults& _scanResults);

  };

}



#endif


