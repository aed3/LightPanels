#ifndef MILO_LIGHTS
#define MILO_LIGHTS

#include "Adafruit_NeoPixel.h"
#include "Arduino.h"

#include <stdint.h>

#include <functional>
#include <list>
#include <map>
#include <vector>

enum SideLocation { SL_TOP = 0, SL_RIGHT = 1, SL_BOTTOM = 2, SL_LEFT = 3 };

/*
  M = Middle
  L = Left
  R = Right

  T = Top
  B = Bottom
*/
enum CornerLocation { CL_LT, CL_MT, CL_RT, CL_RB, CL_MB, CL_LB };

enum LoopDirection { CW, CCW };

typedef uint32_t LEDColor;
typedef unsigned long MilliSec;

struct TriPanelData {
  const int pin;
  const uint16_t numLeds;
  const CornerLocation local;
  const LoopDirection spin;
  const CornerLocation startLocation;

  TriPanelData(int pin, uint16_t numLeds, CornerLocation local,
    LoopDirection spin, CornerLocation startLocation)
      : pin(pin),
        numLeds(numLeds),
        local(local),
        spin(spin),
        startLocation(startLocation) {}
};

extern MilliSec currentTime;

class TriPanel;
class PanelSegment;
class LED;

class LED {
 private:
  const uint16_t stripIndex;
  PanelSegment& mySide;
  TriPanel& myPanel;

 public:
  bool nextColorAvailable;
  LEDColor currentColor;
  LEDColor nextColor;
  MilliSec nextColorChangeTime;

  static const LEDColor Color(int r, int g, int b);

  LED(uint16_t index, PanelSegment& ps, TriPanel& p);
  ~LED();

  void resetColor();

  void setColor(LEDColor color, MilliSec timeDelay = 0);
};

class PanelSegment {
 private:
  const int numLeds;

  std::vector<LED> leds;

  TriPanel& myPanel;

  SideLocation cornersOtherSide(CornerLocation corner);

  template <class Function>
  void forEachLed(Function fn);

 public:
  const int minPixelIndex;
  const int maxPixelIndex;
  const SideLocation side;

  SideLocation nextSegLocation;
  SideLocation prevSegLocation;

  PanelSegment(SideLocation s, int min, int max, TriPanel& p);
  ~PanelSegment();

  void resetPixelColor(uint16_t stripIndex);

  bool fill(double percent, LEDColor color, CornerLocation startLocation,
    MilliSec duration = 0);

  LEDColor getPixelColor(uint16_t stripIndex);
  void setPixelColor(
    uint16_t stripIndex, LEDColor color, MilliSec timeDelay = 0);
  void setColor(LEDColor color, MilliSec timeDelay = 0);
};

class TriPanel {
 private:
  const LoopDirection lightDirection;
  const CornerLocation overallLocation;
  const CornerLocation stripStartLoctation;

  const SideLocation calcFirstSegmentLocation();
  const SideLocation nextSegmentLocation(SideLocation currentSeg);
  const SideLocation nextSegmentLocation(
    SideLocation currentSeg, LoopDirection direction);
  const SideLocation OppositeSideOfCorner(CornerLocation corner);

  const std::pair<SideLocation, SideLocation> corner2Side(
    CornerLocation corner);

  void fadeController(uint8_t brightness, MilliSec& timePassed,
    MilliSec timeBetweenChange, bool constantColor);
  void runFunctionLater(std::function<void()> fn, MilliSec timeDelay = 0);

  void playFunctionSequence();
  void showDelayedLEDs();

  template <class Function>
  void forEachSegment(Function fn);

 public:
  bool LEDchanged;
  CornerLocation cornerAtCenter;
  SideLocation outerSide;

  std::list<std::pair<MilliSec, std::function<void()>>> functionSequence;
  std::vector<PanelSegment> segments;
  std::map<SideLocation, int> segSideIndex;
  std::list<LED*> delayedLEDs;

  Adafruit_NeoPixel lights;

  TriPanel(int pin, uint16_t numLeds, CornerLocation local, LoopDirection spin,
    CornerLocation startLocation);
  ~TriPanel();

  static double spinSpeed2Duration(uint8_t speed);
  void changeLEDLater(LED* led);

  void breathe(uint8_t maxBrightness = 50, MilliSec fadeDuration = 5000,
    LEDColor color = 0, bool cc = false);
  void fadeIn(uint8_t maxBrightness = 50, bool constantColor = false,
    MilliSec duration = 1000);
  void fadeOut(uint8_t minBrightness = 0, bool constantColor = false,
    MilliSec duration = 1000);

  void fillFromCorner(double percent, LEDColor color, MilliSec duration = 0);
  void fillFromCorner(double percent, LEDColor color,
    CornerLocation startLocation, MilliSec duration = 0);

  void fillToCorner(double percent, LEDColor color, MilliSec duration = 0);
  void fillToCorner(double percent, LEDColor color,
    CornerLocation startLocation, MilliSec duration = 0);

  void rainbow(double loops = 5, uint8_t speed = 250);
  void rainbowTimed(MilliSec duration = 5000, uint8_t speed = 250);

  void colorSpin(double loops = 5, uint8_t speed = 250);

  void resetPixelColor(uint16_t stripIndex);

  void clearFunctions();

  void begin(uint8_t brightness = 50);
  uint8_t getBrightness();
  void setBrightness(uint8_t b);
  void setColor(LEDColor color, MilliSec timeDelay = 0);
  void setColor(std::vector<LEDColor> colors, MilliSec timeDelay = 0);
  std::vector<LEDColor> getColor();
  LEDColor getPixelColor(uint16_t stripIndex);
  void setPixelColor(
    uint16_t stripIndex, LEDColor color, MilliSec timeDelay = 0);
  void show();
};

class Hexagon {
 private:
  std::list<std::pair<MilliSec, std::function<void()>>> functionSequence;

  void playFunctionSequence();

  template <class Function>
  void forEachPanel(Function fn);

 public:
  std::vector<TriPanel*> panels;

  Hexagon();
  Hexagon(TriPanelData panelData[]);
  ~Hexagon();

  void breathe(uint8_t maxBrightness = 50, MilliSec fadeDuration = 5000,
    LEDColor color = 0);

  void colorShift(MilliSec timeDelay = 0, uint16_t shifts = 1);

  void rainbowTimed(MilliSec duration = 5000, uint8_t speed = 250);
  void rainbow(double loops = 5, uint8_t speed = 250);

  void runFunctionLater(std::function<void()> fn, MilliSec timeDelay = 0);
  void clearFunctions();

  void begin(uint8_t brightness = 50);
  void setBrightness(uint8_t b);
  void setColor(LEDColor color, MilliSec timeDelay = 0);
  void show();
};

static const uint32_t PROGMEM rainbowColors[512] = {0xFF0000, 0xFF0000,
  0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000, 0xFF0100,
  0xFF0100, 0xFF0100, 0xFF0100, 0xFF0200, 0xFF0200, 0xFF0200, 0xFF0300,
  0xFF0300, 0xFF0400, 0xFF0500, 0xFF0500, 0xFF0600, 0xFF0700, 0xFF0800,
  0xFF0900, 0xFF0A00, 0xFF0B00, 0xFF0C00, 0xFF0D00, 0xFF0E00, 0xFF1000,
  0xFF1100, 0xFF1300, 0xFF1400, 0xFF1600, 0xFF1800, 0xFF1900, 0xFF1B00,
  0xFF1D00, 0xFF1F00, 0xFF2200, 0xFF2400, 0xFF2600, 0xFF2900, 0xFF2A00,
  0xFF2D00, 0xFF3000, 0xFF3300, 0xFF3600, 0xFF3900, 0xFF3C00, 0xFF3F00,
  0xFF4200, 0xFF4600, 0xFF4900, 0xFF4D00, 0xFF5100, 0xFF5500, 0xFF5900,
  0xFF5D00, 0xFF6100, 0xFF6600, 0xFF6A00, 0xFF6F00, 0xFF7300, 0xFF7800,
  0xFF7D00, 0xFF8200, 0xFF8800, 0xFF8D00, 0xFF9200, 0xFF9800, 0xFF9E00,
  0xFFA400, 0xFFAA00, 0xFFB000, 0xFFB600, 0xFFBC00, 0xFFC300, 0xFFCA00,
  0xFFD100, 0xFFD700, 0xFFDF00, 0xFFE600, 0xFFED00, 0xFFF500, 0xFFFC00,
  0xFAFF00, 0xF2FF00, 0xEBFF00, 0xE3FF00, 0xDCFF00, 0xD5FF00, 0xCEFF00,
  0xC7FF00, 0xC1FF00, 0xBAFF00, 0xB4FF00, 0xAEFF00, 0xA8FF00, 0xA2FF00,
  0x9CFF00, 0x96FF00, 0x91FF00, 0x8BFF00, 0x86FF00, 0x81FF00, 0x7CFF00,
  0x77FF00, 0x72FF00, 0x6DFF00, 0x69FF00, 0x64FF00, 0x60FF00, 0x5CFF00,
  0x58FF00, 0x54FF00, 0x50FF00, 0x4CFF00, 0x48FF00, 0x45FF00, 0x41FF00,
  0x3EFF00, 0x3BFF00, 0x38FF00, 0x35FF00, 0x32FF00, 0x2FFF00, 0x2CFF00,
  0x2AFF00, 0x28FF00, 0x26FF00, 0x23FF00, 0x21FF00, 0x1FFF00, 0x1DFF00,
  0x1BFF00, 0x19FF00, 0x17FF00, 0x15FF00, 0x14FF00, 0x12FF00, 0x11FF00,
  0x0FFF00, 0x0EFF00, 0x0DFF00, 0x0BFF00, 0x0AFF00, 0x09FF00, 0x08FF00,
  0x07FF00, 0x06FF00, 0x06FF00, 0x05FF00, 0x04FF00, 0x04FF00, 0x03FF00,
  0x03FF00, 0x02FF00, 0x02FF00, 0x01FF00, 0x01FF00, 0x01FF00, 0x01FF00,
  0x00FF00, 0x00FF00, 0x00FF00, 0x00FF00, 0x00FF00, 0x00FF00, 0x00FF00,
  0x00FF00, 0x00FF00, 0x00FF00, 0x00FF00, 0x00FF00, 0x00FF00, 0x00FF00,
  0x00FF00, 0x00FF00, 0x00FF01, 0x00FF01, 0x00FF01, 0x00FF01, 0x00FF02,
  0x00FF02, 0x00FF02, 0x00FF03, 0x00FF03, 0x00FF04, 0x00FF05, 0x00FF05,
  0x00FF06, 0x00FF07, 0x00FF08, 0x00FF09, 0x00FF0A, 0x00FF0B, 0x00FF0C,
  0x00FF0D, 0x00FF0F, 0x00FF10, 0x00FF12, 0x00FF13, 0x00FF15, 0x00FF16,
  0x00FF18, 0x00FF1A, 0x00FF1C, 0x00FF1E, 0x00FF20, 0x00FF22, 0x00FF25,
  0x00FF27, 0x00FF2A, 0x00FF2B, 0x00FF2E, 0x00FF31, 0x00FF34, 0x00FF37,
  0x00FF3A, 0x00FF3D, 0x00FF40, 0x00FF44, 0x00FF47, 0x00FF4B, 0x00FF4E,
  0x00FF52, 0x00FF56, 0x00FF5A, 0x00FF5E, 0x00FF63, 0x00FF67, 0x00FF6C,
  0x00FF70, 0x00FF75, 0x00FF7A, 0x00FF7F, 0x00FF84, 0x00FF89, 0x00FF8F,
  0x00FF94, 0x00FF9A, 0x00FFA0, 0x00FFA6, 0x00FFAC, 0x00FFB2, 0x00FFB8,
  0x00FFBF, 0x00FFC5, 0x00FFCC, 0x00FFD3, 0x00FFDA, 0x00FFE1, 0x00FFE8,
  0x00FFF0, 0x00FFF7, 0x00FFFF, 0x00F7FF, 0x00F0FF, 0x00E8FF, 0x00E1FF,
  0x00DAFF, 0x00D3FF, 0x00CCFF, 0x00C5FF, 0x00BFFF, 0x00B8FF, 0x00B2FF,
  0x00ACFF, 0x00A6FF, 0x00A0FF, 0x009AFF, 0x0094FF, 0x008FFF, 0x0089FF,
  0x0084FF, 0x007FFF, 0x007AFF, 0x0075FF, 0x0070FF, 0x006CFF, 0x0067FF,
  0x0063FF, 0x005EFF, 0x005AFF, 0x0056FF, 0x0052FF, 0x004EFF, 0x004BFF,
  0x0047FF, 0x0044FF, 0x0040FF, 0x003DFF, 0x003AFF, 0x0037FF, 0x0034FF,
  0x0031FF, 0x002EFF, 0x002BFF, 0x002AFF, 0x0027FF, 0x0025FF, 0x0022FF,
  0x0020FF, 0x001EFF, 0x001CFF, 0x001AFF, 0x0018FF, 0x0016FF, 0x0015FF,
  0x0013FF, 0x0012FF, 0x0010FF, 0x000FFF, 0x000DFF, 0x000CFF, 0x000BFF,
  0x000AFF, 0x0009FF, 0x0008FF, 0x0007FF, 0x0006FF, 0x0005FF, 0x0005FF,
  0x0004FF, 0x0003FF, 0x0003FF, 0x0002FF, 0x0002FF, 0x0002FF, 0x0001FF,
  0x0001FF, 0x0001FF, 0x0001FF, 0x0000FF, 0x0000FF, 0x0000FF, 0x0000FF,
  0x0000FF, 0x0000FF, 0x0000FF, 0x0000FF, 0x0000FF, 0x0000FF, 0x0000FF,
  0x0000FF, 0x0000FF, 0x0000FF, 0x0000FF, 0x0000FF, 0x0100FF, 0x0100FF,
  0x0100FF, 0x0100FF, 0x0200FF, 0x0200FF, 0x0300FF, 0x0300FF, 0x0400FF,
  0x0400FF, 0x0500FF, 0x0600FF, 0x0600FF, 0x0700FF, 0x0800FF, 0x0900FF,
  0x0A00FF, 0x0B00FF, 0x0D00FF, 0x0E00FF, 0x0F00FF, 0x1100FF, 0x1200FF,
  0x1400FF, 0x1500FF, 0x1700FF, 0x1900FF, 0x1B00FF, 0x1D00FF, 0x1F00FF,
  0x2100FF, 0x2300FF, 0x2600FF, 0x2800FF, 0x2A00FF, 0x2C00FF, 0x2F00FF,
  0x3200FF, 0x3500FF, 0x3800FF, 0x3B00FF, 0x3E00FF, 0x4100FF, 0x4500FF,
  0x4800FF, 0x4C00FF, 0x5000FF, 0x5400FF, 0x5800FF, 0x5C00FF, 0x6000FF,
  0x6400FF, 0x6900FF, 0x6D00FF, 0x7200FF, 0x7700FF, 0x7C00FF, 0x8100FF,
  0x8600FF, 0x8B00FF, 0x9100FF, 0x9600FF, 0x9C00FF, 0xA200FF, 0xA800FF,
  0xAE00FF, 0xB400FF, 0xBA00FF, 0xC100FF, 0xC700FF, 0xCE00FF, 0xD500FF,
  0xDC00FF, 0xE300FF, 0xEB00FF, 0xF200FF, 0xFA00FF, 0xFF00FC, 0xFF00F5,
  0xFF00ED, 0xFF00E6, 0xFF00DF, 0xFF00D7, 0xFF00D1, 0xFF00CA, 0xFF00C3,
  0xFF00BC, 0xFF00B6, 0xFF00B0, 0xFF00AA, 0xFF00A4, 0xFF009E, 0xFF0098,
  0xFF0092, 0xFF008D, 0xFF0088, 0xFF0082, 0xFF007D, 0xFF0078, 0xFF0073,
  0xFF006F, 0xFF006A, 0xFF0066, 0xFF0061, 0xFF005D, 0xFF0059, 0xFF0055,
  0xFF0051, 0xFF004D, 0xFF0049, 0xFF0046, 0xFF0042, 0xFF003F, 0xFF003C,
  0xFF0039, 0xFF0036, 0xFF0033, 0xFF0030, 0xFF002D, 0xFF002A, 0xFF0029,
  0xFF0026, 0xFF0024, 0xFF0022, 0xFF001F, 0xFF001D, 0xFF001B, 0xFF0019,
  0xFF0018, 0xFF0016, 0xFF0014, 0xFF0013, 0xFF0011, 0xFF0010, 0xFF000E,
  0xFF000D, 0xFF000C, 0xFF000B, 0xFF000A, 0xFF0009, 0xFF0008, 0xFF0007,
  0xFF0006, 0xFF0005, 0xFF0005, 0xFF0004, 0xFF0003, 0xFF0003, 0xFF0002,
  0xFF0002, 0xFF0002, 0xFF0001, 0xFF0001, 0xFF0001, 0xFF0001, 0xFF0000,
  0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000};

#endif
