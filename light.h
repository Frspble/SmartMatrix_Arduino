#ifndef __LIGHT_H
#define __LIGHT_H

#include <Adafruit_NeoMatrix.h>

extern int clockColor[3];
extern int brightness;
extern uint16_t mainColor;
extern uint16_t weekColor;
extern Adafruit_NeoMatrix matrix;
extern int timePage;
extern int rhythmPage;
extern int animPage;
extern int anniversaryPage;
extern int rhythmBandsModel;
extern int timeModel;
extern int brightModel;
extern int brightSamplingTime;
extern int brightSamplingValue;
extern int matrixArray[8][32];
extern byte peak[];
extern int oldBarHeights[];
extern int lightedCount;
extern int clockH;
extern int clockM;
extern int clockBellNum;
extern int tmpClockH;
extern int tmpClockM;
extern int tmpClockBellNum;
extern bool clockOpen;
extern int clockChoosed;

extern bool birthdayOpen;
extern bool anniversaryOpen;
extern unsigned long birthday;
extern unsigned long anniversaryA;
extern unsigned long anniversaryB;
extern bool drawTimeFirstTime;
extern int timeIndex;
void initMatrix();
void drawText(int x, int y, String text);
void drawCheckTimeText();
void drawFailed(int textX, int failedX, String text);
void drawSuccess(int textX, int successX, String text);
void clearMatrix();
void drawTime();
void drawBright();
void drawAnim();
void drawRHYTHM();
void resetTmpClockData();
void drawClock();
void calculateBrightnessValue();
void clearBrightSampling();
void drawAnniversary();

#endif

