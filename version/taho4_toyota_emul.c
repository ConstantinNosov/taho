#include <TM1637Display.h>
#include <FreqCount.h>

#define CLK_PIN 2
#define DIO_PIN 3

TM1637Display display(CLK_PIN, DIO_PIN);

// Переменные для плавного набора
float currentDisplayValue = 0;
float targetDisplayValue = 0;
const float SMOOTHING_FACTOR = 0.3; // Скорость изменения (0-1, чем меньше, тем плавнее)

void setup() {
  display.setBrightness(0x0f);
  display.clear();
  FreqCount.begin(1000);
}

void loop() {
  if (FreqCount.available()) {
    unsigned long counts = FreqCount.read();
    float frequencyHz = counts;      // без фильтрации
    targetDisplayValue = frequencyHz * 30; // Целевое значение (частота * 30)
  }
  
  // Плавное изменение текущего значения к целевому
  if (abs(currentDisplayValue - targetDisplayValue) > 0.1) {
    // Линейная интерполяция для плавного движения
    currentDisplayValue += (targetDisplayValue - currentDisplayValue) * SMOOTHING_FACTOR;
  } else {
    currentDisplayValue = targetDisplayValue;
  }
  
  // Отображение с округлением до целого
  display.showNumberDec((int)currentDisplayValue);
  
  // Небольшая задержка для контроля скорости обновления
  delay(30); // Немного увеличена задержка для лучшей визуализации плавности
}
