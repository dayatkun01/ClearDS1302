#include <ClearDS1302.h>

int RTCrstPin = 2;
int RTCclkPin = 4;
int RTCdatPin = 3;

// Pin: RST, DAT, CLK
ClearDS1302 RTC1(RTCdatPin, RTCrstPin, RTCclkPin);

void setup() {
  Serial.begin(9600);

  // Time Set: second, minute, hour, day (0=Sunday - 7 = Saturday), date, month, year
  RTC1.set.time(0, 30, 12, 2, 26, 7, 2025); // Example: Monday 26 July 2025, 12:30:00
}

void loop() {
  // Show full time with format
  Serial.println(RTC1.get.time.full());

  delay(1000); // refresh every second
}

