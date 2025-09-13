#include "UITask.h"
#include <Arduino.h>
#include <helpers/CommonCLI.h>

#ifndef AUTO_OFF_MILLIS
  #define AUTO_OFF_MILLIS      60000  // 1 min
#endif
#ifdef LILYGO_TECHO
#define BOOT_SCREEN_MILLIS   8000   // 8 seconds
#else
#define BOOT_SCREEN_MILLIS   4000   // 4 seconds
#endif

void sendMsg(char * txt);

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
    new_lines = false;
  } else if (_screen == SENSORS) {
    refresh_sensors();
    char buf[30];
    char name[30];
    _display->setColor(DisplayDriver::Color::YELLOW);
    _display->setCursor(
        _display->width() -_display->getTextWidth(_node_prefs->node_name) - 1, 3);
    _display->print(_node_prefs->node_name);
    _display->drawRect(0,15,_display->width(),1);
    _display->setColor(DisplayDriver::Color::LIGHT);
    int y = 18;

    LPPReader r(_sensors_lpp.getBuffer(), _sensors_lpp.getSize());
    for (int i = 0; i < scroll_offset; i++) {
      uint8_t channel, type;
      r.readHeader(channel, type);
      r.skipData(type);
    }

    for (int i = 0; i < (scroll?DISPLAY_LINES-1:_sensors_nb); i++) {
      uint8_t channel, type;
      if (!r.readHeader(channel, type)) { // reached end, reset
        r.reset();
        r.readHeader(channel, type);
      }

      _display->setCursor(0, y);
      float v;
      switch (type) {
        case LPP_GPS: // GPS
          float lat, lon, alt;
          r.readGPS(lat, lon, alt);
          strcpy(name, "gps"); sprintf(buf, "%.4f %.4f", lat, lon);
          break;
        case LPP_VOLTAGE:
          r.readVoltage(v);
          strcpy(name, "voltage"); sprintf(buf, "%6.2f", v);
          break;
        case LPP_CURRENT:
          r.readCurrent(v);
          strcpy(name, "current"); sprintf(buf, "%.3f", v);
          break;
        case LPP_TEMPERATURE:
          r.readTemperature(v);
          strcpy(name, "temperature"); sprintf(buf, "%.2f", v);
          break;
        case LPP_RELATIVE_HUMIDITY:
          r.readRelativeHumidity(v);
          strcpy(name, "humidity"); sprintf(buf, "%.2f", v);
          break;
        case LPP_BAROMETRIC_PRESSURE:
          r.readPressure(v);
          strcpy(name, "pressure"); sprintf(buf, "%.2f", v);
          break;
        case LPP_ALTITUDE:
          r.readAltitude(v);
          strcpy(name, "altitude"); sprintf(buf, "%.0f", v);
          break;
        case LPP_POWER:
          r.readPower(v);
          strcpy(name, "power"); sprintf(buf, "%6.2f", v);
          break;
        default:
          r.skipData(type);
          strcpy(name, "unk"); sprintf(buf, "");
      }
      _display->setCursor(0, y);
      _display->print(name);
      _display->setCursor(
      _display->width()-_display->getTextWidth(buf)-1, y
      );
      _display->print(buf);
      y = y + 12;
    }
    if (scroll) scroll_offset = (scroll_offset+1)%_sensors_nb;
    else scroll_offset = 0;
    new_lines = false;
  } else if (_screen == KEYBOARD) {
    int x, y;
    y = 160;
    x = 0;
    char buf[2]="1";
    _display->setTextSize(2);
    _display->setCursor(10,2);
    _display->print(text_buffer);
    for (int i = 0; i < 4; i ++) {
      for (int j = 0; j < 10; j++) {
        _display->setCursor(x+20,y);
        buf[0] = keys[i][j];
        _display->print(buf);
        x = x + 48;
      }
      y = y + 40; 
      x = 0;
    }
    _display->setTextSize(1);
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
  _sensors_lpp.reset();
  _sensors_nb = 0;
  _sensors_lpp.addVoltage(TELEM_CHANNEL_SELF, (float)board.getBattMilliVolts() / 1000.0f);
  sensors.querySensors(0xFF, _sensors_lpp);
  LPPReader reader (_sensors_lpp.getBuffer(), _sensors_lpp.getSize());
  uint8_t channel, type;
  while(reader.readHeader(channel, type)) {
    reader.skipData(type);
    _sensors_nb ++;
  }
  scroll = _sensors_nb > DISPLAY_LINES - 1;
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
    if (!_display->isOn()) {
      _display->turnOn();
    } 
    _screen=HOME;
    _auto_off = millis() + AUTO_OFF_MILLIS;
  }

#ifdef HAS_TOUCH
  if (millis () >= _next_read) {
    int x, y;
    if (display.getTouch(&x, &y)) {
      if (!display.isOn()) {
        display.turnOn();
      } else { 
        switch(_screen) {
          case KEYBOARD:
            if (y < 120) {
              if (x < 160) { // left
                int len = strlen(text_buffer);
                if (len > 0) {
                  text_buffer[len - 1] = 0;
                }
              } else if (x > 320) { // right
                _screen = HOME;
              } else { //center
                sendMsg(text_buffer);
                strcpy(text_buffer, "");
              }
            } else if (y>140){
              int i = (y - 140) / 48;
              int j = x / 48;
              int l = strlen(text_buffer);
              text_buffer[l] = keys[i][j];
              text_buffer[l+1] = 0;
            }
            break;
          default:  
            if (y > 200) {
              _screen = KEYBOARD;
            } else if (x < 240) {
              _screen = HOME;
            } else {
              _screen = SENSORS;
            }
        }
      }
      new_lines = true;
      _auto_off = millis() + AUTO_OFF_MILLIS;   // extend auto-off timer 
    }
    _next_read = millis() + 200;
  }
#endif

#ifdef PIN_USER_BTN
  if (millis() >= _next_read) {
    int btnState = digitalRead(PIN_USER_BTN);
    if (btnState != _prevBtnState) {
      if (btnState == LOW) {  // pressed?
        time_btn_pressed = millis();
      } else {
        if (millis() - time_btn_pressed > 3000) {
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
#if defined(DISP_BACKLIGHT) && defined(BACKLIGHT_BTN)
  if (millis() > next_backlight_btn_check) {
    bool touch_state = digitalRead(PIN_BUTTON2);
    digitalWrite(DISP_BACKLIGHT, !touch_state);
    next_backlight_btn_check = millis() + 300;
  }
#endif
  
  if (_display->isOn()) {
    if (new_lines || ((millis() >= _next_refresh))) {
      _display->startFrame();
      renderCurrScreen();
      _display->endFrame();

      _next_refresh = millis() + 5000;   // refresh every 10 seconds if no new line
    }
    
#if AUTO_OFF_MILLIS > 0
    if (millis() > _auto_off) {
      _display->turnOff();
    }
#endif
  }
}
