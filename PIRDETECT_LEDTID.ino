#define PIR D1
#define LED_PIN D6

int val = 0;
bool motionState = false;
unsigned long lastMotionTime = 0;   // เวลา ล่าสุดที่ตรวจเจอการเคลื่อนไหว
const unsigned long timeout = 5000; // หน่วงเวลา 5 วินาทีหลังจากไม่มีการเคลื่อนไหว

void setup() {
  pinMode(PIR, INPUT);
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  val = digitalRead(PIR);

  if (val == HIGH) {                         // ถ้ามีการเคลื่อนไหว
    digitalWrite(LED_PIN, HIGH);             // LED ติด
    lastMotionTime = millis();               // เก็บเวลาล่าสุดที่ตรวจเจอ
    if (!motionState) {                      // ถ้าเพิ่งตรวจจับได้ครั้งแรก
      Serial.println("Motion Detected!");
      motionState = true;
    }
  } else {
    if (millis() - lastMotionTime > timeout) { // ถ้าไม่มีการเคลื่อนไหวเกิน 5 วิ
      digitalWrite(LED_PIN, LOW);             // LED ดับ
      if (motionState) {                      // ถ้าสถานะก่อนหน้านี้มีการเคลื่อนไหว
        Serial.println("Motion Ended!");
        motionState = false;
      }
    }
  }

  delay(100);
}
