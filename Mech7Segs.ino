/*
dat
clk
str
oe
*/
#include <Wire.h>
#include "RTClib.h"

#define LIGHT_TRIGGER 850

DateTime last, current;
RTC_DS1307 rtc;

int hourOE = 9;
//Pin connected to strobe
int latchPin = 10;
////Pin connected to data
int dataPin = 11;
//Pin connected to clock
int clockPin = 12;
////Pin connected to !OE
int minuteOE = 13;

byte segs[11] = {
  B11111100, // 0
  B01100000, // 1
  B11011010, // 2
  B11110010, // 3
  B01100110, // 4
  B10110110, // 5
  B10111110, // 6
  B11100000, // 7
  B11111110, // 8
  B11110110, // 9
  B00000000, // []
};

word upDownBytes(byte segs) {
  byte nsegs = ~segs;
  word result = 0;
  for(int i = 15; i >= 0; i--) {
    result |= (segs >> i) & 1;
    result <<= 1;
    result |= (nsegs >> i) & 1;
    if (i != 0) {
      result <<= 1;
    }
  }
  Serial.println(segs, BIN);
  Serial.println(result, BIN);
  return result;
}

void setup() {
  Serial.begin(9600);
  
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(minuteOE, OUTPUT);
  pinMode(hourOE, OUTPUT);
  digitalWrite(latchPin, LOW);
  digitalWrite(dataPin, LOW);
  digitalWrite(clockPin, LOW);
  digitalWrite(minuteOE, HIGH);
  digitalWrite(hourOE, HIGH);
  
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
}

void blackOut() {
  for (int i=0; i<4; i++) {
    shiftDigit(10,0);
  }
  latchIn();
  while (analogRead(A0) > LIGHT_TRIGGER) {
    delay(100);
  }
  shiftTime(current);
  latchIn();
}

void loop() {
  current = rtc.now();
  if (analogRead(A0) > LIGHT_TRIGGER) {
    blackOut();
  } else {
    if (last.minute() != current.minute()) {
      shiftTime(current);
      latchIn();
    }
  }
  last = current;
  delay(500);
}

void latchIn() {
  digitalWrite(latchPin, HIGH);
  delay(1);
  digitalWrite(latchPin, LOW);
  digitalWrite(minuteOE, LOW);
  delay(200);
  digitalWrite(minuteOE, HIGH);
  digitalWrite(hourOE, LOW);
  delay(200);
  digitalWrite(hourOE, HIGH);
}

void shiftDigit(int d, bool p) {
  byte digit = segs[d];
  if (p) {
    digit = digit + B00000001;
  }
  word data = upDownBytes(digit);
  shift(dataPin,clockPin,LSBFIRST,data);
}

void shiftTime(DateTime t) {
  int h = t.hour()%12; // Convert from military time
  int m = t.minute();
  // Deal with hours and drop the zero if less than 10
  if (h < 10) {
    shiftDigit(10,0);
  } else {
    shiftDigit(h/10,0);
  }
  shiftDigit(h%10,0);
  // Deal with minutes part
  shiftDigit(m/10,0);
  shiftDigit(m%10,1);
}

void shift(byte dataPin, byte clockPin, byte bitOrder, word val) {
  for (byte i = 0; i < 16; i++)  {
    if (bitOrder == LSBFIRST)
      digitalWrite(dataPin, !!(val & (1 << i)));
    else {
      digitalWrite(dataPin, !!(val & (1 << (15 - i))));
    }
    digitalWrite(clockPin, HIGH);
    digitalWrite(clockPin, LOW);
  }
}

