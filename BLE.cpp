#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "common.h"
#include "preferencesUtil.h"
#include "light.h"
#include "RTC.h"
#include "net.h"
#include "task.h"


BLEServer *pServer = NULL;                    //BLEServer指针 pServer
BLEAddress *authorizedDevice = NULL;

BLECharacteristic *pTxCharacteristic = NULL;  //BLECharacteristic指针 pTxCharacteristic
BLECharacteristic *pRxCharacteristic = NULL;
BLECharacteristic *pCharacteristicBrightness = NULL;
BLECharacteristic *pCharacteristicColor = NULL;
BLECharacteristic *pCharacteristicAnniversary = NULL;
BLECharacteristic *pCharacteristicBirthday = NULL;
BLECharacteristic *pCharacteristicWIFI = NULL;
BLECharacteristic *pCharacteristicAnim = NULL;

bool BLEConnected = false;                //蓝牙连接状态
bool advertising = false;                 //蓝牙广播状态

// 生成 UUID 网站: https://www.uuidgenerator.net/
#define SERVICE1_UUID           "14d59bde-ba3d-4477-ba91-3a7d6589b164"   // UART service UUID
#define BRIGHTNESS_UUID         "be8948de-9de8-4736-9da8-8a8169265a0e"
#define COLOR_UUID              "515ced63-935b-48f8-8087-4b93e2e64f88"
#define ANNIVERSARY_UUID        "e56b3238-0af0-4502-82ad-473dc6f3d9f9"
#define BIRTHDAY_UUID           "0ceae7bf-16ed-46f5-b6c3-17dc8439d574"
#define WIFI_UUID               "da75e597-b36a-444c-b771-9b058fb1fcd8"
#define ANIM_UUID               "8e952b12-dd4e-4c78-8bc6-810dc018c22b"


#define SERVICE2_UUID           "5738a36e-8e61-48cf-ad73-06fce0d5843d"   // UART service UUID
#define CHARACTERISTIC_UUID_RX  "836a72f2-c6c7-44c9-b0ea-ce09cc23038f"   //通用接收特性
#define CHARACTERISTIC_UUID_TX  "a0ea1b56-2f4d-464c-8de8-d523fbb9b284"   //通用发送特性

void startAdvertising();

class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer, esp_ble_gatts_cb_param_t *param)
    {
        BLEConnected = true;
        setBLEInfo();
        auto p = param->connect;
        BLEAddress address(p.remote_bda);
        BLEAddress peerAddress = BLEAddress(p.remote_bda);
        uint16_t conn_id = p.conn_id;
        Serial.print("已连接");
        Serial.println(peerAddress.toString().c_str());
        currentPage = SETTING;
        clearMatrix();
        drawSuccess(6, 21, "BLE");
    };

    void onDisconnect(BLEServer *pServer)
    {
        BLEConnected = false;
        advertising = false;
        Serial.println("BLE断开连接");
        startAdvertising();
    }
};

class MyCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string rValue = pCharacteristic->getValue(); //接收信息
        BLEUUID characteristic_uuid = pCharacteristic->getUUID();

        Serial.println(rValue.c_str()); //输出调试信息

        if (!rValue.empty()){
            if (characteristic_uuid.equals(BLEUUID(BRIGHTNESS_UUID))) {
                Serial.println("亮度设置");
                // 处理亮度设置
                if (rValue != "AUTO"){
                    brightModel = BRIGHT_MODEL_MANUAL;
                    int rValueInt = std::stoi(rValue); // 将字符串转换为整数
                    brightness = map(rValueInt, 1, 100, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
                    recordBrightness();
                } else {
                    brightModel = BRIGHT_MODEL_AUTO;
                    clearBrightSampling();
                    recordBrightness();
                }
               
            } else if (characteristic_uuid.equals(BLEUUID(COLOR_UUID))) {
                Serial.println("颜色设置");
                // 处理颜色设置
                int r, g, b;
                sscanf(rValue.c_str(), "%d,%d,%d", &r, &g, &b);
                recordClockColor(r, g, b);
            } else if (characteristic_uuid.equals(BLEUUID(ANNIVERSARY_UUID))) {
                Serial.println("纪念日设置");
                // 处理纪念日设置
                size_t pos = rValue.find('|');
                if (pos != std::string::npos) {
                    anniversaryOpen = rValue.substr(0, pos) == "1";
                    size_t pos2 = rValue.find(',', pos + 1);
                    if (pos2 != std::string::npos) {
                        anniversaryA = atol(rValue.substr(pos + 1, pos2 - pos - 1).c_str());
                        anniversaryB = atol(rValue.substr(pos2 + 1).c_str());
                        recordAnniversary();
                    }
                }
            } else if (characteristic_uuid.equals(BLEUUID(BIRTHDAY_UUID))) {
                Serial.println("生日设置");
                // 处理生日设置
                size_t pos = rValue.find('|');
                if (pos != std::string::npos) {
                    birthdayOpen = rValue.substr(0, pos) == "1";
                    birthday = atol(rValue.substr(pos + 1).c_str());
                    recordBirthday();
                }
            } else if (characteristic_uuid.equals(BLEUUID(WIFI_UUID))) {
                Serial.println("WIFI设置");
                // 处理WIFI设置
                size_t pos = rValue.find('|');
                if (pos != std::string::npos) {
                    ssid = rValue.substr(0, pos).c_str();
                    pass = rValue.substr(pos + 1).c_str();
                    recordWifiConfig();
                }
            } else if(characteristic_uuid.equals(BLEUUID(ANIM_UUID))) {
                Serial.println("动画设置");
                // 处理动画设置
                if (rValue == "true") {
                    timeModel = TIME_MODEL_ANIM;
                } else if (rValue == "false") {
                    timeModel = TIME_MODEL_DIRECT;
                }
                recordAnim();
            } else if (characteristic_uuid.equals(BLEUUID(CHARACTERISTIC_UUID_RX))) {
                Serial.println("RX接收到数据");
                if (rValue[0] == 'T') {     // 蓝牙对时
                    try {
                        std::string numberString = rValue.substr(1); // 获取从第二个字符开始的子字符串
                        int timestamp = std::stoi(numberString); // 将子字符串转换为整数
                        setenv("TZ", "CST-8", 1); // 设置时区
                        tzset();
                        Serial.println(timestamp);
                        struct timeval tv = {.tv_sec = timestamp};
                        if (settimeofday(&tv, NULL) == 0) {   // 如果 settimeofday 返回非零值，表示设置时间失败
                            RTCSuccess = true;
                        }
                        setRTCtime(timestamp);
                    } catch (...) {
                        // 捕获所有其他类型的异常
                        Serial.println("处理对时数据失败");
                    }
                }
                pTxCharacteristic->setValue(rValue);
                pTxCharacteristic->notify();
                // 处理接收到的数据
            }
        }
    }
};

void initBLE() {
    BLEDevice::init("SmartMatrix");
    BLEDevice::setPower(ESP_PWR_LVL_N24); // 设置蓝牙功耗级别，降低蓝牙功耗

    // 创建一个新的服务
    // 创建一个 BLE 服务
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks()); //设置回调
    MyCallbacks* pCallbacks = new MyCallbacks();

    BLEService *pService2 = pServer->createService(SERVICE2_UUID);

    // 创建通用发送特性
    pTxCharacteristic = pService2->createCharacteristic(
        CHARACTERISTIC_UUID_TX, 
        BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ
    );
    pTxCharacteristic->addDescriptor(new BLE2902());

    // 创建通用接收特性
    pRxCharacteristic = pService2->createCharacteristic(
        CHARACTERISTIC_UUID_RX, 
        BLECharacteristic::PROPERTY_WRITE
    );
    pRxCharacteristic->setCallbacks(pCallbacks); //设置回调

    BLEService *pService = pServer->createService(SERVICE1_UUID);

    // 创建亮度特性
    pCharacteristicBrightness = pService->createCharacteristic(
        BRIGHTNESS_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
    );
    pCharacteristicBrightness->setCallbacks(pCallbacks);

    // 创建颜色特性
    pCharacteristicColor = pService->createCharacteristic(
        COLOR_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
    );
    pCharacteristicColor->setCallbacks(pCallbacks);

    // 创建纪念日特性
    pCharacteristicAnniversary = pService->createCharacteristic(
        ANNIVERSARY_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
    );
    pCharacteristicAnniversary->setCallbacks(pCallbacks);

    // 创建生日特性
    pCharacteristicBirthday = pService->createCharacteristic(
        BIRTHDAY_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
    );
    pCharacteristicBirthday->setCallbacks(pCallbacks);

    // 创建WIFI特性
    pCharacteristicWIFI = pService->createCharacteristic(
        WIFI_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
    );
    pCharacteristicWIFI->setCallbacks(pCallbacks);

    // 创建动画特性
    pCharacteristicAnim = pService->createCharacteristic(
        ANIM_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
    );
    pCharacteristicAnim->setCallbacks(pCallbacks);

    pService2->start();                    // 开始服务2
    pService->start();                    // 开始服务1
    Serial.println("BLE初始化完成");
}

// 开启蓝牙广播
void startAdvertising() {
    if (!advertising) {
        pServer->getAdvertising()->start();
        advertising = true;
        Serial.println("蓝牙开始广播");
    }
}

// 停止蓝牙广播
void stopAdvertising() {
    if (advertising){
        pServer->getAdvertising()->stop();
        advertising = false;
        Serial.println("蓝牙停止广播");
    }
}

// 断开蓝牙连接
void disconnectBLE() {
    if (BLEConnected) {
        pServer->disconnect(pServer->getConnId());
        BLEConnected = false;
    }
}

// 发送数据
void sendBLEData(String data) {
    if (BLEConnected) {
        pTxCharacteristic->setValue(data.c_str());
        pTxCharacteristic->notify();
    }
}

// 设置亮度特征值
void setBLEBrightness(int brightness) {
    if (BLEConnected){
        if (brightModel == BRIGHT_MODEL_MANUAL) {
            int rValueInt = map(brightness, MIN_BRIGHTNESS, MAX_BRIGHTNESS, 1, 100);
            std::string rValue = std::to_string(rValueInt);
            pCharacteristicBrightness->setValue(rValue);
            sendBLEData("Brightness");
        }else if (brightModel == BRIGHT_MODEL_AUTO){
            pCharacteristicBrightness->setValue("AUTO");
            sendBLEData("Brightness");
        }
    }
}

// 设置颜色特征值
void setBLEColor(int r, int g, int b) {
    if (BLEConnected){
        std::string rValue = std::to_string(r) + "," + std::to_string(g) + "," + std::to_string(b);
        pCharacteristicColor->setValue(rValue.c_str());
        sendBLEData("Color");
    }
}

// 设置纪念日特征值
void setBLEAnniversary(bool open, unsigned long anniversaryA, unsigned long anniversaryB) {
    if (BLEConnected){
        std::string rValue = std::to_string(open) + "|" + std::to_string(anniversaryA) + "," + std::to_string(anniversaryB);
        pCharacteristicAnniversary->setValue(rValue);
        sendBLEData("Anniversary");
    }
}

// 设置生日特征值
void setBLEBirthday(bool open, unsigned long birthday) {
    if (BLEConnected){
        std::string rValue = std::to_string(open) + "|" + std::to_string(birthday);
        pCharacteristicBirthday->setValue(rValue);
        sendBLEData("Birthday");
    }
}

// 设置WIFI特征值
void setBLEWIFI(String ssid, String pass) {
    if (BLEConnected){
        std::string rValue = std::string(ssid.c_str()) + "|" + std::string(pass.c_str());
        pCharacteristicWIFI->setValue(rValue);
        sendBLEData("WIFI");
    }
}

// 设置时间动画特征值
void setBLEAnim(int mode) {
    if (BLEConnected){
        if (mode == TIME_MODEL_ANIM){
            pCharacteristicAnim->setValue("true");
        } else if (mode == TIME_MODEL_DIRECT){
            pCharacteristicAnim->setValue("false");
        }
        sendBLEData("Anim");
    }
}
