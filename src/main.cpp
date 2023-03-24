#define FASTLED_INTERNAL  // prevents pragma error
#include <RTClib.h>  // RTC library
#include <FastLED.h>  // LED library
#include <ezButton.h>  // button library

RTC_DS3231 rtc;

int upload_time;
bool ch = true; // state change checker for events
int h_c = 0; // hour count for clock change - 5 or 7 hours will cover clock uniformly
int cl = random(3); // set first clock switch position
int h;
int m;
int s;

int push_curr = HIGH;  // the current state of the pushbutton input pin
int push_prev = HIGH;  // the previous state of the pushbutton input pin
int dst = 0;  // DST offset

#define NUM_LEDS 60 // Number of LEDs in strip

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
#define CLOCK_PIN 1 // SCLK - B1 (YELLOW)
#define DATA_PIN 2 // MOSI - B2 (GREEN)
#define DST_PIN 20 // ADC1 - F1 (YELLOW)

// Define the array of LEDs
CRGB leds[NUM_LEDS];


///////////////////
// Serial Output //
///////////////////
void serial_print(int ch,int h_c,int cl,int h,int m,int s,int dst) {
  Serial.print(ch);
  Serial.print(' ');
  Serial.print(h_c);
  Serial.print(' ');
  Serial.print(cl);
  Serial.print(' ');
  Serial.print(h);
  Serial.print(':');
  Serial.print(m);
  Serial.print(':');
  Serial.print(s);
  Serial.print(' ');
  Serial.print(dst);
  Serial.println();
}


////////////
// Clocks //
////////////
// Clock - Basic troubleshooter
void clock(int h, int m, int s) {
  //for( int j = 0; j <= m; j++ ) {
  //  leds[j % NUM_LEDS] = CRGB( 0, 200, 0 ); // fill minute line
  //}
  leds[5 * (h % 12) + 5 * m / 60] = CRGB( 0, 0, 200 ); // fill hour tick
  leds[m] = CRGB( 0, 200, 0 ); // fill minute tick
  //for( int j = 0; j < 12; j++ ) {
  //  leds[j * 5] += CHSV( 0, 0, 255 ); // stamp hour ticks (+ brightness if value already filled)
  //}
  //leds[s] = CRGB( 200, 0, 0 ); // second ticker
  FastLED.show();
  FastLED.clear();
}

// Clock 2 - Eric's input
void clock2(int h, int m, int s) {
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
  FastLED.show();
  FastLED.clear();
}

// Clock 2a - Tail ticks
void clock2a(int h, int m, int s) {
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
  FastLED.show();
  FastLED.clear();
}

// Clock 2b - Simple
void clock2b(int h, int m, int s) {
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
  FastLED.show();
  FastLED.clear();
}

// Clock 2c - Simple Tick Illum
void clock2c(int h, int m, int s) {
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
  FastLED.show();
  FastLED.clear();
}

// Clock 3 - Ziiro inspired
void clock3(int h, int m, int s) {
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
  FastLED.show();
  FastLED.clear();
}

// Clock 4 - Tone Dr Study
void clock4(int h, int m, int s) {
  for( int j = 2; j >= 0; j-- ){
    leds[(j + (5 * (h % 12) + 5 * m / 60) - 1 + NUM_LEDS) % NUM_LEDS] = CHSV( 14, 255, 100 ); // fill hour fat tick
    }
  for( int j = 1; j >= 0; j-- ){
    leds[(j + m + NUM_LEDS - 1) % NUM_LEDS] = CHSV( 240, 255, 100); // fill medium minute tick
    }
  if( s % 1 == 0 ) {
    leds[s] = CHSV( 38, 255, 100 ); // second ticker
  }
  FastLED.show();
  FastLED.clear();
}

// Dumb Clock - no DS1307
void dumb_clock(void) {
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
    FastLED.show();
    delay(1000);
    FastLED.clear();
    s++;
  }
}


////////////////////
// Light Patterns //
////////////////////
// All lights rainbow fade - moving along strip
void rainbow_spin(int h) {
  int inter = 256 / NUM_LEDS;
  for( int i = 0; i < 256; i++ ) {
    for( int j = 0; j < NUM_LEDS; j++ ) {
      leds[j] = CHSV( (i - j * inter) % 256, 255, 90 );
    }
  leds[5 * (h % 12)] = CHSV( 0, 0, 255 ); // added to show hour
  FastLED.show();
  delay(30);
  }
}

// All lights random - strobe
void random_strobe(int h) {
  int maxi = 6;
  while( maxi > 0 ) {
    leds[random(NUM_LEDS)] = CHSV( random(256), 255, 255 );
    maxi--;
  }
  leds[5 * (h % 12)] = CHSV( 0, 0, 255 ); // added to show hour
  FastLED.show();
  delay(30);
  FastLED.clear();
}

// Error - all red
void error_lights(){
  for( int j = 0; j < NUM_LEDS; j++ ) {
    leds[j] = CRGB( 255, 0, 0 );
  }
  FastLED.show();
  FastLED.clear();
}


/////////////////////
// Clock Scheduler //
/////////////////////
void clock_scheduler(int cl,int h,int m,int s) {
  if( h == 0 && m == 0 ) { // run "midnight rave" for one minute
    random_strobe(h);
  }
  else if( h >= 0 && h < 6 ) { // minimal mode for night time
    clock(h,m,s);
  }
  else if( m == 0 ) { // run "rainbow hour" on the hour for one minute
    rainbow_spin(h);
  }
  else { // run through default sequence of clock programs (mod every 3x5 or 3x7 hours to completely cover clock)
    switch( cl ) {
      case 0 :
        clock2c(h,m,s);
        break;
      case 1 :
        clock2a(h,m,s);
        break;
      case 2 :
        clock3(h,m,s);
        break;
    }
  }
}


///////////
// SETUP //
///////////
void setup () {
  Serial.begin(57600);

  // Check serial connection
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC!");
    error_lights();
    while (1);
  }

  // Set the time
  // Upload this code, comment these three lines out, and then re-upload
  //upload_time = 4;
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  // Set time based on computer's system clock
  //rtc.adjust(DateTime(rtc.now().unixtime() + upload_time));  // Adjust time to account for upload

  // Set up FastLED
  FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR>(leds, NUM_LEDS); // Red & Blue are flipped

  // Set up DST pushbutton
  pinMode(DST_PIN,  INPUT);
}


//////////
// MAIN //
//////////
void loop () { // Define MAIN program(s)
  // Check DST pin reading
  push_curr = digitalRead(DST_PIN);
  if (push_prev == HIGH && push_curr == LOW)
    dst++;
    if (dst > 1)
      dst = -1;  // Reset toggle
  push_prev = push_curr;

  // Poll time
  DateTime now = rtc.now();  // Poll DS3231
  now = (now.unixtime() + dst * 3600);  // Add an hour if set

  // Extract time elements
  h = now.hour();
  m = now.minute();
  s = now.second();

  if( m == 0 && ch ) { // update master hour variable and set state change variable to false
    h_c++;
    ch = false;
  }
  else if( m == 59 ) { // ensure state change variable set to true before top of the hour
    ch = true;
  }
  else if( m == 0 && h_c == 4 ) {
    cl = random(3);
    h_c = 0;
  }

  clock_scheduler(cl,h,m,s); // run clock scheduler
  serial_print(ch,h_c,cl,h,m,s,dst); // debug printout -- state bool, hour count, clock switch, h:m:s, dst

  delay(100); // update 10 times per second
}