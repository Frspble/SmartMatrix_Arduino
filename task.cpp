#include <OneButton.h>
#include "Ticker.h"
#include "net.h"
#include "common.h"
#include "light.h"
#include "buzzer.h"
#include "preferencesUtil.h"
#include "BLE.h"

enum CurrentPage currentPage = SETTING;
bool playingMusic = false; // 是否正在播放音乐
bool belling = false; // 是否正在播放铃声
bool birthdayBell = false; // 是否正在播放生日歌
bool isCheckingTime = false; // 是否正在NTP对时中

void startShowText(void *param);
void startPlaySongs(void *param);
void startPlayBell(void *param);
void startPlayBirthdaySong(void *param);
void startTickerClock(int32_t seconds);
int32_t getClockRemainSeconds();
void btn1click();
void btn2click();
void btn3click();
void btn1LongClick();
void btn2LongClick();
void btn3LongClick();
void createPlaySongsTask();
void createBellTask();
void createPlayBirthdaySongTask();
void cancelBell();

///////////////////////////////////Freertos区域///////////////////////////////////////
// 按钮
OneButton button1(BTN1, true);
OneButton button2(BTN2, true);
OneButton button3(BTN3, true);
//闹钟响铃任务句柄
TaskHandle_t bellTask;
//播放歌曲的任务句柄(闹钟页面使用)
TaskHandle_t playSongsTask;
//播放生日歌的任务句柄
TaskHandle_t playBirthdaySongTask;
//系统启动动画的任务句柄
TaskHandle_t showTextTask;

//创建闹铃任务
void createBellTask(){
  xTaskCreate(startPlayBell, "startPlayBell", 1000, NULL, 1, &bellTask);
}
//创建音乐播放任务(闹钟页面使用)
void createPlaySongsTask(){
  xTaskCreate(startPlaySongs, "startPlaySongs", 1000, NULL, 1, &playSongsTask);
}
//创建生日歌播放任务
void createPlayBirthdaySongTask(){
  xTaskCreate(startPlayBirthdaySong, "startPlayBirthdaySong", 4096, NULL, 1, &playBirthdaySongTask);
}
//创建文字显示任务
void createShowTextTask(const char *text){
  xTaskCreate(startShowText, "startShowText", 1000, (void *)text, 1, &showTextTask);
}

// 启动文字显示
void startShowText(void *param){
  String text = (char *)param;
  int x;
  if(text.equals("START")){
    x = 2;
  }else if(text.equals("CONFIG")){
    x = 0;
  }else if(text.equals("BLE")){
    x = 7;
  }
  int index = 0;
  while (true) {
    if(index % 3 == 0){
      drawText(x, 6, text + ".");
    }else if(index % 3 == 1){
      drawText(x, 6, text + "..");
    }else if(index % 3 == 2){
      drawText(x, 6, text + "...");
    }
    vTaskDelay(1000);
    index++;
  }
  Serial.println("退出启动文字显示");
}

// 播放歌曲(闹钟页面使用)
void startPlaySongs(void *param){
  while (true) {
    playSong(false);
    vTaskDelay(3000);
  }
  Serial.println("退出音乐播放");
}
// 响铃
void startPlayBell(void *param){
  while (true) {
    playSong(true);
    vTaskDelay(3000);
  }
  Serial.println("退出响铃任务");
}
// 生日歌播放
void startPlayBirthdaySong(void *param){
  while (true) {
    playBirthdaySong();
    vTaskDelay(3000);
  }
  Serial.println("退出生日歌播放");
}

//////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////定时器区域///////////////////////////////////////
Ticker tickerClock;
Ticker tickerCheckTime;
// NTP对时
void checkTime(){
  // 只要将对时标志置为true即可，在主循环中进行对时操作
  isCheckingTime = true;
}
// 根据NVS中的闹钟时间计算出定时器需要多久之后触发
int32_t getClockRemainSeconds(){
  // 获取RTC时间
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)){
    Serial.println("获取RTC时间失败");
    return -1;
  }
  int32_t nowTime = (timeinfo.tm_hour * 60 + timeinfo.tm_min) * 60 + timeinfo.tm_sec;
  int32_t clockTime = (clockH * 60 + clockM) * 60;
  if(clockTime > nowTime){ // 闹钟时间在当天
    return (clockTime - nowTime);
  }else{ // 闹钟时间在第二天
    return (clockTime + ONE_DAY_SECONDS - nowTime);
  }
}
// 奏响闹铃
void ringingBell() {
  // 配置页面和闹钟设置页面不响铃
  if(currentPage == SETTING || currentPage == CLOCK){
    return;
  }
  // 进入时钟页面
  if(currentPage == RHYTHM){
    // 将二维数组重置为0
    memset(matrixArray,0,sizeof(matrixArray));
    // 将已点亮灯的个数置零
    lightedCount = 0;
  }
  currentPage = TIME;
  drawTimeFirstTime = true;
  // 创建播放音乐的任务
  createBellTask();
  belling = true;
  // 将定时器重置
  tickerClock.detach();
  startTickerClock(getClockRemainSeconds());
}
// 开启定时器
void startTickerClock(int32_t seconds){
  // seconds秒后，执行一次
  tickerClock.once(seconds, ringingBell);
}
void startTickerCheckTime(){
  // 每隔一段时间进行一次NTP对时
  tickerCheckTime.attach(TIME_CHECK_INTERVAL, checkTime);
}
// 在闹钟响铃时，任何按键按下，都取消闹铃
void cancelBell(){
  vTaskDelete(bellTask);
  delay(300);
  belling = false;
}

//////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////// 按键区////////////////////////////////////////
// 初始化各按键
void btnInit(){
  button1.attachClick(btn1click);
  button1.setDebounceMs(10); //设置消抖时长 
  button2.attachClick(btn2click);
  button2.setDebounceMs(10); //设置消抖时长 
  button3.attachClick(btn3click);
  button3.setDebounceMs(10); //设置消抖时长 
  button1.attachLongPressStart(btn1LongClick);
  button1.setPressMs(1200); //设置长按时间
  button2.attachLongPressStart(btn2LongClick);
  button2.setPressMs(1200); //设置长按时间
  button3.attachLongPressStart(btn3LongClick);
  button3.setPressMs(1200); //设置长按时间
}
// 监控按键
void watchBtn(){
  button1.tick();
  button2.tick();
  button3.tick();
}
// 按键方法
// >按键
void btn1click(){
  if(belling){
    cancelBell();
    return;
  }
  if (playBirthdaySongTask != NULL) {   // 在播放生日歌时，任何按键按下，都取消播放，并且后续不再播放
    vTaskDelete(playBirthdaySongTask);
    playBirthdaySongTask = NULL;
    return;
  }
  switch(currentPage){
    case TIME:
      if(timePage == TIME_H_M_S){
        timePage = TIME_H_M;
      }else if(timePage == TIME_H_M){
        timePage = TIME_DATE;
      }else{
        timePage = TIME_H_M_S;
      }
      // 清屏
      clearMatrix();
      // 保存设置
      recordExtensionPage();
      // 绘制时间
      drawTimeFirstTime = true;
      drawTime();
      break;
    case RHYTHM:
      if(rhythmPage == RHYTHM_MODEL1){
        rhythmPage = RHYTHM_MODEL2;
      }else if(rhythmPage == RHYTHM_MODEL2){
        rhythmPage = RHYTHM_MODEL3;
      }else if(rhythmPage == RHYTHM_MODEL3){
        rhythmPage = RHYTHM_MODEL4;
      }else if(rhythmPage == RHYTHM_MODEL4){
        rhythmPage = RHYTHM_MODEL1;
      }
      // 保存设置
      recordExtensionPage();
      break;
    case ANIM:
      if(animPage == ANIM_MODEL1){
        animPage = ANIM_MODEL2;
      }else if(animPage == ANIM_MODEL2){
        // 将已点亮灯的个数置零
        lightedCount = 0;
        animPage = ANIM_MODEL3;
      }else if(animPage == ANIM_MODEL3){       
        animPage = ANIM_MODEL1;
      }
      // 将二维数组重置为0
      memset(matrixArray,0,sizeof(matrixArray));
      // 保存设置
      recordExtensionPage();
      // 清屏
      clearMatrix();
      break;
    case CLOCK:
      if(!clockOpen){
        return;
      }
      if(clockChoosed == CLOCK_H){
        tmpClockH++;
        if(tmpClockH == 24){
          tmpClockH = 0;
        }
        drawClock();
      }else if(clockChoosed == CLOCK_M){
        tmpClockM++;
        if(tmpClockM == 60){
          tmpClockM = 0;
        }
        drawClock();
      }else if(clockChoosed == CLOCK_BELL){
        tmpClockBellNum++;
        if(tmpClockBellNum == songCount){
          tmpClockBellNum = 0;
        }
        // 停止音乐播放
        vTaskDelete(playSongsTask);
        delay(300);
        // 开始音乐播放
        createPlaySongsTask();       
      }
      break;
    case BRIGHT:
      if(brightModel == BRIGHT_MODEL_AUTO) return;
      if(brightness + BRIGHTNESS_SPACING > MAX_BRIGHTNESS){
        brightness = MAX_BRIGHTNESS;
        recordBrightness();
        setBLEBrightness(brightness);
        drawBright();
        Serial.println("已达最大亮度");
        return;
      }
      brightness+=BRIGHTNESS_SPACING;
      Serial.print("当前亮度：");Serial.println(brightness);
      recordBrightness();
      setBLEBrightness(brightness);
      drawBright();
      break;
    case ANNIVERSARY:
      if(anniversaryPage == ANNIVERSARY_A){
        anniversaryPage = ANNIVERSARY_B;
      }else if (anniversaryPage == ANNIVERSARY_B){
        anniversaryPage = ANNIVERSARY_A;
      }
    default:
      break;
  }
}
// <按键
void btn2click(){
  if(belling){
    cancelBell();
    return;
  }
  if (playBirthdaySongTask != NULL) {
    vTaskDelete(playBirthdaySongTask);
    playBirthdaySongTask = NULL;
    return;
    }
  switch(currentPage){
    case TIME:     
      if(timePage == TIME_H_M_S){
        timePage = TIME_DATE;
      }else if(timePage == TIME_H_M){
        timePage = TIME_H_M_S;
      }else{
        timePage = TIME_H_M;
      }
      // 清屏
      clearMatrix();
      // 保存设置
      recordExtensionPage();
      // 绘制时间
      drawTimeFirstTime = true;
      drawTime();
      break;
    case RHYTHM:
      if(rhythmPage == RHYTHM_MODEL1){
        rhythmPage = RHYTHM_MODEL4;
      }else if(rhythmPage == RHYTHM_MODEL2){
        rhythmPage = RHYTHM_MODEL1;
      }else if(rhythmPage == RHYTHM_MODEL3){
        rhythmPage = RHYTHM_MODEL2;
      }else if(rhythmPage == RHYTHM_MODEL4){
        rhythmPage = RHYTHM_MODEL3;
      }
      // 保存设置
      recordExtensionPage();
      break;
    case ANIM:
      if(animPage == ANIM_MODEL1){
        // 将已点亮灯的个数置零
        lightedCount = 0;
        animPage = ANIM_MODEL3;
      }else if(animPage == ANIM_MODEL2){
        animPage = ANIM_MODEL1;
      }else if(animPage == ANIM_MODEL3){
        animPage = ANIM_MODEL2;
      }
      // 将二维数组重置为0
      memset(matrixArray,0,sizeof(matrixArray));
      // 保存设置
      recordExtensionPage();
      // 清屏
      clearMatrix();
      break;
    case CLOCK:
      if(!clockOpen){
        return;
      }
      if(clockChoosed == CLOCK_H){
        tmpClockH--;
        if(tmpClockH == -1){
          tmpClockH = 23;
        }
        drawClock();
      }else if(clockChoosed == CLOCK_M){
        tmpClockM--;
        if(tmpClockM == -1){
          tmpClockM = 59;
        }
        drawClock();
      }else if(clockChoosed == CLOCK_BELL){
        tmpClockBellNum--;
        if(tmpClockBellNum == -1){
          tmpClockBellNum = (songCount - 1);
        }
        // 停止音乐播放
        vTaskDelete(playSongsTask);
        delay(300);
        // 开始音乐播放
        createPlaySongsTask();
      }
      break;
    case BRIGHT:
      if(brightModel == BRIGHT_MODEL_AUTO) return;
      if(brightness <= MIN_BRIGHTNESS + BRIGHTNESS_SPACING){
        brightness = MIN_BRIGHTNESS;
        recordBrightness();
        setBLEBrightness(brightness);
        drawBright();
        Serial.println("已达最小亮度");
        return;
      }
      brightness -= BRIGHTNESS_SPACING;
      Serial.print("当前亮度：");Serial.println(brightness);
      recordBrightness();
      setBLEBrightness(brightness);
      drawBright();
      break;
    case ANNIVERSARY:
      if(anniversaryPage == ANNIVERSARY_A){
        anniversaryPage = ANNIVERSARY_B;
      }else if (anniversaryPage == ANNIVERSARY_B){
        anniversaryPage = ANNIVERSARY_A;
      }
    default:
      break;
  }
}
void btn3click(){
  if(belling){
    cancelBell();
    return;
  }
  if (playBirthdaySongTask != NULL) {
    vTaskDelete(playBirthdaySongTask);
    playBirthdaySongTask = NULL;
    return;
  }
  switch(currentPage){
    case SETTING:
      // 关闭等待蓝牙连接的动画任务
      if (showTextTask != NULL && !RTCSuccess) {
        vTaskDelete(showTextTask);
        showTextTask = NULL;
        delay(300);
        // 清空屏幕
        clearMatrix();
        // 进入节奏灯界面
        currentPage = RHYTHM;
      }
      break;
    case TIME:
      // 清屏
      clearMatrix();
      currentPage = RHYTHM;
      break;
    case RHYTHM:
      // 将二维数组重置为0
      memset(matrixArray,0,sizeof(matrixArray));
      // 将已点亮灯的个数置零
      lightedCount = 0;
      // 清屏
      clearMatrix();
      currentPage = ANIM;
      break;
    case ANIM:
      if (!RTCSuccess){
        // 至亮度调节页面
        currentPage = BRIGHT;
        drawBright();
      }else{
        currentPage = CLOCK;
        resetTmpClockData();
        drawClock();
      }  
      break;
    case CLOCK:
      // 保存当前设置的闹钟信息
      clockH = tmpClockH;
      clockM = tmpClockM;
      clockBellNum = tmpClockBellNum;
      // 记录闹钟配置信息
      recordClockPage();
      // 将定时器重置
      tickerClock.detach();
      if(clockOpen){
        startTickerClock(getClockRemainSeconds());
      }
      // 如果正在播放音乐，则停止音乐播放
      if(playingMusic){
        vTaskDelete(playSongsTask);
        delay(300);
      }
      playingMusic = false;
      currentPage = BRIGHT;
      drawBright();
      break;
    case BRIGHT:
      if (anniversaryOpen){
        currentPage = ANNIVERSARY;
      } else {
        currentPage = TIME;
      }
      break; 
    case ANNIVERSARY:
      if (!RTCSuccess){
        // 至节奏灯页面
        currentPage = RHYTHM;
      }else{
        currentPage = TIME;
        drawTimeFirstTime = true;
      }
      break;
    default:
      break;
  }
}
void btn1LongClick(){
  if(belling){
    cancelBell();
    return;
  }
  if(currentPage != CLOCK || !clockOpen){
    return;
  }
  if(clockChoosed == CLOCK_H){
    clockChoosed = CLOCK_M;
    drawClock();
  }else if(clockChoosed == CLOCK_M){
    clockChoosed = CLOCK_BELL;
    drawClock();
    // 演奏当前铃声
    createPlaySongsTask();
    playingMusic = true;
  }else if(clockChoosed == CLOCK_BELL){
    // 停止演奏
    vTaskDelete(playSongsTask);
    delay(300);
    playingMusic = false;
    clockChoosed = CLOCK_H;
    drawClock();
  }
}
void btn2LongClick(){
  if(belling){
    cancelBell();
    return;
  }
  if(currentPage != CLOCK || !clockOpen){
    return;
  }
  if(clockChoosed == CLOCK_H){
    clockChoosed = CLOCK_BELL;
    drawClock();
    // 演奏当前铃声
    createPlaySongsTask();
    playingMusic = true;
  }else if(clockChoosed == CLOCK_M){
    clockChoosed = CLOCK_H;
    drawClock();
  }else if(clockChoosed == CLOCK_BELL){
    // 停止演奏
    vTaskDelete(playSongsTask);
    delay(300);
    playingMusic = false;
    clockChoosed = CLOCK_M;
    drawClock();
  }
}
void btn3LongClick(){
  if(belling){
    cancelBell();
    return;
  }
  if(currentPage == SETTING){
    if (advertising && !BLEConnected) {
      if (RTCSuccess){
        disconnectBLE();
        stopAdvertising();
        if (showTextTask != NULL) {
          vTaskDelete(showTextTask);
          showTextTask = NULL;
        }
        clearMatrix();
        drawFailed(6, 21, "BLE");
        currentPage = TIME;
      }
    }
    return;
  }
  if(currentPage == CLOCK){ // 闹钟页面长按，关闭/开启闹钟
    if(clockChoosed == CLOCK_BELL){
      if(playingMusic){
        // 停止演奏
        vTaskDelete(playSongsTask);
        delay(300);
        playingMusic = false;
      }else{
        // 演奏当前铃声
        createPlaySongsTask();
        playingMusic = true;
      }
    }
    clockOpen = !clockOpen;
    drawClock();
    recordClockPage();
  }else if(currentPage == RHYTHM){ // 节奏灯页面长按，切换频段模式
    if(rhythmBandsModel == RHYTHM_BANDS_MODEL1){
      rhythmBandsModel = RHYTHM_BANDS_MODEL2;
    }else{
      rhythmBandsModel = RHYTHM_BANDS_MODEL1;
    }
    // 将相关数组重置为0
    memset(peak,0,NUM_BANDS);
    memset(oldBarHeights,0,NUM_BANDS);
    // 保存新模式
    recordExtensionPage();
  }else if(currentPage == BRIGHT){ // 亮度页面长按，切换亮度调节模式
    if(brightModel == BRIGHT_MODEL_MANUAL){
      brightModel = BRIGHT_MODEL_AUTO;
      setBLEBrightness(brightness);
      // 重置采样值
      clearBrightSampling();
    }else{
      brightModel = BRIGHT_MODEL_MANUAL;
      setBLEBrightness(brightness);
    }
    // 保存新模式
    recordExtensionPage();
    // 重新读取NVS中的亮度值
    getBrightness();
    // 刷新页面
    drawBright();
  }else if(currentPage == TIME){ // 时钟页面长按，开关蓝牙
    if (advertising || BLEConnected){
      clearMatrix();
      drawFailed(6, 21, "BLE");
      disconnectBLE();
      stopAdvertising();
      currentPage = SETTING;
    } else if (!advertising && !BLEConnected) {
      currentPage = SETTING;
      startAdvertising();
      createShowTextTask("BLE");
    } else if (advertising && !BLEConnected) {
      if (RTCSuccess){
        disconnectBLE();
        stopAdvertising();
        if (showTextTask != NULL) {
          vTaskDelete(showTextTask);
          showTextTask = NULL;
        }
        clearMatrix();
        drawFailed(6, 21, "BLE");
        currentPage = TIME;
      }
    }
  } 
}
///////////////////////////////////////////////////////////////

