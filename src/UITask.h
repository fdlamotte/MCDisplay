#pragma once

#include <helpers/ui/DisplayDriver.h>
#include <helpers/CommonCLI.h>
#include <target.h>

#ifndef LINE_LENGTH
#define LINE_LENGTH 20
#endif

class UITask {
  DisplayDriver* _display;
  unsigned long _next_read, _next_refresh, _auto_off;
  int _prevBtnState;
  NodePrefs* _node_prefs;
  char _version_info[32];
  bool new_lines = true;
  bool scroll = false;
  int scroll_offset = 0;
  enum {HOME, SENSORS} _screen = HOME;
  DynamicJsonDocument _sensors_doc;
  JsonArray _sensors_arr;

  char display_lines[DISPLAY_LINES][LINE_LENGTH+1] = {
  "",
  "Hello from MeshCore"
  };

  void renderCurrScreen();
public:
  UITask(DisplayDriver& display) : _display(&display), _sensors_doc(2048) { _next_read = _next_refresh = 0; _sensors_arr=_sensors_doc.to<JsonArray>(); }
  void begin(NodePrefs* node_prefs, const char* build_date, const char* firmware_version);
  void refresh_sensors();
  void add_line (char* s);
  void loop();
};