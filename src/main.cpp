#define FASTLED_INTERNAL  // prevents pragma error
#include <RTClib.h>  // RTC library
#include <FastLED.h>  // LED library
#include <ezButton.h>  // button library

// Teensy and LED parameters - fixed global definitions
#define CLOCK_PIN                   1 // SCLK - B1 (YELLOW)
#define DATA_PIN                    2 // MOSI - B2 (GREEN)
#define BUTTON_PIN                  20 // ADC1 - F1 (YELLOW)
#define LED_TYPE                    APA102
#define COLOR_ORDER                 BGR

// realtime clock setup
RTC_DS3231 rtc;

// button setup
// test that this is being used with a pullup and not a pulldown resistor...
// keeps the button HIGH when not pressed (vs LOW)
ezButton button( BUTTON_PIN );

// strip setup - fixed global definitions
#define NUM_LEDS                    60 // Number of LEDs in strip
CRGB leds[NUM_LEDS];

// animation parameters - fixed global definitions
#define BRIGHTNESS                  128  // 96
#define FRAMES_PER_SECOND           10  // note: lower framerates (even 120) fail to capture fast moving LED patterns

// program parameters
bool state_change = true; // state change checker for events
int hour_counter = 0; // hour count for clock change - 5 or 7 hours will cover clock uniformly
int clock_index = random( 3 ); // index for clock "switch"

// time parameters
int upload_time;
int h;
int m;
int s;
int dst = 0;  // DST offset


///////////////////
// Serial Output //
///////////////////
void serial_print( int state_change, int hour_counter, int clock_index, int h, int m, int s, int dst ) {
  Serial.print( state_change );
  Serial.print( " " );
  Serial.print( hour_counter );
  Serial.print( " " );
  Serial.print( clock_index );
  Serial.print( " " );
  Serial.print( h );
  Serial.print( ":" );
  Serial.print( m );
  Serial.print( ":" );
  Serial.print( s );
  Serial.print( " " );
  Serial.print( dst );
  Serial.print( "\n" );
}


////////////
// Clocks //
////////////
// Clock - Basic troubleshooter
void clock( int h, int m, int s ) {
  // clear LEDs
  FastLED.clear();

  //for( int j = 0; j <= m; j++ ) {
  //  leds[j % NUM_LEDS] = CRGB( 0, 200, 0 ); // fill minute line
  //}
  leds[5 * (h % 12) + 5 * m / 60] = CRGB( 0, 0, 200 ); // fill hour tick
  leds[m] = CRGB( 0, 200, 0 ); // fill minute tick
  //for( int j = 0; j < 12; j++ ) {
  //  leds[j * 5] += CHSV( 0, 0, 255 ); // stamp hour ticks (+ brightness if value already filled)
  //}
  //leds[s] = CRGB( 200, 0, 0 ); // second ticker
}

// Clock 2 - Eric's input
void clock2( int h, int m, int s ) {
  // clear LEDs
  FastLED.clear();

  int tail = 20;
  for( int j = tail; j >= 0; j-- ) {
    leds[(m - j + NUM_LEDS) % NUM_LEDS] = CHSV( 104, (tail - j) * 255 / tail, (tail - j) * 140 / tail ); // fill minute line
  }
  for( int j = 2; j >= 0; j-- ){
    leds[(j + (5 * (h % 12) + 5 * m / 60) - 1 + NUM_LEDS) % NUM_LEDS] = CHSV( 160, 255, 255 ); // fill hour fat tick
    if( (j + (5 * (h % 12) + 5 * m / 60) - 1 + NUM_LEDS) % 5 == 0 ) { // fill hour ticks in fat hour tick
      leds[(j + (5 * (h % 12) + 5 * m / 60) - 1 + NUM_LEDS) % NUM_LEDS] += CHSV( 200, 255, 255 );
    }
  }
  //for( int tick = 0; tick < 12; tick++ ) {
  //  leds[tick * 5] += CHSV( 0, 0, 255 ); // stamp hour ticks (+ brightness if value already filled)
  leds[0] += CHSV( 200, 255, 255 ); // stamp top tick
  //}
  if( s % 2 == 0 ) {
    leds[s] = CHSV( 0, 255, 100 ); // second ticker
  }
}

// Clock 2a - Tail ticks
void clock2a( int h, int m, int s ) {
  // clear LEDs
  FastLED.clear();

  int tail = 20;
  for( int j = 2; j >= 0; j-- ){
    leds[(j + (5 * (h % 12) + 5 * m / 60) - 1 + NUM_LEDS) % NUM_LEDS] += CHSV( 160, 255, 255 ); // fill hour fat tick
  }
  for( int j = tail; j >= 0; j-- ) {
    leds[(m - j + NUM_LEDS) % NUM_LEDS] += CHSV( 104, (tail - j) * 255 / tail, (tail - j) * 140 / tail ); // fill minute line
    if( (m - j + NUM_LEDS) % 5 == 0 ) {
      leds[(m - j + NUM_LEDS) % NUM_LEDS] += CHSV( 200, 255, (tail - j) * 255 / tail ); // fill hour ticks in minute line
    }
  }
  //for( int tick = 0; tick < 12; tick++ ) {
  //  leds[tick * 5] += CHSV( 0, 0, 255 ); // stamp hour ticks (+ brightness if value already filled)
  leds[0] += CHSV( 200, 255, 255 ); // stamp top tick
  //}
  if( s % 5 == 0 ) {
    leds[s] = CHSV( 0, 255, 100 ); // second ticker
  }
}

// Clock 2b - Simple
void clock2b( int h, int m, int s ) {
  // clear LEDs
  FastLED.clear();

  for( int j = 2; j >= 0; j-- ){
    leds[(j + (5 * (h % 12) + 5 * m / 60) - 1 + NUM_LEDS) % NUM_LEDS] = CHSV( 160, 255, 255 ); // fill hour fat tick
    if( (j + (5 * (h % 12) + 5 * m / 60) - 1 + NUM_LEDS) % 5 == 0 ) { // fill hour ticks in fat hour tick
      leds[(j + (5 * (h % 12) + 5 * m / 60) - 1 + NUM_LEDS) % NUM_LEDS] += CHSV( 200, 255, 255 );
    }
  }
  for( int j = 1; j >= 0; j-- ){
    leds[(j + m + NUM_LEDS - 1) % NUM_LEDS] = CHSV( 104, 255, 140); // fill medium minute tick
    if( (j + m + NUM_LEDS - 1) % 5 == 0 ) { // fill hour ticks in medium minute tick
      leds[(j + m + NUM_LEDS - 1) % NUM_LEDS] += CHSV( 200, 255, 255 );
    }
  }
  leds[0] += CHSV( 200, 255, 255 ); // stamp top tick
  if( s % 2 == 0 ) {
    leds[s] = CHSV( 0, 255, 100 ); // second ticker
  }
}

// Clock 2c - Simple Tick Illum
void clock2c( int h, int m, int s ) {
  // clear LEDs
  FastLED.clear();

  for( int j = 2; j >= 0; j-- ){
    leds[(j + (5 * (h % 12) + 5 * m / 60) - 1 + NUM_LEDS) % NUM_LEDS] = CHSV( 160, 255, 255 ); // fill hour fat tick
    if( (j + (5 * (h % 12) + 5 * m / 60) - 1 + NUM_LEDS) % 5 == 0 ) { // fill hour ticks in fat hour tick
      leds[(j + (5 * (h % 12) + 5 * m / 60) - 1 + NUM_LEDS) % NUM_LEDS] += CHSV( 200, 255, 255 );
    }
  }
  for( int j = 10; j >= 0; j-- ){
    leds[m % NUM_LEDS] = CHSV( 104, 255, 140); // fill minute tick
    if( (m - j + NUM_LEDS + 5) % 5 == 0 ) { // fill hour ticks near minute tick - 10 / 5  is 5 ahead / 5 behind
      leds[(m - j + NUM_LEDS + 5) % NUM_LEDS] += CHSV( 200, 255, 255 );
    }
  }
  leds[0] += CHSV( 200, 255, 255 ); // stamp top tick
  if( s % 2 == 0 ) {
    leds[s] = CHSV( 0, 255, 100 ); // second ticker
  }
}

// Clock 3 - Ziiro inspired
void clock3( int h, int m, int s ) {
  // clear LEDs
  FastLED.clear();

  int h_tail = 30;
  int m_tail = 20;
  int s_tail = 10;
  for( int j = h_tail; j >= 0; j-- ){
    leds[((5 * (h % 12) + 5 * m / 60) - j + NUM_LEDS) % NUM_LEDS] += CHSV( 160, (h_tail - j) * 255 / h_tail, (h_tail - j) * 140 / h_tail ); // hour line
  }
  for( int j = m_tail; j >= 0; j-- ) {
    leds[(m - j + NUM_LEDS) % NUM_LEDS] += CHSV( 104, (m_tail - j) * 255 / m_tail, (m_tail - j) * 140 / m_tail ); // minute line
    if( (m - j + NUM_LEDS) % 15 == 0 ) { // fill quarter ticks in minute line
      leds[(m - j + NUM_LEDS) % NUM_LEDS] += CHSV( 0, 0, 32 + (m_tail - j) * 200 / m_tail ); // fill ticks in minute line
    }
  }
  if( s % 1 == 0 ) {
    for( int j = s_tail; j >= 0; j-- ) {
      leds[(s - j + NUM_LEDS) % NUM_LEDS] += CHSV( 0, (s_tail - j) * 255 / s_tail, (s_tail - j) * 140 / s_tail ); // second line
    }
  }
  //leds[0] += CHSV( 0, 0, 32 ); // stamp top tick
}

// Clock 4 - Tone Dr Study
void clock4( int h, int m, int s ) {
  // clear LEDs
  FastLED.clear();

  for( int j = 2; j >= 0; j-- ){
    leds[(j + (5 * (h % 12) + 5 * m / 60) - 1 + NUM_LEDS) % NUM_LEDS] = CHSV( 14, 255, 100 ); // fill hour fat tick
    }
  for( int j = 1; j >= 0; j-- ){
    leds[(j + m + NUM_LEDS - 1) % NUM_LEDS] = CHSV( 240, 255, 100); // fill medium minute tick
    }
  if( s % 1 == 0 ) {
    leds[s] = CHSV( 38, 255, 100 ); // second ticker
  }
}

// Dumb Clock - no DS1307
void dumb_clock() {
  // clear LEDs
  FastLED.clear();

  int s = 23580; // 43200 (Day)
  int m = 0;
  int h = 0;
  while( 1 ) {
    m = (s / 60 + 1) % 60; // convert seconds to minutes
    h = (5 * (s / 60) / 60) % 60; // convert seconds to hours
    for( int j = 0; j < m; j++ ) {
      leds[j % NUM_LEDS] = CRGB( 0, 20, 0 ); // fill minute line
    }
    leds[h % NUM_LEDS] = CRGB( 0, 0, 20 ); // fill hour tick
    for( int tick = 0; tick < 12; tick++ ) {
      leds[tick * 5] += CRGB( 0, 0, 40 ); // stamp hour ticks (+ brightness if value already filled)
    }
    leds[s % NUM_LEDS] += CRGB( 20, 0, 0 ); // second ticker
    s++;
  }
}


////////////////////
// Light Patterns //
////////////////////
// shared parameters
int8_t hue;

// all lights rainbow fade - moving along strip
void rainbow_spin( int h ) {
  // clear LEDs
  FastLED.clear();

  // rainbow fill with offset
  for( int i = 0; i < NUM_LEDS; i++ ) {
    uint8_t offset = (255.0 / NUM_LEDS) * i;
    leds[i] = CHSV(hue + offset, 255, 90);
  }

  // update hue variable
  EVERY_N_MILLIS( 1 ) {
    hue--;
  }

  // show hour tick
  leds[5 * (h % 12)] = CHSV( 0, 0, 255 );
}

// all lights random - strobe
void random_strobe( int h ) {
  // clear LEDs
  FastLED.clear();

  int maxi = 6;
  while( maxi > 0 ) {
    leds[random(NUM_LEDS)] = CHSV( random(256), 255, 255 );
    maxi--;
  }
  leds[5 * (h % 12)] = CHSV( 0, 0, 255 ); // added to show hour
}

// error - all red
void error_lights() {
  // clear LEDs
  FastLED.clear();

  for( int j = 0; j < NUM_LEDS; j++ ) {
    leds[j] = CRGB( 255, 0, 0 );
  }
}


/////////////////////
// Clock Scheduler //
/////////////////////
void clock_scheduler( int clock_index, int h, int m, int s ) {
  if( h == 0 && m == 0 ) { // run "midnight rave" for one minute
    random_strobe( h );
  }
  else if( h >= 0 && h < 6 ) { // minimal mode for night time
    clock( h, m, s );
  }
  else if( m == 0 ) { // run "rainbow hour" on the hour for one minute
    rainbow_spin( h );
  }
  else { // run through default sequence of clock programs (mod every 3x5 or 3x7 hours to completely cover clock)
    switch( clock_index ) {
      case 0 :
        clock2c( h, m, s );
        break;
      case 1 :
        clock2a( h, m, s );
        break;
      case 2 :
        clock3( h, m, s );
        break;
    }
  }
}


///////////
// SETUP //
///////////
void setup () {
  delay(3000);  // 3 second delay for bootup

  // check serial connection
  if (! rtc.begin()) {
    Serial.print( "Couldn't find RTC!" );
    error_lights();
    while ( 1 );  // trap program
  }

  // set the time
  // upload this code, comment these three lines out, and then re-upload
  //upload_time = 4;
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  // Set time based on computer's system clock
  //rtc.adjust(DateTime(rtc.now().unixtime() + upload_time));  // Adjust time to account for upload

  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE, DATA_PIN, CLOCK_PIN, COLOR_ORDER>( leds, NUM_LEDS );  // .setCorrection( TypicalLEDStrip );

  // set master brightness control
  FastLED.setBrightness( BRIGHTNESS );

  // set up DST pushbutton
  button.setDebounceTime( 30 ); // set debounce time to 30 milliseconds
}


//////////
// MAIN //
//////////
void loop () {
    
  // check button
  button.loop(); // MUST call the loop() function before any other button code
  int count = button.getCount();
  if (count > 0) {
    // tick dst variable
    dst++;
    if (dst > 1) {  // jump values over 1 back to -1
      dst = -1;
    }

    // report change
    Serial.print( "button pressed" );
    Serial.print( ", DST set to: " );
    Serial.print( dst );
    Serial.print( "\n" );

    // reset button counter
    button.resetCount();
  }

  // updates
  EVERY_N_MILLISECONDS( 1000 / FRAMES_PER_SECOND ) {
    // poll time
    DateTime now = rtc.now();  // Poll DS3231
    now = ( now.unixtime() + dst * 3600 );  // Add an hour if set

    // extract time elements
    h = now.hour();
    m = now.minute();
    s = now.second();

    // check for state changes
    if( m == 0 && state_change ) { // update hour counter once only at the top of the hour
      hour_counter++;
      state_change = false;
    }
    else if( m == 59 ) { // reset state change variable before top of the hour
      state_change = true;
    }
    else if( m == 0 && hour_counter == 4 ) {
      clock_index = random( 3 );
      hour_counter = 0;
    }

    // run main function(s)
    clock_scheduler( clock_index, h, m, s ); // run clock scheduler
    serial_print( state_change, hour_counter, clock_index, h, m, s, dst ); // debug printout -- state bool, hour count, clock switch, h:m:s, dst
    
    // show LEDs
    FastLED.show();  // send the 'leds' array out to the actual LED strip
  }
}