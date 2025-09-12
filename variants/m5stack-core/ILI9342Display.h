#pragma once

#define LGFX_AUTODETECT 


#include <helpers/ui/DisplayDriver.h>

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#include <LGFX_AUTODETECT.hpp> 

class ILI9342Display : public DisplayDriver {
  LGFX display;
  LGFX_Sprite sprite;
  bool _isOn;
  uint8_t _color = TFT_WHITE;

public:
  ILI9342Display() : DisplayDriver(320, 240), sprite(&display) { _isOn = false; }
  bool begin();

  bool isOn() override { return _isOn; }
  void turnOn() override;
  void turnOff() override;
  void clear() override;
  void startFrame(Color bkg = DARK) override;
  void setTextSize(int sz) override;
  void setColor(Color c) override;
  void setCursor(int x, int y) override;
  void print(const char* str) override;
  void fillRect(int x, int y, int w, int h) override;
  void drawRect(int x, int y, int w, int h) override;
  void drawXbm(int x, int y, const uint8_t* bits, int w, int h) override;
  uint16_t getTextWidth(const char* str) override;
  void endFrame() override;
};
