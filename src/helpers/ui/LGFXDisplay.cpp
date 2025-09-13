#include "LGFXDisplay.h"

bool LGFXDisplay::begin() {
  turnOn();
  display->init();
  display->setRotation(1);
  display->setBrightness(64);
  display->setColorDepth(8);
  display->setTextColor(TFT_WHITE);

  buffer.setColorDepth(8);
  buffer.setPsram(true);
  buffer.createSprite(width(), height());

  return true;
}

void LGFXDisplay::turnOn() {
//  display->wakeup();
  if (!_isOn) {
    display->wakeup();
  }
  _isOn = true;
}

void LGFXDisplay::turnOff() {
  if (_isOn) {
    display->sleep();
  }
  _isOn = false;
}

void LGFXDisplay::clear() {
//  display->clearDisplay();
  buffer.clearDisplay();
}

void LGFXDisplay::startFrame(Color bkg) {
//  display->startWrite();
//  display->getScanLine();
  buffer.clearDisplay();
  buffer.setTextColor(TFT_WHITE);
}

void LGFXDisplay::setTextSize(int sz) {
  buffer.setTextSize(sz);
}

void LGFXDisplay::setColor(Color c) {
  // _color = (c != 0) ? ILI9342_WHITE : ILI9342_BLACK;
  _color = TFT_WHITE;
  buffer.setTextColor(TFT_WHITE);
}

void LGFXDisplay::setCursor(int x, int y) {
  buffer.setCursor(x, y);
}

void LGFXDisplay::print(const char* str) {
  buffer.println(str);
//  Serial.println(str);
}

void LGFXDisplay::fillRect(int x, int y, int w, int h) {
  buffer.fillRect(x, y, w, h, _color);
}

void LGFXDisplay::drawRect(int x, int y, int w, int h) {
  buffer.drawRect(x, y, w, h, _color);
}

void LGFXDisplay::drawXbm(int x, int y, const uint8_t* bits, int w, int h) {
  buffer.drawBitmap(x, y, bits, w, h, TFT_BLUE);
}

uint16_t LGFXDisplay::getTextWidth(const char* str) {
  return display->textWidth(str);
}

void LGFXDisplay::endFrame() {
  Serial.println(display->getScanLine());
  display->startWrite();
  buffer.pushSprite(display, 0, 0);
  display->endWrite();
}

bool LGFXDisplay::getTouch(int *x, int *y) {
  lgfx::v1::touch_point_t point;
  display->getTouch(&point);
  *x = point.x;
  *y = point.y;
  return (*x >= 0) && (*y >= 0);
}