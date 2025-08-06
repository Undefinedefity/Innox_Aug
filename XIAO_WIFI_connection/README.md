# XIAO ESP32S3 AP模式文件服务器

这是一个基于 XIAO ESP32S3 开发板的 WiFi AP模式文件服务器项目。ESP32会创建一个WiFi热点，允许其他设备直接连接到ESP32并从其microSD卡中下载文件。

## 硬件要求

- XIAO ESP32S3 开发板
- microSD 卡模块（已集成在开发板上）
- microSD 卡

## 软件要求

### Arduino IDE
- Arduino IDE 2.3.6 或更高版本
- ESP32 by Espressif 开发板包 3.3.0 或更高版本

### Python 环境
- Python 3.6 或更高版本
- 需要安装的 Python 包：
  ```bash
  pip install requests
  ```

## 安装和配置

### 1. Arduino 代码配置

1. 打开 `XIAO_WIFI_connection.ino` 文件
2. 修改 AP 配置（可选）：
   ```cpp
   const char *ap_ssid = "XIAO_ESP32S3_FileServer";  // WiFi热点名称
   const char *ap_password = "12345678";              // WiFi热点密码
   const int ap_channel = 1;                          // WiFi频道
   const int ap_max_connections = 4;                  // 最大连接数
   ```
3. 如果需要，调整 SD 卡 CS 引脚：
   ```cpp
   #define SD_CS_PIN 21  // XIAO ESP32S3 Sense 使用 GPIO21
   ```
4. 将代码上传到 XIAO ESP32S3 开发板

### 2. 检查 SD 卡初始化

如果 SD 卡初始化失败，请尝试以下方法：

1. **方法一：使用默认参数**
   ```cpp
   if (!SD.begin()) {
   ```

2. **方法二：指定 CS 引脚**
   ```cpp
   if (!SD.begin(SD_CS_PIN)) {
   ```

3. **方法三：指定所有 SPI 引脚**
   ```cpp
   if (!SD.begin(SD_CS_PIN, SPI)) {
   ```

## 使用方法

### 1. 启动AP模式服务器

1. 将代码上传到开发板
2. 打开串口监视器（波特率：115200）
3. 观察AP启动状态，记录显示的WiFi热点信息

### 2. 连接到ESP32的WiFi热点

1. 在您的设备（手机、电脑等）上打开WiFi设置
2. 找到名为 `XIAO_ESP32S3_FileServer` 的WiFi热点
3. 输入密码：`12345678`
4. 连接到该热点

### 3. 访问文件服务器

#### 方法一：浏览器访问
在浏览器中访问：`http://192.168.4.1`
- 将显示一个美观的文件列表页面
- 显示当前连接设备数和AP状态
- 点击文件名即可下载

#### 方法二：Python 脚本下载

1. 确保已连接到ESP32的WiFi热点
2. 运行 Python 脚本：
   ```bash
   python3 download_files.py
   ```

3. 输入ESP32的IP地址（默认：192.168.4.1）

4. 选择下载方式：
   - 下载所有文件
   - 选择特定文件
   - 退出

### 4. API 接口

#### 获取文件列表
```
GET http://192.168.4.1/list
```
返回 JSON 格式的文件列表：
```json
{
  "files": [
    {"name": "test.txt", "size": 1024},
    {"name": "data.csv", "size": 2048}
  ]
}
```

#### 下载文件
```
GET http://192.168.4.1/file?name=文件名
```
- 支持的文件类型：txt, csv, json, xml, html, css, js, jpg, png, gif, pdf, zip 等
- 自动设置正确的 MIME 类型
- 支持分块传输

#### 获取AP状态
```
GET http://192.168.4.1/status
```
返回 JSON 格式的AP状态信息：
```json
{
  "ap_ssid": "XIAO_ESP32S3_FileServer",
  "ap_ip": "192.168.4.1",
  "connected_devices": 2,
  "max_connections": 4,
  "free_memory": 123456,
  "uptime": 60000
}
```

## 功能特性

### Arduino 端功能
- ✅ AP模式WiFi热点创建
- ✅ 多设备连接支持（最多4个）
- ✅ SD 卡文件系统访问
- ✅ Web 服务器（HTTP）
- ✅ 文件列表显示（HTML + JSON）
- ✅ 文件下载服务（支持大文件分块传输）
- ✅ 正确的 MIME 类型支持
- ✅ 错误处理和状态反馈
- ✅ 美观的 Web 界面
- ✅ 内存使用监控
- ✅ 传输进度显示
- ✅ 连接设备监控
- ✅ AP状态信息API

### Python 端功能
- ✅ 自动获取文件列表
- ✅ 批量下载文件
- ✅ 交互式文件选择
- ✅ 下载进度显示
- ✅ 错误处理和重试
- ✅ 支持断点续传
- ✅ 自动创建下载目录

## 故障排除

### 常见问题

1. **SD 卡初始化失败**
   - 检查 SD 卡是否正确插入
   - 确认 SD 卡格式为 FAT32
   - 尝试不同的 CS 引脚设置

2. **无法连接到ESP32的WiFi热点**
   - 检查ESP32是否已启动AP模式
   - 确认WiFi热点名称是否正确（XIAO_ESP32S3_FileServer）
   - 确认密码是否正确（12345678）
   - 检查ESP32是否正常工作

3. **无法访问文件服务器**
   - 确认已连接到ESP32的WiFi热点
   - 检查IP地址是否正确（192.168.4.1）
   - 确认防火墙设置
   - 检查连接设备数是否超过限制（最多4个）

4. **Python 脚本连接失败**
   - 确认已连接到ESP32的WiFi热点
   - 检查 IP 地址输入是否正确（默认：192.168.4.1）
   - 确认网络连接正常
   - 检查 Python 依赖包是否安装

5. **大文件下载失败**
   - 检查可用内存（串口会显示内存使用情况）
   - 确认网络连接稳定
   - 观察传输进度，如果卡住可能是网络问题
   - 尝试重新下载或重启设备

### 大文件传输优化说明

本项目已针对大文件传输进行了优化：

- **分块传输**：每次传输1KB数据，避免内存溢出
- **进度监控**：每10KB显示一次传输进度
- **连接检查**：实时检查客户端连接状态
- **超时保护**：5分钟传输超时保护
- **内存监控**：每30秒显示可用内存
- **WiFi优化**：禁用睡眠模式，设置最大发射功率

### 测试大文件传输

1. 运行 `create_test_file.py` 创建测试文件：
   ```bash
   python3 create_test_file.py
   ```

2. 将生成的测试文件复制到SD卡

3. 通过浏览器或Python脚本测试下载

4. 观察串口输出的传输进度和内存使用情况

### 调试信息

通过串口监视器可以查看详细的调试信息：
- WiFi 连接状态
- SD 卡初始化状态
- 文件访问日志
- 下载请求信息

## 扩展功能

### 可以添加的功能
- 文件上传功能
- 文件删除功能
- 目录浏览功能
- 文件搜索功能
- 用户认证
- HTTPS 支持
- 文件压缩

### 性能优化
- ✅ 分块传输（避免内存溢出）
- ✅ WiFi 优化设置（禁用睡眠，最大功率）
- ✅ 32KB 缓冲区设置
- ✅ 内存使用监控
- ✅ 传输超时保护
- ✅ AP模式优化（固定IP，多连接支持）
- 使用 SPIFFS 替代 SD 卡（小文件）
- 启用 GZIP 压缩
- 实现文件缓存

## 许可证

本项目采用 MIT 许可证。

## 贡献

欢迎提交 Issue 和 Pull Request！

## 联系方式

如有问题，请通过以下方式联系：
- 提交 GitHub Issue
- 发送邮件至：[您的邮箱]

---

**注意**：请确保您的网络环境安全，避免在公共网络中暴露敏感文件。 