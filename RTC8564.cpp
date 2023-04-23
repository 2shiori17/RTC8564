#include "RTC8564.h"

// -----------------------------------------------------------------------------
//   utils
// -----------------------------------------------------------------------------

inline int toBCD(int decimal) {
  return (((decimal / 10) << 4) | (decimal % 10));
}

inline int fromBCD(int bcd) {
  return ((bcd >> 4) * 10 + (bcd & 0x0f));
}

// -----------------------------------------------------------------------------
//   DateTime
// -----------------------------------------------------------------------------

void DateTime::encode(uint8_t *data) {
  data[0] = toBCD(second);
  data[1] = toBCD(minute);
  data[2] = toBCD(hour);
  data[3] = toBCD(day);
  data[4] = toBCD(weekday);
  data[5] = toBCD(month);
  if (year > 100) {
    data[6] = toBCD(year - 100);
    data[5] |= RTC8564_CAL_CENTURY;
  } else {
    data[6] = toBCD(year);
  }
}

int DateTime::decode(uint8_t *data) {
  if (data[0] & RTC8564_CAL_VL) {
    return -1;
  }

  second  = fromBCD(data[0] & 0x7f);
  minute  = fromBCD(data[1] & 0x7f);
  hour    = fromBCD(data[2] & 0x3f);
  day     = fromBCD(data[3] & 0x3f);
  weekday = fromBCD(data[4] & 0x07);
  month   = fromBCD(data[5] & 0x1f);
  year    = fromBCD(data[6]);

  if (data[5] & RTC8564_CAL_CENTURY) {
    year += 100;
  }

  return 0;
}

// -----------------------------------------------------------------------------
//   RTC8564Class
// -----------------------------------------------------------------------------

void RTC8564Class::begin(struct DateTime *dt) {
  Wire.begin();
  initialize(dt);
}

// 13.5. フローチャート
// (1) 初期化例, (2) 電源投入時の処理例, (3) バックアップ状態から復帰したときの処理例
void RTC8564Class::initialize(struct DateTime *dt) {
  uint8_t vl;
  uint8_t control[2];
  struct AlarmTime at = {0, 0, 0, 0};

  getRegisters(RTC8564_SECONDS, 1, &vl);

  if (vl & RTC8564_CAL_VL) {
    delay(1000);

    control[0] = RTC8564_STOP_BIT, control[1] = 0x00;
    setRegisters(RTC8564_CONTROL1, 2, control);
    setDateTime(dt);
    setAlarm(RTC8564_AE_NONE, &at, false);
    setClkoutFrequency(false, RTC8564_CLKOUT_32768HZ);
    setTimer(false, false, RTC8564_CLK_244US, 0, false);
  }
}

// 13.5. フローチャート
// (4) 時計・カレンダの書き込み例
void RTC8564Class::setDateTime(struct DateTime *dt) {
  uint8_t data[7];

  data[0] = RTC8564_STOP_BIT;
  setRegisters(RTC8564_CONTROL1, 1, data);

  dt->encode(data);
  setRegisters(RTC8564_SECONDS, 7, data);

  data[0] = 0x00;
  setRegisters(RTC8564_CONTROL1, 1, data);
}

// 13.5. フローチャート
// (5) 時計・カレンダの読み出し例
int RTC8564Class::getDateTime(struct DateTime *dt) {
  uint8_t data[7];

  getRegisters(RTC8564_SECONDS, 7, data);
  return dt->decode(data);
}

void RTC8564Class::setAlarm(uint8_t enableFlags, struct AlarmTime *at, uint8_t interruptEnable) {
  uint8_t control2;
  uint8_t data[4];

  // Read Control2 register
  getRegisters(RTC8564_CONTROL2, 1, &control2);

  control2 &= (~RTC8564_AIE_BIT);
  setRegisters(RTC8564_CONTROL2, 1, &control2);

  data[0] = (enableFlags & RTC8564_AE_MINUTE)  ? toBCD(at->minute)  : RTC8564_AE_BIT;
  data[1] = (enableFlags & RTC8564_AE_HOUR)    ? toBCD(at->hour)    : RTC8564_AE_BIT;
  data[2] = (enableFlags & RTC8564_AE_DAY)     ? toBCD(at->day)     : RTC8564_AE_BIT;
  data[3] = (enableFlags & RTC8564_AE_WEEKDAY) ? toBCD(at->weekday) : RTC8564_AE_BIT;
  setRegisters(RTC8564_MINUTE_ALARM, 4, data);

  if (interruptEnable) {
    control2 |= RTC8564_AIE_BIT;
  } else {
    control2 &= ~RTC8564_AIE_BIT;
  }
  setRegisters(RTC8564_CONTROL2, 1, &control2);
}

void RTC8564Class::getAlarm(uint8_t *enableFlags, struct AlarmTime *at) {
  uint8_t data[4];

  getRegisters(RTC8564_MINUTE_ALARM, 4, data);

  at->minute  = fromBCD(data[0] & 0x7f);
  at->hour    = fromBCD(data[1] & 0x3f);
  at->day     = fromBCD(data[2] & 0x3f);
  at->weekday = fromBCD(data[3] & 0x07);

  *enableFlags = 0;
  for (int i = 0; i < 4; i++) {
    if (!(data[i] & RTC8564_AE_BIT)) {
      *enableFlags |= (1 << i);
    }
  }
}

int RTC8564Class::getAlarmFlag() {
  uint8_t control2;

  getRegisters(RTC8564_CONTROL2, 1, &control2);

  return (control2 & RTC8564_AF_BIT) ? true : false;
}

void RTC8564Class::clearAlarmFlag() {
  uint8_t control2;

  getRegisters(RTC8564_CONTROL2, 1, &control2);
  control2 &= ~RTC8564_AF_BIT;
  setRegisters(RTC8564_CONTROL2, 1, &control2);
}

// 13.5. フローチャート
// (6) タイマ割り込み機能の設定例
void RTC8564Class::setTimer(uint8_t enableFlag, uint8_t repeatMode, uint8_t clockMode, uint8_t counter, uint8_t interruptEnable) {
  uint8_t control2;
  uint8_t data[4];

  data[0] = 0;
  setRegisters(RTC8564_TIMER_CONTROL, 1, data);

  if (enableFlag) {
    getRegisters(RTC8564_CONTROL2, 1, &control2);
    control2 &= ~(RTC8564_TITP_BIT | RTC8564_TF_BIT | RTC8564_TIE_BIT);
    setRegisters(RTC8564_CONTROL2, 1, &control2);

    if (repeatMode) {
      control2 |= RTC8564_TITP_BIT;
    }

    if (interruptEnable) {
      control2 |= RTC8564_TIE_BIT;
    }

    setRegisters(RTC8564_CONTROL2, 1, &control2);
    setRegisters(RTC8564_TIMER, 1, &counter);

    clockMode |= RTC8564_TE_BIT;
    setRegisters(RTC8564_TIMER_CONTROL, 1, &clockMode);
  }
}

int RTC8564Class::getTimerFlag() {
  uint8_t control2;

  getRegisters(RTC8564_CONTROL2, 1, &control2);

  return (control2 & RTC8564_TF_BIT) ? true : false;
}

void RTC8564Class::clearTimerFlag() {
  uint8_t control2;

  getRegisters(RTC8564_CONTROL2, 1, &control2);
  control2 &= ~RTC8564_TF_BIT;
  setRegisters(RTC8564_CONTROL2, 1, &control2);
}

void RTC8564Class::setClkoutFrequency(uint8_t enableFlag, uint8_t frequency) {
  if (enableFlag) {
    frequency |= RTC8564_FE_BIT;
  } else {
    frequency &= ~RTC8564_FE_BIT;
  }

  setRegisters(RTC8564_CLKOUT_FREQUENCY, 1, &frequency);
}

// 13.6.6. I2C-BUSプロトコル
// (1) アドレス指定の書き込み手順
void RTC8564Class::setRegisters(uint8_t address, int length, uint8_t *data) {
  Wire.beginTransmission(RTC8564_I2C_ADDRESS);
  Wire.write(address);
  Wire.write(data, length);
  Wire.endTransmission();
}

// 13.6.6. I2C-BUSプロトコル
// (2) アドレス指定の読み出し手順
void RTC8564Class::getRegisters(uint8_t address, int length, uint8_t *data) {
  Wire.beginTransmission(RTC8564_I2C_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();

  Wire.requestFrom(RTC8564_I2C_ADDRESS, length);
  for (int i = 0; i < length; i++) {
    while(Wire.available() < 1) {
      ;
    }
    data[i] = Wire.read();
  }
}

RTC8564Class RTC8564;
