#ifndef __PreferencesUtil_H
#define __PreferencesUtil_H

void getInfos();
void recordInfos(String ssid,String pass,int r,int g,int b,bool apConfig);
void recordBrightness();
void getBrightness();
void setApConfigWhenStart(bool apConfig);
void recordExtensionPage();
void recordClockPage();
void recordWifiConfig();
void recordClockColor(int r,int g,int b);
void setBLEInfo();
void recordBirthday();
void recordAnniversary();
void recordAnim();

#endif
