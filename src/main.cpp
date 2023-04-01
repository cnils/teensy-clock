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
#define QUERIES_PER_SECOND          4  // number of times the RTC is queried per second
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
#define FRAMES_PER_SECOND           500  // note: lower framerates (even 120) fail to capture fast moving LED patterns

// program parameters
uint8_t clock_index;
uint8_t clock_elapsed;
uint8_t clock_duration = 5;  // number of hours before clock is cycled
bool triggered;

// time parameters
int upload_time;
uint8_t h;
uint8_t m;
uint8_t s;
int8_t dst;  // DST offset


///////////////////
// Serial Output //
///////////////////
void serial_print( uint8_t h, uint8_t m, uint8_t s, uint8_t clock_index, int8_t dst ) {
  Serial.print( h );
  Serial.print( ":" );
  Serial.print( m );
  Serial.print( ":" );
  Serial.print( s );
  Serial.print( ", clock index: " );
  Serial.print(clock_index);
  Serial.print( ", dst: " );
  Serial.print( dst );
  Serial.print( "\n" );
}


////////////
// Clocks //
////////////
// Clock - Basic troubleshooter
void clock( uint8_t h, uint8_t m, uint8_t s ) {
  // clear LEDs
  FastLED.clear();

  // fill LEDs
  leds[5 * (h % 12) + 5 * m / 60] = CRGB( 0, 0, 200 ); // fill hour tick
  leds[m] = CRGB( 0, 200, 0 ); // fill minute tick
}

// Clock 2 - Eric's input
void clock2( uint8_t h, uint8_t m, uint8_t s ) {
  // clear LEDs
  FastLED.clear();

  uint8_t tail = 20;
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
void clock2a( uint8_t h, uint8_t m, uint8_t s ) {
  // clear LEDs
  FastLED.clear();

  uint8_t tail = 20;
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
void clock2b( uint8_t h, uint8_t m, uint8_t s ) {
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
void clock2c( uint8_t h, uint8_t m, uint8_t s ) {
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
void clock3( uint8_t h, uint8_t m, uint8_t s ) {
  // clear LEDs
  FastLED.clear();

  uint8_t h_tail = 30;
  uint8_t m_tail = 20;
  uint8_t s_tail = 10;
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
void clock4( uint8_t h, uint8_t m, uint8_t s ) {
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


////////////////////
// Light Patterns //
////////////////////
// shared parameters
int8_t hue;

// all lights rainbow fade - moving along strip
void rainbow_spin( uint8_t h ) {
  // clear LEDs
  FastLED.clear();

  // rainbow fill with offset
  uint8_t interp = 255.0 / NUM_LEDS;  // interpolate LEDs to fill strip with rainbow
  for( int i = 0; i < NUM_LEDS; i++ ) {
    leds[i] = CHSV(hue + interp * i, 255, 90);
  }

  // update hue variable
  EVERY_N_MILLIS( 1 ) {
    hue--;
  }

  // show hour tick
  leds[5 * (h % 12)] = CHSV( 0, 0, 255 );
}

// all lights random - strobe
void random_strobe( uint8_t h ) {
  // clear LEDs
  FastLED.clear();

  uint8_t n_strobes = 6;
  while( n_strobes > 0 ) {
    leds[random8() % NUM_LEDS] = CHSV( random8(), 255, 255 );
    n_strobes--;
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
// list of clocks to cycle through.  Each is defined as a separate function below.
typedef void ( *ClockList[] )(uint8_t h, uint8_t m, uint8_t s);
ClockList clock_patterns = { clock2a, clock2c, clock3 };
// PatternList led_patterns = { single_snake };

// cycling function(s)
#define ARRAY_SIZE(A) ( sizeof(A) / sizeof( A[0] ) )
void next_pattern() {
  // add one to the current pattern number, and wrap around at the end
  clock_index = ( clock_index + random8() ) % ARRAY_SIZE( clock_patterns );
}

// scheduler function
void clock_scheduler( uint8_t h, uint8_t m, uint8_t s, uint8_t clock_index ) {
  // order matters for these...
  if( h == 0 && m == 0 ) { // run "midnight rave" for one minute
    random_strobe( h );
  }
  else if( h >= 0 && h < 6 ) { // minimal mode for night time
    clock( h, m, s );
  }
  else if( m == 0 ) { // run "rainbow hour" on the hour for one minute
    rainbow_spin( h );
    // code to run once per hour (top of the hour)
    if( !triggered ) {
      clock_elapsed = ( clock_elapsed + 1 ) % clock_duration;
      // cycle clock
      if( clock_elapsed == clock_duration - 1) {
        next_pattern();
      }
      // set trigger
      triggered = 1;
    }
  }
  else { // run through default sequence of clock programs (mod every 3x5 or 3x7 hours to completely cover clock)
    clock_patterns[clock_index]( h, m, s );
    serial_print( h, m, s, clock_index, dst ); // debug printout -- state bool, hour count, clock switch, h:m:s, dst
  }

  // reset trigger once per hour (bottom of the hour)
  if(m == 30) {
    triggered = 0;
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

  // Set seed for PRNG
  srand( 42 );

  // set up DST pushbutton
  button.setDebounceTime( 30 ); // set debounce time to 30 milliseconds
}


//////////
// MAIN //
//////////
void loop () {
    
  // check button
  button.loop(); // MUST call the loop() function before any other button code
  uint8_t count = button.getCount();
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

  // query time and update clocks
  EVERY_N_MILLISECONDS( 1000 / QUERIES_PER_SECOND ) {
    // poll time
    DateTime now = rtc.now();  // Poll DS3231
    now = ( now.unixtime() + dst * 3600 );  // Add an hour if set

    // extract time elements
    h = now.hour();
    m = now.minute();
    s = now.second();

    // run clock scheduler
    clock_scheduler( h, m, s, clock_index ); // run clock scheduler
  }

  // update LEDs
  EVERY_N_MILLISECONDS( 1000 / FRAMES_PER_SECOND ) {
    // show LEDs
    FastLED.show();  // send the 'leds' array out to the actual LED strip
  }
}