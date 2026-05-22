#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// =======================
// LCD
// =======================

LiquidCrystal_I2C lcd(0x27, 16, 2);

// =======================
// DHT11
// =======================

#define DHTPIN 7
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// =======================
// PINS
// =======================

#define gasSensor A0
#define tiltSensor 2
#define buzzer 8

#define led1 9
#define led2 10
#define led3 11

// =======================
// THRESHOLDS
// =======================

int gasThreshold = 250;
float tempThreshold = 38.0;

// =======================
// VARIABLES
// =======================

float temperature = 0;

unsigned long lastTempRead = 0;

// Alternate display
bool showTemp = true;

unsigned long lastLCDSwitch = 0;

// =======================
// WARNING STATES
// =======================

bool previousHighTemp = false;
bool previousGasLeak = false;

bool showTempCheck = false;
bool showGasCheck = false;

unsigned long tempCheckStart = 0;
unsigned long gasCheckStart = 0;

// =======================
// SETUP
// =======================

void setup() {

  pinMode(tiltSensor, INPUT_PULLUP);

  pinMode(buzzer, OUTPUT);

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);

  Serial.begin(9600);

  // LCD
  lcd.init();
  lcd.backlight();

  // DHT11
  dht.begin();

  // Startup
  lcd.setCursor(0,0);
  lcd.print("SMART AI SYS");

  lcd.setCursor(0,1);
  lcd.print("Initializing");

  delay(2000);

  lcd.clear();
}

// =======================
// LOOP
// =======================

void loop() {

  // =======================
  // SENSOR READINGS
  // =======================

  int gasValue = analogRead(gasSensor);

  int tiltState = digitalRead(tiltSensor);

  bool unsafeTilt = (tiltState == HIGH);

  bool gasDetected = (gasValue > gasThreshold);

  // =======================
  // READ DHT11 EVERY 2 SEC
  // =======================

  if (millis() - lastTempRead > 2000) {

    float temp = dht.readTemperature();

    if (!isnan(temp)) {
      temperature = temp;
    }

    lastTempRead = millis();
  }

  bool highTemp = (temperature >= tempThreshold);

  // =======================
  // DETECT TEMP RETURNING NORMAL
  // =======================

  if (previousHighTemp && !highTemp) {

    showTempCheck = true;
    tempCheckStart = millis();

    lcd.clear();
  }

  previousHighTemp = highTemp;

  // =======================
  // DETECT GAS RETURNING NORMAL
  // =======================

  if (previousGasLeak && !gasDetected) {

    showGasCheck = true;
    gasCheckStart = millis();

    lcd.clear();
  }

  previousGasLeak = gasDetected;

  // =======================
  // LCD SWITCH EVERY 3 SEC
  // =======================

  if (millis() - lastLCDSwitch > 3000) {

    showTemp = !showTemp;

    lastLCDSwitch = millis();

    lcd.clear();
  }

  // =======================
  // SERIAL MONITOR
  // =======================

  Serial.print("Gas: ");
  Serial.print(gasValue);

  Serial.print(" Temp: ");
  Serial.print(temperature);

  Serial.print(" Tilt: ");
  Serial.println(unsafeTilt);

  // ==================================
  // BOTH GAS + TEMP HIGH
  // ==================================

  if (gasDetected && highTemp) {

    digitalWrite(buzzer, HIGH);

    fastBlink();

    lcd.setCursor(0,0);
    lcd.print("CRITICAL!!!  ");

    lcd.setCursor(0,1);
    lcd.print("T:");
    lcd.print(temperature);

    lcd.print(" G:");
    lcd.print(gasValue);
    lcd.print(" ");
  }

  // ==================================
  // FIRE RISK CRITICAL
  // ==================================

  else if (highTemp) {

    digitalWrite(buzzer, HIGH);

    fastBlink();

    lcd.setCursor(0,0);
    lcd.print("FIRE RISK!!! ");

    // LIVE TEMP MONITOR
    lcd.setCursor(0,1);
    lcd.print("Temp:");
    lcd.print(temperature);
    lcd.print("C   ");
  }

  // ==================================
  // TEMP NORMALIZED
  // ==================================

  else if (showTempCheck) {

    digitalWrite(buzzer, LOW);

    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);

    lcd.setCursor(0,0);
    lcd.print("DOUBLE CHECK ");

    lcd.setCursor(0,1);
    lcd.print("EVERYTHING   ");

    if (millis() - tempCheckStart >= 5000) {

      showTempCheck = false;

      lcd.clear();
    }
  }

  // ==================================
  // GAS LEAK DETECTED
  // ==================================

  else if (gasDetected) {

    digitalWrite(buzzer, HIGH);

    slowBlink();

    lcd.setCursor(0,0);
    lcd.print("GAS LEAKING! ");

    // LIVE GAS MONITOR
    lcd.setCursor(0,1);
    lcd.print("Gas:");
    lcd.print(gasValue);
    lcd.print("    ");
  }

  // ==================================
  // GAS NORMALIZED
  // ==================================

  else if (showGasCheck) {

    digitalWrite(buzzer, LOW);

    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);

    lcd.setCursor(0,0);
    lcd.print("CHECK GAS    ");

    lcd.setCursor(0,1);
    lcd.print("TANK         ");

    if (millis() - gasCheckStart >= 5000) {

      showGasCheck = false;

      lcd.clear();
    }
  }

  // ==================================
  // TILT ONLY
  // ==================================

  else if (unsafeTilt) {

    digitalWrite(buzzer, HIGH);

    slowBlink();

    lcd.setCursor(0,0);
    lcd.print("TANK TILTED  ");

    lcd.setCursor(0,1);
    lcd.print("Unsafe Pos.  ");
  }

  // ==================================
  // SAFE MODE
  // ==================================

  else {

    digitalWrite(buzzer, LOW);

    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);

    // ==========================
    // TEMPERATURE DISPLAY
    // ==========================

    if (showTemp) {

      lcd.setCursor(0,0);
      lcd.print("TEMP MONITOR ");

      lcd.setCursor(0,1);
      lcd.print("Temp:");
      lcd.print(temperature);
      lcd.print("C    ");
    }

    // ==========================
    // GAS DISPLAY
    // ==========================

    else {

      lcd.setCursor(0,0);
      lcd.print("GAS MONITOR  ");

      lcd.setCursor(0,1);
      lcd.print("Gas:");
      lcd.print(gasValue);
      lcd.print("    ");
    }
  }

  delay(50);
}

// =======================
// SLOW BLINK
// =======================

void slowBlink() {

  digitalWrite(led1, HIGH);
  digitalWrite(led2, HIGH);
  digitalWrite(led3, HIGH);

  delay(100);

  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);

  delay(100);
}

// =======================
// FAST BLINK
// =======================

void fastBlink() {

  digitalWrite(led1, HIGH);
  digitalWrite(led2, HIGH);
  digitalWrite(led3, HIGH);

  delay(30);

  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);

  delay(30);
}