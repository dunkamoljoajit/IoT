/****************************************************
 * ESP8266 + Blynk IoT + Servo + Water Sensor
 * Sensor priority: มีน้ำ → หุบทันที
 * Manual priority: ไม่มีน้ำ → ปุ่มกดปุ๊บ Servo ทำปั๊บ
 ****************************************************/

#define BLYNK_TEMPLATE_ID "TMPL6RLdUYDMd"   
#define BLYNK_TEMPLATE_NAME "Quickstart Template" 
#define BLYNK_AUTH_TOKEN "jlSfwbE3VgfJBP1oVqZhAiS0oYaew9oe" 

#define WIFI_SSID "vivo Y3s"       
#define WIFI_PASSWORD "05022567"   

#include <ESP8266WiFi.h>         
#include <BlynkSimpleEsp8266.h>  
#include <Servo.h>               

Servo myServo; 

// -------------------- ขา --------------------
const int waterPin = A0;     
const int servoPin = D5;     

const int SERVO_OPEN_ANGLE  = 0;   // กางราว
const int SERVO_CLOSE_ANGLE = 85;  // หุบราว
const int WATER_THRESHOLD   = 60;  // ค่า sensor ตัดสินน้ำ

int servoState = SERVO_OPEN_ANGLE;  
int waterValue = 0;                 // ค่าจาก sensor

BlynkTimer timer;          

// -------------------- ตรวจน้ำทุก 0.5s --------------------
void checkWaterSensor() {
  waterValue = analogRead(waterPin);
  Serial.print("Water sensor value: ");
  Serial.println(waterValue);

  // มีน้ำ → Sensor ชนะเสมอ
  if (waterValue > WATER_THRESHOLD && servoState != SERVO_CLOSE_ANGLE) {
    myServo.write(SERVO_CLOSE_ANGLE);
    servoState = SERVO_CLOSE_ANGLE;
    Blynk.virtualWrite(V0, 1); // sync ปุ่ม → หุบ
    Serial.println("Sensor: พบฝน → Servo หุบ");
  }
}

// -------------------- ปุ่ม Manual (V0) --------------------
BLYNK_WRITE(V0) {
  int buttonState = param.asInt();

  // อ่าน sensor แบบ real-time ก่อน
  waterValue = analogRead(waterPin);

  // ถ้ามีน้ำ → Sensor ชนะ Manual
  if (waterValue > WATER_THRESHOLD) {
    Serial.println("ฝนตก → Servo หุบ (Manual ใช้ไม่ได้)");
    myServo.write(SERVO_CLOSE_ANGLE);
    servoState = SERVO_CLOSE_ANGLE;
    Blynk.virtualWrite(V0, 1); // บังคับปุ่มกลับไปหุบ
    return;
  }

  // ถ้าไม่มีน้ำ → Manual ชนะ (กดปุ๊บ Servo ทำปั๊บ)
  if (buttonState == 1) {
    myServo.write(SERVO_CLOSE_ANGLE);
    servoState = SERVO_CLOSE_ANGLE;
    Serial.println("Manual: กด ON → Servo หุบ");
  } else {
    myServo.write(SERVO_OPEN_ANGLE);
    servoState = SERVO_OPEN_ANGLE;
    Serial.println("Manual: กด OFF → Servo กาง");
  }
}

// -------------------- Setup --------------------
void setup() {
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASSWORD);
  
  myServo.attach(servoPin);
  myServo.write(SERVO_OPEN_ANGLE);   
  servoState = SERVO_OPEN_ANGLE;

  Blynk.syncVirtual(V0);             // sync ปุ่มตอนเริ่ม
  checkWaterSensor();                // ตรวจทันทีตอนบูต
  timer.setInterval(500L, checkWaterSensor); // ตรวจน้ำทุก 0.5s
}

// -------------------- Loop --------------------
void loop() {
  Blynk.run();
  timer.run();
}
