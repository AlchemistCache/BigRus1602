// BigRussianFont.h
// ASCII tab https://commons.wikimedia.org/wiki/File:ASCII-Table.svg
//

#pragma once _BIGRUS1602_h

#include "Arduino.h"
#include "LiquidCrystal.h"
#include <avr/pgmspace.h> //инициализируем работу с программной памятью

#define BC_NUM_OF(x) (sizeof(x) / sizeof(x[0])) //макрос для подсчета количества объектов в массиве

//буффер символов для одномоментного вывода на экран, каждый символ занимает 2-5 знаков. Суммарно нельзя превышать 40 символов
#define BC_DEFAULT_BUF_SIZE 7

const uint8_t grapheme[8][8] PROGMEM = {	//Графемы для записи в CGRAM
	{0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111}, // byte 0 (0x40)
	{0b00011, 0b00111, 0b01111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111}, // byte 1 (0x41)
	{0b11000, 0b11100, 0b11110, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111}, // byte 2 (0x42)
	{0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b01111, 0b00111, 0b00011}, // byte 3 (0x43)
	{0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11110, 0b11100, 0b11000}, // byte 4 (0x44)
	{0b11111, 0b11111, 0b11111, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111}, // byte 5 (0x45)
	{0b11111, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111}, // byte 6 (0x46)
	{0b11111, 0b11111, 0b11111, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000}, // byte 7 (0x47)
};

class BigRus1602
{
public:
	BigRus1602(LiquidCrystal *);
	void begin(uint8_t initColumn = 16, uint8_t initRow = 2);
	void print(uint32_t inputData, uint8_t column = 0);
	#if !defined(BC_ONLY_NUMBER)	//если будем использовать буквы
	void print(char *inputData, uint8_t column = 0);
	#endif
	uint16_t getCarret() { return _column; } // получить положение каретки
	void scrollText(bool scroll, uint16_t scrollSpeed = 1000);
	void bufSize(uint8_t bufSize); // буффер для резервирования максимального количества "чистых" символов

private:
	void write(uint8_t *inputData, uint8_t lenClearCharArray);
	void doScrollText();
	LiquidCrystal *_lcd;
	uint16_t _column = 0;
	uint16_t _row = 0;
	uint16_t _scrollSpeed;
	uint8_t _scrollIterration = 0;
	uint8_t _bufSize = 100;
	bool _scrollText = false;

	uint32_t _lastMillis;
};
