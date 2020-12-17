#ifndef MILO_LIGHT_LED
#define MILO_LIGHT_LED

#include "Lights.h"

const LEDColor LED::Color(int r, int g, int b) {
  return Adafruit_NeoPixel::Color(r, g, b);
}

LED::LED(uint16_t index, PanelSegment& ps, TriPanel& p)
    : stripIndex(index), mySide(ps), myPanel(p) {
  nextColorAvailable = false;
  nextColor = 0;
  nextColorChangeTime = 0;
}

LED::~LED() {}

void LED::resetColor() {
  myPanel.lights.setPixelColor(stripIndex, currentColor);
  myPanel.LEDchanged = true;
}

void LED::setColor(LEDColor color, MilliSec timeDelay) {
  nextColorChangeTime = currentTime + timeDelay;

  if (timeDelay) {
    nextColorAvailable = true;
    nextColor = color;
    myPanel.changeLEDLater(this);
  }
  else {
    nextColorAvailable = false;
    nextColorChangeTime = 0;
    currentColor = color;
    myPanel.lights.setPixelColor(stripIndex, color);
    myPanel.LEDchanged = true;
  }
}

#endif  // MILO_LIGHT_LED
