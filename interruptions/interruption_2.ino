const byte LED_PIN = 13;
volatile bool ledState = false;  //volatile — обязательно для переменных в ISR

void setup() {
  pinMode(LED_PIN, OUTPUT);

  noInterrupts();
  //Отключаем таймеры
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  // TOP
  OCR1A = 2499;
  //СТС
  TCCR1B |= (1 << WGM12);
  //Таймер по предделютелю 64 
  TCCR1B |= (1 << CS12) | (1 << CS10); // prescaler = 1024 (CS12=1, CS11=0, CS10=1)
  //Прерывание по совпадению А
  TIMSK1 |= (1 << OCIE1A);           // разрешить прерывание по совпадению A
  //Глоб прерывания
  interrupts();
}
//Обработчик
ISR(TIMER1_COMPA_vect) {
  ledState = !ledState;
  digitalWrite(LED_PIN, ledState);
}

void loop() {
  //ничего не делает
}
