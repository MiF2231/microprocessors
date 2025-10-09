#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// === LCD ===
LiquidCrystal_I2C lcd(0x27, 16, 2);

// === Клавиатура ===
const byte ROWS = 4;
const byte COLS = 4;
const byte rowPins[ROWS] = {8, 9, 10, 11};
const byte colPins[COLS] = {4, 5, 6, 7};

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// === Переменные для прерываний ===
volatile unsigned long timerCounter = 0;
volatile byte currentRow = 0;

// Новая переменная: последняя нажатая клавиша (0 = ничего)
volatile char detectedKey = 0;

// === Timer1: 2 Гц (500 мс) для счётчика ===
ISR(TIMER1_COMPA_vect) {
  timerCounter++;
}

// === Timer2: ~200 Гц (каждые 5 мс) — сканирование клавиатуры ===
ISR(TIMER2_COMPA_vect) {
  // Деактивировать текущую строку
  digitalWrite(rowPins[currentRow], HIGH);

  // Перейти к следующей строке
  currentRow = (currentRow + 1) % ROWS;

  // Активировать новую строку
  digitalWrite(rowPins[currentRow], LOW);

  // Проверить все столбцы на нажатие в этой строке
  for (byte c = 0; c < COLS; c++) {
    if (digitalRead(colPins[c]) == LOW) {
      // Нажатие обнаружено — запоминаем клавишу
      detectedKey = keys[currentRow][c];
      break; // можно убрать, если нужно обрабатывать несколько клавиш
    }
  }
}

// === Настройка Timer1: 2 Гц (500 мс) ===
void setupTimer1() {
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 31249;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS12);
  TIMSK1 |= (1 << OCIE1A);
  interrupts();
}

// === Настройка Timer2: ~200 Гц (5 мс) ===
void setupTimer2() {
  noInterrupts();
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2 = 0;
  OCR2A = 77; // ~5 мс при prescaler=1024
  TCCR2A |= (1 << WGM21);
  TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20); // prescaler = 1024
  TIMSK2 |= (1 << OCIE2A);
  interrupts();
}

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.print("Ready...");
  delay(1000);
  lcd.clear();
  lcd.print("Cnt: 0");

  // Настройка строк
  for (byte i = 0; i < ROWS; i++) {
    pinMode(rowPins[i], OUTPUT);
    digitalWrite(rowPins[i], HIGH);
  }

  // Настройка столбцов
  for (byte j = 0; j < COLS; j++) {
    pinMode(colPins[j], INPUT_PULLUP);
  }

  // Таймеры
  setupTimer1();
  setupTimer2();
}

void loop() {
  // Обновление счётчика
  static unsigned long lastDisplay = 0;
  if (millis() - lastDisplay >= 200) {
    lastDisplay = millis();
    lcd.setCursor(0, 0);
    lcd.print("Cnt: ");
    lcd.print(timerCounter);
  }

  // Обработка нажатой клавиши
  if (detectedKey != 0) {
    char key = detectedKey;
    detectedKey = 0; // сброс

    Serial.print("Клавиша: ");
    Serial.println(key);

    lcd.setCursor(0, 1);
    lcd.print("Key: ");
    lcd.print(key);
    lcd.print("   ");
  }
}