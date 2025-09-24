#include <Servo.h>  // เรียกใช้ไลบรารี Servo สำหรับควบคุมมอเตอร์

Servo myServo;      // สร้างวัตถุ Servo ชื่อ myServo

const int waterPin = A0;     // กำหนดขา Analog ของ Water Sensor เป็น A0
const int servoPin = D5;     // กำหนดขา Signal ของ Servo เป็น D5
const int threshold = 50;    // กำหนดค่าต่ำสุดสำหรับจับน้ำหยดเดียว (ปรับได้ตามจริง)

void setup() {
  myServo.attach(servoPin);  // เชื่อมต่อ Servo กับขาที่กำหนด
  myServo.write(0);          // เริ่มต้น Servo ที่มุม 0° = กางราวผ้า
  Serial.begin(115200);      // เริ่ม Serial Monitor สำหรับแสดงค่าที่อ่านได้
}

// ฟังก์ชันหมุน Servo แบบนุ่มนวล
void moveServoSmooth(int startAngle, int endAngle, int stepDelay = 15) {
  if(startAngle < endAngle){           // ตรวจสอบว่ามุมเริ่มต้นน้อยกว่ามุมปลายทาง
    for(int pos = startAngle; pos <= endAngle; pos++){  // หมุน Servo ทีละ 1° จาก start → end
      myServo.write(pos);              // ส่งค่าให้ Servo หมุนไปตำแหน่งปัจจุบัน
      delay(stepDelay);                // หน่วงเวลาเล็กน้อยเพื่อให้การหมุนเรียบ
    }
  } else {                             // กรณี startAngle > endAngle
    for(int pos = startAngle; pos >= endAngle; pos--){ // หมุน Servo จาก start → end ลดทีละ 1°
      myServo.write(pos);              // ส่งค่าให้ Servo หมุนไปตำแหน่งปัจจุบัน
      delay(stepDelay);                // หน่วงเวลาเล็กน้อย
    }
  }
}

void loop() {
  int waterValue = analogRead(waterPin);  // อ่านค่าจาก Water Sensor (0-1023)
  Serial.print("Water sensor value: ");   // แสดงข้อความก่อนค่าที่อ่านได้
  Serial.println(waterValue);             // แสดงค่าที่อ่านจากเซ็นเซอร์

  if (waterValue > threshold) {           // ถ้าค่าเซ็นเซอร์มากกว่า threshold → เจอน้ำ
    Serial.println("Water detected! Folding clothesline.");  // แสดงข้อความว่าพับราว
    moveServoSmooth(myServo.read(), 90);  // หมุน Servo ไป 90° แบบเรียบ → พับราว
  } else {                                // ถ้าค่าเซ็นเซอร์น้อยกว่าหรือเท่ากับ threshold → ไม่มีน้ำ
    Serial.println("No water. Clothesline open.");           // แสดงข้อความว่ากางราว
    moveServoSmooth(myServo.read(), 0);   // หมุน Servo ไป 0° แบบเรียบ → กางราว
  }

  delay(500); // หน่วงเวลา 0.5 วินาที ก่อนอ่านค่าใหม่
}
