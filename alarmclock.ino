#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ================= Clock ===================

unsigned char clockHours = 0;
unsigned char clockMinutes = 0;
unsigned char clockSeconds = 0;

unsigned long previousMillis = 0;
const long interval = 1000;

char clockString[16];

// ================= Alarm ===================

unsigned char alarmHours = 0;
unsigned char alarmMinutes = 0;
unsigned char alarmSeconds = 0;

unsigned long alarmStartMillis = 0;
unsigned long alarmLastMillis = 0;
const long alarmInterval = 200;
const long alarmDuration = 5000;

bool isAlarmActivated = false;
bool isAlarmRinging = false;
bool isBuzzerActivated = false;

// ============= I2C LCD display =============

const int LCD_I2C_ADDRESS = 0x27;
LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, 16, 2);

// ============== 4x4 Keypad =================

const byte rows = 1;
const byte columns = 4;

const byte ROW_PINS[rows] = { 8 };
const byte COLUMN_PINS[columns] = { 9, 10, 11, 12 };

const char BUTTONS[rows][columns] = {
  {'1', '2', '3', '4'}
};

Keypad keypad = Keypad(makeKeymap(BUTTONS), ROW_PINS, COLUMN_PINS, rows, columns);

// ================ Other ====================

const unsigned char buzzerPin = 7;

// ===========================================

unsigned char setTimeSerial() {
  Serial.println("Enter time value:");
  while (true) {
    if (Serial.available() > 0) {
      String input = Serial.readStringUntil('\n');
      input.trim();
      if (input.length() > 0 && input.toInt() >= 0) {
        unsigned char timeValue = (unsigned char)input.toInt();
        Serial.print("Received data: ");
        Serial.println(timeValue);
        return timeValue;
      } else {
        Serial.println("Invalid input!");
      }
    }
  }
}

void updateLCD() {
  sprintf(clockString, "%02d:%02d:%02d", clockHours, clockMinutes, clockSeconds);
  if (isAlarmActivated) strcat(clockString, "A");
  lcd.clear();
  lcd.print(clockString);
}

void incrementTime() {
  clockSeconds++;
  if (clockSeconds >= 60) {
    clockSeconds = 0;
    clockMinutes++;
  }
  if (clockMinutes >= 60) {
    clockMinutes = 0;
    clockHours++;
  }
  if (clockHours >= 24) {
    clockHours = 0;
  }
}

void handleKeyPress(char keyPressed) {
  // Keypad buttons:
  // 1 - set hours for clock or alarm
  // 2 - set minutes for clock or alarm
  // 3 - show alarm and change it by pressing 1 or 2
  // 4 - turn on/off alarm(you will see 'A' near seconds)
  switch (keyPressed) {
    case '1':
      clockHours = setTimeSerial();
      break;

    case '2':
      clockMinutes = setTimeSerial();
      break;

    case '3':
      lcd.clear();
      sprintf(clockString, "%02d:%02d:%02d", alarmHours, alarmMinutes, alarmSeconds);
      lcd.print(clockString);
      while ((keyPressed = keypad.getKey()) == 0) {};
      if (keyPressed == '1') {
        alarmHours = setTimeSerial();
      } else if (keyPressed == '2') {
        alarmMinutes = setTimeSerial();
      }
      break;

    case '4':
      isAlarmActivated = !isAlarmActivated;
      isAlarmRinging = false;
      noTone(buzzerPin);
      break;

    default:
      break;
  }
}

void alarm() {
  if (isAlarmRinging) {
    unsigned long currentMillis = millis();
    if (currentMillis - alarmLastMillis >= alarmInterval) {
      alarmLastMillis = currentMillis;
      if (isBuzzerActivated) {
        tone(buzzerPin, 2500);
        isBuzzerActivated = false;
      } else {
        noTone(buzzerPin);
        isBuzzerActivated = true;
      }
    }
    if (currentMillis - alarmStartMillis >= alarmDuration) {
      isAlarmRinging = false;
      noTone(buzzerPin);
    }
  } else if (clockHours == alarmHours && clockMinutes == alarmMinutes && clockSeconds == alarmSeconds) {
    isAlarmRinging = true;
    isBuzzerActivated = true;
    alarmStartMillis = millis();
    alarmLastMillis = millis();
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(buzzerPin, OUTPUT);
  lcd.begin();
  lcd.backlight();
  clockSeconds = 50;
  alarmMinutes = 1;
  isAlarmActivated = true;
}

void loop() {
  unsigned long currentMillis = millis();
  
  char keyPressed = keypad.getKey();
  if (keyPressed) {
    handleKeyPress(keyPressed);
    updateLCD();
  }

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    incrementTime();
    updateLCD();
  }

  if (isAlarmActivated) {
    alarm();
  }
}