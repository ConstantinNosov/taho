#include <TM1637Display.h>

#define CLK_PIN 2
#define DIO_PIN 3
#define SIGNAL_PIN 5

TM1637Display display(CLK_PIN, DIO_PIN);

// --- настройки ---
const float PULSES_PER_REV = 2.0;
const int AVG_PULSES = 6;   // больше = точнее

// --- переменные ---
volatile unsigned long lastTime = 0;
volatile unsigned long timeSum = 0;
volatile int pulseCount = 0;
volatile unsigned long avgPeriod = 0;

float currentRPM = 0;
float targetRPM = 0;

unsigned long lastDisplayUpdate = 0;
unsigned long lastPulseTime = 0;

// =============================
// ⚡ прерывание PCINT
// =============================
ISR(PCINT2_vect) {
  static uint8_t lastState = 0;
  uint8_t state = PIND & (1 << PIND5);

  if (lastState && !state) {

    unsigned long now = micros();
    unsigned long period = now - lastTime;
    lastTime = now;

    if (period > 200) {

      timeSum += period;
      pulseCount++;

      if (pulseCount >= AVG_PULSES) {

        // 🔥 КЛЮЧЕВОЕ ИСПРАВЛЕНИЕ:
        // берем не среднее периода, а среднее времени → потом считаем частоту
        avgPeriod = timeSum / AVG_PULSES;

        timeSum = 0;
        pulseCount = 0;
      }
    }

    lastPulseTime = millis();
  }

  lastState = state;
}

void setup() {
  display.setBrightness(0x0f);
  display.clear();

  pinMode(SIGNAL_PIN, INPUT_PULLUP);

  PCICR |= (1 << PCIE2);
  PCMSK2 |= (1 << PCINT21);
}

void loop() {

  unsigned long localPeriod;

  noInterrupts();
  localPeriod = avgPeriod;
  interrupts();

  if (localPeriod > 0) {

    // 🔥 более точная формула
    float freq = 1000000.0 / localPeriod;
    targetRPM = freq * 60.0 / PULSES_PER_REV;
  }

  // ⛔ пропадание сигнала
  if (millis() - lastPulseTime > 300) {
    targetRPM = 0;
  }

  // 🎯 сглаживание
  float diff = abs(targetRPM - currentRPM);

  float k = (diff > 1000) ? 0.4 :
            (diff > 300)  ? 0.25 : 0.1;

  currentRPM += (targetRPM - currentRPM) * k;

  // 📟 вывод
  if (millis() - lastDisplayUpdate > 50) {
    display.showNumberDec(constrain((int)currentRPM, 0, 9999));
    lastDisplayUpdate = millis();
  }
}
