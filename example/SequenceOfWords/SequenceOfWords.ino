//Sequence of words
#include "BigRus1602.h"

const int rs = 8, en = 9, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
BigRus1602 lcdRus(&lcd);

void setup()
{
  lcdRus.begin();

  for (uint8_t i = 0; i <= 100; i++)
  {
    lcdRus.print(i);
    lcd.setCursor(lcdRus.getCarret(), 1);
    lcd.print("% ");
    delay(50);
  }
  delay(3000);
}

void loop()
{
  lcd.clear();
  for (uint8_t min = 0; min < 24; min++)
  {
    for (uint8_t sec = 0; sec < 60; sec++)
    {
      lcdRus.print(min);
      lcdRus.print(":", lcdRus.getCarret() - 1);
      lcdRus.print(sec, lcdRus.getCarret() - 1);

      //заполним "хвост" пробелами чтобы избежать появления артефактов от старого текста. Можно заменить на lcd.clear()
      lcdRus.print("    ", lcdRus.getCarret() - 1);

      delay(1000);
    }
  }
}