#include "UITask.h"
#include <Arduino.h>
#include <helpers/CommonCLI.h>

#define AUTO_OFF_MILLIS      60000  // 1 min
#ifdef LILYGO_TECHO
#define BOOT_SCREEN_MILLIS   8000   // 8 seconds
#else
#define BOOT_SCREEN_MILLIS   4000   // 4 seconds
#endif

// 'meshcore', 128x13px
static const uint8_t meshcore_logo [] PROGMEM = {
    0x3c, 0x01, 0xe3, 0xff, 0xc7, 0xff, 0x8f, 0x03, 0x87, 0xfe, 0x1f, 0xfe, 0x1f, 0xfe, 0x1f, 0xfe, 
    0x3c, 0x03, 0xe3, 0xff, 0xc7, 0xff, 0x8e, 0x03, 0x8f, 0xfe, 0x3f, 0xfe, 0x1f, 0xff, 0x1f, 0xfe, 
    0x3e, 0x03, 0xc3, 0xff, 0x8f, 0xff, 0x0e, 0x07, 0x8f, 0xfe, 0x7f, 0xfe, 0x1f, 0xff, 0x1f, 0xfc, 
    0x3e, 0x07, 0xc7, 0x80, 0x0e, 0x00, 0x0e, 0x07, 0x9e, 0x00, 0x78, 0x0e, 0x3c, 0x0f, 0x1c, 0x00, 
    0x3e, 0x0f, 0xc7, 0x80, 0x1e, 0x00, 0x0e, 0x07, 0x1e, 0x00, 0x70, 0x0e, 0x38, 0x0f, 0x3c, 0x00, 
    0x7f, 0x0f, 0xc7, 0xfe, 0x1f, 0xfc, 0x1f, 0xff, 0x1c, 0x00, 0x70, 0x0e, 0x38, 0x0e, 0x3f, 0xf8, 
    0x7f, 0x1f, 0xc7, 0xfe, 0x0f, 0xff, 0x1f, 0xff, 0x1c, 0x00, 0xf0, 0x0e, 0x38, 0x0e, 0x3f, 0xf8, 
    0x7f, 0x3f, 0xc7, 0xfe, 0x0f, 0xff, 0x1f, 0xff, 0x1c, 0x00, 0xf0, 0x1e, 0x3f, 0xfe, 0x3f, 0xf0, 
    0x77, 0x3b, 0x87, 0x00, 0x00, 0x07, 0x1c, 0x0f, 0x3c, 0x00, 0xe0, 0x1c, 0x7f, 0xfc, 0x38, 0x00, 
    0x77, 0xfb, 0x8f, 0x00, 0x00, 0x07, 0x1c, 0x0f, 0x3c, 0x00, 0xe0, 0x1c, 0x7f, 0xf8, 0x38, 0x00, 
    0x73, 0xf3, 0x8f, 0xff, 0x0f, 0xff, 0x1c, 0x0e, 0x3f, 0xf8, 0xff, 0xfc, 0x70, 0x78, 0x7f, 0xf8, 
    0xe3, 0xe3, 0x8f, 0xff, 0x1f, 0xfe, 0x3c, 0x0e, 0x3f, 0xf8, 0xff, 0xfc, 0x70, 0x3c, 0x7f, 0xf8, 
    0xe3, 0xe3, 0x8f, 0xff, 0x1f, 0xfc, 0x3c, 0x0e, 0x1f, 0xf8, 0xff, 0xf8, 0x70, 0x3c, 0x7f, 0xf8, 
};

void UITask::begin(NodePrefs* node_prefs, const char* build_date, const char* firmware_version) {
  _display->turnOn();
  _prevBtnState = HIGH;
  _auto_off = millis() + AUTO_OFF_MILLIS;
  _node_prefs = node_prefs;
  // strip off dash and commit hash by changing dash to null terminator
  // e.g: v1.2.3-abcdef -> v1.2.3
  char *version = strdup(firmware_version);
  char *dash = strchr(version, '-');
  if(dash){
    *dash = 0;
  }

  // v1.2.3 (1 Jan 2025)
  sprintf(_version_info, "%s (%s)", version, build_date);
}

void UITask::renderCurrScreen() {
  char tmp[80];
  if (millis() < BOOT_SCREEN_MILLIS) { // boot screen
    // meshcore logo
    _display->setColor(DisplayDriver::BLUE);
    int logoWidth = 128;
    _display->drawXbm((_display->width() - logoWidth) / 2, 3, meshcore_logo, logoWidth, 13);

    // version info
    _display->setColor(DisplayDriver::LIGHT);
    _display->setTextSize(1);
    uint16_t versionWidth = _display->getTextWidth(_version_info);
    _display->setCursor((_display->width() - versionWidth) / 2, 22);
    _display->print(_version_info);
    // node type
    const char* node_type = "< Sensor >";
    uint16_t typeWidth = _display->getTextWidth(node_type);
    _display->setCursor((_display->width() - typeWidth) / 2, 35);
    _display->print(node_type);
  } else if (_screen == SENSORS) {
    refresh_sensors();
    char buf[100];
    _display->setCursor(
        _display->width()
          -_display->getTextWidth(_node_prefs->node_name) - 1, 3);
    _display->print(_node_prefs->node_name);
    _display->drawRect(0,15,_display->width(),1);
    int y = 18;
    int s_size = _sensors_arr.size();
    for (int i = 0; i < (scroll?DISPLAY_LINES-1:s_size); i++) {
      JsonObject v = _sensors_arr[(i+scroll_offset)%s_size];
      _display->setCursor(5, y);
      switch (v["type"].as<int>()) {
        case 136: // GPS
          sprintf(buf, "%.4f %.4f",
            v["value"]["latitude"].as<float>(),
            v["value"]["longitude"].as<float>());
          break;
        default: // will be a float for now
          sprintf(buf, "%.02f",
            v["value"].as<float>());
      }
      _display->setCursor(5, y);
      _display->print(v["name"].as<JsonString>().c_str());
      _display->setCursor(
        _display->width()-_display->getTextWidth(buf)-1, y
      );
      _display->print(buf);
      y = y + 12;
    }
    if (scroll) scroll_offset = (scroll_offset+1)%s_size;
    else scroll_offset = 0;
    new_lines = false;
  } else {  // home screen
    for (int i=0; i < DISPLAY_LINES; i++) {
      _display->setCursor(3, _display->height() - (12 * (i+1)));
      _display->print(display_lines[i]);
    }
    new_lines = false;
  }
}

void UITask::refresh_sensors() {
  CayenneLPP lpp(200);
  lpp.addVoltage(TELEM_CHANNEL_SELF, (float)board.getBattMilliVolts() / 1000.0f);
  sensors.querySensors(0xFF, lpp);
  _sensors_arr.clear();
  lpp.decode(lpp.getBuffer(), lpp.getSize(), _sensors_arr);
  scroll = _sensors_arr.size() > DISPLAY_LINES - 1; // there is a status line
}

void UITask::add_line(char * l) {
  int i;
  for (i = DISPLAY_LINES - 1; i >= 1; i--) {
    strncpy(display_lines[i], display_lines[i-1], LINE_LENGTH);
  }
  strncpy(display_lines[0], l, LINE_LENGTH);
  display_lines[0][LINE_LENGTH] = 0;

  new_lines = true;
}

bool UITask::toggleGps() {
  int n = sensors.getNumSettings();
  for (int i=0; i < n; i++) {
    if (!strcmp("gps", sensors.getSettingName(i))) {
      if (!strcmp("1", sensors.getSettingValue(i))) {
        sensors.setSettingValue("gps", "0");
        return false;
      } else {
        sensors.setSettingValue("gps", "1");
        return true;
      }
    }
  }
}

void UITask::loop() {
  if (new_lines) {
#ifndef LILYGO_TECHO
    if (!_display->isOn()) {
      _display->turnOn();
    } 
#endif
    _screen=HOME;
    _auto_off = millis() + AUTO_OFF_MILLIS;
  }

#ifdef PIN_USER_BTN
  static int time_pressed;
  if (millis() >= _next_read) {
    int btnState = digitalRead(PIN_USER_BTN);
    if (btnState != _prevBtnState) {
      if (btnState == LOW) {  // pressed?
        time_pressed = millis();
      } else {
        if (millis() - time_pressed > 3000) {
          toggleGps();
          new_lines = true;
        } else {
          if (_display->isOn()) {
            switch (_screen) {
              case HOME:
                _screen = SENSORS;
                _next_forced_refresh = millis() + 500000; // in 5 min
                break;
              case SENSORS:
                _screen = HOME;
                break;
            }
            new_lines = true;
          } else {
            _display->turnOn();
            new_lines = true;
            _screen = HOME;
          }
        }
        _auto_off = millis() + AUTO_OFF_MILLIS;   // extend auto-off timer 
      }
      _prevBtnState = btnState;
    }
    _next_read = millis() + 200;  // 5 reads per second
  }
#endif
  
  if (_display->isOn()) {
  #ifdef LILYGO_TECHO
    if (millis() > _next_forced_refresh && _screen == SENSORS) {
      _next_forced_refresh = millis() + 500000; // force refresh every 5min
      new_lines = true;
    }
  #endif
    if (new_lines 
       || ((millis() >= _next_refresh) && (_screen == SENSORS) && scroll)
        ) {
      _display->startFrame();
      renderCurrScreen();
      _display->endFrame();

      _next_refresh = millis() + 3000;   // refresh every 3 second
    }
    
#ifndef LILYGO_TECHO
    if (millis() > _auto_off) {
      _display->turnOff();
    }
#endif
  }
}
