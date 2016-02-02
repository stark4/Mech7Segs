// Shift the given digit, if p is true, print period
void shiftDigit(int d, bool p) {
  byte digit = segs[d];
  if (p) {
    digit = digit + B00000001;
  }
  word data = upDownBytes(digit);
  shift(dataPin,clockPin,LSBFIRST,data);
}

// Latches serial data into parallel output
void latchIn() {
  digitalWrite(latchPin, HIGH);
  delay(1);
  digitalWrite(latchPin, LOW);
  // Switch the minutes in first
  digitalWrite(minuteOE, LOW);
  delay(200);
  digitalWrite(minuteOE, HIGH);
  // Then do the hour
  digitalWrite(hourOE, LOW);
  delay(200);
  digitalWrite(hourOE, HIGH);
}

// Print the given time
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

// Shifts in val
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
