#include <SD.h>


// 初始化文件对象
File my_file;


void setup() {
  Serial.begin(115200);
  
  if (!SD.begin()) {
    Serial.println("SD卡初始化失败");
    return;
  }

  Serial.println("SD卡初始化成功");

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
}

void loop() {
  // 在这里进行其他操作
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
