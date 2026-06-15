#include <FastLED.h>
#include <esp_now.h>
#include <WiFi.h>

// --- GLOBALS ---
volatile uint8_t MODE = 0; // 0 = Stop, 1 = Run

// --- CALLBACK ---
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  uint8_t incoming;
  memcpy(&incoming, data, sizeof(incoming));
  
  if (incoming == 0) {
    if (MODE != 0) {
      MODE = 0;
      Serial.println("CMD: STOP (0)");
    }
  } 
  else if (incoming == 1) {
    if (MODE == 0) {
      MODE = 1; 
      Serial.println("CMD: START (1)");
    } 
  }
}

// --- SAFE DELAY ---
bool safeDelay(uint32_t duration) {
  uint32_t start = millis();
  while (millis() - start < duration) {
    if (MODE == 0) return true; 
    yield(); 
  }
  return false; 
}

constexpr uint8_t BRIGHTNESS = 32;
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

// --- PINS (Suit 4) ---
#define SHIRT_LEFT_PIN 16
#define SHIRT_RIGHT_PIN 17
#define PANT_RIGHT_PIN 19
#define PANT_LEFT_PIN 18
#define HAND_RIGHT_PIN 26
#define HAND_LEFT_PIN 25
#define HAT_PIN 27        // Hat Pin 27

// --- LED COUNTS (Suit 4) ---
#define SHIRT_LEFT_LEDS 67
#define SHIRT_RIGHT_LEDS 66
#define PANT_LEFT_LEDS 65
#define PANT_RIGHT_LEDS 67
#define HAND_RIGHT_LEDS 66
#define HAND_LEFT_LEDS 58
#define HAT_LEDS 30

template<uint8_t PIN>
class LEDStrip {
public:
  const uint16_t numLEDs;
  CRGB *leds;
  CLEDController *controller;

  LEDStrip(uint16_t numLEDs) : numLEDs(numLEDs) {
    leds = new CRGB[numLEDs];
  }
  ~LEDStrip() { delete[] leds; }
  
  void begin() { controller = &FastLED.addLeds<LED_TYPE, PIN, COLOR_ORDER>(leds, numLEDs); }
  void show() { controller->showLeds(BRIGHTNESS); }
  bool checkValidity(uint16_t f, uint16_t t) { return !(f==0||t==0||f>numLEDs||t>numLEDs||f==t); }
  void setLed(uint16_t LED, CRGB colour) { if (LED > 0 && LED <= numLEDs) leds[LED - 1] = colour; }
  
  bool clear(uint16_t from, uint16_t to, uint32_t time = 0) {
    if (!checkValidity(from, to)) return false;
    from--; to--; int8_t s = (to > from) ? 1 : -1;
    for (int i = from; i != to; i += s) { leds[i] = CRGB::Black; if (time) { show(); if (safeDelay(time/abs(to-from))) return true; } }
    leds[to] = CRGB::Black; show(); return false;
  }
  
  bool setStatic(uint16_t from, uint16_t to, CRGB colour, uint32_t time = 0) {
    if (!checkValidity(from, to)) return false;
    from--; to--; int8_t s = (to > from) ? 1 : -1;
    for (int i = from; i != to; i += s) { leds[i] = colour; if (time) { show(); if (safeDelay(time/abs(to-from))) return true; } }
    leds[to] = colour; show(); return false;
  }
};

// --- STRIPS ---
LEDStrip<SHIRT_LEFT_PIN> shirt_left(SHIRT_LEFT_LEDS);
LEDStrip<SHIRT_RIGHT_PIN> shirt_right(SHIRT_RIGHT_LEDS);
LEDStrip<PANT_LEFT_PIN> pant_left(PANT_LEFT_LEDS);
LEDStrip<PANT_RIGHT_PIN> pant_right(PANT_RIGHT_LEDS);
LEDStrip<HAND_LEFT_PIN> hand_left(HAND_LEFT_LEDS);
LEDStrip<HAND_RIGHT_PIN> hand_right(HAND_RIGHT_LEDS);
LEDStrip<HAT_PIN> hat(HAT_LEDS);

void setup() {
  shirt_left.begin(); shirt_right.begin(); pant_left.begin(); pant_right.begin();
  hand_right.begin(); hand_left.begin(); hat.begin();
  
  FastLED.setBrightness(BRIGHTNESS);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  
  // Startup Check
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    while(true){ set_static_all(CRGB::Red); delay(100); clear_all(); delay(100); }
  } 
  
  esp_now_register_recv_cb(OnDataRecv);
  
  MODE = 0;
  clear_all();
  
  // Flash Green on startup
  for(int i=0; i<3; i++) { set_static_all(CRGB::Green); delay(100); clear_all(); delay(100); }
  Serial.println("Suit 4 Ready.");
}

void set_static_all(CRGB c) {
  shirt_left.setStatic(1,SHIRT_LEFT_LEDS,c); shirt_right.setStatic(1,SHIRT_RIGHT_LEDS,c);
  pant_left.setStatic(1,PANT_LEFT_LEDS,c); pant_right.setStatic(1,PANT_RIGHT_LEDS,c);
  hand_right.setStatic(1,HAND_RIGHT_LEDS,c); hand_left.setStatic(1,HAND_LEFT_LEDS,c);
  hat.setStatic(1,HAT_LEDS,c);
}

void clear_all() {
  shirt_left.clear(1,SHIRT_LEFT_LEDS); shirt_right.clear(1,SHIRT_RIGHT_LEDS);
  pant_left.clear(1,PANT_LEFT_LEDS); pant_right.clear(1,PANT_RIGHT_LEDS);
  hand_right.clear(1,HAND_RIGHT_LEDS); hand_left.clear(1,HAND_LEFT_LEDS);
  hat.clear(1,HAT_LEDS);
}

void set_static_hand(CRGB c){ hand_left.setStatic(1,HAND_LEFT_LEDS,c); hand_right.setStatic(1,HAND_RIGHT_LEDS,c); }
void clear_hand(){ hand_left.clear(1,HAND_LEFT_LEDS); hand_right.clear(1,HAND_RIGHT_LEDS); }

// --- ANIMATION FUNCTIONS ---

bool wipeTopDown(CRGB c, uint32_t duration) {
  int u=max(SHIRT_LEFT_LEDS,SHIRT_RIGHT_LEDS), l=max(PANT_LEFT_LEDS,PANT_RIGHT_LEDS), h=HAT_LEDS;
  int s=duration/(u+l+h);
  for(int i=1; i<=h; i++) { hat.setLed(i,c); hat.show(); if(safeDelay(s)) return true; }
  for(int i=1; i<=u; i++){
    shirt_left.setLed(i,c); shirt_right.setLed(i,c);
    hand_left.setLed(i,c); hand_right.setLed(i,c);
    shirt_left.show(); shirt_right.show(); hand_left.show(); hand_right.show();
    if(safeDelay(s)) return true;
  }
  for(int i=1; i<=l; i++){
    pant_left.setLed(i,c); pant_right.setLed(i,c);
    pant_left.show(); pant_right.show();
    if(safeDelay(s)) return true;
  }
  return false;
}

bool wipeBottomUp(CRGB c, uint32_t duration) {
  int u=max(SHIRT_LEFT_LEDS,SHIRT_RIGHT_LEDS), l=max(PANT_LEFT_LEDS,PANT_RIGHT_LEDS), h=HAT_LEDS;
  int s=duration/(u+l+h);
  for(int i=l; i>=1; i--){
    pant_left.setLed(i,c); pant_right.setLed(i,c);
    pant_left.show(); pant_right.show();
    if(safeDelay(s)) return true;
  }
  for(int i=u; i>=1; i--){
    shirt_left.setLed(i,c); shirt_right.setLed(i,c);
    hand_left.setLed(i,c); hand_right.setLed(i,c);
    shirt_left.show(); shirt_right.show(); hand_left.show(); hand_right.show();
    if(safeDelay(s)) return true;
  }
  for(int i=h; i>=1; i--){ hat.setLed(i,c); hat.show(); if(safeDelay(s)) return true; }
  return false;
}

bool wipeBottomUpTorso(CRGB cPants, CRGB cShirts, CRGB cHat, uint32_t duration) {
  int p=max(PANT_LEFT_LEDS,PANT_RIGHT_LEDS), s=max(SHIRT_LEFT_LEDS,SHIRT_RIGHT_LEDS), h=HAT_LEDS;
  int st=duration/(p+s+h);
  for(int i=p; i>=1; i--){
    pant_left.setLed(i,cPants); pant_right.setLed(i,cPants);
    pant_left.show(); pant_right.show(); if(safeDelay(st)) return true;
  }
  for(int i=s; i>=1; i--){
    shirt_left.setLed(i,cShirts); shirt_right.setLed(i,cShirts);
    shirt_left.show(); shirt_right.show(); if(safeDelay(st)) return true;
  }
  for(int i=h; i>=1; i--){ hat.setLed(i,cHat); hat.show(); if(safeDelay(st)) return true; }
  return false;
}

bool wipeRightToLeft(CRGB c, uint32_t duration) {
  int r=max(SHIRT_RIGHT_LEDS,max(PANT_RIGHT_LEDS,HAND_RIGHT_LEDS));
  int s=duration/2/r;
  for(int i=r; i>=1; i--){
    shirt_right.setLed(i,c); pant_right.setLed(i,c); hand_right.setLed(i,c);
    shirt_right.show(); pant_right.show(); hand_right.show();
    if(safeDelay(s)) return true;
  }
  hat.setStatic(1, HAT_LEDS, c); 
  int l=max(SHIRT_LEFT_LEDS,max(PANT_LEFT_LEDS,HAND_LEFT_LEDS));
  s=duration/2/l;
  for(int i=1; i<=l; i++){
    shirt_left.setLed(i,c); pant_left.setLed(i,c); hand_left.setLed(i,c);
    shirt_left.show(); pant_left.show(); hand_left.show();
    if(safeDelay(s)) return true;
  }
  return false;
}

// --- UPDATED RAINBOW FALL (FASTER) ---
bool rainbow_fall(uint8_t speed = 10, uint8_t hueStep = 5) {
  static uint8_t baseHue = 0; 
  uint16_t offset = 0;
  
  for (uint16_t i = 0; i < SHIRT_LEFT_LEDS; i++) shirt_left.leds[i] = CHSV(baseHue + (offset + i) * hueStep, 255, 255);
  offset += SHIRT_LEFT_LEDS; 
  for (uint16_t i = 0; i < PANT_LEFT_LEDS; i++) pant_left.leds[i] = CHSV(baseHue + (offset + i) * hueStep, 255, 255);
  for (uint16_t i = 0; i < HAND_LEFT_LEDS; i++) hand_left.leds[i] = CHSV(baseHue + (offset + i) * hueStep, 255, 255);
  
  offset = 0; 
  for (uint16_t i = 0; i < SHIRT_RIGHT_LEDS; i++) shirt_right.leds[i] = CHSV(baseHue + (offset + i) * hueStep, 255, 255);
  offset += SHIRT_RIGHT_LEDS; 
  for (uint16_t i = 0; i < PANT_RIGHT_LEDS; i++) pant_right.leds[i] = CHSV(baseHue + (offset + i) * hueStep, 255, 255);
  for (uint16_t i = 0; i < HAND_RIGHT_LEDS; i++) hand_right.leds[i] = CHSV(baseHue + (offset + i) * hueStep, 255, 255);
  
  for (uint16_t i = 0; i < HAT_LEDS; i++) hat.leds[i] = CHSV(baseHue + i * hueStep, 255, 255);
  
  shirt_left.show(); shirt_right.show(); pant_left.show(); pant_right.show(); hand_left.show(); hand_right.show(); hat.show();
  
  baseHue += 4; 
  return safeDelay(speed);
}

bool Random(int duration, int step){
  int t=0;
  while(t<duration){
    uint8_t n=random(1,8);
    switch(n){
      case 1: shirt_left.setStatic(1,SHIRT_LEFT_LEDS,CHSV(random8(),255,255)); break;
      case 2: shirt_right.setStatic(1,SHIRT_RIGHT_LEDS,CHSV(random8(),255,255)); break;
      case 3: pant_left.setStatic(1,PANT_LEFT_LEDS,CHSV(random8(),255,255)); break;
      case 4: pant_right.setStatic(1,PANT_RIGHT_LEDS,CHSV(random8(),255,255)); break;
      case 5: hand_left.setStatic(1,HAND_LEFT_LEDS,CHSV(random8(),255,255)); break;
      case 6: hand_right.setStatic(1,HAND_RIGHT_LEDS,CHSV(random8(),255,255)); break;
      case 7: hat.setStatic(1,HAT_LEDS,CHSV(random8(),255,255)); break;
    }
    t+=step; if(safeDelay(step)) return true; clear_all();
  }
  return false;
}

// --- SUIT 4 CHOREOGRAPHY ---
void runChoreography() {
  Serial.println("Starting Suit 4 Final...");

  // 0:00 - Start Off
  clear_all();
  if (safeDelay(11500)) return; 

  // 0:11.5 - Hands Only On [Dark Blue]
  set_static_hand(CRGB::DarkBlue);
  if (safeDelay(2000)) return; // Hold until 0:13.5

  // 0:13.5 - Hands Off
  clear_hand();
  if (safeDelay(1830)) return; // Wait until 0:15.33

  // 0:15.33 - Left Hand On [Dark Blue]
  hand_left.setStatic(1, HAND_LEFT_LEDS, CRGB::DarkBlue);
  if (safeDelay(1000)) return; // Wait until 0:16.33

  // 0:16.33 - Right Hand On [Dark Blue]
  hand_right.setStatic(1, HAND_RIGHT_LEDS, CRGB::DarkBlue);
  if (safeDelay(1670)) return; // Wait until 0:18

  // 0:18 - Hands Off
  clear_hand();
  if (safeDelay(1000)) return; 

  // 0:19 - All On [Cyan]
  set_static_all(CRGB::Cyan);
  if (safeDelay(8000)) return; // Until 0:27

  // 0:27 - Off
  clear_all();
  if (safeDelay(13000)) return; // Until 0:40

  // 0:40 - MULTICOLOUR [Rainbow Fall]
  // PDF: 0:40 to 0:42 is multicolor? No, row starts 0:27/0:40...
  // Corrected to use Rainbow Fall from 0:40 to 0:42
  uint32_t rbTime = 0;
  while(rbTime < 2000) { if (rainbow_fall(10, 5)) return; rbTime += 10; }

  // 0:42 to 0:47 - RANDOM
  if (Random(5000, 250)) return;

  // 0:47 - All Off
  clear_all();
  if (safeDelay(1000)) return;

  // 0:48 - All On [White]
  set_static_all(CRGB::White);
  if (safeDelay(12000)) return; // Until 1:00

  // 1:00 - All Off (Long Gap for Suit 4)
  clear_all();
  if (safeDelay(15000)) return; // Wait until 1:15

  // 1:15 - Hat On [Purple]
  hat.setStatic(1, HAT_LEDS, CRGB::Purple);
  // Holds until 1:28
  if (safeDelay(13000)) return;

  // 1:28 - Hat Off
  hat.clear(1, HAT_LEDS);
  // Wait until 1:32
  if (safeDelay(4000)) return;

  // 1:32 - All on top to bottom [Purple/DkGreen]
  if (wipeTopDown(CRGB::Purple, 2000)) return;
  // Overwrite jacket/hand with Green instantly to match spec
  shirt_left.setStatic(1, SHIRT_LEFT_LEDS, CRGB::DarkGreen);
  shirt_right.setStatic(1, SHIRT_RIGHT_LEDS, CRGB::DarkGreen);
  set_static_hand(CRGB::DarkGreen);
  // Hold until 1:44
  if (safeDelay(10000)) return; 

  // 1:44 - All off Fade right to left
  if (wipeRightToLeft(CRGB::Black, 2000)) return; 
  if (safeDelay(1000)) return; // Wait until 1:47

  // 1:47 - MULTICOLOUR [Rainbow Fall]
  // PDF: 1:47 - 1:50
  rbTime = 0;
  while(rbTime < 3000) { if (rainbow_fall(10, 5)) return; rbTime += 10; }
  
  clear_all();
  if (safeDelay(5000)) return; // Wait until 1:55

  // 1:55 - All on [White] fade out from bottom
  set_static_all(CRGB::White);
  if (safeDelay(1000)) return; 
  if (wipeBottomUp(CRGB::Black, 1000)) return; 
  if (safeDelay(1000)) return;

  // 1:58 - Fade in Torso [Blue Hat, Green Rest]
  if (wipeBottomUpTorso(CRGB::Green, CRGB::Green, CRGB::Blue, 3000)) return; 
  if (safeDelay(4000)) return; 

  // 2:05 - On/Off [Pink]
  set_static_all(CRGB::DeepPink);
  if (safeDelay(1000)) return; 
  clear_all(); 
  if (safeDelay(1000)) return;

  // 2:07 - Random
  if (Random(1000, 250)) return; 
  clear_all();
  
  // 2:08 - On/Off [Pink]
  set_static_all(CRGB::DeepPink);
  if (safeDelay(1000)) return; 
  clear_all(); 
  if (safeDelay(1000)) return;

  // 2:10 - Random
  if (Random(1000, 250)) return; 
  clear_all();

  // 2:11 - Wait (Gap)
  if (safeDelay(1000)) return;

  // 2:12 - On [Yellow]
  hat.setStatic(1, HAT_LEDS, CRGB::Yellow);
  shirt_left.setStatic(1, SHIRT_LEFT_LEDS, CRGB::Yellow);
  shirt_right.setStatic(1, SHIRT_RIGHT_LEDS, CRGB::Yellow);
  set_static_hand(CRGB::Yellow);
  
  // Hold until 2:17:5
  if (safeDelay(5500)) return; 

  // 2:17:5 - Off / Gap
  clear_all();
  if (safeDelay(1500)) return; // Wait until 2:19

  // 2:19 to 2:37 - Rainbow Fall (Matches other suits)
  rbTime = 0;
  while(rbTime < 18000) { if (rainbow_fall(10, 5)) return; rbTime += 10; }

  // 2:37 - All off (top to bottom)
  if (wipeTopDown(CRGB::Black, 2000)) return; 
  if (safeDelay(4000)) return; 

  // 2:43 - On/Off (All) [Red]
  set_static_all(CRGB::Red);
  if (safeDelay(2000)) return; 
  clear_all();
  if (safeDelay(2000)) return; 

  // 2:47 - On/Off (All) [Red]
  set_static_all(CRGB::Red);
  if (safeDelay(2000)) return; 
  clear_all();
  if (safeDelay(8000)) return; 

  // 2:57 - Hands On [Yellow]
  set_static_hand(CRGB::Yellow);
  if (safeDelay(15000)) return; 

  // 3:12 - On [Orange]
  set_static_all(CRGB::Orange);
  if (safeDelay(5000)) return; 
  clear_all();
  if (safeDelay(4000)) return; 

  // 3:21 - On [Red]
  set_static_all(CRGB::Red);
  if (safeDelay(5000)) return; 
  clear_all();
  
  // 3:26 to 3:37 - Gap (Suit 4 skips 3:31 Mustard)
  if (safeDelay(11000)) return; 

  // 3:37 - On [Dark Blue]
  set_static_all(CRGB::DarkBlue);
  if (safeDelay(1000)) return; 
  clear_all();
  if (safeDelay(5000)) return; 

  // 3:43 - On [Dark Blue]
  set_static_all(CRGB::DarkBlue);
  if (safeDelay(1000)) return; 
  clear_all();
  if (safeDelay(8000)) return; 

  // 3:52 - On [Light Blue]
  set_static_all(CRGB::LightBlue);
  if (safeDelay(3000)) return; 
  clear_all();
  if (safeDelay(10000)) return; 

  // 4:05 - On [Light Green]
  set_static_all(CRGB::LightGreen);
  if (safeDelay(4000)) return; 
  clear_all();
  if (safeDelay(31000)) return; 

  // 4:40 - On [Yellow]
  set_static_all(CRGB::Yellow);
  if (safeDelay(2000)) return; 
  clear_all();
  if (safeDelay(7000)) return; 

  // 4:49 - On [Red]
  set_static_all(CRGB::Red);
  if (safeDelay(9000)) return; 
  clear_all();
  if (safeDelay(43000)) return; 

  // 5:41 - On [Pink]
  set_static_all(CRGB::DeepPink);
  if (safeDelay(1000)) return; 
  clear_all();
  if (safeDelay(1000)) return; 
  set_static_all(CRGB::DeepPink); if(safeDelay(1000)) return; 
  clear_all(); if(safeDelay(1000)) return; 
  set_static_all(CRGB::DeepPink); if(safeDelay(5000)) return; 
  
  clear_all();
  if (safeDelay(47000)) return; // Gap until 6:37

  // 6:37 - On/Off [Pink]
  set_static_all(CRGB::DeepPink); if(safeDelay(2000)) return; 
  clear_all(); if(safeDelay(7000)) return; 

  // 6:46 - On/Off [Red]
  set_static_all(CRGB::Red); if(safeDelay(5000)) return; 
  clear_all(); if(safeDelay(5000)) return; 

  // 6:56 - On/Off [Red]
  set_static_all(CRGB::Red); if(safeDelay(1000)) return; 
  clear_all(); if(safeDelay(3000)) return; 

  // 7:00 - On then fade top down [White]
  set_static_all(CRGB::White);
  if (safeDelay(3000)) return;
  if (wipeTopDown(CRGB::Black, 1000)) return;

  Serial.println("Suit 4 Choreography Done.");
  MODE = 2; 
}

void loop() {
  if (MODE == 1) {
    runChoreography();
  } else {
    if(MODE == 0) clear_all();
    delay(10); 
  }
}