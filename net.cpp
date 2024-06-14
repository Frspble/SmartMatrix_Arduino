#include <WiFi.h>
#include "common.h"
#include "preferencesUtil.h"
#include "light.h"
#include "task.h"
#include "RTC.h"

// Wifi相关
String ssid = "";  //WIFI名称
String pass = "";  //WIFI密码
bool apConfig; // 系统启动时是否需要配网
// 是否顺利连接上wifi
bool wifiConnected = false;
// 是否NTP对时成功
bool RTCSuccess = false;

// 连接WiFi
void connectWiFi(int timeOut_s){
  int connectTime = 0; //用于连接计时，如果长时间连接不成功，则提示失败
  Serial.print("正在连接网络");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    connectTime++;
    if (connectTime > 2 * timeOut_s){ //长时间连接不上，提示连接失败
      Serial.println("网络连接失败...");
      // 停止启动加载动画
      if (showTextTask != NULL) {
        vTaskDelete(showTextTask);
        showTextTask = NULL;
        delay(300);
      }
      // 屏幕显示wifi连接失败
      drawFailed(4, 24, "WIFI");
      delay(1000);
      // 清空屏幕
      clearMatrix();
      return;
    }
  }
  wifiConnected = true;
  Serial.println("网络连接成功");
  Serial.print("本地IP： ");
  Serial.println(WiFi.localIP());
}

// 获取NTP并同步RTC时间
void getNTPTime(){
  // 8 * 3600 东八区时间修正
  // 使用夏令时 daylightOffset_sec 就填写3600，否则就填写0；
  Serial.println("执行NTP对时");
  configTime( 8 * 3600, 0, NTP1, NTP2, NTP3);
  struct tm now;
  if (getLocalTime(&now)){
    setRTCtime(now);
  }
}

// 系统启动时进行NTP对时
void checkTime(int limitTime){
  // 记录此时的时间，在NTP对时时，超过一定的时间，就直接结束
  time_t start;
  time(&start);
  // 获取NTP并同步至RTC，第一次同步失败，就一直尝试同步
  getNTPTime();
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)){
    time_t end;
    time(&end);
    if((end - start) > limitTime){
      Serial.println("网络对时超时...");
      // 停止启动加载动画
      if (showTextTask != NULL) {
        vTaskDelete(showTextTask);
        showTextTask = NULL;
      }
      clearMatrix();
      // 屏幕显示TIME获取失败
      drawFailed(4, 24, "TIME");
      delay(1000);
      // 清空屏幕
      clearMatrix();
      // 跳出循环
      return;
    }
    Serial.println("时钟对时失败...");
    getNTPTime();
  }
  // 到了这一步，说明对时成功
  RTCSuccess = true;
  setRTCtime(timeinfo);
}

// 关闭wifi
void disConnectWifi(){
  WiFi.disconnect();
}

// 定时对时
void checkTimeTicker(){
  // 重新连接wifi
  Serial.println("重新连接网络...");
  int connectTime = 0; //连接计时
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    connectTime++;
    if (connectTime > 10){ //循环10次（5秒）连接不上，就退出
      Serial.println("网络连接失败...");
      wifiConnected = false;
      // 跳出循环
      break;
    }
  }
  // wifi连接成功，进行对时
  if(WiFi.status() == WL_CONNECTED){
    Serial.println("网络连接成功");
    // 只对一次，不管是否成功，因为启动时已经成功进行了对时，这次无关紧要
    getNTPTime();
    // Serial.println("对时完成");
    // 对时后如果闹钟开启，就重置闹钟时间
    if(clockOpen){
      tickerClock.detach();
      startTickerClock(getClockRemainSeconds());
    }
  }
  // 关闭wifi功能
  disConnectWifi();
}



