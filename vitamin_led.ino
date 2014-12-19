#include "FastSPI_LED2.h"

#define NUM_LEDS 30
#define NUM_LEDS2 240
#define DATA_PIN 2
// #define DATA_PIN 6
// Clock pin only needed for SPI based chipsets when not using hardware SPI
// #define CLOCK_PIN 8

#define SLICE_TICKS 1500 // 25 min x 60 secs
#define PAUSE_TICKS 300 // 5 min x 60secs

CRGB leds[NUM_LEDS2];
String input_cmd;
int ticks = SLICE_TICKS;

enum VitaminR_States{
  IdleState,
  TimeSliceState,
  PausedState,
  TimeSliceExtensionState,
  TimeSliceElapsedUIState,
  TimedBreakState,
  NewTimeSliceUIState,
};

enum VitaminR_States state = IdleState;

void setup() {
  delay(1000);
  FastLED.addLeds<WS2811, DATA_PIN, GRB>(leds, NUM_LEDS);
  // FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<WS2801, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<LPD8806, DATA_PIN, CLOCK_PIN, BRG>(leds, NUM_LEDS2);
  FastLED.setBrightness(128);
  FastLED.clear();
  
  Serial.begin(9600);
}

void show_ticks(const struct CRGB & color) {
  int led_count = 0;
  
  if (ticks > 0) {
    led_count = ((ticks - 1) / 60) + 1;
  }
  show_leds(led_count, color);
}

void show_leds(int led_count, const struct CRGB & color) {
  struct CRGB color_light = color;
  int led_spacer = 0;
  
  for(int i = 0; i < led_count; i++) {
    leds[i + led_spacer] = color;
    if ((i+1) % 5 == 0) {
      led_spacer++;
    }
  }
  for(int i = led_count; i < NUM_LEDS; i++) {
    leds[i + led_spacer] = CRGB::Black;
  }

  FastLED.show();
} 

void loop() {
  if (Serial.available() > 0) {
    input_cmd = Serial.readStringUntil('\n');
    if (input_cmd == "IdleState") {
      ticks = SLICE_TICKS;
      state = IdleState;
    } else if (input_cmd == "TimeSliceState") {
      ticks = SLICE_TICKS;
      state = TimeSliceState;
    } else if (input_cmd == "PausedState") {
      state = PausedState;
    } else if (input_cmd == "TimeSliceExtensionState") {
      state = TimeSliceExtensionState;
    } else if (input_cmd == "TimeSliceElapsedUIState") {
      ticks = SLICE_TICKS;
      state = TimeSliceElapsedUIState;
    } else if (input_cmd == "TimedBreakState") {
      ticks = PAUSE_TICKS;
      state = TimedBreakState;
    } else if (input_cmd == "NewTimeSliceUIState") {
      ticks = SLICE_TICKS;
      state = NewTimeSliceUIState;
    }
    Serial.print("state=");
    Serial.print(state);
    Serial.print(", ticks=");
    Serial.println(ticks);
  }
   
  switch(state) {
    case IdleState:
    case TimeSliceElapsedUIState:
    case NewTimeSliceUIState:
      show_ticks(CRGB::Green);
    break;
    case TimeSliceState:
    case TimeSliceExtensionState:
      if (input_cmd == "eachTick") {
        ticks--;
      }
      show_ticks(CRGB::Red);
    break;
    case PausedState:
      show_ticks(CRGB::Yellow);
    break;
    case TimedBreakState:
      if (input_cmd == "eachTick") {
        ticks--;
      }
      show_ticks(CRGB::Yellow);
    break;
  }
 
  input_cmd = "";
  delay(25);
}
