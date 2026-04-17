#include <TM1637Display.h>
#include <FreqCount.h>

#define CLK_PIN 2
#define DIO_PIN 3

TM1637Display display(CLK_PIN, DIO_PIN);

// =========================
const float SMOOTHING_FACTOR = 0.3;

// =========================
float targetValue = 0;
float currentValue = 0;

// лёгкое скользящее среднее
float buffer[4] = {0};
uint8_t idx = 0;
bool filled = false;

// =========================
// детектор отсутствия сигнала
unsigned long lastSignalTime = 0;
const unsigned long SIGNAL_TIMEOUT_MS = 1000; // 1 сек

// =========================

float getAvg() {
  float sum = 0;
  int n = filled ? 4 : idx;

  if (n == 0) return 0;

  for (int i = 0; i < n; i++) {
    sum += buffer[i];
  }

  return sum / n;
}

void addValue(float v) {
  buffer[idx++] = v;

  if (idx >= 4) {
    idx = 0;
    filled = true;
  }
}

void resetFilter() {
  for (int i = 0; i < 4; i++) buffer[i] = 0;

  idx = 0;
  filled = false;
}

void setup() {
  display.setBrightness(0x0f);
  display.clear();

  FreqCount.begin(500);
}

void loop() {

  if (FreqCount.available()) {
    unsigned long counts = FreqCount.read();

    if (counts > 0) {

      lastSignalTime = millis();

      float frequencyHz = counts * 2.0;

      addValue(frequencyHz);

      float filtered = getAvg();

      targetValue = filtered * 30.0;
    }
  }

  // =========================
  // ОТСУТСТВИЕ СИГНАЛА
  // =========================
  if (millis() - lastSignalTime > SIGNAL_TIMEOUT_MS) {

    targetValue = 0;

    resetFilter();
  }

  // =========================
  // EMA фильтр
  // =========================
  currentValue += (targetValue - currentValue) * SMOOTHING_FACTOR;

  int out = (int)(currentValue + 0.5);

  display.showNumberDec(constrain(out, 0, 9999));
}
