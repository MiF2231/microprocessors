void setup() {
  DDRB |= (1 << DDB5) | (1 << DDB4) | (1 << DDB3); // LED на PB3, PB4, PB5 как выход
  DDRD &= ~((1 << DDD3) | (1 << DDD4) | (1 << DDD5)); // кнопки на PD3, PD4, PD5 как вход
  Serial.begin(9600);
}

void loop() {
  uint8_t buttons = PIND; // читаем состояние всех пинов D
  uint8_t leds = (~buttons >> 3) << 3; // инвертируем состояние кнопок 
  PORTB = leds; // включаем соответствующие светодиоды
  Serial.print("PORTB: ");
  for (int i = 7; i >= 0; i--) {
    Serial.print((PORTB >> i) & 1);
  }
  Serial.println();
  // Проверяем каждую кнопку и выводим сообщение
  if (!(buttons & (1 << PIND3))) Serial.println("Кнопка 1 нажата");
  if (!(buttons & (1 << PIND4))) Serial.println("Кнопка 2 нажата");
  if (!(buttons & (1 << PIND5))) Serial.println("Кнопка 3 нажата");

  delay(200);
}
