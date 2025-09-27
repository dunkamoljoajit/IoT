#define BLYNK_TEMPLATE_ID           "TMPL6RLdUYDMd"
#define BLYNK_TEMPLATE_NAME         "Quickstart Template"
#define BLYNK_AUTH_TOKEN            "vvTJWdm1Jwp51r81L9Er1qAYcCIR5Md"
#define WIFI_SSID "vivo Y3s"       
#define WIFI_PASSWORD "05022567"   

#include <ESP8266WiFi.h>         
#include <BlynkSimpleEsp8266.h>  
#include <Servo.h>               

Servo myServo;   
BlynkTimer timer;           

const int waterPin = A0;     
const int servoPin = D5;     
const int SERVO_OPEN_ANGLE  = 0;
const int SERVO_CLOSE_ANGLE = 120;
const int WATER_THRESHOLD   = 60;

int servoState = SERVO_OPEN_ANGLE;
int waterValue = 0;

void moveServo(int angle) {
  if (servoState != angle) {
    myServo.write(angle);     
    servoState = angle;       
    Blynk.virtualWrite(V0, (angle == SERVO_CLOSE_ANGLE) ? 1 : 0);
  }
}

void checkWaterSensor() {
  waterValue = analogRead(waterPin);         
  Blynk.virtualWrite(V1, waterValue);        
  Serial.print("Water sensor value: ");      
  Serial.println(waterValue);                

  if (waterValue > WATER_THRESHOLD) {
    if (servoState != SERVO_CLOSE_ANGLE) {
      moveServo(SERVO_CLOSE_ANGLE);             
      Serial.println("Sensor: ตรวจพบฝน → สั่ง Servo หุบ (อัตโนมัติ)"); 
    } else {
      Blynk.virtualWrite(V0, 1); 
    }
  }
}

BLYNK_WRITE(V0) {
  int buttonState = param.asInt();     
  waterValue = analogRead(waterPin);   

  if (waterValue > WATER_THRESHOLD) {
    if (buttonState == 0) {
      Serial.println("ฝนตก → ไม่อนุญาตให้กาง Manual! Servo หุบ");
      moveServo(SERVO_CLOSE_ANGLE);     
      Blynk.virtualWrite(V0, 1);         
      return; 
    } 
    Serial.println("ฝนตก → รับคำสั่ง Manual 'หุบ' (แต่ระบบอัตโนมัติก็หุบอยู่แล้ว)");
  } else { 
    if (buttonState == 1) {
      moveServo(SERVO_CLOSE_ANGLE);     
      Serial.println("Manual: กด ON → Servo หุบ");
    } else { 
      moveServo(SERVO_OPEN_ANGLE);      
      Serial.println("Manual: กด OFF → Servo กาง");
    }
  }
}

void setupServoSystem() {
  myServo.attach(servoPin);                 
  moveServo(SERVO_OPEN_ANGLE);              
}

void setupBlynkConnection() {
  Serial.println("กำลังเชื่อมต่อ WiFi และ Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASSWORD); 
}

void setupTimers() {
  timer.setInterval(500L, checkWaterSensor); 
}

void setup() {
  Serial.begin(115200);          
  setupServoSystem();            
  setupBlynkConnection();        
  checkWaterSensor();            
  setupTimers();                 
}

void loop() {
  Blynk.run();                   
  timer.run();                   
}
