#include "messages.h"

#include "swarm_config.h"

  int command_number = 0;
    
SensorStatus::SensorStatus()
  : lat(0), lon(0), heading(0) {
    memset(debug_message, sizeof(debug_message), 0);
}

void SensorStatus::toJson(JsonObject& report) {
  report["gtime"] = gps_time_ms;
  report["mtime"] = message_time_ms;
  
  report["lat"] = lat;
  report["long"] = lon;
  report["hdg"] = heading;
  report["pid"] = swarmID();

  
  report["x"] = ecef_pos_cm.x;
  report["y"] = ecef_pos_cm.y;
  report["z"] = ecef_pos_cm.z;
  
  report["vx"] = ecef_vel_cmps.x;
  report["vy"] = ecef_vel_cmps.y;
  report["vz"] = ecef_vel_cmps.z;
  
  report["mx"] = magnetometer.x;
  report["my"] = magnetometer.y;
  report["mz"] = magnetometer.z;
  
  report["gstate"] = gps_fix_state;
  
  report["e"] = enu_cm.x;
  report["n"] = enu_cm.y;
  report["u"] = enu_cm.z;
  
  report["vbattery"] = vbattery;
  report["vin"] = vin;

  if(debug_message[0] != '\0') {
    report["dbg"] = debug_message;
  }
}

ScanResults::ScanResults()
  : lat(0), lon(0), heading(0) {
}

void ScanResults::toJson(JsonObject& report) {
  report["gtime"] = gps_time_ms;
  report["mtime"] = message_time_ms;
  
  report["lat"] = lat;
  report["long"] = lon;
  report["hdg"] = heading;
  report["pid"] = swarmID();
  
  report["gstate"] = gps_fix_state;
  
  // arrays of stuff (ranges & IR detections)
  
  JsonArray&  obstructionArray  = report.createNestedArray("obstruction");
  for(int i = 0; i < SCANFSM_NUMBER_OF_SCAN_SAMPLES; i++) {
    obstructionArray.add(sonarScanResults[i]);
  }
  
  JsonArray&  beaconArray  = report.createNestedArray("beacon");
  for(int i = 0; i < SCANFSM_NUMBER_OF_SCAN_SAMPLES; i++) {
    beaconArray.add(irScanResults[i]);
  }
}

bool DriveCommand::fromJson(JsonObject& cmd) {
  // get the command type 
  if(!cmd.containsKey("type")) return false;
  
  const char* type = cmd["type"];
  if(strcmp(type, "SET_HEADING") == 0) {
    command = SET_HEADING;
  } else if(strcmp(type, "DRIVE") == 0) {
    command = DRIVE;
  } else if(strcmp(type, "SCAN") == 0) {
    command = SCAN;
  } else { 
    Serial.print("unrecognized type ");
    Serial.println(type);
    return false;
  }
  
  if(!cmd.containsKey("cid") || !cmd.containsKey("pid")) {
    return false;
  }
  
  cid = cmd["cid"];
  pid = cmd["pid"];
 
  // get the command parameters
  if(command == DRIVE) {
    if(!cmd.containsKey("speed") || !cmd.containsKey("heading") || !cmd.containsKey("duration")) return false;
    //set command parameters
    payload.drive.duration = cmd["duration"];
    payload.drive.speed = cmd["speed"];
    payload.drive.heading = cmd["heading"];
    Serial.println(F("Drive Command"));
    
    if (command_number < 100) {
      command_number += 1;
    }
    Serial.println("Command Number");
    Serial.print (command_number);
    
  //  return true;
  } else if(command == SET_HEADING) {
    if(!cmd.containsKey("heading") || !cmd.containsKey("duration")) return false;
    
    payload.heading.duration = cmd["duration"];
    payload.heading.heading = cmd["heading"];
       Serial.println(F("Set Heading"));
    return true;
  } else if(command == SCAN) {
    // no parameters for scanning...? 
   
    return true;
  } else {
    return false;
  }
 
}
