#include "SensorMesh.h"

char display_lines[DISPLAY_LINES][25] = {
  "",
  "Hello from MeshCore"
};

void refresh_screen() {
  int i;
  display.startFrame();
  for (i=0; i < DISPLAY_LINES; i++) {
    display.setCursor(5, 112 - (12 * i));
    display.print(display_lines[i]);
  }
  display.endFrame();
}

void add_line(char * l) {
  int i;
  for (i = DISPLAY_LINES - 1; i >= 1; i--) {
    strncpy(display_lines[i], display_lines[i-1], 23);
  }
  strncpy(display_lines[0], l, 23);
  display_lines[0][23] = 0;

  refresh_screen();
}

class MyMesh : public SensorMesh {
public:
  MyMesh(mesh::MainBoard& board, mesh::Radio& radio, mesh::MillisecondClock& ms, mesh::RNG& rng, mesh::RTCClock& rtc, mesh::MeshTables& tables)
     : SensorMesh(board, radio, ms, rng, rtc, tables), 
       battery_data(2, 60*60)    // 2 values, check every hour
  {
  }

protected:
  /* ========================== custom logic here ========================== */
  Trigger low_batt, critical_batt, serial;
  TimeSeriesData  battery_data;

  void onSensorDataRead() override {
    float batt_voltage = getVoltage(TELEM_CHANNEL_SELF);

    battery_data.recordData(getRTCClock(), batt_voltage);   // record battery
    /* no alert
    alertIf(batt_voltage < 3.4f, critical_batt, HIGH_PRI_ALERT, "Battery is critical!");
    alertIf(batt_voltage < 3.6f, low_batt, LOW_PRI_ALERT, "Battery is low");
    */
  }

  int querySeriesData(uint32_t start_secs_ago, uint32_t end_secs_ago, MinMaxAvg dest[], int max_num) override {
    battery_data.calcMinMaxAvg(getRTCClock(), start_secs_ago, end_secs_ago, &dest[0], TELEM_CHANNEL_SELF, LPP_VOLTAGE);
    return 1;
  }

  bool handleCustomCommand(uint32_t sender_timestamp, char* command, char* reply) override {
    if (strcmp(command, "magic") == 0) {    // example 'custom' command handling
      strcpy(reply, "**Magic now done**");
      return true;   // handled
    } else if (memcmp(command, "sout ", 5) == 0) {
      //SERIAL_GW.println(&command[5]);
      add_line(&command[5]);
      strcpy(reply, "ok");
      return true;
    }
    return false;  // not handled
  }

  bool handleIncomingMsg(ContactInfo& from, uint32_t timestamp, uint8_t* data, uint flags, size_t len) override {
    if (len > 3 && !memcmp(data, "s> ", 3)) {
      data[len] = 0;
      add_line((char*) &data[3]);
      return true;
    } else {
      data[len] = 0;
      add_line((char*)data);
      return true;
    }

    return SensorMesh::handleIncomingMsg(from, timestamp, data, flags, len);
  }

public:
  void loop() {
    SensorMesh::loop();
  }
  /* ======================================================================= */
};

StdRNG fast_rng;
SimpleMeshTables tables;

MyMesh the_mesh(board, radio_driver, *new ArduinoMillis(), fast_rng, rtc_clock, tables);

void halt() {
  while (1) ;
}

static char command[160];

void setup() {

  Serial.begin(115200);

  delay(1000);

  board.begin();

#ifdef DISPLAY_CLASS
  if (display.begin()) {
    refresh_screen();
  }
#endif

  if (!radio_init()) { halt(); }

  fast_rng.begin(radio_get_rng_seed());

  FILESYSTEM* fs;
#if defined(NRF52_PLATFORM) || defined(STM32_PLATFORM)
  InternalFS.begin();
  fs = &InternalFS;
  IdentityStore store(InternalFS, "");
#elif defined(ESP32)
  SPIFFS.begin(true);
  fs = &SPIFFS;
  IdentityStore store(SPIFFS, "/identity");
#elif defined(RP2040_PLATFORM)
  LittleFS.begin();
  fs = &LittleFS;
  IdentityStore store(LittleFS, "/identity");
  store.begin();
#else
  #error "need to define filesystem"
#endif
  if (!store.load("_main", the_mesh.self_id)) {
    MESH_DEBUG_PRINTLN("Generating new keypair");
    the_mesh.self_id = radio_new_identity();   // create new random identity
    int count = 0;
    while (count < 10 && (the_mesh.self_id.pub_key[0] == 0x00 || the_mesh.self_id.pub_key[0] == 0xFF)) {  // reserved id hashes
      the_mesh.self_id = radio_new_identity(); count++;
    }
    store.save("_main", the_mesh.self_id);
  }

  Serial.print("Sensor ID: ");
  mesh::Utils::printHex(Serial, the_mesh.self_id.pub_key, PUB_KEY_SIZE);
  Serial.println();

  command[0] = 0;

  sensors.begin();

  the_mesh.begin(fs);

  // send out initial Advertisement to the mesh
  the_mesh.sendSelfAdvertisement(16000);

  digitalWrite(DISP_BACKLIGHT, LOW);
}

void loop() {
  int len = strlen(command);
  while (Serial.available() && len < sizeof(command)-1) {
    char c = Serial.read();
    if (c != '\n') {
      command[len++] = c;
      command[len] = 0;
    }
    Serial.print(c);
  }
  if (len == sizeof(command)-1) {  // command buffer full
    command[sizeof(command)-1] = '\r';
  }

  if (len > 0 && command[len - 1] == '\r') {  // received complete line
    command[len - 1] = 0;  // replace newline with C string null terminator
    char reply[160];
    the_mesh.handleCommand(0, command, reply);  // NOTE: there is no sender_timestamp via serial!
    if (reply[0]) {
      Serial.print("  -> "); Serial.println(reply);
    }

    command[0] = 0;  // reset command buffer
  }
  the_mesh.loop();
  sensors.loop();
}
