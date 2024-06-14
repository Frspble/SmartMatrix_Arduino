#ifndef RTC_H
#define RTC_H

void printTime();
void initRTC();
void setSysTime(int year, int month, int day, int hour, int minute, int second);
bool RTCtoSysTime();
void setRTCtime(int year, int month, int day, int hour, int minute, int second, int week);
void setRTCtime(tm t);
void setRTCtime(int timestamp);

#endif