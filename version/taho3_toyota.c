#include <TM1637Display.h>
#include <FreqCount.h>

#define CLK_PIN 2
#define DIO_PIN 3

TM1637Display display(CLK_PIN, DIO_PIN);

void setup() {
  display.setBrightness(0x0f);
  display.clear();
  FreqCount.begin(1000);
}

void loop() {
  if (FreqCount.available()) {
    unsigned long counts = FreqCount.read();
    float frequencyHz = counts;      // без фильтрации
    float result = frequencyHz * 30;
    display.showNumberDec((int)result);
    delay(10);
  }
}
