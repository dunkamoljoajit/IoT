// กำหนดข้อมูล Blynk IoT (ต้องใช้จากแอป Blynk)
#define BLYNK_TEMPLATE_ID           "TMPL6RLdUYDMd"
#define BLYNK_TEMPLATE_NAME         "Quickstart Template"
#define BLYNK_AUTH_TOKEN            "vvTJWdm1Jwp51r81L9Er1qAYcCIR5Md"
// กำหนดชื่อ WiFi และรหัสผ่านที่ต้องการเชื่อมต่อ
#define WIFI_SSID "vivo Y3s"       
#define WIFI_PASSWORD "05022567"   

// เรียกใช้ไลบรารีของ ESP8266 WiFi
#include <ESP8266WiFi.h>         

// ไลบรารีของ Blynk สำหรับ ESP8266
#include <BlynkSimpleEsp8266.h>  

// ไลบรารีควบคุม Servo Motor
#include <Servo.h>               

// สร้างอ็อบเจ็กต์ควบคุม Servo
Servo myServo;   

// สร้างตัวแปรจับเวลา (ใช้แทน delay)
BlynkTimer timer;           

// -------------------- Pin และค่าคงที่ --------------------

// ขาอ่านค่าน้ำ (Analog)
const int waterPin = A0;     

// ขาเชื่อมต่อ Servo (Digital)
// NOTE: สำหรับ ESP8266, D5 คือ GPIO14
const int servoPin = D5;     

// มุมเซอร์โวเมื่อกาง (Open)
const int SERVO_OPEN_ANGLE  = 0;

// มุมเซอร์โวเมื่อหุบ (Close/Retract)
const int SERVO_CLOSE_ANGLE = 120;

// ค่าที่ใช้ตัดสินว่ามีน้ำหรือไม่ (ค่าต่ำกว่านี้คือแห้ง)
const int WATER_THRESHOLD   = 60;

// เก็บสถานะปัจจุบันของเซอร์โว (กาง/หุบ) เพื่อใช้ในการตรวจสอบเงื่อนไข
int servoState = SERVO_OPEN_ANGLE;

// เก็บค่าที่อ่านได้จากเซนเซอร์น้ำ
int waterValue = 0;

// -------------------- ฟังก์ชันควบคุม Servo --------------------

// ฟังก์ชันสำหรับสั่งให้ Servo หมุนไปที่มุมที่ต้องการ
void moveServo(int angle) {
  // ตรวจสอบว่ามุมที่สั่งไม่ซ้ำกับสถานะปัจจุบัน เพื่อลดการส่งคำสั่งที่ไม่จำเป็น
  if (servoState != angle) {
    myServo.write(angle);     // สั่งเซอร์โวหมุนไปยังมุมที่กำหนด
    servoState = angle;       // บันทึกสถานะล่าสุดของเซอร์โว
    // อัปเดตสถานะปุ่ม Blynk (V0) ทุกครั้งที่สถานะเซอร์โวเปลี่ยน
    Blynk.virtualWrite(V0, (angle == SERVO_CLOSE_ANGLE) ? 1 : 0);
  }
}

// -------------------- ฟังก์ชันเช็กค่าน้ำ (โหมดอัตโนมัติ) --------------------

// ฟังก์ชันนี้จะถูกเรียกทุกๆ 0.5 วินาที
void checkWaterSensor() {
  waterValue = analogRead(waterPin);         // อ่านค่าจากเซนเซอร์น้ำ
  Blynk.virtualWrite(V1, waterValue);        // ส่งค่าเซนเซอร์น้ำไป V1 (ถ้ามี Datastream V1)
  
  Serial.print("Water sensor value: ");      // พิมพ์ข้อความลง Serial
  Serial.println(waterValue);                // แสดงค่าที่อ่านได้

  // ถ้าค่ามากกว่าเกณฑ์ แสดงว่ามีน้ำ และเซอร์โวยังไม่ได้หุบ
  if (waterValue > WATER_THRESHOLD) {
    // สั่งหุบ (SERVO_CLOSE_ANGLE)
    if (servoState != SERVO_CLOSE_ANGLE) {
      moveServo(SERVO_CLOSE_ANGLE);             // หุบราวทันที (และอัพเดต servoState, Blynk V0)
      Serial.println("Sensor: ตรวจพบฝน → สั่ง Servo หุบ (อัตโนมัติ)"); 
    } else {
      // ถ้าหุบอยู่แล้ว, ตรวจสอบให้แน่ใจว่าปุ่ม Blynk เป็น ON
      Blynk.virtualWrite(V0, 1); 
    }
  }
}

// -------------------- ฟังก์ชันจัดการ Manual จาก Blynk --------------------

// ฟังก์ชันนี้จะทำงานเมื่อผู้ใช้กดปุ่มบนแอป Blynk
BLYNK_WRITE(V0) {
  int buttonState = param.asInt();     // 1 = ON (หุบ), 0 = OFF (กาง)
  
  // ตรวจสอบสถานะฝนซ้ำก่อนอนุญาตให้ทำงาน Manual
  waterValue = analogRead(waterPin);   
  
  // -------------- ระบบความปลอดภัย: ฝนตกต้องหุบเท่านั้น --------------
  if (waterValue > WATER_THRESHOLD) {
    // ถ้ามีน้ำ และผู้ใช้กำลังพยายาม 'กาง' (buttonState == 0)
    if (buttonState == 0) {
      Serial.println("ฝนตก → ไม่อนุญาตให้กาง Manual! Servo หุบ");
      moveServo(SERVO_CLOSE_ANGLE);     // สั่งหุบ (และอัพเดต servoState)
      Blynk.virtualWrite(V0, 1);         // บังคับปุ่มกลับเป็น ON (หุบ)
      return; // ออกจากฟังก์ชัน
    } 
    // ถ้าฝนตกและผู้ใช้กด 'หุบ' (buttonState == 1) ก็ให้หุบตามคำสั่ง (ปกติ)
    Serial.println("ฝนตก → รับคำสั่ง Manual 'หุบ' (แต่ระบบอัตโนมัติก็หุบอยู่แล้ว)");
  }

  // -------------- โหมด Manual (เมื่อไม่มีน้ำ) --------------
  else { 
    // ไม่มีน้ำ → ใช้ manual ได้
    if (buttonState == 1) { // ผู้ใช้สั่ง ON (หุบ)
      moveServo(SERVO_CLOSE_ANGLE);     // หุบทันที (ใช้ moveServo เพื่ออัปเดต servoState)
      Serial.println("Manual: กด ON → Servo หุบ");
    } else { // ผู้ใช้สั่ง OFF (กาง)
      moveServo(SERVO_OPEN_ANGLE);      // กางทันที (ใช้ moveServo เพื่ออัปเดต servoState)
      Serial.println("Manual: กด OFF → Servo กาง");
    }
  }
}


// -------------------- ฟังก์ชันตั้งค่าระบบ --------------------

// ฟังก์ชันตั้งค่าเซอร์โวเริ่มต้น
void setupServoSystem() {
  myServo.attach(servoPin);                 // กำหนดขาเซอร์โว
  moveServo(SERVO_OPEN_ANGLE);              // กางราวตอนเริ่มต้น (และอัพเดต servoState)
}

// ฟังก์ชันเชื่อมต่อ Blynk และ sync ปุ่ม
void setupBlynkConnection() {
  Serial.println("กำลังเชื่อมต่อ WiFi และ Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASSWORD); // เชื่อม Blynk
}

// ฟังก์ชันตั้งเวลาตรวจค่าน้ำ
void setupTimers() {
  timer.setInterval(500L, checkWaterSensor); // เรียก checkWaterSensor ทุก 0.5 วินาที
}

// -------------------- setup() --------------------

// ฟังก์ชันเริ่มต้นทำงาน (ทำครั้งเดียวตอนบูต)
void setup() {
  Serial.begin(115200);          // เริ่ม Serial Monitor ที่ 115200 bps

  setupServoSystem();            // ตั้งค่าเซอร์โว
  setupBlynkConnection();        // เชื่อมต่อ Blynk
  checkWaterSensor();            // ตรวจค่าน้ำทันที
  setupTimers();                 // เริ่มตั้งเวลาให้ตรวจน้ำทุก 0.5 วินาที
}

// -------------------- loop() --------------------

// ฟังก์ชันที่ทำงานวนลูปตลอดเวลา
void loop() {
  Blynk.run();                   // รัน Blynk เพื่อรับส่งข้อมูล
  timer.run();                   // ทำงานตามเวลา timer ที่กำหนด
}
