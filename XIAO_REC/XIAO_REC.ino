#include <ESP_I2S.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

const char* ssid = "Innoxsz-2.4G";
const char* password = "innox2.4#";
bool isWIFIConnected;
WiFiClientSecure client;


  
// constants won't change. They're used here to set pin numbers:
const int buttonPin = 0;  // the number of the pushbutton pin
const int ledPin = 21;    // the number of the LED pin
const int green_LED_Pin = 2;    // the number of the LED pin
const int blue_LED_Pin = 3;    // the number of the LED pin
const int startbuttonPin = 4;  // the number of the pushbutton pin
/*
GPIO2  D1	  GREENLED  
GPIO4  D3 	REDLED
GPIO5  D4 	BUTTON
按键按下，绿灯亮，开始录音。按键再次按下，绿灯灭，结束录音。
*/

// variables will change:
int buttonState = 0;      // variable for reading the pushbutton status
int isRecording = 0;      // 录音状态标志
int assfilename = 0;      // 文件名计数器
char *exfilename = "recoder";  // 文件名前缀

// I2S和录音相关变量
I2SClass i2s;
const int bufferSize = 8192;  // 音频缓冲区大小
uint8_t audioBuffer[8192];    // 音频数据缓冲区
File recordFile;              // 录音文件对象

// 按钮状态检测变量
int lastButtonState = HIGH;   // 上一次按钮状态
unsigned long lastDebounceTime = 0;  // 上一次按钮状态变化时间
unsigned long debounceDelay = 50;    // 消抖延时


void wifiConnect(void *pvParameters){
  isWIFIConnected = false;
  Serial.print("尝试连接到 ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    vTaskDelay(500);
    Serial.print(".");
  }
  Serial.println("Wi-Fi 已连接！");
  isWIFIConnected = true;
  // 忽略 SSL 证书验证
  client.setInsecure();
  while(true){
    vTaskDelay(1000);
  }
}

// 开始录音函数
void startRecording() {
  Serial.println("开始录音...");
  digitalWrite(ledPin, LOW);  // LED亮表示正在录音
    digitalWrite(ledPin, LOW);  // LED亮表示正在录音
  // 创建动态文件名：exfilename + assfilename + .wav
  String filename = String(exfilename) + String(assfilename) + ".wav";
  
  // 创建WAV文件头信息
  uint32_t sampleRate = 16000;  // 采样率
  uint16_t numChannels = 1;     // 单声道
  uint16_t bitsPerSample = 16;  // 16位采样
  
  // 打开文件准备写入
  recordFile = SD.open("/" + filename, FILE_WRITE);
  if (!recordFile) {
    Serial.println("无法打开文件进行写入!");
    return;
  }
  
  // 写入WAV文件头
  writeWavHeader(recordFile, sampleRate, numChannels, bitsPerSample);
  
  Serial.println("正在录音到文件: " + filename);
  isRecording = 1;
}

// 停止录音函数
void stopRecording() {
  if (isRecording == 0 || !recordFile) {
    return;
  }
  
  Serial.println("停止录音...");
  digitalWrite(ledPin, HIGH);  // LED灭表示录音结束
  
  // 更新WAV文件头中的数据大小
  updateWavHeader(recordFile);
  
  // 关闭文件
  recordFile.close();
  
  Serial.println("录音已保存");

  listDir(SD, "/", 0);
    String filename = "/" + String(exfilename) + String(assfilename) + ".wav";
  Serial.println("准备上传文件: " + filename);
  int maxRetries = 3;
  for (int i = 0; i < maxRetries; i++) {
    if (uploadFile(filename)) {
      Serial.println("文件上传成功！");
      break;
    } else if (i == maxRetries - 1) {
      Serial.println("文件上传失败，已达最大重试次数！");
    } else {
      Serial.println("上传失败，正在重试...");
      delay(1000);
    }
  }
  
  // 每次录音完成后assfilename+1
  assfilename++;
  isRecording = 0;
}

// 写入WAV文件头
void writeWavHeader(File file, uint32_t sampleRate, uint16_t numChannels, uint16_t bitsPerSample) {
  uint32_t byteRate = sampleRate * numChannels * bitsPerSample / 8;
  uint16_t blockAlign = numChannels * bitsPerSample / 8;
  
  // RIFF头
  file.write((const uint8_t*)"RIFF", 4);
  file.write((const uint8_t*)"\x00\x00\x00\x00", 4);  // 文件大小，稍后更新
  file.write((const uint8_t*)"WAVE", 4);
  
  // fmt子块
  file.write((const uint8_t*)"fmt ", 4);
  file.write((const uint8_t*)"\x10\x00\x00\x00", 4);  // 子块大小 (16 bytes)
  file.write((const uint8_t*)"\x01\x00", 2);          // 音频格式 (PCM = 1)
  
  // 写入通道数
  file.write((const uint8_t*)&numChannels, 2);
  
  // 写入采样率
  file.write((const uint8_t*)&sampleRate, 4);
  
  // 写入字节率
  file.write((const uint8_t*)&byteRate, 4);
  
  // 写入块对齐
  file.write((const uint8_t*)&blockAlign, 2);
  
  // 写入位深度
  file.write((const uint8_t*)&bitsPerSample, 2);
  
  // data子块
  file.write((const uint8_t*)"data", 4);
  file.write((const uint8_t*)"\x00\x00\x00\x00", 4);  // 数据大小，稍后更新
}

// 更新WAV文件头中的数据大小
void updateWavHeader(File file) {
  uint32_t fileSize = file.size();
  uint32_t dataSize = fileSize - 44;  // 减去头部大小
  
  // 更新RIFF块大小
  file.seek(4);
  uint32_t riffSize = fileSize - 8;
  file.write((uint8_t*)&riffSize, 4);
  
  // 更新data块大小
  file.seek(40);
  file.write((uint8_t*)&dataSize, 4);
}
void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("列出目录: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("无法打开目录");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("不是目录");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  目录 : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.path(), levels -1);
            }
        } else {
            Serial.print("  文件: ");
            Serial.print(file.name());
            Serial.print("  大小: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

bool uploadFile(String filename) {
  HTTPClient http;
  http.begin("http://10.10.43.149:5002/voice");
  
  String boundary = "----WebKitFormBoundary" + String(random(0xFFFFFFFF), HEX);
  http.addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);

  File file = SD.open(filename, FILE_READ);
  if (!file) {
    Serial.println("无法打开文件进行上传！");
    return false;
  }

  String postData = "--" + boundary + "\r\n";
  postData += "Content-Disposition: form-data; name=\"audio\"; filename=\"audiom1\"\r\n";
  postData += "Content-Type: audio/wav\r\n\r\n";

  // 写入文件数据
  uint8_t buffer[512];
  size_t bytesRead;
  while ((bytesRead = file.read(buffer, sizeof(buffer))) > 0) {
    postData += String((const char*)buffer, bytesRead);
  }
  postData += "\r\n--" + boundary + "--\r\n";

  int httpResponseCode = http.POST(postData);
  file.close();

  String response = http.getString();
  Serial.println("服务器返回完整信息：");
  Serial.println("HTTP 状态码: " + String(httpResponseCode));
  Serial.println("响应内容: " + response);
  
  if (httpResponseCode == 200) {
    Serial.println("上传成功！");
    return true;
  } else {
    Serial.println("上传失败！");
    return false;
  }
}

// 处理录音数据
void processRecording() {
  if (isRecording == 1 && recordFile) {
    // 读取I2S数据
    size_t bytesRead = i2s.readBytes((char*)audioBuffer, bufferSize);
    
    if (bytesRead > 0) {
      // 写入文件
      recordFile.write(audioBuffer, bytesRead);
    }
  }
}

void setup() {
  // 初始化LED引脚为输出
  pinMode(ledPin, OUTPUT);
  // 初始化按钮引脚为输入
  pinMode(buttonPin, INPUT);
/*
const int green_LED_Pin = 2;    // the number of the LED pin
const int blue_LED_Pin = 4;    // the number of the LED pin
const int startbuttonPin = 5;  // the number of the pushbutton pin
*/

  // 初始化LED引脚为输出
  pinMode(green_LED_Pin, OUTPUT);
    // 初始化LED引脚为输出
  pinMode(blue_LED_Pin, OUTPUT);
  // 初始化按钮引脚为输入
  pinMode(startbuttonPin, INPUT);

  // 初始化串口
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  Serial.println("初始化I2S总线...");

  // 设置音频输入引脚
  i2s.setPinsPdmRx(42, 41);

  // 以16 kHz、16位采样启动I2S
  if (!i2s.begin(I2S_MODE_PDM_RX, 16000, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO)) {
    Serial.println("I2S初始化失败!");
    while (1); // 停止执行
  }

  Serial.println("I2S总线初始化完成.");
  Serial.println("初始化SD卡...");

  // 设置SD卡访问引脚
  if(!SD.begin(21)){
    Serial.println("SD卡挂载失败!");
    while (1);
  }
  Serial.println("SD卡初始化完成.");
  xTaskCreate(wifiConnect, "wifi_Connect", 4096, NULL, 0, NULL);
  Serial.println("配网中。");
  Serial.println("系统就绪，按下按钮开始录音，再次按下停止录音。");
}

void loop() {
  // 读取当前按钮状态
  int reading = digitalRead(startbuttonPin);
    Serial.println("读取当前按钮状态: " + String(reading));
  // 检查按钮状态是否发生变化
  if (reading != lastButtonState) {
    // 重置消抖计时器
    lastDebounceTime = millis();
  }
  
  // 如果按钮状态稳定超过消抖时间
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // 如果按钮状态与记录的状态不同
    if (reading != buttonState) {
      buttonState = reading;
      
      // 只在按钮按下时（LOW）执行操作
      if (buttonState == LOW) {
        if (isRecording == 0) {
          // 如果当前未录音，则开始录音
          startRecording();
        } else {
          // 如果当前正在录音，则停止录音
          stopRecording();
        }
      }
    }
  }
  
  // 处理录音数据
  if (isRecording == 1) {
    processRecording();
  }
  
  // 保存当前按钮状态用于下一次比较
  lastButtonState = reading;
  
  // 如果正在录音，每秒打印一个点表示录音进行中
  static unsigned long lastPrintTime = 0;
  if (isRecording == 1 && (millis() - lastPrintTime > 1000)) {
    Serial.print(".");
    lastPrintTime = millis();
  }

  // 短暂延时以减少CPU负载
  delay(5);
}