#ifndef __TASK_H
#define __TASK_H

#include "common.h"
#include "Ticker.h"

extern enum CurrentPage currentPage;
extern TaskHandle_t btnTask;
extern TaskHandle_t showTextTask;
extern TaskHandle_t showIpTask;
extern TaskHandle_t playBirthdaySongTask;
void watchBtn();
void createShowTextTask(const char *text);
void createBellTask();
void createPlaySongsTask();
void createPlayBirthdaySongTask();
void btnInit();
int32_t getClockRemainSeconds();
extern Ticker tickerClock;
extern Ticker tickerCheckTime;
extern Ticker tickerAnim;
extern bool isCheckingTime;
void startTickerClock(int32_t seconds);
void startTickerCheckTime();
extern bool birthdayBell;

#endif

