// 定义按键引脚
const int buttonPin = 4;

void setup() {
  // 初始化串口通信
  Serial.begin(115200);
  
  // 设置按键引脚为输入，启用内部上拉电阻
  pinMode(buttonPin, INPUT_PULLUP);
  
  Serial.println("引脚电平检测程序启动");
  Serial.println("实时输出引脚4的电平状态");
}

void loop() {
  // 读取引脚电平
  int pinState = digitalRead(buttonPin);
  
  // 输出引脚状态
  Serial.print("引脚4电平: ");
  if (pinState == HIGH) {
    Serial.println("HIGH (高电平)");
  } else {
    Serial.println("LOW (低电平)");
  }
  
  // 延时500毫秒，避免输出过于频繁
  delay(500);
}
