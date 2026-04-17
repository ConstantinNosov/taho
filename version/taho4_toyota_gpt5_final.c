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

void setup() {
  display.setBrightness(0x0f);
  display.clear();

  FreqCount.begin(500);
}

void loop() {

  if (FreqCount.available()) {
    unsigned long counts = FreqCount.read();

    if (counts > 0) {

      float frequencyHz = counts * 2.0; // компенсация 500 ms

      // добавляем в фильтр
      addValue(frequencyHz);

      float filtered = getAvg();

      // финальное значение
      targetValue = filtered * 30.0;
    }
  }

  // EMA (финальная стабилизация)
  currentValue += (targetValue - currentValue) * SMOOTHING_FACTOR;

  int out = (int)(currentValue + 0.5);
  display.showNumberDec(constrain(out, 0, 9999));
}
