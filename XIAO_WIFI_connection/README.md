# XIAO ESP32S3 文件服务器

这是一个基于 XIAO ESP32S3 开发板的 WiFi 文件服务器项目，允许您通过 WiFi 从开发板的 microSD 卡中下载文件。

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

1. 打开 `WIFI_connection.ino` 文件
2. 修改 WiFi 配置：
   ```cpp
   const char* ssid = "您的WiFi名称";
   const char* password = "您的WiFi密码";
   ```
3. 如果需要，调整 SD 卡 CS 引脚：
   ```cpp
   #define SD_CS_PIN 10  // 根据您的开发板调整
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

### 1. 启动服务器

1. 将代码上传到开发板
2. 打开串口监视器（波特率：115200）
3. 观察连接状态，记录显示的 IP 地址

### 2. 访问文件服务器

#### 方法一：浏览器访问
在浏览器中访问：`http://[ESP32的IP地址]`
- 例如：`http://192.168.1.100`
- 将显示一个美观的文件列表页面
- 点击文件名即可下载

#### 方法二：Python 脚本下载

1. 运行 Python 脚本：
   ```bash
   python3 download_files.py
   ```

2. 输入 ESP32S3 的 IP 地址

3. 选择下载方式：
   - 下载所有文件
   - 选择特定文件
   - 退出

### 3. API 接口

#### 获取文件列表
```
GET http://[IP地址]/list
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
GET http://[IP地址]/file?name=文件名
```
- 支持的文件类型：txt, csv, json, xml, html, css, js, jpg, png, gif, pdf, zip 等
- 自动设置正确的 MIME 类型
- 支持断点续传

## 功能特性

### Arduino 端功能
- ✅ WiFi 连接和自动重连
- ✅ SD 卡文件系统访问
- ✅ Web 服务器（HTTP）
- ✅ 文件列表显示（HTML + JSON）
- ✅ 文件下载服务
- ✅ 正确的 MIME 类型支持
- ✅ 错误处理和状态反馈
- ✅ 美观的 Web 界面

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

2. **WiFi 连接失败**
   - 检查 WiFi 名称和密码
   - 确认网络信号强度
   - 检查路由器设置

3. **无法访问文件服务器**
   - 确认 ESP32S3 已成功连接到 WiFi
   - 检查 IP 地址是否正确
   - 确认防火墙设置

4. **Python 脚本连接失败**
   - 检查 IP 地址输入是否正确
   - 确认网络连接正常
   - 检查 Python 依赖包是否安装

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
- 使用 SPIFFS 替代 SD 卡（小文件）
- 启用 GZIP 压缩
- 实现文件缓存
- 优化内存使用

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