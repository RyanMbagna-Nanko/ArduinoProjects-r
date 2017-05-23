//pfsm tells the bot what to do when each state is reached

#include "pfsm.h"
#include "swarm_config.h"

#define VBATTERY A5

using namespace pfsm;

int count = 0;
int ack_count = 0;

// will be overwritten with our actual swarm id at runtime
char command_endpoint[] = "/commands?pid=000";

void ProtocolFSM::StateString() {
    switch(state) {
    case State::POWER_ON: Serial.println("Power On");
    case State::POWER_ON_RESET: Serial.println("Power On RESET");
    case State::STARTUP: Serial.println("STARTUP");
    case State::ENABLE: Serial.println("ENABLE");
    case State::RESET: Serial.println("RESET");
    case State::DISCONNECTED_WIFI: Serial.println("DISCONNECTED_WIFI");
    case State::CONNECTING_WIFI: Serial.println("CONNECTING_WIFI");
    case State::DISCONNECTED_REST: Serial.println("DISCONNECTED_REST");
    case State::IDLE: Serial.println("IDLE");
    case State::FETCH_COMMAND: Serial.println("Fetch Command");
    case State::AWAITING_COMMAND: Serial.println("Awaiting Command");
    case State::ACK_COMMAND: Serial.println("Ack Command");
    case State::AWAITING_ACK: Serial.println("Awaiting Ack");
    case State::SENDING_STATUS: Serial.println("SENDING_STATUS");
    case State::SENDING_SCAN_RESULTS: Serial.println("Sending Scan Results");
    default: Serial.println("   ");
 }
}
 
void insertBase10(char* string, uint8_t value) {
  char buf[3];
  uint8_t idx = 0;
  do {
    buf[idx] = '0' + value % 10;
    value /= 10;
    idx++;
  } while (value > 0);

  for (uint8_t ii = 0; ii < idx; ++ii) {
    string[ii] = buf[idx - ii - 1];
  }

  string[idx] = '\0';
}

 uint32_t ProtocolFSM::dataLoad() const {
  return static_cast<uint32_t>(state) |
    (wifi_connected << 8) |
    (status_pending << 9) |
    (command_valid << 10) |
    (command_complete << 11);
  }  

ProtocolFSM::ProtocolFSM(Stream& serial, const char* ssid, const char* password, const char* server, uint16_t port)
  : ssid(ssid), password(password), server(server), port(port), delay_end(0), 
    serial(serial), esp(&serial, 4), rest(&esp), state(State::POWER_ON), 
    wifi_connected(false), status_pending(false), scanResults_pending(false), command_valid(false), command_complete(false) {
  // hold the device in reset
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, LOW);
}

void ProtocolFSM::wifiCallback(void* response) {
  uint32_t status;
  RESPONSE res(response);

  if (res.getArgc() == 1) {
  Serial.println("if (res.getArgc() == 1) {");
  Serial.println(" res.popArgs((uint8_t*)&status, 4) if (status == STATION_GOT_IP) {");
  
    res.popArgs((uint8_t*)&status, 4);
    if (status == STATION_GOT_IP) {
      wifi_connected = true;
    } else {
      wifi_connected = false;
    }
    Serial.println("wifi connected = ");
    Serial.println(wifi_connected);
  }
}

long ProtocolFSM::pendingDelay() {
  return delay_end - millis();
}

bool ProtocolFSM::delayComplete() {
  return millis() >= delay_end;
}

bool ProtocolFSM::readyCheckTime() {
  return millis() >= ready_check;
}

bool ProtocolFSM::commandCheck() {
  return millis() >= command_check;
}

bool ProtocolFSM::isResetTime() {
  return millis() >= reset_time;
}

void ProtocolFSM::resetResetTime() {
  reset_time = millis() + 30000;
}

void ProtocolFSM::resetReadyCheck() {
  ready_check = millis() + 5000;
}

void ProtocolFSM::update() {
  if (state > State::RESET) {
    esp.process();
  }

  if (state > State::CONNECTING_WIFI && !wifi_connected) {
    // we're newly disconnected disconnected
    Serial.println("pfsm.state = Disconnected_WIFI_again"); //*******
    state = State::DISCONNECTED_WIFI;
  }

  if (state > State::CONNECTING_WIFI && readyCheckTime()) { 
    Serial.println(F("PFSM: Checking readyness"));
    resetReadyCheck();
    if (!esp.ready()) {
       Serial.println("pfsm.state = ENABLE");
      state = State::ENABLE;
      wifi_connected = false;
    }
  }

  if (state > State::CONNECTING_WIFI && isResetTime()) {
    // nothing conclusive has happened to convince us we still
    // have comms so it's time to reset
    Serial.println(F("PFSM: Forcing reset"));
    resetResetTime();
     Serial.println("pfsm.state = STARTUP");
    state = State::STARTUP;
  }

  if (state == State::POWER_ON) {
    // ESP12 is fairly high current so we want to give it time to stabalize
    // and give the other electronics a chance to stabalize as well
    digitalWrite(RESET_PIN, LOW);
    delay_end = millis() + 1000;
     Serial.println("pfsm.state = POWER_ON_RESET"); //*******
    state = State::POWER_ON_RESET;
    delay(1000);

    // correct our swarm id in the endpoint string
    insertBase10(&command_endpoint[sizeof(command_endpoint) - 4], swarmID());
    Serial.println(command_endpoint); 
  }

  if (state == State::POWER_ON_RESET && delayComplete()) {
    digitalWrite(RESET_PIN, HIGH);
    Serial.println("Startup");
   state = State::STARTUP;
  }

  if (state == State::STARTUP) {
    resetResetTime();
    esp.enable();
    delay_end = millis() + 500;
     Serial.println("pfsm.state = ENABLE");
    state = State::ENABLE;
    delay(1000);
  }

  if (state == State::ENABLE && delayComplete()) {
    resetResetTime();
    esp.reset();
    delay_end = millis() + 500;
    Serial.println(F("pfsm.state = RESET"));
    state = State::RESET;
  }

  if (state == State::RESET && delayComplete() && esp.ready()) {
    resetResetTime();
    Serial.println("Reset resetTime");
    resetReadyCheck();
    Serial.println("Reset resetCheck");
    count = count + 1;  //see how many times bot has reset
    if (count > 2) {
      Serial.println("Warning: RESET"); //any additional action? Notify server?
    }
    state = State::DISCONNECTED_WIFI;
  }


  if (state == State::DISCONNECTED_WIFI) {
    esp.wifiCb.attach(this, &ProtocolFSM::wifiCallback);
    esp.wifiConnect(ssid, password);
    Serial.print(ssid);
    Serial.print(password);
    state = State::CONNECTING_WIFI;
  }

  if (state == State::CONNECTING_WIFI && wifi_connected) {
         Serial.println("pfsm.state = CONNECTING WIFI and wifi_connected");
    state = State::DISCONNECTED_REST;
  }

  if (state == State::DISCONNECTED_REST) {
         Serial.println("pfsm.state = DISCONNECTED REST");
    if (rest.begin(server, port, false)) {
      state = State::IDLE;
    }
  }

  if (state == State::IDLE && !command_valid && commandCheck()) {
    state = State::FETCH_COMMAND;
  }


  if (state == State::IDLE && command_complete) {
    state = State::ACK_COMMAND;
  }

  if (state == State::IDLE && status_pending) { //**
    state = State::SENDING_STATUS;
    status_pending = false;

    StaticJsonBuffer<328> jsonBuffer;
    char buffer[328];

    JsonObject& obj = jsonBuffer.createObject();
    status.toJson(obj);
    obj.printTo(buffer, sizeof(buffer));
    rest.setContentType("application/json");
    rest.put("/status", buffer);
      Serial.println("Printing");

    // prepare for response
    rest.getResponse(NULL, 0, true);
    delay_end = millis() + 1000;
  }

  if (state == State::SENDING_STATUS) {
    char buffer[128];
    int resp;
    if ((resp = rest.getResponse(buffer, sizeof(buffer), false)) == HTTP_STATUS_OK) {
      Serial.println(F("PFSM: Status delivered"));
      resetResetTime();
      resetReadyCheck();
      state = State::IDLE;
    } else if (delayComplete()) {
      Serial.println(F("PFSM: Status timed out"));
      state = State::IDLE;
    }
  }

  if (state == State::IDLE && scanResults_pending) {
    state = State::SENDING_SCAN_RESULTS;
    scanResults_pending = false;

    // 328?
    StaticJsonBuffer<1024> jsonBuffer;
    char buffer[1024];

    JsonObject& obj = jsonBuffer.createObject();
    scanResults.toJson(obj);
    obj.printTo(buffer, sizeof(buffer));
    rest.setContentType("application/json");
    rest.put("/scanResults", buffer);

    // prepare for response
    rest.getResponse(NULL, 0, true);
    delay_end = millis() + 1000;
  }

  if (state == State::SENDING_SCAN_RESULTS) {
    char buffer[128];
    int resp;
    if ((resp = rest.getResponse(buffer, sizeof(buffer), false)) == HTTP_STATUS_OK) {
      Serial.println(F("PFSM: Scan Results delivered, resp = HTTP_STATUS_OK"));
      resetResetTime();
      resetReadyCheck();
      scanResults_pending = false; // don't send it again
      state = State::IDLE;
    } else if (delayComplete()) {
      Serial.println(F("PFSM: Scan results timed out, resp != HTTP_STATUS_OK..."));
      Serial.print("resp = ");
      Serial.println(resp);
      state = State::IDLE;
      // retry sending at next opportunity since scanResults_pending is still true
    }
  }

  if (state == State::FETCH_COMMAND) {
    rest.get(command_endpoint);

    // prepare for response
    rest.getResponse(NULL, 0, true);
    delay_end = millis() + 1000;
    state = State::AWAITING_COMMAND;
  }

  if (state == State::AWAITING_COMMAND) {
    StaticJsonBuffer<128> jsonBuffer;
    char buffer[128];
    int resp;
    if ((resp = rest.getResponse(buffer, sizeof(buffer), false)) == HTTP_STATUS_OK) {
      // parse the response
      resetResetTime();
      resetReadyCheck();
      char* ptr = buffer;
      // skip to the payload
      for (uint16_t ii = 0; ii < sizeof(buffer); ++ii) {
        if (buffer[ii] == '\n') {
          ptr = &buffer[ii + 1];
          break;
        }
      }
      JsonArray& root = jsonBuffer.parseArray(ptr);
      if (!root.success()) { // INCORRECT PATH
        Serial.print(F("PFSM: Failed to parse "));
        Serial.println(ptr);
        command_check = millis() + 1000;
        state = State::IDLE; 
      } else {
        if (root.size() == 0) {
          // nothing in our queue
          Serial.println(F("No commands waiting"));
          command_check = millis() + 1000;
          state = State::IDLE;
        } else {     // CORRECT PATH
          if (command.fromJson(root[0])) {
            command_valid = true;
            command_complete = false;
            state = State::IDLE;
            // don't reset command_check delay so that we can quickly queue up
            // the next command and possibly chain together an interesting
            // series of commands
          } else {
            Serial.print(F("PFSM: Message invalid "));
            Serial.println(buffer);
            command_check = millis() + 1000;
            state = State::IDLE;
          }
        }
      }
    } else if (delayComplete()) {
      Serial.println(F("PFSM: Command request timed out"));
      command_check = millis() + 1000;
      state = State::IDLE;
    }
  }

  if (state == State::ACK_COMMAND) {

    StaticJsonBuffer<328> jsonBuffer;
    char buffer[328];

    JsonObject& obj = jsonBuffer.createObject();
    obj["pid"] = swarmID();
    obj["cid"] = command.cid;
    obj["complete"] = true;
   
    obj.printTo(buffer, sizeof(buffer));
    rest.put(command_endpoint, buffer);

    // prepare for response
    rest.getResponse(NULL, 0, true);
    delay_end = millis() + 750;
    state = State::AWAITING_ACK;
  }

  if (state == State::AWAITING_ACK) {
    char buffer[128];
    int resp;
    if ((resp = rest.getResponse(buffer, sizeof(buffer), false)) == HTTP_STATUS_OK) {
      // assume we got goodness
       Serial.println("states set to idle from AWAITING_ACK");
      state = State::IDLE;
      command_valid = false;
      command_complete = false;
    } else if (delayComplete()) {
      // this is the one case we retry forever. It's important that the mothership know that
      // we did what we said we would do. (Mothership message: PUT Command with no CID)
      //edit: to stop the bots from going into a loop between "Awaiting_ACK" and "ACK_Command"
      // ack_count is created so the mothership is aware that command x was not executed but
      // worker can move onto next command and not get hung up

      ack_count = ack_count + 1;
      Serial.println("Ack_count = ");
      Serial.print(ack_count);

      if (ack_count > 3) {
        state = State::IDLE;
      }
      else {
        Serial.println(F("PFSM: Ack timed out"));
        state = State::ACK_COMMAND;
      }

    }
  }
}

void ProtocolFSM::sendStatus(const SensorStatus& _status) {
  if (!status_pending) {
    status = _status;
    status_pending = true;
  }
}

void ProtocolFSM::sendScanResults(const ScanResults& _scanResults) {
  if (!scanResults_pending) {
    scanResults = _scanResults;
    scanResults_pending = true;
  }
}

