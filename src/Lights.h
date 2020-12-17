#ifndef MILO_LIGHTS
#define MILO_LIGHTS

#ifdef ARDUINO
#include "Adafruit_NeoPixel.h"
#include "Arduino.h"
#else
#include "../Tests/Skeletons/Adafruit_NeoPixel.h"
#include "../Tests/Skeletons/Arduino.h"
#endif

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
        startLocation(startLocation){}
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

  PanelSegment(
    SideLocation s, int min, int max, TriPanel& p);
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

  std::list<std::pair<MilliSec, std::function<void()>>> functionSequence;

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

  std::vector<PanelSegment> segments;
  std::map<SideLocation, int> segSideIndex;
  std::list<LED*> delayedLEDs;

  Adafruit_NeoPixel lights;

  TriPanel(int pin, uint16_t numLeds, CornerLocation local, LoopDirection spin,
    CornerLocation startLocation);
  ~TriPanel();

  static double spinSpeed2Duration(uint8_t speed);
  void changeLEDLater(LED* led);

  void breath(
    uint8_t maxBrightness = 50, MilliSec fadeSpeed = 2000, LEDColor color = 0);
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
  void rainbowTimed(MilliSec duration = 5, uint8_t speed = 250);

  void colorSpin(double loops = 5, uint8_t speed = 250);

  void resetPixelColor(uint16_t stripIndex);

  void clearFunctions();

  void begin();
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

  void breath(
    uint8_t maxBrightness = 50, MilliSec fadeSpeed = 5000, LEDColor color = 0);

  void colorShift(MilliSec timeDelay = 0);

  void rainbowTimed(MilliSec duration = 5, uint8_t speed = 250);
  void rainbow(double loops = 5, uint8_t speed = 250);

  void runFunctionLater(std::function<void()> fn, MilliSec timeDelay = 0);
  void clearFunctions();

  void begin();
  void setBrightness(uint8_t b);
  void setColor(LEDColor color, MilliSec timeDelay = 0);
  void show();
};

static const uint32_t rainbowColors[] = {16728129, 16662594, 16597059, 16531524,
  16465989, 16400454, 16334919, 16269384, 16203849, 16138314, 16072779,
  16007244, 15941709, 15876174, 15810639, 15745104, 15679569, 15614034,
  15548499, 15482964, 15417429, 15351894, 15286359, 15220824, 15155289,
  15089754, 15024219, 14958684, 14893149, 14827614, 14762079, 14696544,
  14631009, 14565474, 14499939, 14434404, 14368869, 14303334, 14237799,
  14172264, 14106729, 14041194, 13975659, 13910124, 13844589, 13779054,
  13713519, 13647984, 13582449, 13516914, 13451379, 13385844, 13320309,
  13254774, 13189239, 13123704, 13058169, 12992634, 12927099, 12861564,
  12796029, 12730494, 12664959, 12599424, 12533889, 12468354, 12402819,
  12337284, 12271749, 12206214, 12140679, 12075144, 12009609, 11944074,
  11878539, 11813004, 11747469, 11681934, 11616399, 11550864, 11485329,
  11419794, 11354259, 11288724, 11223189, 11157654, 11092119, 11026584,
  10961049, 10895514, 10829979, 10764444, 10698909, 10633374, 10567839,
  10502304, 10436769, 10371234, 10305699, 10240164, 10174629, 10109094,
  10043559, 9978024, 9912489, 9846954, 9781419, 9715884, 9650349, 9584814,
  9519279, 9453744, 9388209, 9322674, 9257139, 9191604, 9126069, 9060534,
  8994999, 8929464, 8863929, 8798394, 8732859, 8667324, 8601789, 8536254,
  8470719, 8405184, 8339649, 8274114, 8208579, 8143044, 8077509, 8011974,
  7946439, 7880904, 7815369, 7749834, 7684299, 7618764, 7553229, 7487694,
  7422159, 7356624, 7291089, 7225554, 7160019, 7094484, 7028949, 6963414,
  6897879, 6832344, 6766809, 6701274, 6635739, 6570204, 6504669, 6439134,
  6373599, 6308064, 6242529, 6176994, 6111459, 6045924, 5980389, 5914854,
  5849319, 5783784, 5718249, 5652714, 5587179, 5521644, 5456109, 5390574,
  5325039, 5259504, 5193969, 5128434, 5062899, 4997364, 4931829, 4866294,
  4800759, 4735224, 4669689, 4604154, 4538619, 4473084, 4407549, 4342014,
  4276479, 4211199, 4211454, 4211709, 4211964, 4212219, 4212474, 4212729,
  4212984, 4213239, 4213494, 4213749, 4214004, 4214259, 4214514, 4214769,
  4215024, 4215279, 4215534, 4215789, 4216044, 4216299, 4216554, 4216809,
  4217064, 4217319, 4217574, 4217829, 4218084, 4218339, 4218594, 4218849,
  4219104, 4219359, 4219614, 4219869, 4220124, 4220379, 4220634, 4220889,
  4221144, 4221399, 4221654, 4221909, 4222164, 4222419, 4222674, 4222929,
  4223184, 4223439, 4223694, 4223949, 4224204, 4224459, 4224714, 4224969,
  4225224, 4225479, 4225734, 4225989, 4226244, 4226499, 4226754, 4227009,
  4227264, 4227519, 4227774, 4228029, 4228284, 4228539, 4228794, 4229049,
  4229304, 4229559, 4229814, 4230069, 4230324, 4230579, 4230834, 4231089,
  4231344, 4231599, 4231854, 4232109, 4232364, 4232619, 4232874, 4233129,
  4233384, 4233639, 4233894, 4234149, 4234404, 4234659, 4234914, 4235169,
  4235424, 4235679, 4235934, 4236189, 4236444, 4236699, 4236954, 4237209,
  4237464, 4237719, 4237974, 4238229, 4238484, 4238739, 4238994, 4239249,
  4239504, 4239759, 4240014, 4240269, 4240524, 4240779, 4241034, 4241289,
  4241544, 4241799, 4242054, 4242309, 4242564, 4242819, 4243074, 4243329,
  4243584, 4243839, 4244094, 4244349, 4244604, 4244859, 4245114, 4245369,
  4245624, 4245879, 4246134, 4246389, 4246644, 4246899, 4247154, 4247409,
  4247664, 4247919, 4248174, 4248429, 4248684, 4248939, 4249194, 4249449,
  4249704, 4249959, 4250214, 4250469, 4250724, 4250979, 4251234, 4251489,
  4251744, 4251999, 4252254, 4252509, 4252764, 4253019, 4253274, 4253529,
  4253784, 4254039, 4254294, 4254549, 4254804, 4255059, 4255314, 4255569,
  4255824, 4256079, 4256334, 4256589, 4256844, 4257099, 4257354, 4257609,
  4257864, 4258119, 4258374, 4258629, 4258884, 4259139, 4259394, 4259649,
  4325184, 4390464, 4455744, 4521024, 4586304, 4651584, 4716864, 4782144,
  4847424, 4912704, 4977984, 5043264, 5108544, 5173824, 5239104, 5304384,
  5369664, 5434944, 5500224, 5565504, 5630784, 5696064, 5761344, 5826624,
  5891904, 5957184, 6022464, 6087744, 6153024, 6218304, 6283584, 6348864,
  6414144, 6479424, 6544704, 6609984, 6675264, 6740544, 6805824, 6871104,
  6936384, 7001664, 7066944, 7132224, 7197504, 7262784, 7328064, 7393344,
  7458624, 7523904, 7589184, 7654464, 7719744, 7785024, 7850304, 7915584,
  7980864, 8046144, 8111424, 8176704, 8241984, 8307264, 8372544, 8437824,
  8503104, 8568384, 8633664, 8698944, 8764224, 8829504, 8894784, 8960064,
  9025344, 9090624, 9155904, 9221184, 9286464, 9351744, 9417024, 9482304,
  9547584, 9612864, 9678144, 9743424, 9808704, 9873984, 9939264, 10004544,
  10069824, 10135104, 10200384, 10265664, 10330944, 10396224, 10461504,
  10526784, 10592064, 10657344, 10722624, 10787904, 10853184, 10918464,
  10983744, 11049024, 11114304, 11179584, 11244864, 11310144, 11375424,
  11440704, 11505984, 11571264, 11636544, 11701824, 11767104, 11832384,
  11897664, 11962944, 12028224, 12093504, 12158784, 12224064, 12289344,
  12354624, 12419904, 12485184, 12550464, 12615744, 12681024, 12746304,
  12811584, 12876864, 12942144, 13007424, 13072704, 13137984, 13203264,
  13268544, 13333824, 13399104, 13464384, 13529664, 13594944, 13660224,
  13725504, 13790784, 13856064, 13921344, 13986624, 14051904, 14117184,
  14182464, 14247744, 14313024, 14378304, 14443584, 14508864, 14574144,
  14639424, 14704704, 14769984, 14835264, 14900544, 14965824, 15031104,
  15096384, 15161664, 15226944, 15292224, 15357504, 15422784, 15488064,
  15553344, 15618624, 15683904, 15749184, 15814464, 15879744, 15945024,
  16010304, 16075584, 16140864, 16206144, 16271424, 16336704, 16401984,
  16467264, 16532544, 16597824, 16663104, 16728384};

#endif
