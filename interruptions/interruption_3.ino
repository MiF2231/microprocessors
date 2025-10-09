// Задание 3: Сканирование клавиатуры 4x4 в ISR Timer0 (1 мс)

// Подключение клавиатуры
const byte ROWS = 4;
const byte COLS = 4;

const byte rowPins[ROWS] = {8, 9, 10, 11}; // Строки
const byte colPins[COLS] = {4, 5, 6, 7};   // Столбцы

// Макет клавиатуры
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

volatile byte currentRow = 0; // текущая строка для сканирования (volatile!)

void setup() {
  Serial.begin(9600);

  // Настройка строк как выходов
  for (byte i = 0; i < ROWS; i++) {
    pinMode(rowPins[i], OUTPUT);
    digitalWrite(rowPins[i], HIGH); // неактивное состояние — HIGH
  }

  // Настройка столбцов как входов с подтяжкой
  for (byte j = 0; j < COLS; j++) {
    pinMode(colPins[j], INPUT_PULLUP);
  }

  // === Настройка Timer0 в режиме CTC для 1 мс ===
  noInterrupts();

  TCCR0A = 0;
  TCCR0B = 0;
  TCNT0 = 0;

  // Режим CTC: WGM01 = 1
  TCCR0A |= (1 << WGM01);

  // Предделитель 64: CS01=1, CS00=1 → CS02=0
  TCCR0B |= (1 << CS01) | (1 << CS00);

  // OCR0A = 249 → 1 мс
  OCR0A = 249;

  // Разрешить прерывание по совпадению A
  TIMSK0 |= (1 << OCIE0A);

  interrupts();
}

// Обработчик прерывания Timer0 (каждые 1 мс)
ISR(TIMER0_COMPA_vect) {
  // Деактивировать предыдущую строку
  digitalWrite(rowPins[currentRow], HIGH);

  // Перейти к следующей строке
  currentRow = (currentRow + 1) % ROWS;

  // Активировать новую строку (LOW)
  digitalWrite(rowPins[currentRow], LOW);

  // Проверить все столбцы на нажатие
  for (byte col = 0; col < COLS; col++) {
    if (digitalRead(colPins[col]) == LOW) {
      // Нажатие обнаружено — выводим в Serial
      // ⚠️ Serial.print() в ISR — не рекомендуется, но допустимо для отладки на низкой частоте
      Serial.print("Нажата: ");
      Serial.println(keys[currentRow][col]);
      // Можно добавить debounce через флаг, но по условию — просто вывод
    }
  }
}

void loop() {
  // Основной цикл пуст — всё в прерывании
}
