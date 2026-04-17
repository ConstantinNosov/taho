#include <TM1637Display.h>
#include <FreqCount.h>

#define CLK_PIN 2
#define DIO_PIN 3

TM1637Display display(CLK_PIN, DIO_PIN);

// =========================
// НАСТРОЙКИ
// =========================
const float SMOOTHING_FACTOR = 0.3;   // физическая плавность
const float VISUAL_SMOOTHING = 0.25;  // визуальная плавность

// =========================
// ПЕРЕМЕННЫЕ
// =========================
float targetValue = 0;        // измеренное значение (физика)
float currentValue = 0;      // сглаженная физика
float visualValue = 0;       // сглаженное отображение

void setup() {
  display.setBrightness(0x0f);
  display.clear();

  FreqCount.begin(1000); // окно 1 сек
}

void loop() {

  // =========================
  // 1. ИЗМЕРЕНИЕ ЧАСТОТЫ
  // =========================
  if (FreqCount.available()) {
    unsigned long counts = FreqCount.read();

    float frequencyHz = counts;

    // твоя формула (важно: НЕ округляем!)
    targetValue = frequencyHz * 30.0;
  }

  // =========================
  // 2. ФИЗИЧЕСКАЯ ПЛАВНОСТЬ
  // =========================
  if (abs(currentValue - targetValue) > 0.1) {
    currentValue += (targetValue - currentValue) * SMOOTHING_FACTOR;
  } else {
    currentValue = targetValue;
  }

  // =========================
  // 3. ВИЗУАЛЬНАЯ ПЛАВНОСТЬ
  // (убирает ступеньки)
  // =========================
  visualValue += (currentValue - visualValue) * VISUAL_SMOOTHING;

  // =========================
  // 4. ВЫВОД
  // =========================
  int out = (int)(visualValue + 0.5); // мягкое округление только тут

  display.showNumberDec(constrain(out, 0, 9999));
}
