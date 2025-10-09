const byte LED_PIN = 13;
const byte BUTTON_PIN = 2;  // D2 = внешнее прерывание 0

volatile bool fastMode = false;  // флаг режима: false = 1 Гц, true = 5 Гц
volatile unsigned long lastToggle = 0;

// Обработчик внешнего прерывания
void handleButton() {
  // Простейшая защита от дребезга: игнорировать, если прошло < 200 мс
  static unsigned long lastInterrupt = 0;
  unsigned long now = millis();
  if (now - lastInterrupt > 200) {
    fastMode = !fastMode;  // переключаем режим
    lastInterrupt = now;
  }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // используем внутренний подтяж

  // Подключаем прерывание на FALLING фронт (нажатие кнопки)
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButton, FALLING);
}

void loop() {
  unsigned long currentMillis = millis();
  unsigned long interval = fastMode ? 100 : 500;  // 5 Гц → 100 мс, 1 Гц → 500 мс

  if (currentMillis - lastToggle >= interval) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));  // инвертируем состояние
    lastToggle = currentMillis;
  }
}
