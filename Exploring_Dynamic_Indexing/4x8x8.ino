#include <LedControl.h>

// Пины подключения MAX7219
const int DIN_PIN = 11;
const int CLK_PIN = 13;
const int CS_PIN  = 10;

// Количество каскадных модулей
const uint8_t DEVICES = 4;            // 4 модуля по 8×8
const uint8_t WIDTH = DEVICES * 8;    // 32 столбца в ширину

LedControl lc = LedControl(DIN_PIN, CLK_PIN, CS_PIN, DEVICES);

// Буфер: каждый элемент = 1 столбец (8 бит = 8 строк)
uint8_t buffer_[WIDTH];

// Настройки скорости прокрутки
unsigned long lastScroll = 0;
unsigned long scrollInterval = 80; // мс между шагами

// Простой 5×7 шрифт (можно расширить)
const uint8_t font5x7[][5] = {
  {0x00,0x00,0x00,0x00,0x00}, // пробел
  {0x7E,0x11,0x11,0x11,0x7E}, // A
  {0x7F,0x49,0x49,0x49,0x36}, // B
  {0x3E,0x41,0x41,0x41,0x22}, // C
  {0x7F,0x41,0x41,0x22,0x1C}, // D
  {0x7F,0x49,0x49,0x49,0x41}, // E
  {0x7F,0x09,0x09,0x09,0x01}, // F
  {0x3E,0x41,0x49,0x49,0x7A}, // G
  {0x7F,0x08,0x08,0x08,0x7F}, // H
  {0x00,0x41,0x7F,0x41,0x00}, // I
  {0x20,0x40,0x41,0x3F,0x01}, // J
  {0x7F,0x08,0x14,0x22,0x41}, // K
  {0x7F,0x01,0x01,0x01,0x01}, // L (исправлено)
  {0x7F,0x04,0x08,0x04,0x7F}, // M
  {0x7F,0x04,0x08,0x10,0x7F}, // N
  {0x3E,0x41,0x41,0x41,0x3E}, // O
  {0x7F,0x09,0x09,0x09,0x06}, // P
  {0x3E,0x41,0x51,0x21,0x5E}, // Q
  {0x7F, 0x09, 0x11, 0x21, 0x46}, // R (исправлено)
  {0x46,0x49,0x49,0x49,0x31}, // S
  {0x01,0x01,0x7F,0x01,0x01}, // T
  {0x3F,0x40,0x40,0x40,0x3F}, // U
  {0x1F,0x20,0x40,0x20,0x1F}, // V
  {0x7F,0x08,0x10,0x08,0x7F}, // W (исправлено)
  {0x63,0x14,0x08,0x14,0x63}, // X
  {0x07,0x08,0x70,0x08,0x07}, // Y
  {0x61,0x51,0x49,0x45,0x43}, // Z
  {0x3E,0x45,0x49,0x51,0x3E}, // 0 (исправлено)
  {0x00,0x42,0x7F,0x40,0x00}, // 1
  {0x42,0x61,0x51,0x49,0x46}, // 2
  {0x21,0x41,0x45,0x4B,0x31}, // 3
  {0x18,0x14,0x12,0x7F,0x10}, // 4
  {0x27,0x45,0x45,0x45,0x39}, // 5
  {0x3C,0x4A,0x49,0x49,0x30}, // 6
  {0x01,0x71,0x09,0x05,0x03}, // 7
  {0x36,0x49,0x49,0x49,0x36}, // 8
  {0x06,0x49,0x49,0x29,0x1E}, // 9
  {0x00,0x00,0x00,0x00,0x00}, // (резерв)
};


const char *message = " HELLO WORLD "; // текст для прокрутки

int currentChar = 0;   // индекс текущего символа в строке
int columnInChar = 0;  // какая колонка шрифта сейчас вставляется

// --- Инициализация ---
void setup() {
  for (uint8_t i = 0; i < DEVICES; i++) {
    lc.shutdown(i, false);    // включаем модуль
    lc.setIntensity(i, 8);    // яркость 0..15
    lc.clearDisplay(i);       // очистка
  }

  // Очистим буфер
  for (int i = 0; i < WIDTH; i++) buffer_[i] = 0x00;
  lastScroll = millis();
}

// --- Главный цикл ---
void loop() {
  unsigned long now = millis();
  if (now - lastScroll >= scrollInterval) {
    lastScroll = now;
    scrollStep();     // сдвиг и добавление новой колонки
    updateDisplay();  // вывод на матрицы
  }
}

// --- Один шаг прокрутки ---
void scrollStep() {
  // 1. Сдвигаем буфер влево
  for (int i = 0; i < WIDTH - 1; i++) {
    buffer_[i] = buffer_[i + 1];
  }

  // 2. Получаем новую колонку
  uint8_t newCol = 0x00;
  char ch = message[currentChar];

  // Индекс символа в таблице (A=1, т.к. 0=пробел)
  int index;
  if (ch == ' ') index = 0;
  else if (ch >= 'A' && ch <= 'Z') index = ch - 'A' + 1;
  else if (ch >= '0' && ch <= '9') index = ch - '0' + 27;
  else index = 0; // если символ неизвестен — пробел

  // если колонка меньше 5 — берём столбец символа
  if (columnInChar < 5) {
    newCol = font5x7[index][columnInChar];
  } else {
    // после 5 колонок — добавляем пустую колонку (пробел между символами)
    newCol = 0x00;
  }

  // 3. Вставляем новую колонку в конец буфера
  buffer_[WIDTH - 1] = newCol;

  // 4. Переход к следующей колонке / символу
  columnInChar++;
  if (columnInChar > 5) {  // 5 столбцов + 1 пробел = 6
    columnInChar = 0;
    currentChar++;
    if (message[currentChar] == '\0') currentChar = 0; // зацикливаем
  }
}

// --- Вывод буфера на матрицы ---
void updateDisplay() {
  for (int gcol = 0; gcol < WIDTH; gcol++) {
    uint8_t data = buffer_[gcol];
    // Определяем, к какому модулю относится столбец
    int device = (WIDTH - 1 - gcol) / 8;  // если текст идёт в обратную сторону — убрать "WIDTH - 1 -"
    int colOnDevice = gcol % 8;
    lc.setColumn(device, colOnDevice, data);
  }
}