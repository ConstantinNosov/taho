#include <TM1637Display.h>
#include <FreqCount.h>

#define CLK_PIN 2
#define DIO_PIN 3
#define FILTER_SIZE 5  // Размер буфера для медианного фильтра

TM1637Display display(CLK_PIN, DIO_PIN);

float freqBuffer[FILTER_SIZE];
byte bufferIndex = 0;
bool bufferFull = false;

// Функция медианного фильтра
float medianFilter(float newValue) {
  freqBuffer[bufferIndex] = newValue;
  bufferIndex = (bufferIndex + 1) % FILTER_SIZE;
  
  if (bufferIndex == 0) bufferFull = true;
  
  if (bufferFull) {
    // Копируем буфер для сортировки
    float temp[FILTER_SIZE];
    memcpy(temp, freqBuffer, sizeof(freqBuffer));
    
    // Сортировка пузырьком
    for (int i = 0; i < FILTER_SIZE - 1; i++) {
      for (int j = 0; j < FILTER_SIZE - i - 1; j++) {
        if (temp[j] > temp[j + 1]) {
          float swap = temp[j];
          temp[j] = temp[j + 1];
          temp[j + 1] = swap;
        }
      }
    }
    // Возвращаем медиану
    return temp[FILTER_SIZE / 2];
  }
  return newValue;
}

void setup() {
  display.setBrightness(0x0f);
  display.clear();
  FreqCount.begin(1000);  // Измеряем за 1 секунду для получения частоты в Гц
}

void loop() {
  if (FreqCount.available()) {
    unsigned long counts = FreqCount.read();
    // counts за 1000 мс = частота в Герцах
    float frequencyHz = counts;
    
    // Применяем медианный фильтр
    float filteredHz = medianFilter(frequencyHz);
    
    // Умножаем на 30
    float result = filteredHz * 30;
    
    // Отображаем результат - убрали display.clear()
    // showNumberDec сам обновит только изменившиеся цифры
    display.showNumberDec((int)result);
    
    // Уменьшенная задержка для лучшей отзывчивости
    delay(10);
  }
}
