//
//
//

#include "BigRus1602.h"
#include "BigRus1602Font.h"

BigRus1602::BigRus1602(LiquidCrystal *lcd)
{
	_lcd = lcd;
}

void BigRus1602::begin(uint8_t initColumn, uint8_t initRow) //запишем свои символы (8 штук) в память экрана CGRAM (Character generator RAM)
{
	_lcd->begin(initColumn, initRow);
	uint8_t buffer[8];
	for (uint8_t k = 0; k < 8; k++)
	{
		for (uint8_t i = 0; i < 8; i++)
		{
			buffer[i] = pgm_read_word_near(grapheme[k] + i);
		}
		_lcd->createChar(k, buffer);
	}
}

void BigRus1602::bufSize(uint8_t bufSize) //размер буффера с "чистыми" символами
{
	_bufSize = bufSize;
}

void BigRus1602::scrollText(bool scroll, uint16_t scrollSpeed) //Скроллинг текста по вертикали и скорость
{
	_scrollText = scroll;
	_scrollSpeed = scrollSpeed;
}

void BigRus1602::doScrollText()
{
	for (uint8_t i = 0; i < _scrollIterration; i++) //_scrollIterration
	{
		//смещаемся влево
		_lcd->command(0x10 | 0x08); //0x10 | 0x08 | 0x00		LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT

		while ((millis() - _lastMillis) < _scrollSpeed)
		{
		}
		_lastMillis = millis();
	}
	_lcd->clear();
}

#if defined(BC_STRONG_FONT) || defined(BC_SMOOTH_FONT)

void BigRus1602::print(char *inputData, uint8_t column)
{
	//все пришедшие символы считаем "грязными"
	uint16_t lenDirtCharArray = strlen(inputData); // высчитываем количество "грязных" символов
	uint8_t clearCharArray[_bufSize];			   // выделяем память для "чистых" символов
	uint8_t lenClearCharArray = 0;				   // колич. "чистых" символов
	uint8_t buff[BC_DEFAULT_BUF_SIZE];			   // здесь храним часть от clearCharArray[], которую пользователь будет видеть на экране (max 40 символов в строке)

	/*Все рус. символы занимают два байта. Если буква имеет верх. регистр., то первый байт = 0xD0 (208, DEC)
	Если буква имеет нижн. регистр., то первый байт = 0xD0 (208, DEC) или = 0xD1 (209, DEC)	*/

	for (uint16_t numbDirtSymbol = 0; numbDirtSymbol < lenDirtCharArray; numbDirtSymbol++) //перебор "грязных" символов
	{
		//если символ в пределах рус. алфавита или цифра, то запишем его в "чистые"
		if ((uint8_t)inputData[numbDirtSymbol] == 32 ||												  //пробел
			(uint8_t)inputData[numbDirtSymbol] >= 144 && (uint8_t)inputData[numbDirtSymbol] <= 175 || //144 - А		175 - Я
			(uint8_t)inputData[numbDirtSymbol] >= 48 && (uint8_t)inputData[numbDirtSymbol] <= 57 ||	  //48 - 0		57 - 9
			(uint8_t)inputData[numbDirtSymbol] == 33 ||												  //!
			(uint8_t)inputData[numbDirtSymbol] >= 44 && (uint8_t)inputData[numbDirtSymbol] <= 46 ||	  //44 - ,	45 - -		46 - .
			(uint8_t)inputData[numbDirtSymbol] == 58 ||												  //:
			(uint8_t)inputData[numbDirtSymbol] == 59 ||												  //;
			(uint8_t)inputData[numbDirtSymbol] == 63 ||												  //?
			(uint8_t)inputData[numbDirtSymbol] == 129												//Ё
		)
		{
			clearCharArray[lenClearCharArray++] = (uint8_t)inputData[numbDirtSymbol];
		}
	}

	for (uint8_t i = 0; i < lenClearCharArray; i++) //переберем все чистые символ
	{
		_column = column;

		//если символов осталось вывести больше чем максимально вмещается в экран, то выводим их порционно
		if ((lenClearCharArray - i) >= BC_DEFAULT_BUF_SIZE)
		{
			memcpy(buff, &(clearCharArray[i]), BC_DEFAULT_BUF_SIZE);
			this->write((uint8_t *)buff, BC_DEFAULT_BUF_SIZE);
		}

		else //если все символы умещаются в экран за раз, то выводим все сразу
		{
			memcpy(buff, &(clearCharArray[i]), (lenClearCharArray - i));
			this->write((uint8_t *)buff, (lenClearCharArray - i));
		}

		if (_scrollText == false)
			i = lenClearCharArray; // если скролл отключен, то притянем счетчик перебора к последнему символу
		else					   //иначе скроллим
		{
			_scrollIterration -= column;
			this->doScrollText();
		}
	}
}
#endif

void BigRus1602::print(uint32_t inputData, uint8_t column)
{
	_column = column;

	if (inputData == 0) //если пришел нуль
	{
		this->write((uint8_t *)0x1F, 1);
	}

	else //если пришел не нуль
	{
		uint8_t numLenght = 10;	 // количество цифр в пришедшем long числе (максимум 10)
		uint8_t buff[numLenght]; // буффер цифр

		//разбиваем число на цифры и вычисляем их реальное количество
		numLenght = 0; //сбросим numLenght чтобы пересчитать реальное кол-во цифр в числе
		while (inputData != 0)
		{
			buff[numLenght++] = inputData % 10;
			inputData /= 10;
		}

		uint8_t bytes[numLenght];

		//инвертируем буффер
		for (uint8_t i = 0; i < numLenght; i++)
		{
			bytes[i] = buff[numLenght - (i + 1)];
		}

		this->write((byte *)bytes, numLenght);
	}
}

//принимаем текст для печати и количество символов
void BigRus1602::write(uint8_t *inputData, uint8_t sizeArr)
{
	for (uint8_t i = 0; i < sizeArr; i++)
	{
		_row = 0;
		switch (inputData[i])
		{
#if defined BC_STRONG_FONT

		case 0x20: //space
			_lcd->setCursor(_column, _row);
			_lcd->write(" ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(" ");
			_column -= 2;
			break;

		case 0x21: //!
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write(" ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\10\ ");
			_column -= 2;
			break;

		case 0x2C: //,
			_lcd->setCursor(_column, _row);
			_lcd->write(" ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(",");
			_column -= 2;
			break;

		case 0x2D: //-
			_lcd->setCursor(_column, _row);
			_lcd->write("\10\10\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\   ");
			_column -= 1;
			break;

		case 0x2E: //.
			_lcd->setCursor(_column, _row);
			_lcd->write(" ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(".");
			_column -= 2;
			break;

		case 0x3A: //:
			_lcd->setCursor(_column, _row);
			_lcd->write(".");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(".");
			_column -= 2;
			break;

		case 0x3B: //;
			_lcd->setCursor(_column, _row);
			_lcd->write(".");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(",");
			_column -= 2;
			break;

		case 0x3F: //?
			_lcd->setCursor(_column, _row);
			_lcd->write("\5\2\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\6\  ");
			_column -= 1;
			break;

		case 0:
		case 0x1F:
		case 0x30: //0
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\7");
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\10");
			_lcd->write(255);
			_lcd->write("\ ");
			break;

		case 1:
		case 0x31: //1
			_lcd->setCursor(_column, _row);
			_lcd->write("\7");
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\ ");
			_lcd->write(255);
			_lcd->write("\ ");
			_column -= 1;
			break;

		case 2:
		case 0x32: //2
			_lcd->setCursor(_column, _row);
			_lcd->write("\5\5");
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\10\10\ ");
			break;

		case 3:
		case 0x33: //3
			_lcd->setCursor(_column, _row);
			_lcd->write("\5\5");
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\10\10");
			_lcd->write(255);
			_lcd->write("\ ");
			break;

		case 4:
		case 0x34: //4
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\10");
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\ \ ");
			_lcd->write(255);
			_lcd->write("\ ");
			break;

		case 5:
		case 0x35: //5
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\5\5\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\10\10");
			_lcd->write(255);
			_lcd->write("\ ");
			break;

		case 6:
		case 0x36: //6
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\5\5\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\10");
			_lcd->write(255);
			_lcd->write("\ ");
			break;

		case 7:
		case 0x37: //7
			_lcd->setCursor(_column, _row);
			_lcd->write("\7\7");
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\ \7");
			_lcd->write(255);
			_lcd->write("\ ");
			break;

		case 8:
		case 0x38: //8
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\5");
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\10");
			_lcd->write(255);
			_lcd->write("\ ");
			break;

		case 9:
		case 0x39: //9
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\5");
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\10\10");
			_lcd->write(255);
			_lcd->write("\ ");
			break;

		case 0x90: //А
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\5");
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->write(255);
			_lcd->write("\ ");
			break;

		case 0x91: //Б
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\5\5\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\10");
			_lcd->write(255);
			_lcd->write("\ ");
			break;

		case 0x92: //В
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\5\4\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\5\2\ ");
			break;

		case 0x93: //Г
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\7\7\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\ \ \ ");
			break;

		case 0x94: //Д
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\7\2\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\10\4\ ");
			break;

		case 0x81: //Ё
		case 0x95: //Е
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\5\5\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\6\6\ ");
			break;

		case 0x96: //Ж
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\10");
			_lcd->write(255);
			_lcd->write("\10");
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->write(255);
			_lcd->write("\ ");
			_column += 2;
			break;

		case 0x97: //З
			_lcd->setCursor(_column, _row);
			_lcd->write("\5\5");
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\6\6");
			_lcd->write(255);
			_lcd->write("\ ");
			break;

		case 0x98: //И
		case 0x99: //Й
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\1");
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\4");
			_lcd->write(255);
			_lcd->write("\ ");
			break;

		case 0x9A: //К
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\10\4\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\ \2\ ");
			break;

		case 0x9B: //Л
			_lcd->setCursor(_column, _row);
			_lcd->write("\1\7");
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->write(255);
			_lcd->write("\ ");
			break;

		case 0x9C: //М
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\7");
			_lcd->write(255);
			_lcd->write("\7");
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->write(255);
			_lcd->write("\ ");
			_column += 2;
			break;

		case 0x9D: //Н
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\10");
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->write(255);
			_lcd->write("\ ");
			break;

		case 0x9E: //О
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\7");
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\10");
			_lcd->write(255);
			_lcd->write("\ ");
			break;

		case 0x9F: //П
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\7");
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->write(255);
			_lcd->write("\ ");
			break;

		case 0xA0: //Р
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\5");
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\   ");
			break;

		case 0xA1: //С
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\7\7\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\10\10\ ");
			break;

		case 0xA2: //Т
			_lcd->setCursor(_column, _row);
			_lcd->write("\7");
			_lcd->write(255);
			_lcd->write("\7\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\ ");
			_lcd->write(255);
			_lcd->write("\ \ ");
			break;

		case 0xA3: //У
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\10");
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\10\10\4\ ");
			break;

		case 0xA4: //Ф
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\5");
			_lcd->write(255);
			_lcd->write("\5");
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\  ");
			_lcd->write(255);
			_lcd->write("\ \ \ ");
			_column += 2;
			break;

		case 0xA5: //Х
			_lcd->setCursor(_column, _row);
			_lcd->write("\3\10\4\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\1\ \2\ ");
			break;

		case 0xA6: //Ц
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->write(255);
			_lcd->write("\ \ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\10");
			_lcd->write(255);
			_lcd->write("\10\ ");
			_column += 1;
			break;

		case 0xA7: //Ч
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\10");
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\ \ ");
			_lcd->write(255);
			_lcd->write("\ ");
			break;

		case 0xA8: //Ш
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\ \ \ ");
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\10");
			_lcd->write(255);
			_lcd->write("\10");
			_lcd->write(255);
			_lcd->write("\ ");
			_column += 2;
			break;

		case 0xA9: //Щ
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\ \ \ ");
			_lcd->write(255);
			_lcd->write("\ \ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\10");
			_lcd->write(255);
			_lcd->write("\10");
			_lcd->write(255);
			_lcd->write("\10");
			_lcd->write("\ ");
			_column += 3;
			break;

		case 0xAA: //Ъ
			_lcd->setCursor(_column, _row);
			_lcd->write("\7");
			_lcd->write(255);
			_lcd->write("\ \ \ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\ ");
			_lcd->write(255);
			_lcd->write("\6");
			_lcd->write(255);
			_lcd->write("\ ");
			_column += 1;
			break;

		case 0xAB: //Ы
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\ \ \ ");
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\6");
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->write(255);
			_lcd->write("\ ");
			_column += 2;
			break;

		case 0xAC: //Ь
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\ \ \ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\6");
			_lcd->write(255);
			_lcd->write("\ ");
			break;

		case 0xAD: //Э
			_lcd->setCursor(_column, _row);
			_lcd->write("\5\5\2\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\6\6\4\ ");
			break;

		case 0xAE: //Ю
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\10");
			_lcd->write(255);
			_lcd->write("\7");
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->write(255);
			_lcd->write("\10");
			_lcd->write(255);
			_lcd->write("\ ");
			_column += 2;
			break;

		case 0xAF: //Я
			_lcd->setCursor(_column, _row);
			_lcd->write("\1\5");
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\1\7");
			_lcd->write(255);
			_lcd->write("\ ");
			break;

#elif defined BC_SMOOTH_FONT
		case 0x20: //space
			_lcd->setCursor(_column, _row);
			_lcd->write(" ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(" ");
			_column -= 2;
			break;

		case 0x21: //!
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write(" ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\10\ ");
			_column -= 2;
			break;

		case 0x2C: //,
			_lcd->setCursor(_column, _row);
			_lcd->write(" ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(",");
			_column -= 2;
			break;

		case 0x2D: //-
			_lcd->setCursor(_column, _row);
			_lcd->write("\10\10\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\ \ \ ");
			_column -= 1;
			break;

		case 0x2E: //.
			_lcd->setCursor(_column, _row);
			_lcd->write(" ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(".");
			_column -= 2;
			break;

		case 0x3A: //:
			_lcd->setCursor(_column, _row);
			_lcd->write(".");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(".");
			_column -= 2;
			break;

		case 0x3B: //;
			_lcd->setCursor(_column, _row);
			_lcd->write(".");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(",");
			_column -= 2;
			break;

		case 0x3F: //?
			_lcd->setCursor(_column, _row);
			_lcd->write("\5\2\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\6\ \ ");
			_column -= 1;
			break;

		case 0:
		case 0x1F:
		case 0x30: //0
			_lcd->setCursor(_column, _row);
			_lcd->write("\1\7\2\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\3\10\4\ ");
			break;

		case 1:
		case 0x31: //1
			_lcd->setCursor(_column, _row);
			_lcd->write("\7\2\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\ ");
			_lcd->write(255);
			_lcd->write("\ ");
			_column -= 1;
			break;

		case 2:
		case 0x32: //2
			_lcd->setCursor(_column, _row);
			_lcd->write("\7\5\4\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\1\6\10\ ");
			break;

		case 3:
		case 0x33: //3
			_lcd->setCursor(_column, _row);
			_lcd->write("\5\5\4\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\6\6\2\ ");
			break;

		case 4:
		case 0x34: //4
			_lcd->setCursor(_column, _row);
			_lcd->write("\1\10\1\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\ \ \4\ ");
			break;

		case 5:
		case 0x35: //5
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\5\7\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\6\6\2\ ");
			break;

		case 6:
		case 0x36: //6
			_lcd->setCursor(_column, _row);
			_lcd->write("\1\5\5\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\6");
			_lcd->write(255);
			_lcd->write("\ ");
			break;

		case 7:
		case 0x37: //7
			_lcd->setCursor(_column, _row);
			_lcd->write("\7\7\2\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\ \7");
			_lcd->write(255);
			_lcd->write("\ ");
			break;

		case 8:
		case 0x38: //8
			_lcd->setCursor(_column, _row);
			_lcd->write("\1\5\2\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\3\6\4\ ");
			break;

		case 9:
		case 0x39: //9
			_lcd->setCursor(_column, _row);
			_lcd->write("\1\5\2\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\6\6\4\ ");
			break;

		case 0x90: //А
			_lcd->setCursor(_column, _row);
			_lcd->write("\1\5\2\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\3\ \4\ ");
			break;

		case 0x91: //Б
			_lcd->setCursor(_column, _row);
			_lcd->write("\1\5\5\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\6\4");
			_lcd->write("\ ");
			break;

		case 0x92: //В
			_lcd->setCursor(_column, _row);
			_lcd->write("\1\5\4\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\6\2\ ");
			break;

		case 0x93: //Г
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\7\7\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\ \ \ ");
			break;

		case 0x94: //Д
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\7\2\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\10\4\ ");
			break;

		case 0x81: //Ё
		case 0x95: //Е
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\5\5\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\6\6\ ");
			break;

		case 0x96: //Ж
			_lcd->setCursor(_column, _row);
			_lcd->write("\1\10\1\10\2\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\3");
			_lcd->write("\ ");
			_lcd->write("\4");
			_lcd->write("\ ");
			_lcd->write("\4\ ");
			_column += 2;
			break;

		case 0x97: //З
			_lcd->setCursor(_column, _row);
			_lcd->write("\5\5\2\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\6\6\4\ ");
			break;

		case 0x98: //И
		case 0x99: //Й
			_lcd->setCursor(_column, _row);
			_lcd->write("\1\1\2\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\3\4\4\ ");
			break;

		case 0x9A: //К
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\10\4\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\ \2\ ");
			break;

		case 0x9B: //Л
			_lcd->setCursor(_column, _row);
			_lcd->write("\1\7\2\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\4\ ");
			_lcd->write(255);
			_lcd->write("\ ");
			break;

		case 0x9C: //М
			_lcd->setCursor(_column, _row);
			_lcd->write("\1\7\4\7\2\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\3\   \4\ ");
			_column += 2;
			break;

		case 0x9D: //Н
			_lcd->setCursor(_column, _row);
			_lcd->write("\1\10\2\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\3\ \4\ ");
			break;

		case 0x9E: //О
			_lcd->setCursor(_column, _row);
			_lcd->write("\1\7\2\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\3\10\4\ ");
			break;

		case 0x9F: //П
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\7");
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->write(255);
			_lcd->write("\ ");
			break;

		case 0xA0: //Р
			_lcd->setCursor(_column, _row);
			_lcd->write("\1\5\2\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\ ");
			break;

		case 0xA1: //С
			_lcd->setCursor(_column, _row);
			_lcd->write("\1\7\7\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\3\10\10\ ");
			break;

		case 0xA2: //Т
			_lcd->setCursor(_column, _row);
			_lcd->write("\7");
			_lcd->write(255);
			_lcd->write("\7\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\ \4\  ");
			break;

		case 0xA3: //У
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\10\1\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\10\10\4\ ");
			break;

		case 0xA4: //Ф
			_lcd->setCursor(_column, _row);
			_lcd->write("\1\5");
			_lcd->write(255);
			_lcd->write("\5\2\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\  ");
			_lcd->write(255);
			_lcd->write("\   ");
			_column += 2;
			break;

		case 0xA5: //Х
			_lcd->setCursor(_column, _row);
			_lcd->write("\3\10\4\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\1\ \2\ ");
			break;

		case 0xA6: //Ц
			_lcd->setCursor(_column, _row);
			_lcd->write("\1\ \2\  ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\3\10");
			_lcd->write(255);
			_lcd->write("\10\ ");
			_column += 1;
			break;

		case 0xA7: //Ч
			_lcd->setCursor(_column, _row);
			_lcd->write("\1\10\1\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\ \ \4\ ");
			break;

		case 0xA8: //Ш
			_lcd->setCursor(_column, _row);
			_lcd->write("\1\   \1\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\3\10\1\10\4\ ");
			_column += 2;
			break;

		case 0xA9: //Щ
			_lcd->setCursor(_column, _row);
			_lcd->write("\1\   \1\  ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\3\10\1\10");
			_lcd->write(255);
			_lcd->write("\10\ ");
			_column += 3;
			break;

		case 0xAA: //Ъ
			_lcd->setCursor(_column, _row);
			_lcd->write("\7\2\  ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\ ");
			_lcd->write(255);
			_lcd->write("\6\4\ ");
			_column += 1;
			break;

		case 0xAB: //Ы
			_lcd->setCursor(_column, _row);
			_lcd->write("\1\   \1\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\6\4\ \4\ ");
			_column += 2;
			break;

		case 0xAC: //Ь
			_lcd->setCursor(_column, _row);
			_lcd->write("\1\   ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\6\4\ ");
			break;

		case 0xAD: //Э
			_lcd->setCursor(_column, _row);
			_lcd->write("\5\5\2\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\6\6\4\ ");
			break;

		case 0xAE: //Ю
			_lcd->setCursor(_column, _row);
			_lcd->write("\1\10\1\7\2\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\4\ \3\10\4\ ");
			_column += 2;
			break;

		case 0xAF: //Я
			_lcd->setCursor(_column, _row);
			_lcd->write("\1\5");
			_lcd->write(255);
			_lcd->write("\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\1\7");
			_lcd->write(255);
			_lcd->write("\ ");
			break;

#else BC_ONLY_NUMBER
		case 0:
		case 0x1F:
			_lcd->setCursor(_column, _row);
			_lcd->write("\1\7\2\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\3\10\4\ ");
			break;

		case 1:
			_lcd->setCursor(_column, _row);
			_lcd->write("\7\2\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\ ");
			_lcd->write(255);
			_lcd->write("\ ");
			_column -= 1;
			break;

		case 2:
			_lcd->setCursor(_column, _row);
			_lcd->write("\7\5\4\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\1\6\10\ ");
			break;

		case 3:
			_lcd->setCursor(_column, _row);
			_lcd->write("\5\5\4\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\6\6\2\ ");
			break;

		case 4:
			_lcd->setCursor(_column, _row);
			_lcd->write("\1\10\1\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\ \ \4\ ");
			break;

		case 5:
			_lcd->setCursor(_column, _row);
			_lcd->write(255);
			_lcd->write("\5\7\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\6\6\2\ ");
			break;

		case 6:
			_lcd->setCursor(_column, _row);
			_lcd->write("\1\5\5\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write(255);
			_lcd->write("\6");
			_lcd->write(255);
			_lcd->write("\ ");
			break;

		case 7:
			_lcd->setCursor(_column, _row);
			_lcd->write("\7\7\2\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\ \7");
			_lcd->write(255);
			_lcd->write("\ ");
			break;

		case 8:
			_lcd->setCursor(_column, _row);
			_lcd->write("\1\5\2\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\3\6\4\ ");
			break;

		case 9:
			_lcd->setCursor(_column, _row);
			_lcd->write("\1\5\2\ ");
			_lcd->setCursor(_column, ++_row);
			_lcd->write("\6\6\4\ ");
			break;

#endif // FONT
		default:	//если пришел неизвестный символ
			break;
		}

		_column += 4;

		if (i == 0)
			_scrollIterration = _column;
	}
}
