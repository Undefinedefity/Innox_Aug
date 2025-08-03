  
// constants won't change. They're used here to set pin numbers:
const int green_LED_Pin = 2;    // the number of the LED pin
const int blue_LED_Pin = 3;    // the number of the LED pin
const int startbuttonPin = 4;  // the number of the pushbutton pin

// 按键状态变量
int buttonState = HIGH;        // 当前按键状态
int lastButtonState = HIGH;    // 上次按键状态
int ledState = 0;             // LED状态：0=都灭，1=绿灯亮，2=蓝灯亮

/*
GPIO2  D1	  GREENLED  
GPIO3  D2 	REDLED
GPIO4  D3 	BUTTON
按键第一次按下， 绿灯亮，第二次按下， 蓝灯亮，绿灯灭，第三次按下绿灯亮，以此类推
*/

void setup() {
  // 初始化串口通信
  Serial.begin(115200);
  
  // 设置引脚模式
  pinMode(startbuttonPin, INPUT_PULLUP);  // 按键引脚，启用上拉电阻
  pinMode(green_LED_Pin, OUTPUT);         // 绿灯引脚
  pinMode(blue_LED_Pin, OUTPUT);          // 蓝灯引脚
  
  // 初始化LED状态（都熄灭）
  digitalWrite(green_LED_Pin, LOW);
  digitalWrite(blue_LED_Pin, LOW);
  
  Serial.println("按键LED控制程序启动");
  Serial.println("按键按下时切换LED状态");
}

void loop() {
  // 读取按键状态
  buttonState = digitalRead(startbuttonPin);
  
  // 检测按键按下（按键按下时引脚为LOW）
  if (buttonState == LOW && lastButtonState == HIGH) {
    // 按键被按下，切换LED状态
    ledState = (ledState + 1) % 3;  // 循环切换：0->1->2->0
    
    // 根据状态控制LED
    switch (ledState) {
      case 0:  // 都熄灭
        digitalWrite(green_LED_Pin, LOW);
        digitalWrite(blue_LED_Pin, LOW);
        Serial.println("状态0: 所有LED熄灭");
        break;
      case 1:  // 绿灯亮
        digitalWrite(green_LED_Pin, HIGH);
        digitalWrite(blue_LED_Pin, LOW);
        Serial.println("状态1: 绿灯亮");
        break;
      case 2:  // 蓝灯亮
        digitalWrite(green_LED_Pin, LOW);
        digitalWrite(blue_LED_Pin, HIGH);
        Serial.println("状态2: 蓝灯亮");
        break;
    }
    
    // 防抖延时
    delay(50);
  }
  
  // 更新上次按键状态
  lastButtonState = buttonState;
  
  // 短暂延时
  delay(10);
}
