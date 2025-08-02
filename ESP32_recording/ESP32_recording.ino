/*
 * 分段 WAV 录音机 for Seeed XIAO ESP32S3 Sense
 * 每 10 秒录制并写入一段 WAV 文件，共 3 段，避免 RAM 占满
 */

#include <Arduino.h>
#include <driver/i2s.h>
#include <SD.h>

#define SD_CS_PIN 21
#define SAMPLE_RATE 16000
#define BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_16BIT
#define CHANNELS 1
#define SEGMENT_DURATION_SECONDS 10
#define FILE_PREFIX "/segment_"
#define MAX_SEGMENTS 3

File audioFile;
i2s_config_t i2s_config;
i2s_pin_config_t pin_config;

void writeWavHeader(File file, int sampleRate, int bitsPerSample, int channels, uint32_t dataSize)
{
  file.seek(0);
  file.write((const uint8_t *)"RIFF", 4);
  uint32_t chunkSize = dataSize + 36;
  file.write((uint8_t *)&chunkSize, 4);
  file.write((const uint8_t *)"WAVE", 4);
  file.write((const uint8_t *)"fmt ", 4);
  uint32_t subchunk1Size = 16;
  uint16_t audioFormat = 1;
  file.write((uint8_t *)&subchunk1Size, 4);
  file.write((uint8_t *)&audioFormat, 2);
  file.write((uint8_t *)&channels, 2);
  file.write((uint8_t *)&sampleRate, 4);
  uint32_t byteRate = sampleRate * channels * (bitsPerSample / 8);
  uint16_t blockAlign = channels * (bitsPerSample / 8);
  file.write((uint8_t *)&byteRate, 4);
  file.write((uint8_t *)&blockAlign, 2);
  file.write((uint8_t *)&bitsPerSample, 2);
  file.write((const uint8_t *)"data", 4);
  file.write((uint8_t *)&dataSize, 4);
}

void setupI2S()
{
  i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = SAMPLE_RATE,
      .bits_per_sample = BITS_PER_SAMPLE,
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
      .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 8,
      .dma_buf_len = 1024,
      .use_apll = false,
      .tx_desc_auto_clear = false,
      .fixed_mclk = 0};

  pin_config = {
      .bck_io_num = 41, // I2S BCK
      .ws_io_num = 42,  // I2S WS/LRCK
      .data_out_num = I2S_PIN_NO_CHANGE,
      .data_in_num = 2  // I2S DATA
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_set_clk(I2S_NUM_0, SAMPLE_RATE, BITS_PER_SAMPLE, I2S_CHANNEL_MONO);
}

void setup()
{
  Serial.begin(115200);
  delay(2000);

  if (!SD.begin(SD_CS_PIN))
  {
    Serial.println("❌ SD 卡初始化失败");
    while (1)
      ;
  }

  setupI2S();
  Serial.println("🎹 分段录音开始，每段 10 秒...");

  const uint32_t segmentBytes = SAMPLE_RATE * (BITS_PER_SAMPLE / 8) * SEGMENT_DURATION_SECONDS;
  uint8_t *buffer = (uint8_t *)ps_malloc(segmentBytes);
  if (!buffer)
  {
    Serial.println("❌ 无法分配缓冲区");
    while (1)
      ;
  }

  for (int segment = 0; segment < MAX_SEGMENTS; segment++)
  {
    String filename = FILE_PREFIX + String(segment) + ".wav";
    Serial.printf("📁 正在录制: %s\n", filename.c_str());
    File audioFile = SD.open(filename.c_str(), FILE_WRITE);
    writeWavHeader(audioFile, SAMPLE_RATE, 16, 1, 0);

    size_t bytesRead = 0, totalBytes = 0;
    while (totalBytes < segmentBytes)
    {
      size_t toRead = ((segmentBytes - totalBytes) < 1024) ? (segmentBytes - totalBytes) : 1024;
      i2s_read(I2S_NUM_0, buffer + totalBytes, toRead, &bytesRead, portMAX_DELAY);
      totalBytes += bytesRead;
    }

    audioFile.write(buffer, totalBytes);
    writeWavHeader(audioFile, SAMPLE_RATE, 16, 1, totalBytes);
    audioFile.close();
    Serial.printf("✅ 本段完成，大小: %d bytes\n", totalBytes);
    Serial.printf("已经存储 %d 段，共 %d 秒\n", segment + 1, (segment + 1) * SEGMENT_DURATION_SECONDS);
  }

  i2s_driver_uninstall(I2S_NUM_0);
  free(buffer);
  Serial.println("🎉 所有段落录音完成");
}

void loop()
{
  delay(1000);
}
