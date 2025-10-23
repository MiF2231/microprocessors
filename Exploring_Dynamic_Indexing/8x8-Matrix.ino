#include <LedControl.h>

LedControl Lc = LedControl(11, 13, 10, 1);

const byte MAX_STARS = 5;
const unsigned long newStarInterval = 1500; // новая звезда каждые 1.5 сек
const unsigned long stepDelay = 800;        // длительность каждой фазы (в мс)

unsigned long lastNewStarTime = 0;

// 4 направления: вверх, вниз, влево, вправо для рисования звезды
const int dx[4] = {0, 0, -1, 1};
const int dy[4] = {-1, 1, 0, 0};

struct Star {
  bool active;
  int x, y;
  int phase;                // 0 = только что создана, 1 = центр, 2 = центр+кольцо, 3 = центр, 4 = завершена
  unsigned long nextPhaseTime;
};

Star stars[MAX_STARS];

void setup() {
  randomSeed(analogRead(0));
  Lc.shutdown(0, false);
  Lc.setIntensity(0, 7);
  Lc.clearDisplay(0);
  for (byte i = 0; i < MAX_STARS; i++) stars[i].active = false;
}

void loop() {
  unsigned long now = millis();

  // Создание новой звезды
  if (now - lastNewStarTime >= newStarInterval) {
    for (byte i = 0; i < MAX_STARS; i++) {
      if (!stars[i].active) {
        stars[i].active = true;
        stars[i].x = random(8);
        stars[i].y = random(8);
        stars[i].phase = 0;
        stars[i].nextPhaseTime = now;
        lastNewStarTime = now;
        break;
      }
    }
  }

  // Буфер кадра
  bool frame[8][8] = {false};

  for (byte i = 0; i < MAX_STARS; i++) {
    if (!stars[i].active) continue;

    if (now >= stars[i].nextPhaseTime) {
      stars[i].phase++;
      stars[i].nextPhaseTime = now + stepDelay;

      if (stars[i].phase >= 4) {
        stars[i].active = false;
        continue;
      }
    }

    if (stars[i].phase >= 1) {
      // Центр горит в фазах 1, 2, 3
      frame[stars[i].x][stars[i].y] = true;
    }

    if (stars[i].phase == 2) {
      // Кольцо горит ТОЛЬКО в фазе 2
      for (int d = 0; d < 4; d++) {
        int nx = stars[i].x + dx[d];
        int ny = stars[i].y + dy[d];
        if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8) {
          frame[nx][ny] = true;
        }
      }
    }
  }

  // Вывод кадра на дисплей
  for (int r = 0; r < 8; r++) {
    for (int c = 0; c < 8; c++) {
      Lc.setLed(0, r, c, frame[r][c]);
    }
  }

  delay(10);
}