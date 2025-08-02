#include <WiFi.h>
#include <WebServer.h>
#include <SD.h>
#include <SPI.h>

const char *ssid = "Innoxsz-2.4G";
const char *password = "innox2.4#";

// const char *ssid = "ESP32-Hotspot";
// const char *password = "12345678";

WebServer server(80);

// ESP32 的 SD 卡 CS 引脚定义（根据您的硬件配置调整）
#define SD_CS_PIN 5 // 常见的 ESP32 SD 卡 CS 引脚，请根据实际硬件调整

// 初始化文件对象
File my_file;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("ESP32 SD 卡文件服务器启动中...");

  // 优化 WiFi 设置以提高传输速度
  WiFi.setSleep(false);  // 禁用 WiFi 睡眠模式
  WiFi.setTxPower(WIFI_POWER_19_5dBm);  // 设置最大发射功率

  // 连接 WiFi
  WiFi.begin(ssid, password);
  Serial.print("正在连接 WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi 连接成功!");
  Serial.print("IP 地址: ");
  Serial.println(WiFi.localIP());

  // 初始化 SD 卡
  if (!SD.begin(SD_CS_PIN))
  {
    Serial.println("SD 卡初始化失败！请检查：");
    Serial.println("1. SD 卡是否正确插入");
    Serial.println("2. CS 引脚是否正确设置");
    Serial.println("3. 如果还是失败，尝试使用 SD.begin() 不带参数");
    return;
  }
  Serial.println("SD 卡初始化成功！");

  // 原有的 SD 卡测试功能
  Serial.println("=== 开始 SD 卡功能测试 ===");
  
  // 读取文件列表
  list_files("/");

  // 创建一个新的文件并写入数据
  my_file = SD.open("/test.txt", FILE_WRITE); 

  if (my_file) {
    my_file.println("你好，SD 卡"); // 写入数据
    my_file.close(); // 关闭文件
    Serial.println("数据写入完成");
  } else {
    Serial.println("无法打开文件");
  }

  delay(1000);

  // 读取文件内容
  my_file = SD.open("/test.txt");
  while (my_file.available()) {
      Serial.write(my_file.read());
    }
  my_file.close();
  Serial.println("\n文件读取完成");

  delay(1000);

  // 修改文件内容
  my_file = SD.open("/test.txt", FILE_APPEND);
  my_file.println("修改文件");
  my_file.close();
  Serial.println("文件修改成功");

  delay(1000);

  // 读取文件内容
  Serial.println("读取文件内容");
  my_file = SD.open("/test.txt");
  while (my_file.available()) {
      Serial.write(my_file.read());
    }
  my_file.close();
  Serial.println("\n文件读取完成");

  delay(1000);

  // 删除文件
  if (SD.remove("/test.txt")) {
    Serial.println("文件删除成功");
  } else {
    Serial.println("文件删除失败");
  }

  // 读取文件列表
  list_files("/");
  
  Serial.println("=== SD 卡功能测试完成 ===");

  // 设置 Web 服务器路由
  setupWebServer();
  
  // 启动服务器
  server.begin();
  Serial.println("HTTP 服务器已启动");
  Serial.println("访问 http://" + WiFi.localIP().toString() + " 查看文件列表");
  Serial.println("使用 /file?name=文件名 下载特定文件");
  Serial.println("使用 /list 获取 JSON 格式的文件列表");
  Serial.println("优化设置: WiFi 睡眠禁用，最大发射功率");
}

void loop() {
  server.handleClient();

  // 检查 WiFi 连接状态
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi 连接断开，尝试重新连接...");
    WiFi.reconnect();
    delay(5000);
  }
}

// 设置 Web 服务器路由
void setupWebServer() {
  // 设置路由：根路径 - 显示文件列表
  server.on("/", HTTP_GET, []()
            {
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
    html += "<title>ESP32 文件服务器</title>";
    html += "<style>";
    html += "body{font-family:Arial,sans-serif;margin:20px;background:#f5f5f5;}";
    html += ".container{max-width:800px;margin:0 auto;background:white;padding:20px;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1);}";
    html += "h1{color:#333;text-align:center;}";
    html += ".file-list{list-style:none;padding:0;}";
    html += ".file-item{padding:10px;margin:5px 0;background:#f8f9fa;border-radius:5px;border-left:4px solid #007bff;}";
    html += ".file-link{color:#007bff;text-decoration:none;font-weight:bold;}";
    html += ".file-link:hover{text-decoration:underline;}";
    html += ".info{background:#e7f3ff;padding:10px;border-radius:5px;margin-bottom:20px;}";
    html += "</style></head><body>";
    html += "<div class='container'>";
    html += "<h1>📁 ESP32 文件服务器</h1>";
    html += "<div class='info'>";
    html += "<strong>IP 地址:</strong> " + WiFi.localIP().toString() + "<br>";
    html += "<strong>使用方法:</strong> 点击文件名下载，或使用 Python 脚本批量下载";
    html += "</div>";
    
    html += "<h2>📋 文件列表:</h2>";
    html += "<ul class='file-list'>";
    
    File root = SD.open("/");
    if (!root) {
      html += "<li class='file-item'>无法打开根目录</li>";
    } else {
      File file = root.openNextFile();
      while (file) {
        if (!file.isDirectory()) {
          String filename = String(file.name());
          if (filename.startsWith("/")) {
            filename = filename.substring(1);
          }
          html += "<li class='file-item'>";
          html += "<a href='/file?name=" + filename + "' class='file-link'>📄 " + filename + "</a>";
          html += " <small>(" + String(file.size()) + " 字节)</small>";
          html += "</li>";
        }
        file = root.openNextFile();
      }
      root.close();
    }
    
    html += "</ul>";
    html += "<p><small>💡 提示：您可以使用 Python 脚本通过 HTTP 请求下载这些文件</small></p>";
    html += "</div></body></html>";
    
    server.send(200, "text/html; charset=utf-8", html); });

  // 设置路由：/file?name=your_file.txt - 下载文件
  server.on("/file", HTTP_GET, []()
            {
    if (!server.hasArg("name")) {
      server.send(400, "text/plain; charset=utf-8", "错误：缺少文件名参数");
      return;
    }

    String filename = "/" + server.arg("name");
    if (!SD.exists(filename)) {
      server.send(404, "text/plain; charset=utf-8", "错误：文件不存在 - " + filename);
      return;
    }

    File file = SD.open(filename, "r");
    if (!file) {
      server.send(500, "text/plain; charset=utf-8", "错误：无法打开文件");
      return;
    }

    // 设置下载文件名
    String downloadName = server.arg("name");
    server.sendHeader("Content-Disposition", "attachment; filename=\"" + downloadName + "\"");
    
    // 根据文件扩展名设置正确的 MIME 类型
    String contentType = "application/octet-stream";
    if (downloadName.endsWith(".txt")) contentType = "text/plain";
    else if (downloadName.endsWith(".csv")) contentType = "text/csv";
    else if (downloadName.endsWith(".json")) contentType = "application/json";
    else if (downloadName.endsWith(".xml")) contentType = "application/xml";
    else if (downloadName.endsWith(".html") || downloadName.endsWith(".htm")) contentType = "text/html";
    else if (downloadName.endsWith(".css")) contentType = "text/css";
    else if (downloadName.endsWith(".js")) contentType = "application/javascript";
    else if (downloadName.endsWith(".jpg") || downloadName.endsWith(".jpeg")) contentType = "image/jpeg";
    else if (downloadName.endsWith(".png")) contentType = "image/png";
    else if (downloadName.endsWith(".gif")) contentType = "image/gif";
    else if (downloadName.endsWith(".pdf")) contentType = "application/pdf";
    else if (downloadName.endsWith(".zip")) contentType = "application/zip";
    
    server.sendHeader("Content-Type", contentType + "; charset=utf-8");
    server.sendHeader("Content-Length", String(file.size()));
    
    Serial.println("正在下载文件: " + filename + " (大小: " + String(file.size()) + " 字节)");
    
    server.streamFile(file, contentType);
    file.close(); });

  // 设置路由：/list - 返回 JSON 格式的文件列表（供 Python 脚本使用）
  server.on("/list", HTTP_GET, []()
            {
    String json = "{\"files\":[";
    bool first = true;
    
    File root = SD.open("/");
    if (root) {
      File file = root.openNextFile();
      while (file) {
        if (!file.isDirectory()) {
          if (!first) json += ",";
          String filename = String(file.name());
          if (filename.startsWith("/")) {
            filename = filename.substring(1);
          }
          json += "{\"name\":\"" + filename + "\",\"size\":" + String(file.size()) + "}";
          first = false;
        }
        file = root.openNextFile();
      }
      root.close();
    }
    
    json += "]}";
    server.send(200, "application/json; charset=utf-8", json); });

  // 优化服务器设置
  server.enableCORS(true);  // 启用跨域支持
  server.enableCrossOrigin(true);  // 启用跨域
}

// 读取文件列表
void list_files(const char* path) {
  Serial.println("文件列表：");

  File root = SD.open(path);

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("目录：");
    } else {
      Serial.print("文件：");
    }
    Serial.println(file.name());

    file = root.openNextFile();
  }
  root.close();
}
