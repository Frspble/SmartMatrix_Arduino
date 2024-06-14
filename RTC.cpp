#include <Arduino.h>
#include <ctime>
#include "DS1302.h"
#include "common.h"
#include "time.h"

DS1302 rtc;

tm create_tm(int year, int month, int day, int hour, int minute, int second, int week) {
  tm t;
  t.tm_year = year - 1900;
  t.tm_mon = month - 1;
  t.tm_mday = day;
  t.tm_hour = hour;
  t.tm_min = minute;
  t.tm_sec = second;
  t.tm_wday = week;
  return t;
  time_t currentTime = time_t(); // 获取当前时间戳
}

// 设置系统时间
void setSysTime(int year, int month, int day, int hour, int minute, int second) {
  tm t = create_tm(year, month, day, hour, minute, second, 0);
  time_t t_of_day = mktime(&t);
  struct timeval tv = {.tv_sec = t_of_day};
  settimeofday(&tv, NULL);
}

// 将RTC时间转换为系统时间
bool RTCtoSysTime() {
  setenv("TZ", "CST-8", 1); // 设置时区
  tzset();
  tm t = rtc.time();
  if (t.tm_year < 124 || t.tm_year > 199) {
    return false;
  }
  time_t t_of_day = mktime(&t);
  struct timeval tv = {.tv_sec = t_of_day};
  settimeofday(&tv, NULL);
  return true;
}

// 初始化RTC
void initRTC() {
  rtc = DS1302(RTC_RST, RTC_DAT, RTC_CLK);
  rtc.writeProtect(true);
  rtc.halt(false);
}

// 设置RTC时间
void setRTCtime(int year, int month, int day, int hour, int minute, int second, int week) {
  rtc.writeProtect(false);
  tm t = create_tm(year, month, day, hour, minute, second, week);
  rtc.time(t);
  rtc.halt(false);
  rtc.writeProtect(true);
}

// 使用 tm 结构设置 RTC 时间
void setRTCtime(tm t) {
  rtc.writeProtect(false);
  rtc.time(t);
  rtc.halt(false);
  rtc.writeProtect(true);
}

// 使用时间戳设置 RTC 时间
void setRTCtime(int timestamp) {
  // 将时间戳转换为 tm 结构
  std::time_t rawtime = timestamp;
  std::tm *timeinfo = std::localtime(&rawtime);
  // 调用已经定义的 setRTCtime(tm t) 函数来设置 RTC 时间
  setRTCtime(*timeinfo);
}

// 获取RTC时间
tm getRTCtime() {
  tm t = rtc.time();
  return t;
}

// 打印RTC时间
void printTime() {
  tm t = rtc.time();
  Serial.println(&t, "%F %A %T");
  delay(1000);
}

