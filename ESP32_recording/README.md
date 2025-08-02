# ESP32 分段录音机

基于 Seeed XIAO ESP32S3 Sense 的分段 WAV 录音机项目。

## 功能特性

- 🎙️ 使用 I2S 接口进行高质量音频录制
- 📁 自动分段录制，每段 60 秒
- 💾 直接保存到 SD 卡，避免 RAM 溢出
- 🔄 支持连续录制多段音频
- 📊 实时显示录制进度和文件大小

## 硬件要求

- Seeed XIAO ESP32S3 Sense
- Micro SD 卡
- 麦克风（板载或外接）

## 引脚连接

| 功能 | 引脚 |
|------|------|
| SD 卡 CS | GPIO 21 |
| I2S BCK | GPIO 42 |
| I2S WS | GPIO 41 |
| I2S DATA | GPIO 2 |

## 配置参数

```cpp
#define SAMPLE_RATE 16000        // 采样率 16kHz
#define BITS_PER_SAMPLE 16       // 16位采样
#define CHANNELS 1               // 单声道
#define SEGMENT_DURATION_SECONDS 60  // 每段60秒
#define MAX_SEGMENTS 3           // 最多录制3段
```

## 使用方法

1. 将代码上传到 ESP32
2. 插入 SD 卡
3. 打开串口监视器（115200 baud）
4. 设备将自动开始录制

## 输出文件

录音文件将保存为：
- `/segment_0.wav`
- `/segment_1.wav`
- `/segment_2.wav`

## 技术细节

- 使用 PSRAM 分配大缓冲区
- 实时音量增益处理
- 标准 WAV 文件格式
- 自动文件头写入

## 注意事项

- 确保 SD 卡有足够空间
- 录音文件较大，注意存储管理
- 可通过修改 `MAX_SEGMENTS` 调整录制段数

## 许可证

MIT License 