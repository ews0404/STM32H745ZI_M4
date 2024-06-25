#pragma once
#include "stm32h745xx.h"
#include <stdint.h>

namespace gpio
{
	// port pin
	enum Pin : uint32_t
	{
		PIN_0 = 0,
		PIN_1 = 1,
		PIN_2 = 2,
		PIN_3 = 3,
		PIN_4 = 4,
		PIN_5 = 5,
		PIN_6 = 6,
		PIN_7 = 7,
		PIN_8 = 8,
		PIN_9 = 9,
		PIN_10 = 10,
		PIN_11 = 11,
		PIN_12 = 12,
		PIN_13 = 13,
		PIN_14 = 14,
		PIN_15 = 15
	};


	// pin mode (RM0399 12.4.1)
	enum Mode : uint32_t
	{
		Input = 0b00,
		Output = 0b01,
		Alternate = 0b10,
		Analog = 0b11
	};


	// pin output type (RM0399 12.4.2)
	enum Type : uint32_t
	{
		PushPull = 0,
		OpenDrain = 1
	};


	// pin drive speed (RM0399 12.4.3, DS12923 Table 72)
	enum Speed : uint32_t
	{
		Low = 0b00,
		Med = 0b01,
		High = 0b10,
		VeryHigh = 0b11
	};


	// pin output pull-up/down resistor (RM0399 12.4.4)
	enum Pull : uint32_t
	{
		None = 0b00,
		Up = 0b01,
		Down = 0b10
	};


	// pin alternate function (RM0399 12.4.9/10, DocID024030 Table 12)
	enum Alternate : uint32_t
	{
		AF0 = 0b0000,
		AF1 = 0b0001,
		AF2 = 0b0010,
		AF3 = 0b0011,
		AF4 = 0b0100,
		AF5 = 0b0101,
		AF6 = 0b0110,
		AF7 = 0b0111,
		AF8 = 0b1000,
		AF9 = 0b1001,
		AF10 = 0b1010,
		AF11 = 0b1011,
		AF12 = 0b1100,
		AF13 = 0b1101,
		AF14 = 0b1110,
		AF15 = 0b1111,
	};


	// pin definition, contains all info to configure and read/write to a pin
	struct pinDef { GPIO_TypeDef* port; Pin pin; Mode mode; Type type; Speed speed; Pull pull; enum Alternate alternate; };


	// initializes a single gpio pin
	void configurePin(pinDef gpio);


	// write to digital IO pin
	void digitalWrite(pinDef gpio, bool value);


	// read from digital IO pin
	bool digitalRead(pinDef gpio);


	// toggle value of digital IO pin
	void toggle(pinDef gpio);
}