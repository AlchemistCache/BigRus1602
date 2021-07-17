//Simple print
//Демонстрация всех доступных символов

#include "BigRus1602.h"

//библиотека позволяет выводить данные типа byte, int, long или массив char[]
char text[] = " АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ 1234567890 !?-.,:;"; //
int num = 1234;

//Пины для подключения LCD Keypad Shield
const int rs = 8, en = 9, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
BigRus1602 lcdRus(&lcd);    //указатель &lcd обязателен

void setup()
{
  lcdRus.begin();
  lcdRus.print("ЛСД");
  delay(3000);
}

void loop()
{
  lcdRus.scrollText(false);
  lcdRus.print(num);

  delay(3000);

  lcdRus.scrollText(true);
  lcdRus.print(text);
}
