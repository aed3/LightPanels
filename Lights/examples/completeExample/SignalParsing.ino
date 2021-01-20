#include <Lights.h>

void bitAddress(uint8_t location, uint8_t& byte, uint8_t& bit) {
  byte = location / 8;
  bit = location % 8;
}

uint8_t getBit(const uint8_t* data, uint8_t location) {
  uint8_t byte, bit = 0;
  bitAddress(location, byte, bit);
  return (data[byte] >> bit) & 1;
}

uint32_t getBitsMask(uint32_t data, uint32_t mask, uint8_t shift = 0) {
  return (data >> shift) & mask;
}

uint32_t getBits(const uint8_t* data, uint8_t startLocation, uint8_t length) {
  const uint32_t MAX32 = 0xffffffff;
  uint8_t byte, bit;
  bitAddress(startLocation, byte, bit);

  uint32_t value = *((uint32_t*)&data[byte]);
  return getBitsMask(value, MAX32 >> (32 - length), bit);
}

template <class ARG>
void fnArgFromSignal(
  const uint8_t* data, ARG& arg, uint8_t& bitsUsed, uint8_t argBits = 0) {
  if (!argBits) {
    argBits = sizeof(arg) << 3;
  }
  arg = getBits(data, bitsUsed, argBits);
  bitsUsed += argBits;
}

void parseHexagon(const uint8_t* data, uint8_t fnCode) {
  uint8_t bitsUsed = 7;

  switch (fnCode) {
    // breathe(uint8_t, MilliSec, LEDColor);
    case 0: {
      uint8_t maxBrightness;
      MilliSec fadeDuration;
      LEDColor color;

      fnArgFromSignal(data, maxBrightness, bitsUsed);
      fnArgFromSignal(data, fadeDuration, bitsUsed, 32);
      fnArgFromSignal(data, color, bitsUsed, 24);
      hex.breathe(maxBrightness, fadeDuration, color);
      break;
    }
    // colorShift(MilliSec, uint16_t);
    case 1: {
      MilliSec timeDelay;
      uint16_t shifts;

      fnArgFromSignal(data, timeDelay, bitsUsed, 32);
      fnArgFromSignal(data, shifts, bitsUsed);
      hex.colorShift(timeDelay, shifts);
      break;
    }
    // rainbowTimed(MilliSec, uint8_t)
    case 2: {
      MilliSec duration;
      uint8_t speed;

      fnArgFromSignal(data, duration, bitsUsed, 32);
      fnArgFromSignal(data, speed, bitsUsed);

      hex.clearFunctions();
      hex.rainbowTimed(duration, speed);
      break;
    }
    // rainbow(double (as uint16_t), uint8_t);
    case 3: {
      uint16_t loops;
      uint8_t speed;

      fnArgFromSignal(data, loops, bitsUsed);
      fnArgFromSignal(data, speed, bitsUsed);

      hex.clearFunctions();
      hex.rainbow(loops, speed);
      break;
    }
    // setColor(LEDColor, MilliSec);
    case 4: {
      LEDColor color;
      MilliSec timeDelay;

      fnArgFromSignal(data, color, bitsUsed, 24);
      fnArgFromSignal(data, timeDelay, bitsUsed, 32);

      hex.clearFunctions();
      hex.setColor(color, timeDelay);
      break;
    }
    // setBrightness(uint8_t);
    case 5: {
      uint8_t brightness;

      fnArgFromSignal(data, brightness, bitsUsed);
      hex.setBrightness(brightness);
      break;
    }
    // set all panels to different colors (uint8_t);
    case 31: {
      bitsUsed = 8;

      for (int i = 0; i < 6; i++) {
        LEDColor color;
        fnArgFromSignal(data, color, bitsUsed, 24);
        hex.panels[i]->setColor(color);
      }
      break;
    }
  }
}

void parseTriPanel(const uint8_t* data, uint8_t fnCode, uint8_t CL) {
  uint8_t bitsUsed = 10;

  switch (fnCode) {
    // breathe(uint8_t, MilliSec, LEDColor);
    case 0: {
      uint8_t maxBrightness;
      MilliSec fadeDuration;
      LEDColor color;

      fnArgFromSignal(data, maxBrightness, bitsUsed);
      fnArgFromSignal(data, fadeDuration, bitsUsed, 32);
      fnArgFromSignal(data, color, bitsUsed, 24);
      hex.panels[CL]->breathe(maxBrightness, fadeDuration, color);
      break;
    }
    // fadeIn(uint8_t, MilliSec);
    case 1: {
      uint8_t maxBrightness;
      MilliSec fadeDuration;

      fnArgFromSignal(data, maxBrightness, bitsUsed);
      fnArgFromSignal(data, fadeDuration, bitsUsed, 32);
      bool constantColor = hex.panels[CL]->delayedLEDs.empty() &&
        hex.panels[CL]->functionSequence.empty();

      hex.panels[CL]->fadeIn(maxBrightness, constantColor, fadeDuration);
      break;
    }
    // fadeOut(uint8_t, MilliSec);
    case 2: {
      uint8_t minBrightness;
      MilliSec fadeDuration;

      fnArgFromSignal(data, minBrightness, bitsUsed);
      fnArgFromSignal(data, fadeDuration, bitsUsed, 32);
      bool constantColor = hex.panels[CL]->delayedLEDs.empty() &&
        hex.panels[CL]->functionSequence.empty();

      hex.panels[CL]->fadeOut(minBrightness, constantColor, fadeDuration);
      break;
    }
    // fillFromCorner(double (as uint8_t), LEDColor, MilliSec, CornerLocation);
    case 3: {
      uint8_t percent;
      LEDColor color;
      MilliSec duration;
      uint8_t startLocation;

      fnArgFromSignal(data, percent, bitsUsed);
      fnArgFromSignal(data, color, bitsUsed, 24);
      fnArgFromSignal(data, duration, bitsUsed, 32);
      fnArgFromSignal(data, startLocation, bitsUsed, 3);

      hex.panels[CL]->clearFunctions();
      if (CL == 0b111) {
        hex.panels[CL]->fillFromCorner(percent / 100.0, color, duration);
      }
      else {
        hex.panels[CL]->fillFromCorner(percent / 100.0, color,
          static_cast<CornerLocation>(startLocation), duration);
      }
      break;
    }
    // fillToCorner(double (as uint8_t), LEDColor, MilliSec, CornerLocation);
    case 4: {
      uint8_t percent;
      LEDColor color;
      MilliSec duration;
      uint8_t startLocation;

      fnArgFromSignal(data, percent, bitsUsed);
      fnArgFromSignal(data, color, bitsUsed, 24);
      fnArgFromSignal(data, duration, bitsUsed, 32);
      fnArgFromSignal(data, startLocation, bitsUsed, 3);

      hex.panels[CL]->clearFunctions();
      if (CL == 0b111) {
        hex.panels[CL]->fillToCorner(percent / 100.0, color, duration);
      }
      else {
        hex.panels[CL]->fillToCorner(percent / 100.0, color,
          static_cast<CornerLocation>(startLocation), duration);
      }
      break;
    }
    // colorSpin(double (as uint16_t), uint8_t);
    case 5: {
      uint16_t loops;
      uint8_t speed;

      fnArgFromSignal(data, loops, bitsUsed);
      fnArgFromSignal(data, speed, bitsUsed);

      hex.panels[CL]->clearFunctions();
      hex.panels[CL]->colorSpin(loops, speed);
      break;
    }
    // rainbowTimed(MilliSec, uint8_t)
    case 6: {
      MilliSec duration;
      uint8_t speed;

      fnArgFromSignal(data, duration, bitsUsed, 32);
      fnArgFromSignal(data, speed, bitsUsed);

      hex.panels[CL]->clearFunctions();
      hex.panels[CL]->rainbowTimed(duration, speed);
      break;
    }
    // rainbow(double (as uint16_t), uint8_t);
    case 7: {
      uint16_t loops;
      uint8_t speed;

      fnArgFromSignal(data, loops, bitsUsed);
      fnArgFromSignal(data, speed, bitsUsed);

      hex.panels[CL]->clearFunctions();
      hex.panels[CL]->rainbow(loops, speed);
      break;
    }
    // setColor(LEDColor, MilliSec);
    case 8: {
      LEDColor color;
      MilliSec timeDelay;

      fnArgFromSignal(data, color, bitsUsed, 24);
      fnArgFromSignal(data, timeDelay, bitsUsed, 32);

      hex.panels[CL]->clearFunctions();
      hex.panels[CL]->setColor(color, timeDelay);
      break;
    }
    // setBrightness(uint8_t);
    case 9: {
      uint8_t brightness;

      fnArgFromSignal(data, brightness, bitsUsed);
      hex.panels[CL]->setBrightness(brightness);
      break;
    }
    // collective setColor (LEDColor)
    case 31: {
      bitsUsed -= 3;
      LEDColor color;

      fnArgFromSignal(data, color, bitsUsed, 24);

      for (size_t i = 0; i < 6; i++) {
        if (getBit(data, bitsUsed + i)) {
          hex.panels[i]->clearFunctions();
          hex.panels[i]->setColor(color);
        }
      }
    }
  }
}

void parsePanelSegment(
  const uint8_t* data, uint8_t fnCode, uint8_t CL, uint8_t sl) {
  uint8_t bitsUsed = 12;
  PanelSegment* segment = nullptr;

  if (fnCode != 31) {
    SideLocation SL = static_cast<SideLocation>(sl);
    segment = &hex.panels[CL]->segments[hex.panels[CL]->segSideIndex[SL]];
  }
  hex.panels[CL]->clearFunctions();

  switch (fnCode) {
    // setColor(LEDColor, MilliSec);
    case 0: {
      LEDColor color;
      MilliSec timeDelay;

      fnArgFromSignal(data, color, bitsUsed, 24);
      fnArgFromSignal(data, timeDelay, bitsUsed, 32);

      segment->setColor(color, timeDelay);
      break;
    }
    // collective setColor (LEDColor)
    case 31: {
      bitsUsed -= 2;
      LEDColor color;

      fnArgFromSignal(data, color, bitsUsed, 24);
      for (size_t i = 0; i < 4; i++) {
        if (getBit(data, bitsUsed + i)) {
          SideLocation SL = static_cast<SideLocation>(i);
          segment = &hex.panels[CL]->segments[hex.panels[CL]->segSideIndex[SL]];
          segment->setColor(color);
        }
      }
    }
  }
}

void parseLED(
  const uint8_t* data, uint8_t fnCode, uint8_t CL, uint8_t sl, uint8_t LED) {
  uint8_t bitsUsed = 19;
  SideLocation SL = static_cast<SideLocation>(sl);
  PanelSegment* segment =
    &hex.panels[CL]->segments[hex.panels[CL]->segSideIndex[SL]];
  hex.panels[CL]->clearFunctions();

  switch (fnCode) {
    // setColor(LEDColor, MilliSec);
    case 0: {
      LEDColor color;
      MilliSec timeDelay;

      fnArgFromSignal(data, color, bitsUsed, 24);
      fnArgFromSignal(data, timeDelay, bitsUsed, 32);
      segment->setPixelColor(LED, color, timeDelay);
      break;
    }
    // collective setColor (LEDColor)
    case 31: {
      bitsUsed -= 7;
      LEDColor color;

      fnArgFromSignal(data, color, bitsUsed, 24);
      for (size_t i = 0; i < 28; i++) {
        if (getBit(data, bitsUsed + i)) {
          segment->setPixelColor(i, color);
        }
      }
    }
  }
}

void parseSignal(const uint8_t* data) {
  const uint8_t level = getBits(data, 0, 2);
  const uint8_t fnCode = getBits(data, 2, 5);
  const uint8_t CL = getBits(data, 7, 3);
  const uint8_t SL = getBits(data, 10, 2);
  const uint8_t LED = getBits(data, 12, 7);

  switch (level) {
    case 0: {
      parseHexagon(data, fnCode);
      break;
    }
    case 1: {
      parseTriPanel(data, fnCode, CL);
      break;
    }
    case 2: {
      parsePanelSegment(data, fnCode, CL, SL);
      break;
    }
    case 3: {
      parseLED(data, fnCode, CL, SL, LED);
      break;
    }
  }
}
