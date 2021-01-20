#ifndef MILO_LIGHT_TRI_PANEL
#define MILO_LIGHT_TRI_PANEL

#include "Lights.h"

TriPanel::TriPanel(int pin, uint16_t numLeds, CornerLocation local,
  LoopDirection spin, CornerLocation start)
    : overallLocation(local),
      lightDirection(spin),
      stripStartLoctation(start),
      lights(Adafruit_NeoPixel(numLeds, pin, NEO_GRB + NEO_KHZ800)),
      LEDchanged(true) {
  const int corner1 = (numLeds - 2) / 3;
  const int corner2 = corner1 * 2 + 1;

  SideLocation segment1Location = calcFirstSegmentLocation();
  SideLocation segment2Location = nextSegmentLocation(segment1Location);
  SideLocation segment3Location = nextSegmentLocation(segment2Location);

  PanelSegment segment1(segment1Location, 0, corner1, *this);
  PanelSegment segment2(segment2Location, corner1 + 1, corner2, *this);
  PanelSegment segment3(segment3Location, corner2 + 1, numLeds - 1, *this);

  segment1.nextSegLocation = segment3.prevSegLocation = segment2Location;
  segment2.nextSegLocation = segment1.prevSegLocation = segment3Location;
  segment3.nextSegLocation = segment2.prevSegLocation = segment1Location;

  segments.push_back(segment1);
  segments.push_back(segment2);
  segments.push_back(segment3);

  segSideIndex[segment1Location] = 0;
  segSideIndex[segment2Location] = 1;
  segSideIndex[segment3Location] = 2;

  switch (local) {
    case CL_MT:
      cornerAtCenter = CL_MB;
      break;
    case CL_MB:
      cornerAtCenter = CL_MT;
      break;
    case CL_LT:
      cornerAtCenter = CL_RB;
      break;
    case CL_LB:
      cornerAtCenter = CL_RT;
      break;
    case CL_RT:
      cornerAtCenter = CL_LB;
      break;
    case CL_RB:
      cornerAtCenter = CL_LT;
      break;
  }

  outerSide = OppositeSideOfCorner(cornerAtCenter);
}

TriPanel::~TriPanel() {}

const SideLocation TriPanel::calcFirstSegmentLocation() {
  switch (stripStartLoctation) {
    case CL_MT:
      return lightDirection == CW ? SL_RIGHT : SL_LEFT;
    case CL_MB:
      return lightDirection == CW ? SL_LEFT : SL_RIGHT;
    case CL_LT:
      return lightDirection == CW ? SL_TOP : SL_LEFT;
    case CL_LB:
      return lightDirection == CW ? SL_LEFT : SL_BOTTOM;
    case CL_RT:
      return lightDirection == CW ? SL_RIGHT : SL_TOP;
    case CL_RB:
      return lightDirection == CW ? SL_BOTTOM : SL_RIGHT;
  }
}

const SideLocation TriPanel::nextSegmentLocation(SideLocation currentSeg) {
  return nextSegmentLocation(currentSeg, lightDirection);
}

const SideLocation TriPanel::nextSegmentLocation(
  SideLocation currentSeg, LoopDirection direction) {
  switch (overallLocation) {
    case CL_MT:
    case CL_LB:
    case CL_RB:
      switch (currentSeg) {
        case SL_TOP:
          return direction == CW ? SL_RIGHT : SL_LEFT;
        case SL_RIGHT:
          return direction == CW ? SL_LEFT : SL_TOP;
        case SL_LEFT:
          return direction == CW ? SL_TOP : SL_RIGHT;
      }
    case CL_MB:
    case CL_LT:
    case CL_RT:
      switch (currentSeg) {
        case SL_RIGHT:
          return direction == CW ? SL_BOTTOM : SL_LEFT;
        case SL_BOTTOM:
          return direction == CW ? SL_LEFT : SL_RIGHT;
        case SL_LEFT:
          return direction == CW ? SL_RIGHT : SL_BOTTOM;
      }
  }
}

const SideLocation TriPanel::OppositeSideOfCorner(CornerLocation corner) {
  switch (corner) {
    case CL_MT:
      return SL_BOTTOM;
    case CL_MB:
      return SL_TOP;
    case CL_LT:
    case CL_LB:
      return SL_RIGHT;
    default:
      return SL_LEFT;
  }
}

const std::pair<SideLocation, SideLocation> TriPanel::corner2Side(
  CornerLocation corner) {
  SideLocation vertical;
  SideLocation horizontal;

  switch (corner) {
    case CL_MT:
    case CL_MB:
      return {SL_LEFT, SL_RIGHT};
    case CL_LT:
    case CL_RT:
      vertical = SL_TOP;
      break;
    default:
      vertical = SL_BOTTOM;
      break;
  }

  switch (corner) {
    case CL_LT:
    case CL_LB:
      horizontal = SL_LEFT;
      break;
    case CL_RT:
    case CL_RB:
      horizontal = SL_RIGHT;
      break;
  }

  return {vertical, horizontal};
}

void TriPanel::fadeController(uint8_t brightness, MilliSec& timePassed,
  MilliSec timeBetweenChange, bool constantColor) {
  runFunctionLater(
    [this, brightness, constantColor]() {
      setBrightness(brightness);
      if (constantColor) {
        for (size_t i = 0; i < lights.numPixels(); i++) {
          resetPixelColor(i);
        }
      }
    },
    timePassed += timeBetweenChange);
}

void TriPanel::runFunctionLater(std::function<void()> fn, MilliSec timeDelay) {
  timeDelay += currentTime;
  if (functionSequence.empty()) {
    functionSequence.push_back({timeDelay, fn});
    return;
  }

  for (auto it = functionSequence.begin(); it != functionSequence.end(); ++it) {
    if (it->first >= timeDelay) {
      functionSequence.insert(it, {timeDelay, fn});
      return;
    }
  }

  functionSequence.push_back({timeDelay, fn});
}

void TriPanel::playFunctionSequence() {
  while (!functionSequence.empty()) {
    const MilliSec waitTime = functionSequence.front().first;
    const std::function<void()> fn = functionSequence.front().second;
    const bool timesUp = waitTime < currentTime;
    if (timesUp) {
      functionSequence.pop_front();
      fn();
    }
    else if (!timesUp) {
      return;
    }
  }
}

void TriPanel::showDelayedLEDs() {
  while (!delayedLEDs.empty()) {
    LED* led = delayedLEDs.front();
    const bool timesUp = led->nextColorChangeTime < currentTime;
    if (led->nextColorAvailable && timesUp) {
      delayedLEDs.pop_front();
      led->setColor(led->nextColor);
    }
    else if (!timesUp) {
      break;
    }
  }
}

template <class Function>
void TriPanel::forEachSegment(Function fn) {
  for (PanelSegment& segment : segments) {
    fn(segment);
  }
}

double TriPanel::spinSpeed2Duration(uint8_t speed) {
  return 256 * round(500 * (1 - speed / 256.0));
}

void TriPanel::changeLEDLater(LED* led) {
  if (delayedLEDs.empty()) {
    delayedLEDs.push_front(led);
    return;
  }

  for (auto it = delayedLEDs.begin(); it != delayedLEDs.end(); ++it) {
    if ((*it)->nextColorChangeTime >= led->nextColorChangeTime) {
      delayedLEDs.insert(it, led);
      return;
    }
  }

  delayedLEDs.push_back(led);
}

void TriPanel::breathe(
  uint8_t maxBrightness, MilliSec fadeDuration, LEDColor color, bool cc) {
  bool constantColor = cc || (delayedLEDs.empty() && functionSequence.empty());
  if (color) {
    setColor(color);
    constantColor = true;
  }
  setBrightness(0);

  fadeIn(maxBrightness, constantColor, fadeDuration);

  runFunctionLater(
    [this, fadeDuration, constantColor]() {
      fadeOut(0, constantColor, fadeDuration);
    },
    fadeDuration * 1.5);
  runFunctionLater(
    [this, color, maxBrightness, fadeDuration, constantColor]() {
      breathe(maxBrightness, fadeDuration, color, constantColor);
    },
    fadeDuration * 2.5);
}

void TriPanel::fadeIn(
  uint8_t maxBrightness, bool constantColor, MilliSec duration) {
  const uint8_t currentBrightness = getBrightness();
  if (currentBrightness >= maxBrightness) return;

  const MilliSec timeBetweenChange =
    (double)duration / (maxBrightness - currentBrightness);

  MilliSec timePassed = 0;
  for (size_t b = currentBrightness; b <= maxBrightness; b++) {
    fadeController(b, timePassed, timeBetweenChange, constantColor);
  }
}

void TriPanel::fadeOut(
  uint8_t minBrightness, bool constantColor, MilliSec duration) {
  const uint8_t currentBrightness = getBrightness();
  if (currentBrightness <= minBrightness) return;

  const MilliSec timeBetweenChange =
    (double)duration / (currentBrightness - minBrightness);

  MilliSec timePassed = 0;
  for (int b = currentBrightness; b >= minBrightness; b--) {
    fadeController(b, timePassed, timeBetweenChange, constantColor);
  }
}

void TriPanel::fillFromCorner(
  double percent, LEDColor color, MilliSec duration) {
  fillFromCorner(percent, color, cornerAtCenter, duration);
}

void TriPanel::fillFromCorner(
  double percent, LEDColor color, CornerLocation start, MilliSec duration) {
  std::pair<SideLocation, SideLocation> sidesUsed = corner2Side(start);

  PanelSegment& segment1 = segments[segSideIndex[sidesUsed.first]];
  PanelSegment& segment2 = segments[segSideIndex[sidesUsed.second]];

  PanelSegment& capSegment =
    segments[segSideIndex[OppositeSideOfCorner(start)]];

  bool allLightsOn1 = segment1.fill(percent, color, start, duration);
  bool allLightsOn2 = segment2.fill(percent, color, start, duration);

  if (allLightsOn1 && allLightsOn2) {
    capSegment.setColor(color, duration);
  }
}

void TriPanel::fillToCorner(double percent, LEDColor color, MilliSec duration) {
  fillToCorner(percent, color, cornerAtCenter, duration);
}

void TriPanel::fillToCorner(
  double percent, LEDColor color, CornerLocation start, MilliSec duration) {
  std::pair<SideLocation, SideLocation> sidesUsed = corner2Side(start);

  PanelSegment& segment1 = segments[segSideIndex[sidesUsed.first]];
  PanelSegment& segment2 = segments[segSideIndex[sidesUsed.second]];

  PanelSegment& capSegment =
    segments[segSideIndex[OppositeSideOfCorner(start)]];

  capSegment.setColor(color);

  segment1.fill(-percent, color, start, duration);
  segment2.fill(-percent, color, start, duration);
}

void TriPanel::rainbow(double loops, uint8_t speed) {
  if (!loops) {
    rainbow(1, speed);
    runFunctionLater(
      [this, speed]() { rainbow(0, speed); }, spinSpeed2Duration(speed));
  }

  const int pixelCount = lights.numPixels();
  const double colorsToSkip = 512.0 / pixelCount;

  for (size_t i = 0; i < pixelCount; i++) {
    setPixelColor(i,
      pgm_read_dword(&rainbowColors[(int)(i * colorsToSkip) % 512]));
  }

  colorSpin(loops, speed);
}

void TriPanel::rainbowTimed(MilliSec duration, uint8_t speed) {
  rainbow(duration / spinSpeed2Duration(speed), speed);
}

void TriPanel::colorSpin(double loops, uint8_t speed) {
  if (!loops) {
    colorSpin(1, speed);
    runFunctionLater(
      [this, speed]() { colorSpin(0, speed); }, spinSpeed2Duration(speed));
    return;
  }

  const int pixelCount = lights.numPixels();
  const int functionDelay = spinSpeed2Duration(speed) / pixelCount;
  const int incr = pixelCount + (lightDirection == CW ? -1 : 1);

  for (size_t currentPixel = 0; currentPixel < pixelCount * loops;
       currentPixel++) {
    auto fn = [this, currentPixel, pixelCount, incr]() {
      LEDColor prevColors[pixelCount];
      for (size_t i = 0; i < pixelCount; i++) {
        prevColors[i] = getPixelColor(i);
      }

      for (size_t i = 0; i < pixelCount; i++) {
        setPixelColor(i, prevColors[(i + incr) % pixelCount]);
      }
    };

    runFunctionLater(fn, currentPixel * functionDelay);
  }
}

void TriPanel::resetPixelColor(uint16_t stripIndex) {
  for (PanelSegment& segment : segments) {
    if (segment.minPixelIndex <= stripIndex &&
        segment.maxPixelIndex >= stripIndex) {
      segment.resetPixelColor(stripIndex);
      return;
    }
  }
}

void TriPanel::clearFunctions() {
  functionSequence.clear();
  delayedLEDs.clear();
}

void TriPanel::begin(uint8_t brightness) {
  lights.begin();
  lights.clear();
  setBrightness(brightness);
  show();
}

uint8_t TriPanel::getBrightness() { return lights.getBrightness(); }

void TriPanel::setBrightness(uint8_t b) {
  lights.setBrightness(b);
  LEDchanged = true;
}

void TriPanel::setColor(LEDColor color, MilliSec timeDelay) {
  forEachSegment(
    [=](PanelSegment& segment) { segment.setColor(color, timeDelay); });
}

void TriPanel::setColor(std::vector<LEDColor> colors, MilliSec timeDelay) {
  const double incr = (double)colors.size() / lights.numPixels();
  for (size_t i = 0; i < lights.numPixels(); i++) {
    setPixelColor(i, colors[i * incr], timeDelay);
  }
}

std::vector<LEDColor> TriPanel::getColor() {
  std::vector<LEDColor> colors(lights.numPixels());
  for (size_t i = 0; i < lights.numPixels(); i++) {
    colors[i] = getPixelColor(i);
  }
  return colors;
}

LEDColor TriPanel::getPixelColor(uint16_t stripIndex) {
  for (PanelSegment& segment : segments) {
    if (segment.minPixelIndex <= stripIndex &&
        segment.maxPixelIndex >= stripIndex) {
      return segment.getPixelColor(stripIndex);
    }
  }
}

void TriPanel::setPixelColor(
  uint16_t stripIndex, LEDColor color, MilliSec timeDelay) {
  for (PanelSegment& segment : segments) {
    if (segment.minPixelIndex <= stripIndex &&
        segment.maxPixelIndex >= stripIndex) {
      segment.setPixelColor(stripIndex, color, timeDelay);
      return;
    }
  }
}

void TriPanel::show() {
  playFunctionSequence();
  showDelayedLEDs();

  if (LEDchanged) {
    lights.show();
    LEDchanged = false;
  }
}

#endif  // MILO_LIGHT_TRI_PANEL
