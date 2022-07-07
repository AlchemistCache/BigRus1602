//Scroll speed
#include "BigRus1602.h"

char textSlow[] = " МЕДЛЕННО";
char textFast[] = " БЫСТРО";

//Пины для подключения LCD Keypad Shield
const int rs = 8, en = 9, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
BigRus1602 lcdRus(&lcd);

void setup()
{
  lcdRus.begin();
}

void loop()
{
  //второй аргумент - временной интервал между сдвигами, вводить его не обязательно(по умолчанию 1000 мс)
  lcdRus.scrollText(true, 500);
  lcdRus.print(textFast);

  lcdRus.scrollText(true, 1000);
  lcdRus.print(textSlow);
}
