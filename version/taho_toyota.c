#include <TM1637Display.h>

#define CLK_PIN 2
#define DIO_PIN 3
#define SENSOR_PIN 4

TM1637Display display(CLK_PIN, DIO_PIN);

void setup() {
  pinMode(SENSOR_PIN, INPUT_PULLUP);
  display.setBrightness(0x0f);
  display.clear();
}
void loop() {
  unsigned long pulses = 0;
  
  // Считаем импульсы 0.5 секунды (в 2 раза быстрее)
  unsigned long start = millis();
  while (millis() - start < 500) {  // Изменено с 1000 на 500
    if (digitalRead(SENSOR_PIN) == LOW) {
      pulses++;
      while (digitalRead(SENSOR_PIN) == LOW) {}
    }
  }
  display.showNumberDec(pulses * 60);  // Для 2 импульсов/оборот
  delay(50);  // Пауза для стабильности дисплея
}
