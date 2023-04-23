#include <Wire.h>
#include "RTC8564.h"

void setup() {
  struct DateTime dt = {0, 0, 0, 9, 7, 23, 0};

  Serial.begin(9600);
  RTC8564.begin(&dt);
}

void loop() {
  struct DateTime dt;
  char s[20];

  RTC8564.getDateTime(&dt);
  sprintf(
    s, "%4d/%02d/%02d %02d:%02d:%02d ",
    dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second
  );
  Serial.println(s);
  delay(1000);
}
