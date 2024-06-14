#ifndef __COMMON_H
#define __COMMON_H

#include "img/timeAnim0.h"
#include "img/timeAnim1.h"
#include "img/timeAnim2.h"
#include "img/timeAnim3.h"
#include "img/timeAnim4.h"
#include "img/timeAnim5.h"
#include "img/timeAnim6.h"
#include "img/timeAnim7.h"
#include "img/timeAnim8.h"
#include "img/animHack.h"
#include "img/bell.h"
#include "img/checkingTime.h"
#include "img/success.h"
#include "img/heart.h"
#include "img/heart2.h"
#include "img/cake.h"

#define DATAPIN               6 // 灯珠矩阵输入引脚
#define LIGHTCOUNT            256 // 灯珠个数
#define BTN1                  5 // 按钮1
#define BTN2                  4 // 按钮2
#define BTN3                  8 // 按钮3
#define BUZZER                0 // 蜂鸣器
#define AUDIO_IN_PIN          1 // 拾音器
#define LIGHT_ADC             2 // 光敏电阻ADC引脚
#define RTC_RST               12 // RTC复位引脚
#define RTC_DAT               13 // RTC数据引脚
#define RTC_CLK               3 // RTC时钟引脚
#define RANDOM_SEED_PIN       11 // 随机数种子引脚

#define BRIGHTNESS            40 // 默认亮度
#define MAX_BRIGHTNESS        145 // 最大亮度
#define MIN_BRIGHTNESS        5 // 最小亮度
#define BRIGHTNESS_SPACING    20 //每次调节亮度的间隔大小

#define MATRIX_SIDE           8 //每个矩阵的边长
#define MATRIX_WIDTH          32 //矩阵总宽度
#define MATRIX_COUNT          4 //矩阵数量

#define NTP3                  "ntp4.ntsc.ac.cn"
#define NTP2                  "ntp3.ict.ac.cn"
#define NTP1                  "ntp2.aliyun.com"

#define SAMPLES               256  //采样个数
#define SAMPLING_FREQ         10000 //采样频率
#define AMPLITUDE             1000  //声音强度调整倍率（柱状高度倍率）
#define NUM_BANDS             32  //频段个数 
#define NOISE                 1770   //噪音
#define BAR_WIDTH             1 //每个频段的宽度

#define ONE_DAY_SECONDS       24 * 60 * 60 // 一天的秒数
#define ANIM_INTERVAL         200 // 时钟页面每帧动画间隔（ms）
#define TIME_CHECK_INTERVAL   18000 // NTP对时间隔（s）, 18000秒即为5小时
#define BRIGHT_SAMPLING_TIMES 1 // 每轮亮度采样次数

// 时钟页面下的小页面
const int TIME_H_M_S = 1;
const int TIME_H_M = 2;
const int TIME_DATE = 3;
// 节奏灯页面下的小页面
const int RHYTHM_MODEL1 = 1;
const int RHYTHM_MODEL2 = 2;
const int RHYTHM_MODEL3 = 3;
const int RHYTHM_MODEL4 = 4;
// 节奏灯频率模式
const int RHYTHM_BANDS_MODEL1 = 1;
const int RHYTHM_BANDS_MODEL2 = 2;
// 动画页面下的小页面
const int ANIM_MODEL1 = 1;
const int ANIM_MODEL2 = 2;
const int ANIM_MODEL3 = 3;
// 闹钟页面选中项
const int CLOCK_H = 1;
const int CLOCK_M = 2;
const int CLOCK_BELL = 3;
// 亮度模式
const int BRIGHT_MODEL_MANUAL = 1;
const int BRIGHT_MODEL_AUTO = 2;
// 纪念日页面子项
const int ANNIVERSARY_A = 1;
const int ANNIVERSARY_B = 2;
// 时间跳变模式
const int TIME_MODEL_DIRECT = 1;
const int TIME_MODEL_ANIM = 2;

// 定义页面枚举 SETTING-配网页面  TIME-时间页面  RHYTHM-节奏灯页面  ANIM-动画页面  CLOCK-闹钟设置页面  BRIGHT-亮度调节  ANNIVERSARY-纪念日界面
enum CurrentPage{
  SETTING, TIME, RHYTHM, ANIM, CLOCK, BRIGHT, ANNIVERSARY
};
#endif

