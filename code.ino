#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Sensors
const int sensor1 = 2;
const int sensor2 = 3;

// Potentiometer
const int potPin = A0;

const float distance = 0.08;  // meters
const float g = 9.81;

unsigned long startTime = 0;
unsigned long endTime = 0;

bool readyState = false;
bool started = false;

float lastAngle = -1;
float stableAngle = -1;

unsigned long lastChangeTime = 0;

// ---------------- COUNTDOWN ----------------
void countdown() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready");

  for (int i = 5; i >= 0; i--) {
    lcd.setCursor(0, 1);
    lcd.print("T: ");
    lcd.print(i);
    lcd.print("   ");
    delay(1000);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("READY!");
  delay(1000);
}

// ---------------- SETUP ----------------
void setup() {
  pinMode(sensor1, INPUT);
  pinMode(sensor2, INPUT);

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Set Angle...");
}

// ---------------- LOOP ----------------
void loop() {

  int potValue = analogRead(potPin);
  float angle = map(potValue, 0, 1023, 0, 90);

  // Detect angle change
  if (abs(angle - lastAngle) > 2) {
    lastChangeTime = millis();
    lastAngle = angle;
  }

  // Check if angle stable
  if (!readyState && (millis() - lastChangeTime > 1000)) {

    if (stableAngle != lastAngle) {
      stableAngle = lastAngle;

      countdown();
      readyState = true;
    }
  }

  // START TIMING
  if (digitalRead(sensor1) == LOW && readyState && !started) {
    startTime = micros();
    started = true;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Measuring...");

    delay(50); // safer than while()
  }

  // STOP TIMING
  if (digitalRead(sensor2) == LOW && started) {
    endTime = micros();

    delay(50);

    float timeTaken = (endTime - startTime) / 1000000.0;

    if (timeTaken > 0) {

      float velocity = distance / timeTaken;

      float theta = stableAngle * PI / 180.0;

      float range = (velocity * velocity * sin(2 * theta)) / g;
      float height = (velocity * velocity * sin(theta) * sin(theta)) / (2 * g);
      float timeFlight = (2 * velocity * sin(theta)) / g;

      // -------- DISPLAY VELOCITY --------
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Velocity:");

      lcd.setCursor(0, 1);
      lcd.print(velocity, 2);
      lcd.print(" m/s");

      delay(3000);

      // -------- DISPLAY ANGLE --------
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Angle:");

      lcd.setCursor(0, 1);
      lcd.print(stableAngle, 0);
      lcd.print((char)223);

      delay(2000);

      // -------- DISPLAY RANGE & HEIGHT --------
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("R:");
      lcd.print(range, 2);

      lcd.setCursor(0, 1);
      lcd.print("H:");
      lcd.print(height, 2);

      delay(3000);

      // -------- DISPLAY TIME OF FLIGHT --------
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Time:");

      lcd.setCursor(0, 1);
      lcd.print(timeFlight, 2);
      lcd.print(" s");

      delay(3000);
    }

    // RESET SYSTEM
    readyState = false;
    started = false;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Set Angle...");
  }
}