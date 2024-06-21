#include "inc/gpio.h"

using namespace gpio;

static pinDef m4_led = { .port = GPIOB, .pin = PIN_0, .mode = Output, .type = PushPull, .speed = Low, .pull = None, .alternate = AF0 };

int main(void)
{
	int j = 0;
	
	configurePin(m4_led);
	
	while (1)
	{
		j++;
		if (j > 1000000)
		{
			j = 0;
			toggle(m4_led);
		}
	}
}