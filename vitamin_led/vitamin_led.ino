#include <FastLED.h>
#include <elapsedMillis.h>

#define NUM_LEDS 30
#define DATA_PIN 3
//#define CLOCK_PIN 8
#define BRIGHTNESS 64

#define SLICE_TICKS 1500   // 25 min x 60 secs
#define PAUSE_TICKS 300    //  5 min x 60 secs
#define UNKNOWN_STATE_TIMEOUT 10000 // 35 secs

CRGB leds[NUM_LEDS];
int ticks = SLICE_TICKS;
elapsedMillis timeElapsedTick;

enum VitaminR_States{
  UnknownState,
  IdleState,
  TimeSliceState,
  PausedState,
  TimeSliceExtensionState,
  TimeSliceElapsedUIState,
  TimedBreakState,
  NewTimeSliceUIState,
};

enum VitaminR_States state = UnknownState;

void setup() {
  delay(1000);
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalSMD5050 );
  // FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<WS2801, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<LPD8806, DATA_PIN, CLOCK_PIN, BRG>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  
  Serial.begin(9600);
}

void searching_led() {
  static int pos = 0;

  leds[pos] = CRGB::Green;

  if(pos > 0) {
    leds[pos - 1] = CRGB::Black;
  } else {
    leds[NUM_LEDS - 1] = CRGB::Black;
  }
  pos++;
  if(pos >= NUM_LEDS) {
    pos = 0;
  }
  FastLED.show();
}

void show_ticks(const struct CRGB & color) {
  int led_count = 0;
  
  if (ticks > 0) {
    led_count = ((ticks - 1) / 60) + 1;
  }
  show_leds(led_count, color);
}

void show_leds(int led_count, const struct CRGB & color) {
  int led_spacer = 0;

  for(int i = 0; i < led_count; i++) {
    leds[i + led_spacer] = color;

    if ((i+1) % 5 == 0) {
      led_spacer++;
    }
  }
  for(int i = led_count ; i + led_spacer < NUM_LEDS; i++) {
    leds[i + led_spacer] = CRGB::Black;
  }

  FastLED.show();
} 

String read_serial() {
  String cmd = "";

  if (Serial.available() > 0) {
    cmd = Serial.readStringUntil('\n');

    if (cmd == "IdleState") {
      ticks = SLICE_TICKS;
      state = IdleState;
    } else if (cmd == "TimeSliceState") {
      ticks = SLICE_TICKS;
      state = TimeSliceState;
    } else if (cmd == "PausedState") {
      state = PausedState;
    } else if (cmd == "TimeSliceExtensionState") {
      state = TimeSliceExtensionState;
    } else if (cmd == "TimeSliceElapsedUIState") {
      ticks = SLICE_TICKS;
      state = TimeSliceElapsedUIState;
    } else if (cmd == "TimedBreakState") {
      ticks = PAUSE_TICKS;
      state = TimedBreakState;
    } else if (cmd == "NewTimeSliceUIState") {
      ticks = SLICE_TICKS;
      state = NewTimeSliceUIState;
    } else if (cmd == "eachTick") {
      timeElapsedTick = 0;
    }
    Serial.print("state=");
    Serial.print(state);
    Serial.print(", ticks=");
    Serial.println(ticks);
  }

  return cmd;
}

void state_machine(String cmd) {

  if(timeElapsedTick > UNKNOWN_STATE_TIMEOUT) {
    state = UnknownState;
    ticks = SLICE_TICKS;
  }

  switch(state) {
    case UnknownState:
      searching_led();
      if (cmd == "eachTick") {
        state = IdleState;
        FastLED.clear();
      }
      break;
    case IdleState:
    case TimeSliceElapsedUIState:
    case NewTimeSliceUIState:
      show_ticks(CRGB::Green);
    break;
    case TimeSliceState:
    case TimeSliceExtensionState:
      if (cmd == "eachTick") {
        ticks--;
      }
      show_ticks(CRGB::Red);

    break;
    case PausedState:
      show_ticks(CRGB::Yellow);
    break;
    case TimedBreakState:
      if (cmd == "eachTick") {
        ticks--;
      }
      show_ticks(CRGB::Yellow);
    break;
  }
}

void loop() {
  String cmd = read_serial();
  state_machine(cmd);

  delay(25);
}
