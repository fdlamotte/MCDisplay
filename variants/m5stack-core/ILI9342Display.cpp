#include "ILI9342Display.h"

bool ILI9342Display::begin() {

    display.init();
    display.setRotation(1);
    display.setBrightness(64);
    display.setColorDepth(8);
    display.setTextColor(TFT_WHITE);

    return true;
  }

void ILI9342Display::turnOn() {
  _isOn = true;
}

void ILI9342Display::turnOff() {
  _isOn = false;
}

void ILI9342Display::clear() {
  display.clearDisplay();
//  display.display();
}

void ILI9342Display::startFrame(Color bkg) {
  display.startWrite();
  display.getScanLine();
  display.clearDisplay();
  display.setTextColor(TFT_WHITE);

}

void ILI9342Display::setTextSize(int sz) {
  display.setTextSize(sz);
}

void ILI9342Display::setColor(Color c) {
  // _color = (c != 0) ? ILI9342_WHITE : ILI9342_BLACK;
  _color = TFT_WHITE;
  display.setTextColor(TFT_WHITE);
}

void ILI9342Display::setCursor(int x, int y) {
  display.setCursor(x, y);
}

void ILI9342Display::print(const char* str) {
  display.println(str);
//  Serial.println(str);
}

void ILI9342Display::fillRect(int x, int y, int w, int h) {
  display.fillRect(x, y, w, h, _color);
}

void ILI9342Display::drawRect(int x, int y, int w, int h) {
  display.drawRect(x, y, w, h, _color);
}

void ILI9342Display::drawXbm(int x, int y, const uint8_t* bits, int w, int h) {
  display.drawBitmap(x, y, bits, w, h, TFT_BLUE);
}

uint16_t ILI9342Display::getTextWidth(const char* str) {
  return display.textWidth(str);
}

void ILI9342Display::endFrame() {

  display.endWrite();

  //  Serial.println("End Frame");
}
