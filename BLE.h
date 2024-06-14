#ifndef BLE_H
#define BLE_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "common.h"

extern bool BLEConnected;
extern bool advertising;

void initBLE();
void startAdvertising();
void stopAdvertising();
void disconnectBLE();
void sendBLEData(String data);
void setBLEBrightness(int brightness);
void setBLEColor(int r, int g, int b);
void setBLEAnniversary(bool open, unsigned long anniversaryA, unsigned long anniversaryB);
void setBLEBirthday(bool open, unsigned long birthday);
void setBLEWIFI(String ssid, String pass);
void setBLEAnim(int anim);


#endif // BLE_H