#define PIR D1
#define LED_PIN D6

int val = 0;
bool motionState = false;
unsigned long lastMotionTime = 0;
const unsigned long timeout = 5000;

void setup() {
  pinMode(PIR, INPUT);
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  checkMotion();
  delay(100);
}

void checkMotion() {
  val = digitalRead(PIR);

  if (val == HIGH) {
    handleMotionDetected();
  } else {
    handleNoMotion();
  }
}

void handleMotionDetected() {
  digitalWrite(LED_PIN, HIGH);
  lastMotionTime = millis();
  if (!motionState) {
    Serial.println("Motion Detected!");
    motionState = true;
  }
}

void handleNoMotion() {
  if (millis() - lastMotionTime > timeout) {
    digitalWrite(LED_PIN, LOW);
    if (motionState) {
      Serial.println("Motion Ended!");
      motionState = false;
    }
  }
}
