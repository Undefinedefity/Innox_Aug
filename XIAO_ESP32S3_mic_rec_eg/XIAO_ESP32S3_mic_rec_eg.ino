#include "ESP_I2S.h"
#include "FS.h"
#include "SD.h"

void setup() {
  // 创建 I2SClass 的实例
  I2SClass i2s;

  // 创建变量以存储音频数据
  uint8_t *wav_buffer;
  size_t wav_size;

  // 初始化串口
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  Serial.println("正在初始化 I2S 总线...");

  // 设置用于音频输入的引脚
  i2s.setPinsPdmRx(42, 41);

  // 以 16 kHz 的采样率和 16 位采样启动 I2S
  if (!i2s.begin(I2S_MODE_PDM_RX, 16000, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO)) {
    Serial.println("初始化 I2S 失败！");
    while (1); // 停止运行
  }


  Serial.println("I2S 总线已初始化。");
  Serial.println("正在初始化 SD 卡...");

  // 设置用于 SD 卡访问的引脚
  if(!SD.begin(21)){
    Serial.println("挂载 SD 卡失败！");
    while (1) ;
  }
  Serial.println("SD 卡已初始化。");
  Serial.println("正在录制 20 秒的音频数据...");

  // 录制 20 秒的音频数据
  wav_buffer = i2s.recordWAV(20, &wav_size);

  // 在 SD 卡上创建文件
  File file = SD.open("/arduinor_rec.wav", FILE_WRITE);
  if (!file) {
    Serial.println("打开文件失败！");
    return;
  }

  Serial.println("正在将音频数据写入文件...");

  // 将音频数据写入文件
  if (file.write(wav_buffer, wav_size) != wav_size) {
    Serial.println("写入音频数据失败！");
    return;
  }

  // 关闭文件
  file.close();

  Serial.println("应用程序完成。");
}

void loop() {
  delay(1000);
  Serial.printf(".");
}