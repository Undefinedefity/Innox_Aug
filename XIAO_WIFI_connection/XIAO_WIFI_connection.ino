#include <WiFi.h>
#include <WebServer.h>
#include <SD.h>
#include <SPI.h>

// ESP32 AP模式配置
const char *ap_ssid = "XIAO_ESP32S3";
const char *ap_password = "12345678";
const int ap_channel = 1;  // WiFi频道
const int ap_max_connections = 4;  // 最大连接数

WebServer server(80);

// XIAO ESP32S3 的 SD 卡 CS 引脚定义
#define SD_CS_PIN 21 // XIAO ESP32S3 Sense 使用 GPIO21 作为 CS 引脚

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println("XIAO ESP32S3 AP模式文件服务器启动中...");

  // 优化 WiFi 设置以提高传输速度
  WiFi.setSleep(false);  // 禁用 WiFi 睡眠模式
  WiFi.setTxPower(WIFI_POWER_19_5dBm);  // 设置最大发射功率
  
  // 设置WiFi模式为AP模式
  WiFi.mode(WIFI_AP);
  
  // 配置AP参数
  WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
  
  // 启动AP
  if (WiFi.softAP(ap_ssid, ap_password, ap_channel, false, ap_max_connections)) {
    Serial.println("ESP32 AP模式启动成功!");
    Serial.print("热点名称 (SSID): ");
    Serial.println(ap_ssid);
    Serial.print("热点密码: ");
    Serial.println(ap_password);
    Serial.print("AP IP 地址: ");
    Serial.println(WiFi.softAPIP());
    Serial.print("最大连接数: ");
    Serial.println(ap_max_connections);
  } else {
    Serial.println("ESP32 AP模式启动失败!");
    ESP.restart();
  }

  // 初始化 SD 卡 - XIAO ESP32S3 Sense 使用 GPIO21 作为 CS 引脚
  if (!SD.begin(SD_CS_PIN))
  {
    Serial.println("SD 卡初始化失败！请检查：");
    Serial.println("1. SD 卡是否正确插入");
    Serial.println("2. CS 引脚是否正确设置 (XIAO ESP32S3 Sense 使用 GPIO21)");
    Serial.println("3. 如果还是失败，尝试使用 SD.begin() 不带参数");
    Serial.println("4. 检查 J3 焊盘是否连接以启用 microSD 卡功能");
    return;
  }
  Serial.println("SD 卡初始化成功！");

  // 设置路由：根路径 - 显示文件列表
  server.on("/", HTTP_GET, []()
            {
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
    html += "<title>XIAO ESP32S3 文件服务器</title>";
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
    html += "<h1>📁 XIAO ESP32S3 文件服务器</h1>";
    html += "<div class='info'>";
    html += "<strong>AP IP 地址:</strong> " + WiFi.softAPIP().toString() + "<br>";
    html += "<strong>热点名称:</strong> " + String(ap_ssid) + "<br>";
    html += "<strong>连接设备数:</strong> " + String(WiFi.softAPgetStationNum()) + "/" + String(ap_max_connections) + "<br>";
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

  // 设置路由：/status - 返回AP状态信息
  server.on("/status", HTTP_GET, []()
            {
    String json = "{";
    json += "\"ap_ssid\":\"" + String(ap_ssid) + "\",";
    json += "\"ap_ip\":\"" + WiFi.softAPIP().toString() + "\",";
    json += "\"connected_devices\":" + String(WiFi.softAPgetStationNum()) + ",";
    json += "\"max_connections\":" + String(ap_max_connections) + ",";
    json += "\"free_memory\":" + String(ESP.getFreeHeap()) + ",";
    json += "\"uptime\":" + String(millis());
    json += "}";
    server.send(200, "application/json; charset=utf-8", json); });

  // 优化服务器设置
  server.enableCORS(true);  // 启用跨域支持
  server.enableCrossOrigin(true);  // 启用跨域
  
  server.begin();
  Serial.println("HTTP 服务器已启动");
  Serial.println("访问 http://" + WiFi.softAPIP().toString() + " 查看文件列表");
  Serial.println("使用 /file?name=文件名 下载特定文件");
  Serial.println("使用 /list 获取 JSON 格式的文件列表");
  Serial.println("优化设置: AP模式，WiFi 睡眠禁用，最大发射功率");
}

void loop()
{
  server.handleClient();

  // 定期显示AP状态和内存使用情况（每30秒）
  static unsigned long lastStatusCheck = 0;
  if (millis() - lastStatusCheck > 30000) {
    Serial.printf("可用内存: %d 字节, 连接设备数: %d/%d\n", 
                  ESP.getFreeHeap(), 
                  WiFi.softAPgetStationNum(), 
                  ap_max_connections);
    
    // 显示连接的设备信息
    if (WiFi.softAPgetStationNum() > 0) {
      Serial.println("连接的设备:");
      Serial.printf("  当前连接数: %d/%d\n", WiFi.softAPgetStationNum(), ap_max_connections);
    }
    
    lastStatusCheck = millis();
  }
}
