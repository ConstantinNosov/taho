#include <TM1637Display.h>

#define CLK_PIN 2
#define DIO_PIN 3
#define SIGNAL_PIN 5

TM1637Display display(CLK_PIN, DIO_PIN);

// --- настройки ---
const float PULSES_PER_REV = 2.0;
const int N = 4;  // усреднение по N импульсам

// --- переменные ---
volatile unsigned long lastTime = 0;
volatile unsigned long timeSum = 0;
volatile int count = 0;
volatile unsigned long avgPeriod = 0;

float currentRPM = 0;
float targetRPM = 0;

unsigned long lastDisplay = 0;
unsigned long lastPulseSeen = 0;

// =============================
// ⚡ прерывание PCINT (пин 5)
// =============================
ISR(PCINT2_vect) {
  static uint8_t lastState = 0;
  uint8_t state = PIND & (1 << PIND5);

  if (lastState && !state) {

    unsigned long now = micros();
    unsigned long dt = now - lastTime;
    lastTime = now;

    // фильтр мусора
    if (dt > 200) {

      timeSum += dt;
      count++;

      if (count >= N) {
        avgPeriod = timeSum / N;
        timeSum = 0;
        count = 0;
      }

      lastPulseSeen = millis();
    }
  }

  lastState = state;
}

// =============================
// 🔧 округление до 10 RPM
// =============================
int roundTo10(float value) {
  return ((int)(value + 5) / 10) * 10;
}

void setup() {
  display.setBrightness(0x0f);
  display.clear();

  pinMode(SIGNAL_PIN, INPUT_PULLUP);

  // включаем PCINT для пинов D0–D7
  PCICR |= (1 << PCIE2);
  PCMSK2 |= (1 << PCINT21);
}

void loop() {

  // =============================
  // 🧮 расчет RPM
  // =============================
  unsigned long period;

  noInterrupts();
  period = avgPeriod;
  interrupts();

  if (period > 0) {
    float freq = 1000000.0 / period;
    targetRPM = freq * 60.0 / PULSES_PER_REV;
  }

  // =============================
  // ⛔ если сигнал пропал
  // =============================
  if (millis() - lastPulseSeen > 300) {
    targetRPM = 0;
  }

  // =============================
  // 🎯 сглаживание
  // =============================
  currentRPM += (targetRPM - currentRPM) * 0.2;

  // =============================
  // 📟 вывод
  // =============================
  if (millis() - lastDisplay > 50) {

    int rpm = roundTo10(currentRPM);
    display.showNumberDec(constrain(rpm, 0, 9999));

    lastDisplay = millis();
  }
}
