#pragma once

#include <helpers/ui/DisplayDriver.h>
#include <helpers/CommonCLI.h>
#include <target.h>
#include <helpers/sensors/LPPDataHelpers.h>

#ifndef LINE_LENGTH
#define LINE_LENGTH 20
#endif

class UITask {
  DisplayDriver* _display;
  unsigned long _next_read, _next_refresh, _auto_off, _next_forced_refresh;
  int _prevBtnState;
  NodePrefs* _node_prefs;
  char _version_info[32];
  bool new_lines = true;
  bool scroll = false;
  int scroll_offset = 0;
  enum {HOME, SENSORS, KEYBOARD} _screen = HOME;
  CayenneLPP _sensors_lpp;
  int _sensors_nb = 0;
  int time_btn_pressed;
  int next_backlight_btn_check = 0;
  char text_buffer[100];
  char keys[4][11] = {"1234567890", "azertyuiop", "qsdfghjklm", "wxcvbn    " };


  char display_lines[DISPLAY_LINES][LINE_LENGTH+1] = {
  "",
  "Hello from MeshCore"
  };

  void renderCurrScreen();
public:
  UITask(DisplayDriver& display) : _display(&display), _sensors_lpp(200) { _next_read = _next_refresh = _next_forced_refresh = 0; }
  void begin(NodePrefs* node_prefs, const char* build_date, const char* firmware_version);
  void refresh_sensors();
  void add_line (char* s);
  void loop();
  bool toggleGps();
};