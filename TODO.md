# TODO

- [ ] Add color correction function
- [ ] Add more clocks and use more logical names
- [ ] Fix push button issue

# COMPLETE (v2 -> v3)

- [x] Add time to clock when setting to compensate for upload time
- [x] Integrate a pushbutton for DST adjustment
- [x] Ordered a new DS3231 (upgrade from DS1307), which has an internal oscillator from Adafruit for testing
- [x] Possible code execution affecting clock reads (serial process?) - NOT THE CASE
- [x] Check voltage draw from battery on whole board - NONE
- [x] PySerial script to record system time and Teensy output - NOT THE ISSUE
- [x] Drift could be caused by the battery power vs the VCC power - NOT THE CASE
- [x] Recording battery serial output will require a USB cable without power from the computer to the Teensy (probably still need to power Teensy board with USB though)
- [x] Consider connecting LED screen to display time for now - NOT ENOUGH PINS
