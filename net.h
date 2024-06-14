#ifndef __NET_H
#define __NET_H
#include "common.h"

void connectWiFi(int timeOut_s);
void getNTPTime(void);
void checkTime(int limitTime);
void disConnectWifi();
void checkTimeTicker();
extern String ssid;
extern String pass;
extern bool wifiConnected;
extern bool apConfig;
extern bool RTCSuccess;

#endif