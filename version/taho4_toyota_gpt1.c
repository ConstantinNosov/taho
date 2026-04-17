#include <TM1637Display.h>

#define CLK_PIN 2
#define DIO_PIN 3
#define SIGNAL_PIN 5   // остаётся 5

TM1637Display display(CLK_PIN, DIO_PIN);

// --- настройки ---
const float PULSES_PER_REV = 2.0;
const int AVG_PULSES = 4;   // усреднение по 4 импульсам

// --- переменные ---
volatile unsigned long lastTime = 0;
volatile unsigned long periodSum = 0;
volatile int pulseCount = 0;
volatile unsigned long avgPeriod = 0;

float currentRPM = 0;
float targetRPM = 0;

unsigned long lastDisplayUpdate = 0;
unsigned long lastSignalTime = 0;

// =============================
// ⚡ PCINT для пина 5
// =============================
ISR(PCINT2_vect) {
  static uint8_t lastState = 0;

  uint8_t state = PIND & (1 << PIND5);

  // ловим FALLING фронт
  if (lastState && !state) {

    unsigned long now = micros();
    unsigned long period = now - lastTime;
    lastTime = now;

    // фильтр мусора (слишком короткие импульсы)
    if (period > 200) {  // ~5 кГц максимум
      periodSum += period;
      pulseCount++;

      if (pulseCount >= AVG_PULSES) {
        avgPeriod = periodSum / pulseCount;
        periodSum = 0;
        pulseCount = 0;
      }
    }

    lastSignalTime = millis();
  }

  lastState = state;
}

void setup() {
  display.setBrightness(0x0f);
  display.clear();

  pinMode(SIGNAL_PIN, INPUT_PULLUP);

  // включаем PCINT (пины 0–7)
  PCICR |= (1 << PCIE2);
  PCMSK2 |= (1 << PCINT21); // пин 5
}

void loop() {

  unsigned long localPeriod;

  noInterrupts();
  localPeriod = avgPeriod;
  interrupts();

  // =============================
  // 🧮 расчет RPM
  // =============================
  if (localPeriod > 0) {
    float freq = 1000000.0 / localPeriod;
    targetRPM = freq * 60.0 / PULSES_PER_REV;
  }

  // =============================
  // ⛔ сигнал пропал
  // =============================
  if (millis() - lastSignalTime > 300) {
    targetRPM = 0;
  }

  // =============================
  // 🎯 сглаживание (мягкое)
  // =============================
  float diff = abs(targetRPM - currentRPM);

  float k;
  if (diff > 1000)      k = 0.4;
  else if (diff > 300)  k = 0.25;
  else                  k = 0.1;

  currentRPM += (targetRPM - currentRPM) * k;

  // =============================
  // 📟 вывод
  // =============================
  if (millis() - lastDisplayUpdate > 50) {
    display.showNumberDec(constrain((int)currentRPM, 0, 9999));
    lastDisplayUpdate = millis();
  }
}
