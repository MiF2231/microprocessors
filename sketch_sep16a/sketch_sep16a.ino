//Пины
const int RED = 13;
const int YELLOW = 12;
const int GREEN = 11;
const int BUTTON = 2;

//Состояния
enum State { S_GREEN, S_YELLOW, S_RED, S_WARNING, S_NIGHT, NUM_STATES };
State currentState = S_GREEN;

//События
enum Event { E_NONE, E_TIMER, E_PED, E_EMERGENCY_ON, E_EMERGENCY_OFF, E_NIGHT_TOGGLE, NUM_EVENTS };

//Таблица обработчиков
typedef void (*Handler)(void);
Handler fsmTable[NUM_STATES][NUM_EVENTS];

//Таймер
unsigned long stateStart = 0;
unsigned long duration = 0;

//Флаги
bool pedRequest = false;
bool emergency = false;
bool nightMode = false;

//Антидребезг
unsigned long lastButtonChange = 0;
bool lastButtonState = HIGH;

//Для длинного/двойного нажатия
unsigned long pressStart = 0;
unsigned long lastPressTime = 0;
bool waitingSecondPress = false;

//Управление выводами
void setOutputsForState(State s) {
  digitalWrite(RED, LOW);
  digitalWrite(YELLOW, LOW);
  digitalWrite(GREEN, LOW);

  switch (s) {
    case S_GREEN:   digitalWrite(GREEN, HIGH); Serial.println("State: GREEN"); break;
    case S_YELLOW:  digitalWrite(YELLOW, HIGH); Serial.println("State: YELLOW"); break;
    case S_RED:     digitalWrite(RED, HIGH); Serial.println("State: RED"); break;
    case S_WARNING: Serial.println("State: WARNING (blinking yellow)"); break;
    case S_NIGHT:   Serial.println("State: NIGHT (blinking yellow 1s)"); break;
  }
}

void goToState(State s, unsigned long dur) {
  currentState = s;
  stateStart = millis();
  duration = dur;
  setOutputsForState(s);
}

//Обработчики
void onGreenTimer() { goToState(S_YELLOW, 3000); }

void onYellowTimer() {
  if (pedRequest) { pedRequest = false; goToState(S_RED, 15000); }
  else goToState(S_RED, 10000);
}

void onRedTimer() { goToState(S_GREEN, 10000); }

void onWarningTimer() {
  digitalWrite(YELLOW, !digitalRead(YELLOW));
  stateStart = millis();
}

void onNightTimer() {
  digitalWrite(YELLOW, !digitalRead(YELLOW));
  stateStart = millis();
}

void onPedRequest() {
  Serial.println("Pedestrian request registered");
  pedRequest = true;
}

void onEmergencyOn() { goToState(S_WARNING, 500); }

void onEmergencyOff() { goToState(S_RED, 10000); }

void onNightToggle() {
  nightMode = !nightMode;
  if (nightMode) goToState(S_NIGHT, 1000);
  else goToState(S_GREEN, 10000);
}

//Таблица
void setupTable() {
  for (int s=0; s<NUM_STATES; ++s)
    for (int e=0; e<NUM_EVENTS; ++e)
      fsmTable[s][e] = NULL;

  //Таймеры
  fsmTable[S_GREEN][E_TIMER] = onGreenTimer;
  fsmTable[S_YELLOW][E_TIMER] = onYellowTimer;
  fsmTable[S_RED][E_TIMER] = onRedTimer;
  fsmTable[S_WARNING][E_TIMER] = onWarningTimer;
  fsmTable[S_NIGHT][E_TIMER] = onNightTimer;

  //Пешеход
  fsmTable[S_GREEN][E_PED] = onPedRequest;

  //Авария
  for (int s=0; s<NUM_STATES; ++s) fsmTable[s][E_EMERGENCY_ON] = onEmergencyOn;
  fsmTable[S_WARNING][E_EMERGENCY_OFF] = onEmergencyOff;

  //Ночной режим
  for (int s=0; s<NUM_STATES; ++s) fsmTable[s][E_NIGHT_TOGGLE] = onNightToggle;
}

//Обработка кнопки
void readInputs() {
  bool btn = digitalRead(BUTTON);

  if (btn != lastButtonState && millis() - lastButtonChange > 50) {
    lastButtonChange = millis();
    lastButtonState = btn;

    if (btn == LOW) { pressStart = millis(); }

    if (btn == HIGH && pressStart > 0) {
      unsigned long pressTime = millis() - pressStart;

      if (pressTime >= 2000) {
        //Долгое нажатие → ночной режим
        processEvent(E_NIGHT_TOGGLE);
      } else {
        if (waitingSecondPress && millis() - lastPressTime < 500) {
          //Двойное нажатие → авария
          if (!emergency) { emergency = true; processEvent(E_EMERGENCY_ON); }
          else { emergency = false; processEvent(E_EMERGENCY_OFF); }
          waitingSecondPress = false;
        } else {
          //Ждём второе нажатие
          waitingSecondPress = true;
          lastPressTime = millis();
        }
      }
      pressStart = 0;
    }
  }
  if (waitingSecondPress && millis() - lastPressTime > 500) {
    processEvent(E_PED);
    waitingSecondPress = false;
  }
}
//Генерация событий
void processEvent(Event ev) {
  if (ev == E_NONE) return;
  Handler h = fsmTable[currentState][ev];
  if (h) h();
}

void setup() {
  pinMode(RED, OUTPUT);
  pinMode(YELLOW, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  Serial.begin(9600);

  setupTable();
  goToState(S_GREEN, 10000);
}

void loop() {
  readInputs();

  if (millis() - stateStart >= duration) {
    processEvent(E_TIMER);
  }
}