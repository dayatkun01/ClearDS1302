// TODO: [NOTE: if you done the thing, place "+"]
// Create burst raw write
// Create burst raw read
// Create burst cooked write
// create burst cooked read

#include <Arduino.h>
#include <ClearDS1302.h>

int _RTCdatPin;
int _RTCrstPin;
int _RTCclkPin;

void errorCallback(String file, int line, String errorMessage) {
    Serial.println("[ERROR]: " + String(file) + ":" + String(line) + " - " + errorMessage);
}

byte BitToBCD(byte bit) {
    return ((bit / 10) << 4) | (bit % 10);
}

byte BCDtoBit(byte bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

void RTCwrite(byte address, byte data) {
    address |= 0x80;
    digitalWrite(_RTCrstPin, HIGH);
    pinMode(_RTCdatPin, OUTPUT);

    for (int i = 0; i < 8; i++) {
      digitalWrite(_RTCdatPin, address & 0x01);
      digitalWrite(_RTCclkPin, HIGH);
      delayMicroseconds(5);
      if(i < 7) digitalWrite(_RTCclkPin, LOW);
      address >>= 1;
      delayMicroseconds(5);
    }

    digitalWrite(_RTCclkPin, LOW);

    for (int i = 0; i < 8; i++) {
      digitalWrite(_RTCdatPin, data & 0x01);
      digitalWrite(_RTCclkPin, HIGH);
      delayMicroseconds(5);
      digitalWrite(_RTCclkPin, LOW);
      data >>= 1;
      delayMicroseconds(5);
    }
    delayMicroseconds(5);
    digitalWrite(_RTCrstPin, LOW);
}

byte RTCread(byte address) {
    byte result = 0;
    address |= 0x81;
    digitalWrite(_RTCrstPin, HIGH);
    pinMode(_RTCdatPin, OUTPUT);
    for (int i = 0; i < 8; i++) {
      digitalWrite(_RTCdatPin, address & 0x01);
      digitalWrite(_RTCclkPin, HIGH);
      delayMicroseconds(5);
      if(i < 7) digitalWrite(_RTCclkPin, LOW);
      address >>= 1;
      delayMicroseconds(5);
    }
  pinMode(_RTCdatPin, INPUT);
  digitalWrite(_RTCclkPin, HIGH);
  for (int i = 0; i < 8; i++) {
    digitalWrite(_RTCclkPin, LOW);
    if(digitalRead(_RTCdatPin) == HIGH) result |= (0x1 << i);
    if(i < 7) digitalWrite(_RTCclkPin, HIGH);
  }
  digitalWrite(_RTCrstPin, LOW);
  return result;
}

void RTCwriteBurst(byte second, byte minute, byte hour, byte day, byte month, byte year, byte date, byte ClockRegister) {
    byte address = 0xBE;
    
    byte value[8] = {
        second,
        minute,
        hour,
        day,
        month,
        year,
        date,
        ClockRegister
    };

    digitalWrite(_RTCrstPin, HIGH);
    pinMode(_RTCdatPin, OUTPUT);
    for (int i = 0; i < 8; i++) {
      digitalWrite(_RTCdatPin, address & 0x01);
      digitalWrite(_RTCclkPin, HIGH);
      delayMicroseconds(5);
      if(i < 7) digitalWrite(_RTCclkPin, LOW);
      address >>= 1;
      delayMicroseconds(5);
    }
    digitalWrite(_RTCclkPin, LOW);
    for (int i = 0; i < 8; i++) {
        byte temp = value[i];
        for (int i2 = 0; i2 < 8; i2++) {
            digitalWrite(_RTCdatPin, temp & 0x01);
            digitalWrite(_RTCclkPin, HIGH);
            delayMicroseconds(5);
            if(i < 7) digitalWrite(_RTCclkPin, LOW);
            temp >>= 1;
            delayMicroseconds(5);
        }
    }
    digitalWrite(_RTCclkPin, LOW);    
    digitalWrite(_RTCrstPin, LOW);
}

byte* RTCreadBurst() {
    byte address = 0xBF;
    static byte data[8];

    digitalWrite(_RTCrstPin, HIGH);
    pinMode(_RTCdatPin, OUTPUT);
    for (int i = 0; i < 8; i++) {
        digitalWrite(_RTCdatPin, address & 0x01);
        digitalWrite(_RTCclkPin, HIGH);
        delayMicroseconds(5);
        if(i < 7) digitalWrite(_RTCclkPin, LOW);
        address >>= 1;
        delayMicroseconds(5);
    }
    digitalWrite(_RTCclkPin, LOW);
    pinMode(_RTCdatPin, INPUT);
    for (int i = 0; i < 8; i++) {
        data[i] = 0;
        for (int i2 = 0; i2 < 8; i2++) {
            digitalWrite(_RTCclkPin, LOW);
            if (digitalRead(_RTCdatPin)) data[i] |= (0x1 << i2);
            delayMicroseconds(5);
            if(i < 7) digitalWrite(_RTCclkPin, HIGH);
            delayMicroseconds(5);
        }
    }
    digitalWrite(_RTCrstPin, LOW);
    return data;
}

ClearDS1302::ClearDS1302(int DATpin, int RSTpin, int CLKpin) {
    _RTCdatPin = DATpin;
    _RTCrstPin = RSTpin;
    _RTCclkPin = CLKpin;

    pinMode(_RTCdatPin, OUTPUT);
    pinMode(_RTCrstPin, OUTPUT);
    pinMode(_RTCclkPin, OUTPUT);

    RTCwrite(0x8E, 0x00);

    byte sec = RTCread(0x80);    
    sec &= 0x7F;                  
    RTCwrite(0x80, sec);          
}

void ClearDS1302::reset() {
    RTCwriteBurst(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
}

void ClearDS1302::raw::write(byte address, byte data) {
    RTCwrite(address, data);
}

void ClearDS1302::raw::WriteClockBurst(byte second, byte minutes, byte hour, byte day, byte month, byte year, byte date, byte ClockRegister) {
    RTCwriteBurst(second, minutes, hour, day, month, year, date, ClockRegister);
}

byte ClearDS1302::raw::read(byte address) {
    return RTCread(address);
}

byte* ClearDS1302::raw::ReadClockBurst() {
    return RTCreadBurst();
}

void ClearDS1302::set::BatteryCharging(bool IsActive, bool IsFastCharging, int ResistorLevel) {
    byte data = 0;

    if(ResistorLevel < 1) {
        errorCallback(__FILE__, __LINE__, "ResistorLevel argument cannot be less than 1!");
        return;
    }

    if(ResistorLevel > 3) {
        errorCallback(__FILE__, __LINE__, "ResistorLevel argument cannot be less than 3!");
        return;
    }

    data |= (IsActive ? 0xA0 : 0x0F);
    data |= (IsFastCharging ? 0x08 : 0x04);
    data |= (ResistorLevel == 1 ? 0x01 : (ResistorLevel == 2 ? 0x02 : 0x03));

    RTCwrite(0x90, data);
}

void ClearDS1302::set::WriteProtect(bool isActive) {
    RTCwrite(0x8E, (isActive ? 0x80 : 0x00));
}


void ClearDS1302::set::time::SetAll(int second, int minute, int hour, int day, int date, int month, int year, bool WriteProtect) {
    String errorMessage = "";
    bool is12PM = (RTCread(0x84) & 0x80 ? true : false);
    if (second < 0) {
    errorMessage += String(__FILE__) + ":" + String(__LINE__) + " Cannot set second less than 0!\n";
    }
    if (second > 59) {
        errorMessage += String(__FILE__) + ":" + String(__LINE__) + " Cannot set second more than 59!\n";
    }
    if (minute < 0) {
        errorMessage += String(__FILE__) + ":" + String(__LINE__) + " Cannot set minute less than 0!\n";
    }
    if (minute > 59) {
        errorMessage += String(__FILE__) + ":" + String(__LINE__) + " Cannot set minute more than 59!\n";
    }
    if (hour < 0) {
        errorMessage += String(__FILE__) + ":" + String(__LINE__) + " Cannot set hour less than 0!\n";
    }
    if (hour > 23) {
        errorMessage += String(__FILE__) + ":" + String(__LINE__) + " Cannot set hour more than 23!\n";
    }
    if (day < 1) {
        errorMessage += String(__FILE__) + ":" + String(__LINE__) + " Cannot set day less than 1!\n";
    }
    if (day > 7) {
        errorMessage += String(__FILE__) + ":" + String(__LINE__) + " Cannot set day more than 7!\n";
    }
    if (date < 1) {
        errorMessage += String(__FILE__) + ":" + String(__LINE__) + " Cannot set date less than 1!\n";
    }
    if (date > 31) {
        errorMessage += String(__FILE__) + ":" + String(__LINE__) + " Cannot set date more than 31!\n";
    }
    if (month < 1) {
        errorMessage += String(__FILE__) + ":" + String(__LINE__) + " Cannot set month less than 1!\n";
    }
    if (month > 12) {
        errorMessage += String(__FILE__) + ":" + String(__LINE__) + " Cannot set month more than 12!\n";
    }
    if (year < 0) {
        errorMessage += String(__FILE__) + ":" + String(__LINE__) + " Cannot set year less than 0!\n";
    }   
    if (year > 99) {
        errorMessage += String(__FILE__) + ":" + String(__LINE__) + " Cannot set year more than 99!\n";
    }   

    if (errorMessage.length() > 0) {
        Serial.println(errorMessage);
        return;
    }
    
    
    byte sendSecond = BitToBCD(second);
    byte sendMinute = BitToBCD(minute);
    byte sendHour = (!is12PM ? BitToBCD(hour) : BitToBCD(hour - (hour > 12 ? 12 : (hour < 1 ? -1 : 0))) | (hour > 12 ? 0xA0 : 0x80));
    byte sendDay = BitToBCD(day);
    byte sendDate = BitToBCD(date);
    byte sendMonth = BitToBCD(month);
    byte sendYear = BitToBCD(year);
    byte sendClockRegister = (WriteProtect ? 0x80 : 0x00);

    RTCwriteBurst(sendSecond, sendMinute, sendHour, sendDay, sendDate, sendMonth, sendYear, sendClockRegister);
}

void ClearDS1302::set::HourFormat(bool isHourFormat12Pm) {
    byte rawHour = RTCread(0x84);
    RTCwrite(0x84, ((rawHour & 0x7f) | (isHourFormat12Pm ? 0x80 : 0x00)));
}

void ClearDS1302::set::time::second(int second) {
    if(second > 59) {
        errorCallback(__FILE__, __LINE__, "Cannot set second more than 59!");
        return;
    }
    if(second < 0) {
        errorCallback(__FILE__, __LINE__, "Cannot set second less than 0!");
        return;
    }
    RTCwrite(0x80, BitToBCD(second));
}

void ClearDS1302::set::time::minutes(int minutes) {
    if(minutes > 59) {
        errorCallback(__FILE__, __LINE__, "Cannot set minute more than 59!");
        return;
    }
    if(minutes < 0) {
        errorCallback(__FILE__, __LINE__, "Cannot set minute less than 0!");
        return;
    }
    RTCwrite(0x82, BitToBCD(minutes));
}

void ClearDS1302::set::time::hour(int hour) {
    bool is12PM = (RTCread(0x84) & 0x80 ? true : false);
    if(hour > 23) {
        errorCallback(__FILE__, __LINE__, "Cannot set hour more than 23!");
        return;
    }
    if(hour < 0) {
        errorCallback(__FILE__, __LINE__, "Cannot set hour less than 0!");
        return;
    }
    (!is12PM ? RTCwrite(0x84, BitToBCD(hour) | 0x00) : RTCwrite(0x84, BitToBCD(hour - (hour > 12 ? 12 : (hour < 1 ? -1 : 0))) | (hour > 12 ? 0xA0 : 0x80)));
}

void ClearDS1302::set::time::day(int day) {
    if(day > 7) {
        errorCallback(__FILE__, __LINE__, "Cannot set day more than 7!");
        return;
    }
    if(day < 1) {
        errorCallback(__FILE__, __LINE__, "Cannot set day less than 1!");
        return;
    }
    RTCwrite(0x8A, BitToBCD(day));
}

void ClearDS1302::set::time::month(int month) {
    if(month > 12) {
        errorCallback(__FILE__, __LINE__, "Cannot set month more than 12!");
        return;
    }
    if(month < 1) {
        errorCallback(__FILE__, __LINE__, "Cannot set month less than 1!");
        return;
    }
    RTCwrite(0x88, BitToBCD(month));
}

void ClearDS1302::set::time::year(int year) {
    if(year > 99) {
        errorCallback(__FILE__, __LINE__, "Cannot set year more than 99!");
        return;
    }
    if(year < 0) {
        errorCallback(__FILE__, __LINE__, "Cannot set year less than 0!");
        return;
    }
    RTCwrite(0x8C, BitToBCD(year));
}

void ClearDS1302::set::time::date(int date) {
    if(date > 31) {
        errorCallback(__FILE__, __LINE__, "Cannot set date more than 31!");
        return;
    }
    if(date < 1) {
        errorCallback(__FILE__, __LINE__, "Cannot set date less than 1!");
        return;
    }
    RTCwrite(0x86, BitToBCD(date));
}

int* ClearDS1302::get::BatteryCharging() {
    static int data[3];
    byte rawTrickleCharger = RTCread(0x91);

    data[0] = (rawTrickleCharger & 0x80 ? (rawTrickleCharger & 0x20 ? 1 : 0) : 0);
    data[1] = (rawTrickleCharger & 0x04 ? (rawTrickleCharger & 0x08 ? 2 : 0) : (rawTrickleCharger & 0x08 ? 1 : 0));
    data[2] = (rawTrickleCharger & 0x01 ? (rawTrickleCharger & 0x02 ? 3 : 1) : 2);

    return data;
}

bool ClearDS1302::get::HourFormat() {
    return ((RTCread(0x84) & 0x80) ? true : false);
}

bool ClearDS1302::get::WriteProtect() {
    return ((RTCread(0x8F) & 0x80) ? true : false);
}

byte* ClearDS1302::get::time::GetAll() {
    static byte* data = RTCreadBurst();
    for (int i = 0; i < 8; i++) {
        data[i] = BCDtoBit(data[i]);
    }
    return(data);
}

byte ClearDS1302::get::time::second() {
    return BCDtoBit(RTCread(0x80));
}

byte ClearDS1302::get::time::minutes() {
    return BCDtoBit(RTCread(0x82));
}

String ClearDS1302::get::time::hour() {
    byte rawHour = RTCread(0x84);
    bool is12PMmode = (rawHour & 0x80 ? true : false);
    bool isPM = ((rawHour & 0x20) ? true : false);
    
    return String(BCDtoBit(rawHour & 0x1F)) + (is12PMmode ? (isPM ? "PM" : "AM") : "");
}

byte ClearDS1302::get::time::day() {
    return BCDtoBit(RTCread(0x8A));
}

byte ClearDS1302::get::time::month() {
    return BCDtoBit(RTCread(0x88));
}

byte ClearDS1302::get::time::year() {
    return BCDtoBit(RTCread(0x8C));
}

byte ClearDS1302::get::time::date() {
    return BCDtoBit(RTCread(0x86));
}


void ClearDS1302::print::BatteryCharging() {
    int data[3];
    byte rawTrickleCharger = RTCread(0x91);

    data[0] = (rawTrickleCharger & 0x80 ? (rawTrickleCharger & 0x20 ? 1 : 0) : 0);
    data[1] = (rawTrickleCharger & 0x04 ? (rawTrickleCharger & 0x08 ? 2 : 0) : (rawTrickleCharger & 0x08 ? 1 : 0));
    data[2] = (rawTrickleCharger & 0x01 ? (rawTrickleCharger & 0x02 ? 3 : 1) : 2);

    Serial.print("BatteryCharging: ");
    Serial.print(data[0] ? "ON" : "OFF");
    Serial.print(" | FastCharging: ");
    Serial.print(data[1] ? "YES" : "NO");
    Serial.print(" | ResistorLevel: ");
    Serial.println(data[2]);
}

void ClearDS1302::print::HourFormat() {
    Serial.println(((RTCread(0x84) & 0x80) ? true : false));
}

void ClearDS1302::print::WriteProtect() {
    Serial.println(((RTCread(0x8F) & 0x80) ? true : false));
}

void ClearDS1302::print::time::PrintAll() {
    int data[6];
    byte rawHour = RTCread(0x84);
    bool is12PMmode = (rawHour & 0x80 ? true : false);
    bool isPM = ((rawHour & 0x20) ? true : false);

    data[0] = BCDtoBit(RTCread(0x80));
    data[1] = BCDtoBit(RTCread(0x82));
    data[2] = rawHour;
    data[3] = BCDtoBit(RTCread(0x86));
    data[4] = BCDtoBit(RTCread(0x88));
    data[5] = BCDtoBit(RTCread(0x8A));
    data[6] = BCDtoBit(RTCread(0x8C));

    String Hour = String(BCDtoBit(rawHour & 0x1F)) + (is12PMmode ? (isPM ? "PM" : "AM") : "");

    String day = String(
        data[5] == 1 ? "Sunday" : 
        data[5] == 2 ? "Monday" : 
        data[5] == 3 ? "Tuesday" : 
        data[5] == 4 ? "Wednesday" : 
        data[5] == 5 ? "Thursday" : 
        data[5] == 6 ? "Friday" : 
        data[5] == 7 ? "Saturday" : 
       "Invalid");

    Serial.println(String(data[6]) + "/" + 
    String(data[4]) + "/" + 
    String(data[3]) + " " + 
    day + " " + 
    Hour + ":" + 
    String(data[1]) + ":" + 
    String(data[0]));
}

void ClearDS1302::print::time::second() {
    Serial.println(BCDtoBit(RTCread(0x80)));
}

void ClearDS1302::print::time::minutes() {
    Serial.println(BCDtoBit(RTCread(0x82)));
}

void ClearDS1302::print::time::hour() {
    byte rawHour = RTCread(0x84);
    bool is12PMmode = (rawHour & 0x80 ? true : false);
    bool isPM = ((rawHour & 0x20) ? true : false);
    
    Serial.println(String(BCDtoBit(rawHour & 0x1F)) + (is12PMmode ? (isPM ? "PM" : "AM") : ""));
}

void ClearDS1302::print::time::day() {
    Serial.println(BCDtoBit(RTCread(0x8A)));
}

void ClearDS1302::print::time::month() {
    Serial.println(BCDtoBit(RTCread(0x88)));
}

void ClearDS1302::print::time::year() {
    Serial.println(BCDtoBit(RTCread(0x8C)));
}

void ClearDS1302::print::time::date() {
    Serial.println(BCDtoBit(RTCread(0x86)));
}


void ClearDS1302::settings::AAA__CommingSoon() {
    Serial.println("I told you already this is Comming Soon");

}
