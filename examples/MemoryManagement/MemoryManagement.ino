#include "BigRus1602.h"

char text[] = " АЛХИМИЯ";

//Пины для подключения LCD Keypad Shield
const int rs = 8, en = 9, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
BigRus1602 lcdRus(&lcd);

void setup()
{
  lcdRus.begin();
  //Изменяем буффер для экономии SRAM памяти. Параметр => максимальное количество символов, максимально 255. (по-умолчанию 100)
  lcdRus.bufSize(2);
  lcdRus.scrollText(true);
  lcdRus.print(text);
}

void loop()
{
}
