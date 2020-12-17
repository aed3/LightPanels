#ifndef MILO_PANEL_SEGMENT
#define MILO_PANEL_SEGMENT

#include "Lights.h"

PanelSegment::PanelSegment(
  SideLocation s, int min, int max, TriPanel& p)
    : side(s),
      minPixelIndex(min),
      maxPixelIndex(max),
      numLeds(max - min + 1),
      myPanel(p) {
  for (int i = minPixelIndex; i <= maxPixelIndex; i++) {
    leds.push_back(LED(i, *this, myPanel));
  }
}

PanelSegment::~PanelSegment() {}

SideLocation PanelSegment::cornersOtherSide(CornerLocation corner) {
  switch (corner) {
    case CL_LT:
      return side == SL_LEFT ? SL_TOP : SL_LEFT;
    case CL_RT:
      return side == SL_RIGHT ? SL_TOP : SL_RIGHT;
    case CL_LB:
      return side == SL_LEFT ? SL_BOTTOM : SL_LEFT;
    case CL_RB:
      return side == SL_RIGHT ? SL_BOTTOM : SL_RIGHT;
    default:
      return side == SL_LEFT ? SL_RIGHT : SL_LEFT;
  }
}

template <class Function>
void PanelSegment::forEachLed(Function fn) {
  for (LED& led : leds) {
    fn(led);
  }
}

void PanelSegment::resetPixelColor(uint16_t stripIndex) {
  leds[stripIndex - minPixelIndex].resetColor();
}

bool PanelSegment::fill(double percent, LEDColor color,
  CornerLocation startLocation, MilliSec duration) {
  const SideLocation opposingSide = cornersOtherSide(startLocation);
  const bool loopBack = (nextSegLocation == opposingSide) == (percent > 0);
  const int pixels2Fill = round(numLeds * fabs(percent));
  const int timeBetweenLed = pixels2Fill ? duration / pixels2Fill : 0;

  const int firstLed = loopBack ? numLeds - 1 : 0;
  const int incr = loopBack ? -1 : 1;

  for (int i = 0; i < pixels2Fill; i++) {
    leds[firstLed + i * incr].setColor(color, timeBetweenLed * i);
  }

  return pixels2Fill == numLeds;
}

LEDColor PanelSegment::getPixelColor(uint16_t stripIndex) {
  return leds[stripIndex - minPixelIndex].currentColor;
}

void PanelSegment::setPixelColor(
  uint16_t stripIndex, LEDColor color, MilliSec timeDelay) {
  leds[stripIndex - minPixelIndex].setColor(color, timeDelay);
}

void PanelSegment::setColor(LEDColor color, MilliSec timeDelay) {
  forEachLed([=](LED& led) { led.setColor(color, timeDelay); });
}

#endif  // MILO_PANEL_SEGMENT
