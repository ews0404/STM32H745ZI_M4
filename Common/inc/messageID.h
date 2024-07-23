#pragma once
#include <stdint.h>


enum MessageID:uint16_t
{
	NoOp = 0,
	SetLED = 1,
	PrintString = 2
};