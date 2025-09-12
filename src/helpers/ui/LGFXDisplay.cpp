#include "LGFXDisplay.h"

bool LGFXDisplay::begin() {
  turnOn();
  display->init();
  display->setRotation(1);
  display->setBrightness(64);
  display->setColorDepth(8);
  display->setTextColor(TFT_WHITE);

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
  display->clearDisplay();
//  display->display();
}

void LGFXDisplay::startFrame(Color bkg) {
  display->startWrite();
  display->getScanLine();
  display->clearDisplay();
  display->setTextColor(TFT_WHITE);

}

void LGFXDisplay::setTextSize(int sz) {
  display->setTextSize(sz);
}

void LGFXDisplay::setColor(Color c) {
  // _color = (c != 0) ? ILI9342_WHITE : ILI9342_BLACK;
  _color = TFT_WHITE;
  display->setTextColor(TFT_WHITE);
}

void LGFXDisplay::setCursor(int x, int y) {
  display->setCursor(x, y);
}

void LGFXDisplay::print(const char* str) {
  display->println(str);
//  Serial.println(str);
}

void LGFXDisplay::fillRect(int x, int y, int w, int h) {
  display->fillRect(x, y, w, h, _color);
}

void LGFXDisplay::drawRect(int x, int y, int w, int h) {
  display->drawRect(x, y, w, h, _color);
}

void LGFXDisplay::drawXbm(int x, int y, const uint8_t* bits, int w, int h) {
  display->drawBitmap(x, y, bits, w, h, TFT_BLUE);
}

uint16_t LGFXDisplay::getTextWidth(const char* str) {
  return display->textWidth(str);
}

void LGFXDisplay::endFrame() {

  display->endWrite();

  //  Serial.println("End Frame");
}

bool LGFXDisplay::getTouch(int *x, int *y) {
  lgfx::v1::touch_point_t point;
  display->getTouch(&point);
  *x = point.x;
  *y = point.y;
  return (*x >= 0) && (*y >= 0);
}