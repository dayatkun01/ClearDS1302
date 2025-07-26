// by DayatKun0
#ifndef ClearDS1302_H
#define ClearDS1302_H

#include <Arduino.h>

class ClearDS1302 {
    public:
    ClearDS1302(int DATpin, int RSTpin, int CLKpin);
    
    void reset();
    
    class settings {
            public:
            void AAA__CommingSoon();
            void AAWaitForNextUpdate_UwU();
            void PrintDebug(bool IsActive);
            void AutoTurnWriteProtectoff(bool IsActive);
            void DebugMode(bool IsActive);
        };settings settings;
        
        class print {
            public:
            void WriteProtect();
            void BatteryCharging();
            void HourFormat();
            
            class time {
                public:
                void second();
                void minutes();
                void hour();
                void day();
                void month();
                void year();
                void date();
                void PrintAll();
            };time time;
        };print print;
        
        class set {
            public:
            void WriteProtect(bool isActive);
            void BatteryCharging(bool IsActive, bool FastCharging, int ResistorLevel);
            void HourFormat(bool isHourFormat12PM);

                class time {
                    public:
                        void second(int second);
                        void minutes(int minute);
                        void hour(int hour);
                        void day(int day);
                        void month(int month);
                        void year(int year);
                        void date(int date);
                        void SetAll(int second, int minute, int hour, int day, int date, int month, int year, bool ClockRegister);
                };time time;
        };set set;

        class get {
            public:
                bool WriteProtect();
                int* BatteryCharging();
                bool HourFormat();

                class time {
                    public:
                        byte second();
                        byte minutes();
                        String hour();
                        byte day();
                        byte month();
                        byte year();
                        byte date();
                        byte* GetAll();
                };time time;
        };get get;

        class raw {
            public:
                void write(byte address, byte value);
                byte read(byte address);
                void WriteClockBurst(byte dataSecond, byte dataMinute, byte dataHour, byte dataDay, byte dataMonth, byte dataYear, byte dataDate, byte dataControlRegister);
                byte* ReadClockBurst();
        };raw raw;
};

#endif