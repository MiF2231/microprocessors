#include <SWTFT.h>
#include <math.h>

SWTFT tft;

// Базовые параметры прямоугольника
const int16_t baseWidth  = 60;
const int16_t baseHeight = 30;
const int16_t centerX = 120;
const int16_t centerY = 160;

const float minScale = 0.5f;
const float maxScale = 2.0f;
float angle = 0.0;          // угол вращения
float scale = 1.0;          // текущий масштаб (1.0 = исходный размер)
float scaleDir = 0.02;

const int16_t maxW = (int16_t)(baseWidth * maxScale);
const int16_t maxH = (int16_t)(baseHeight * maxScale);
const int16_t bboxSize = (int16_t)ceil(sqrt(maxW * maxW + maxH * maxH));
const int16_t bboxHalf = bboxSize / 2;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void drawRotatedRect(int16_t cx, int16_t cy, int16_t w, int16_t h, float angle, uint16_t color) {
  float hw = w / 2.0f;
  float hh = h / 2.0f;

  float cosA = cos(angle);
  float sinA = sin(angle);

  int16_t x0 = cx + (int16_t)( hw * cosA - hh * sinA);
  int16_t y0 = cy + (int16_t)( hw * sinA + hh * cosA);

  int16_t x1 = cx + (int16_t)(-hw * cosA - hh * sinA);
  int16_t y1 = cy + (int16_t)(-hw * sinA + hh * cosA);

  int16_t x2 = cx + (int16_t)(-hw * cosA + hh * sinA);
  int16_t y2 = cy + (int16_t)(-hw * sinA - hh * cosA);

  int16_t x3 = cx + (int16_t)( hw * cosA + hh * sinA);
  int16_t y3 = cy + (int16_t)( hw * sinA - hh * cosA);

  tft.drawLine(x0, y0, x1, y1, color);
  tft.drawLine(x1, y1, x2, y2, color);
  tft.drawLine(x2, y2, x3, y3, color);
  tft.drawLine(x3, y3, x0, y0, color);
}

void setup() {
  tft.begin(0x9325);
  tft.fillScreen(tft.color565(0, 0, 0));
}

void loop() {
  // Зарисовывем прямоугльник черным
  int16_t prevW = (int16_t)(baseWidth * scale);
  int16_t prevH = (int16_t)(baseHeight * scale);
  drawRotatedRect(centerX, centerY, prevW, prevH, angle, tft.color565(0, 0, 0));

  angle += 0.05f;
  if (angle >= 2 * M_PI) angle -= 2 * M_PI;

  scale += scaleDir;
  if (scale >= maxScale) {
    scale = maxScale;
    scaleDir = -0.02f;
  } else if (scale <= minScale) {
    scale = minScale;
    scaleDir = 0.02f;
  }

  // Рисуем прямоугльник белым
  int16_t w = (int16_t)(baseWidth * scale);
  int16_t h = (int16_t)(baseHeight * scale);
  drawRotatedRect(centerX, centerY, w, h, angle, tft.color565(255, 255, 255));

  delay(20);
}