#include "WT32Display.h"

bool WT32Display::begin() {

    display.init();
    display.setRotation(1);
    display.setBrightness(64);
    display.setColorDepth(8);
    display.setTextColor(TFT_WHITE);

    return true;
  }

void WT32Display::turnOn() {
  _isOn = true;
}

void WT32Display::turnOff() {
  _isOn = false;
}

void WT32Display::clear() {
  display.clearDisplay();
//  display.display();
}

void WT32Display::startFrame(Color bkg) {
  display.startWrite();
  display.getScanLine();
  display.clearDisplay();
  display.setTextColor(TFT_WHITE);

}

void WT32Display::setTextSize(int sz) {
  display.setTextSize(sz);
}

void WT32Display::setColor(Color c) {
  // _color = (c != 0) ? ILI9342_WHITE : ILI9342_BLACK;
  _color = TFT_WHITE;
  display.setTextColor(TFT_WHITE);
}

void WT32Display::setCursor(int x, int y) {
  display.setCursor(x, y);
}

void WT32Display::print(const char* str) {
  display.println(str);
//  Serial.println(str);
}

void WT32Display::fillRect(int x, int y, int w, int h) {
  display.fillRect(x, y, w, h, _color);
}

void WT32Display::drawRect(int x, int y, int w, int h) {
  display.drawRect(x, y, w, h, _color);
}

void WT32Display::drawXbm(int x, int y, const uint8_t* bits, int w, int h) {
  display.drawBitmap(x, y, bits, w, h, TFT_BLUE);
}

uint16_t WT32Display::getTextWidth(const char* str) {
  return display.textWidth(str);
}

void WT32Display::endFrame() {

  display.endWrite();

  //  Serial.println("End Frame");
}
