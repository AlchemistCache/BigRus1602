//SerialRead
#include "BigRus1602.h"

const int rs = 8, en = 9, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
BigRus1602 lcdRus(&lcd);

void setup()
{
  lcd.begin(16, 2);
  lcdRus.begin();
  Serial.begin(9600);
  lcdRus.scrollText(true);
}

void loop()
{
  char serialText[30];
  if (Serial.available())
  {
    //чем больше символов в строке, тем больше нужно времени чтобы принять ее
    delay(100);

    int availableText = Serial.available();
    for (int i = 0; i < availableText; i++)
    {
      serialText[i] = Serial.read();
      serialText[i + 1] = '\0';
    }
    lcdRus.print(serialText);
  }
}