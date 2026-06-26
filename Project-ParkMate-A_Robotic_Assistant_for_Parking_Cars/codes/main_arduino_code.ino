#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// ---------------- LCD ----------------
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------------- Pins (UNO) ----------------
const int TRIG_ENTRY = 2;
const int ECHO_ENTRY = 3;
const int TRIG_EXIT  = 4;
const int ECHO_EXIT  = 5;

const int IR1 = 6;
const int IR2 = 7;
const int IR3 = 8;

const int SERVO_GATE_ENTRY = 9;
const int SERVO_GATE_EXIT  = 10;

const int SERVO_CLEAN_1 = 11;
const int SERVO_CLEAN_2 = 12;
const int SERVO_CLEAN_3 = 13;

// LEDs on analog pins as digital outputs
const int LED1 = A0;
const int LED2 = A1;
const int LED3 = A2;

const int RELAY_EV = A3;

// ---------------- Servos ----------------
Servo gateEntry, gateExit;
Servo clean1, clean2, clean3;

// ---------------- System config ----------------
const int GATE_OPEN_ANGLE = 90;
const int GATE_CLOSED_ANGLE = 0;
const int GATE_TRIGGER_CM = 3;
const bool IR_ACTIVE_LOW = true;
const unsigned long CLEAN_AFTER_EMPTY_MS = 3000;
const unsigned long STATUS_SEND_PERIOD_MS = 3000;

// ---------------- State ----------------
bool slotOcc[3] = {false, false, false};
bool prevSlotOcc[3] = {false, false, false};

unsigned long slotEmptySince[3] = {0, 0, 0};
bool cleaningDoneForThisEmpty[3] = {false, false, false};

unsigned long lastStatusSendMs = 0;

// ---------------- Helpers ----------------
long readDistanceCm(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000UL);
  if (duration == 0) return 999;
  return duration / 58;
}

bool readIR(int pin) {
  int v = digitalRead(pin);
  return IR_ACTIVE_LOW ? (v == LOW) : (v == HIGH);
}

void setGate(Servo &s, int angle) {
  s.write(angle);
}

void cleanSlot(Servo &s) {
  s.write(0);
  delay(250);
  s.write(180);
  delay(600);
  s.write(0);
  delay(250);
}

void updateLeds() {
  digitalWrite(LED1, slotOcc[0] ? LOW : HIGH);
  digitalWrite(LED2, slotOcc[1] ? LOW : HIGH);
  digitalWrite(LED3, slotOcc[2] ? LOW : HIGH);
}

void updateRelay() {
  digitalWrite(RELAY_EV, slotOcc[0] ? HIGH : LOW);
}

int availableCount() {
  int occ = slotOcc[0] + slotOcc[1] + slotOcc[2];
  return 3 - occ;
}

void updateLCD() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Avail:");
  lcd.print(availableCount());
  lcd.print("  Occ:");
  lcd.print(3 - availableCount());

  lcd.setCursor(0,1);
  lcd.print("S1:");
  lcd.print(slotOcc[0] ? "X" : "O");
  lcd.print(" S2:");
  lcd.print(slotOcc[1] ? "X" : "O");
  lcd.print(" S3:");
  lcd.print(slotOcc[2] ? "X" : "O");
}

// ---------------- SERIAL → ESP8266 ----------------
void sendStatusToESP(bool force) {
  unsigned long now = millis();
  if (!force && (now - lastStatusSendMs) < STATUS_SEND_PERIOD_MS) return;
  lastStatusSendMs = now;

  int avail = availableCount();
  int occ = 3 - avail;
  int ev = slotOcc[0] ? 1 : 0;

  Serial.print("PARK,");
  Serial.print(avail); Serial.print(",");
  Serial.print(occ);   Serial.print(",");
  Serial.print(slotOcc[0] ? 1 : 0); Serial.print(",");
  Serial.print(slotOcc[1] ? 1 : 0); Serial.print(",");
  Serial.print(slotOcc[2] ? 1 : 0); Serial.print(",");
  Serial.println(ev);
}

// ---------------- SETUP ----------------
void setup() {
  pinMode(TRIG_ENTRY, OUTPUT);
  pinMode(ECHO_ENTRY, INPUT);
  pinMode(TRIG_EXIT, OUTPUT);
  pinMode(ECHO_EXIT, INPUT);

  pinMode(IR1, INPUT);
  pinMode(IR2, INPUT);
  pinMode(IR3, INPUT);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  pinMode(RELAY_EV, OUTPUT);
  digitalWrite(RELAY_EV, LOW);

  // ✅ Hardware Serial (D0/D1)
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();

  gateEntry.attach(SERVO_GATE_ENTRY);
  gateExit.attach(SERVO_GATE_EXIT);
  clean1.attach(SERVO_CLEAN_1);
  clean2.attach(SERVO_CLEAN_2);
  clean3.attach(SERVO_CLEAN_3);

  setGate(gateEntry, GATE_CLOSED_ANGLE);
  setGate(gateExit, GATE_CLOSED_ANGLE);

  slotOcc[0] = readIR(IR1);
  slotOcc[1] = readIR(IR2);
  slotOcc[2] = readIR(IR3);

  updateLeds();
  updateRelay();
  updateLCD();
  sendStatusToESP(true);
}

// ---------------- LOOP ----------------
void loop() {
  long dEntry = readDistanceCm(TRIG_ENTRY, ECHO_ENTRY);
  setGate(gateEntry, dEntry <= GATE_TRIGGER_CM ? GATE_OPEN_ANGLE : GATE_CLOSED_ANGLE);

  long dExit = readDistanceCm(TRIG_EXIT, ECHO_EXIT);
  setGate(gateExit, dExit <= GATE_TRIGGER_CM ? GATE_OPEN_ANGLE : GATE_CLOSED_ANGLE);

  bool newOcc[3];
  newOcc[0] = readIR(IR1);
  newOcc[1] = readIR(IR2);
  newOcc[2] = readIR(IR3);

  bool changed = false;
  unsigned long now = millis();

  for (int i = 0; i < 3; i++) {
    prevSlotOcc[i] = slotOcc[i];
    slotOcc[i] = newOcc[i];

    if (slotOcc[i] != prevSlotOcc[i]) {
      changed = true;
      if (!slotOcc[i]) {
        slotEmptySince[i] = now;
        cleaningDoneForThisEmpty[i] = false;
      } else {
        slotEmptySince[i] = 0;
        cleaningDoneForThisEmpty[i] = true;
      }
    }

    if (!slotOcc[i] && slotEmptySince[i] != 0 && !cleaningDoneForThisEmpty[i]) {
      if (now - slotEmptySince[i] >= CLEAN_AFTER_EMPTY_MS) {
        if (i == 0) cleanSlot(clean1);
        if (i == 1) cleanSlot(clean2);
        if (i == 2) cleanSlot(clean3);
        cleaningDoneForThisEmpty[i] = true;
        changed = true;
      }
    }
  }

  if (changed) {
    updateLeds();
    updateRelay();
    updateLCD();
    sendStatusToESP(true);
  } else {
    sendStatusToESP(false);
  }

  delay(50);
}





