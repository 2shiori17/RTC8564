#ifndef RTC8564_h
#define RTC8564_h

#include <Arduino.h>
#include <Wire.h>

// I2C Address
#define RTC8564_I2C_ADDRESS       0x51

// Registers
#define RTC8564_CONTROL1          0x00
#define RTC8564_CONTROL2          0x01
#define RTC8564_SECONDS           0x02
#define RTC8564_MINUTES           0x03
#define RTC8564_HOURS             0x04
#define RTC8564_DAYS              0x05
#define RTC8564_WEEKDAYS          0x06
#define RTC8564_MONTH_CENTURY     0x07
#define RTC8564_YEARS             0x08
#define RTC8564_MINUTE_ALARM      0x09
#define RTC8564_HOUR_ALARM        0x0a
#define RTC8564_DAY_ALARM         0x0b
#define RTC8564_WEEKDAY_ALARM     0x0c
#define RTC8564_CLKOUT_FREQUENCY  0x0d
#define RTC8564_TIMER_CONTROL     0x0e
#define RTC8564_TIMER             0x0f

// Control1 register
#define RTC8564_STOP_BIT          0x20

// Control2 register
#define RTC8564_TIE_BIT           0x01
#define RTC8564_AIE_BIT           0x02
#define RTC8564_TF_BIT            0x04
#define RTC8564_AF_BIT            0x08
#define RTC8564_TITP_BIT          0x10

// Calendar registers
#define RTC8564_CAL_VL            0x80
#define RTC8564_CAL_CENTURY       0x80

// Alarm register
#define RTC8564_AE_NONE           0x00
#define RTC8564_AE_MINUTE         0x01
#define RTC8564_AE_HOUR           0x02
#define RTC8564_AE_DAY            0x04
#define RTC8564_AE_WEEKDAY        0x08
#define RTC8564_AE_ALL            (RTC8564_AE_MINUTE | RTC8564_AE_HOUR | RTC8564_AE_DAY | RTC8564_AE_WEEKDAY)
#define RTC8564_AE_BIT            0x80

// Timer control register
#define RTC8564_CLK_244US         0x00
#define RTC8564_CLK_15MS          0x01
#define RTC8564_CLK_1SEC          0x02
#define RTC8564_CLK_1MIN          0x03
#define RTC8564_TE_BIT            0x80

// Clkout frequency register
#define RTC8564_CLKOUT_32768HZ    0x00
#define RTC8564_CLKOUT_1024HZ     0x01
#define RTC8564_CLKOUT_32HZ       0x02
#define RTC8564_CLKOUT_1HZ        0x03
#define RTC8564_FE_BIT            0x80

struct DateTime {
  uint8_t second;
  uint8_t minute;
  uint8_t hour;
  uint8_t day;
  uint8_t month;
  uint8_t year;
  uint8_t weekday;

  void encode(uint8_t *data);
  int decode(uint8_t *data);
};

struct AlarmTime {
  uint8_t minute;
  uint8_t hour;
  uint8_t day;
  uint8_t weekday;
};

class RTC8564Class {
  public:
    void begin(struct DateTime *dt);
    void initialize(struct DateTime *dt);

    void setDateTime(struct DateTime *dt);
    int getDateTime(struct DateTime *dt);

    void setAlarm(uint8_t enableFlags, struct AlarmTime *at, uint8_t interruptEnable);
    void getAlarm(uint8_t *enableFlags, struct AlarmTime *at);
    int getAlarmFlag();
    void clearAlarmFlag();

    void setTimer(uint8_t enableFlag, uint8_t repeatMode, uint8_t clockMode, uint8_t counter, uint8_t interruptEnable);
    int getTimerFlag();
    void clearTimerFlag();

    void setClkoutFrequency(uint8_t enableFlag, uint8_t flag);

  private:
    void setRegisters(uint8_t address, int numData, uint8_t *data);
    void getRegisters(uint8_t address, int numData, uint8_t *data);
};

extern RTC8564Class RTC8564;

#endif
