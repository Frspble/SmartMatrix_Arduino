#include <Arduino.h>
#include "common.h"
#include "preferencesUtil.h"
#include "light.h"
#include "buzzer.h"
#include "net.h"
#include "task.h"
#include "BLE.h"
#include "RTC.h"

/**
SmartMatrix像素时钟  版本 1.0

本项目基于 B 站 UP 主 "@大聪明的二手脑袋" 的 EasyMatrix v1.4 修改而来。

相比原版新增功能：
添加了蓝牙配置功能，可使用小程序调整亮度、时钟颜色、动画、对时等；
添加外置 DS1302 时钟模块，断电后也可以保持时间，不用每次启动时重新对时；
添加生日功能，生日当天会有特殊提醒，该功能默认关闭，可在小程序中进行开关和设置日期；
新增两首生日歌曲，生日当天会随机播放一首，并且将加入了闹钟列表，可作为闹钟铃声使用；
添加纪念日界面，该功能默认关闭，同样可在小程序中进行开关和设置日期。
删除原版 AP 模式配网相关代码，减少不必要的占用。

注意：
烧录前请将 Erase All Flash 选项选为 Enable，将 Partition Scheme 设置为 Huge App，否则程序可能会有错误。
（所有功能均为现学现写，可能存在冗余代码或逻辑错误，欢迎各位指正）
*/

unsigned long prevDisplay = 0;
unsigned long prevSampling = 0;

void setup() {
  Serial.begin(115200);
  // 初始外置RTC
  initRTC();
  // 从NVS中获取信息
  getInfos();
  // 初始化蓝牙
  initBLE();
  // 初始化点阵屏(包含拾音器)
  initMatrix();
  // 创建加载动画任务
  createShowTextTask("START");
  // 初始化按键
  btnInit();
  Serial.println("各外设初始化成功");

  //先判断ssid是否为空
  if (ssid == "") {
    // 尝试使用RTC时间
    if (RTCtoSysTime()) {
      if (showTextTask != NULL) {   // 删除加载动画任务
        delay(3000);               // 延迟3秒，作为开机动画
        vTaskDelete(showTextTask);
        showTextTask = NULL;
      }
      RTCSuccess = true;
      currentPage = TIME;
      startAdvertising();
    } else {
      // RTC时间失败后进入蓝牙设置
      apConfig = true;
    }
  } else {
    // ssid不为空，先执行WIFI对时
    connectWiFi(5);
    if(wifiConnected){
      checkTime(5);
      disConnectWifi();
      if (RTCSuccess) {
        if (showTextTask != NULL) {
          vTaskDelete(showTextTask);
          showTextTask = NULL;
        }
        currentPage = TIME;
      } else {
        // WIFI对时失败后使用RTC时间
        if (RTCtoSysTime()) {
          RTCSuccess = true;
          currentPage = TIME;
          startAdvertising();
        } else {
          // RTC时间使用失败再进入蓝牙设置
          apConfig = true;
        }
      }
    } else {
      // WIFI连接失败，尝试使用RTC时间
      if (RTCtoSysTime()) {
        RTCSuccess = true;
        currentPage = TIME;
        startAdvertising();
      } else {
        // RTC时间使用失败再进入蓝牙设置
        apConfig = true;
      }
    }
  }


  if (apConfig){
    if (showTextTask != NULL) {
      vTaskDelete(showTextTask);
      showTextTask = NULL;
      delay(300);
    }
    createShowTextTask("BLE");
    startAdvertising();
    currentPage = SETTING;
  } else{
    // 开启对时任务
    startTickerCheckTime();
  }

  // 如果闹钟开启，设置闹钟倒计时
  if(clockOpen){
    startTickerClock(getClockRemainSeconds());
  }
}

void loop() {   
  watchBtn();
  if(brightModel == BRIGHT_MODEL_AUTO && ((millis() - prevSampling) >= 1000 || prevSampling > millis())){
    brightSamplingValue+=analogRead(LIGHT_ADC);
    brightSamplingTime++;
    prevSampling = millis();
    if(brightSamplingTime >= BRIGHT_SAMPLING_TIMES){ // 每轮采样N次重新计算一次亮度值
      calculateBrightnessValue();
      clearBrightSampling();
    }
  }
  if(isCheckingTime){ // 对时中
    Serial.println("开始对时");
    long start = millis(); // 记录开始对时的时间
    // 绘制对时提示文字
    // drawCheckTimeText();
    drawText(4, 6, "TIME...");  // 个人不喜欢对时的提示文字，改为 TIME...
    // 执行对时逻辑
    checkTimeTicker();
    // 将对时标志置为false
    isCheckingTime = false;
    // 让整个对时过程持续超过4秒，不然时间太短，提示文字一闪而过，让人感觉鬼畜了
    while((millis() - start) < 4000){
      delay(200);
    }
    // 清屏
    clearMatrix();
    Serial.println("结束对时");
    // 结束对时后，重新绘制之前的页面
    if(currentPage == CLOCK){
      drawClock();
    }else if(currentPage == BRIGHT){
      drawBright();
    }else if(currentPage == ANIM){
      lightedCount = 0;
      memset(matrixArray,0,sizeof(matrixArray));
    }
  }else{
    switch(currentPage){
      case SETTING:  // 配置页面
        if (!apConfig){   // 如果不是启动时蓝牙配置模式，就延迟3秒再切换时间界面，保证提示文字时长
          delay(3000);
          if (showTextTask != NULL) {
            vTaskDelete(showTextTask);
            showTextTask = NULL;
          }
          clearMatrix();
          currentPage = TIME;
        }
        if (BLEConnected){
          if (showTextTask != NULL) {
            vTaskDelete(showTextTask);
            showTextTask = NULL;
          }
          if (RTCSuccess){
            clearMatrix();
            currentPage = TIME;
          }
        }
        break;
      case TIME: // 时钟页面
        if((millis() - prevDisplay) >= 50 || prevDisplay > millis()){
          // 绘制时间
          drawTime();
          prevDisplay = millis();
        }
        break;
      case RHYTHM: // 节奏灯页面
        drawRHYTHM();
        break;
      case ANIM: // 动画页面  
        drawAnim();
        break;
      case CLOCK: // 闹钟设置页面
        drawClock();
        break;
      case BRIGHT: // 亮度调节页面
        drawBright();
        break;  
      case ANNIVERSARY: // 纪念日页面
        drawAnniversary();
        break;
      default:
        break;
    }
  } 
}
