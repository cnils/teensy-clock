#include "FastLED.h"

// How many leds in your strip?
#define NUM_LEDS 60

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
#define CLOCK_PIN 2 //MOSI - B2 (YELLOW)
#define DATA_PIN 1 //SCLK - B1 (GREEN)

// Define the array of leds
CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR>(leds, NUM_LEDS); // Red & Blue are flipped
}

void loop() { // Define MAIN program(s)
  clock();
}


// Clock
void clock(void) {
  unsigned s = 23580; // 43200 (Day)
  unsigned m = 0;
  unsigned h = 0;
  while(1) {
    m = (s / 60 + 1) % 60; // convert seconds to minutes
    h = (5 * (s / 60) / 60) % 60; // convert seconds to hours
    for( int j = 0; j < m; j++) {
      leds[j % NUM_LEDS] = CRGB( 0, 20, 0 ); // fill minute line
    }
    leds[h % NUM_LEDS] = CRGB( 0, 0, 20 ); // fill hour tick
    for( int tick = 0; tick < 12; tick++) {
      leds[tick * 5] += CHSV( 0, 0, 40 ); // stamp hour ticks (+ brightness if value already filled)
    }
    leds[s % NUM_LEDS] += CRGB( 20, 0, 0 ); // second ticker
    FastLED.show();
    delay(1000);
    FastLED.clear();
    s++;
  }
}

// Specific colors
void specific_colors(void) {
  for( int j = 0; j < NUM_LEDS; j++) {
    int col [] = { 0, 95, 95 }; // Christmas R, G
    int sat [] = { 0, 255, 255, 255 }; // Christmas S, W
    leds[j] = CHSV( col[random(3)], sat[random(4)], 90 );
  }
  FastLED.show();
  delay(2000);
}

// All lights rainbow fade - stationary
void rainbow_all(void) {
  for( int i = 0; i < 256; i++) {
    for( int j = 0; j < NUM_LEDS; j++) {
      leds[j] = CHSV( i, 255, 90 );
    }
  FastLED.show();
  delay(100);
  }
}

// All lights rainbow fade - moving along strip
void rainbow_spin(void) {
  unsigned inter = 256 / NUM_LEDS;
  for( int i = 0; i < 256; i++) {
    for( int j = 0; j < NUM_LEDS; j++) {
      leds[j] = CHSV( (i - j * inter) % 256, 255, 90 );
    }
  FastLED.show();
  delay(100);
  }
}

// All lights random
void random_all(void) {
  for( int j = 0; j < NUM_LEDS; j++) {
      leds[j] = CHSV( random(256), 255, random(90) );
  }
  FastLED.show();
  delay(469); //468.75ms for 128bpm
}

// All lights random - strobe
void random_strobe(void) {
  unsigned maxi = 3;
  while(maxi > 0) {
    leds[random(NUM_LEDS)] = CHSV( random(256), 255, 255 );
    maxi--;
  }
  FastLED.show();
  delay(30);
  FastLED.clear();
}

// Color waves - looped & single color
void color_waves(void) {
  unsigned period = 12; //number of LEDs
  unsigned color = 160;
  for( int j = 0; j < NUM_LEDS; j++) {
    for( int i = j; i < j + period; i++) {
      leds[(i - period) % NUM_LEDS] = CHSV( color, 32 + (i - j) * 223 / period, (i - j) * 127 / period ); //color fade
      //leds[(i - period) % NUM_LEDS] = CHSV( color, (period - (i - j)) * 255 / period, (i - j) * 127 / period ); //comet
    }
    FastLED.show();
    delay(100);
  }
}

// Color waves 2 - single & multi-color
void color_waves2(void) {
  unsigned period = random(NUM_LEDS); //number of LEDs  
  unsigned color = random(256);
  for( int j = -period; j < NUM_LEDS; j++) {
    for( int i = j; i - j < period; i++) {
      if( 0 <= i and i <= NUM_LEDS ) {
        leds[i] = CHSV( color, 32 + (i - j) * 223 / period, (i - j) * 127 / period ); //color fade
        //leds[i] = CHSV( color, (period - (i - j)) * 255 / period, (i - j) * 127 / period ); //comet
      }
    }
    FastLED.show();
    delay(100);
  }
  delay(random(6000));
}

// Random color seek proto - add function to go around the full 'circle'
void random_single(unsigned color) { //run with random(256) as initial parameter
  unsigned color2 = random(256);
  if (color > color2) {
    unsigned diff = color - color2;
    while (diff > 0) {
      leds[21] = CHSV( color2 + diff, 255, 90 );
      Serial.print("Color2 LESS    ");
      Serial.println(color2 + diff);
      diff--;
      FastLED.show();
      delay(40);
    }
    random_single(color2);
  }
  else {
    unsigned diff = color2 - color;
    while (diff > 0) {
      leds[21] = CHSV( color2 - diff, 255, 90 );
      Serial.print("Color2 GREATER ");
      Serial.println(color2 - diff);
      diff--;
      FastLED.show();
      delay(40);
    }
    random_single(color2);
  }
}
