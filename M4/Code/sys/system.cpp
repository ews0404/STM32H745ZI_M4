#include "system.h"
#include "inc/stm32h7xx.h"
#include "inc/gpio.h"

using namespace gpio;

static pinDef m4_led = { .port = GPIOB, .pin = PIN_0, .mode = Output, .type = PushPull, .speed = Low, .pull = None, .alternate = AF0 };
static uint32_t m4_ledCounter = 0;

static void startM7(void);
static void waitForM7(void);

void sys::init(void)
{
	// let the M4 do its conifiguration while the M7 waits
	configurePin(m4_led);
	
	// make the M4 wait while the M7 does its configuration
	startM7();
	waitForM7();
}


void sys::update(void)
{
	m4_ledCounter++;
	if (m4_ledCounter > 100000) { m4_ledCounter = 0; toggle(m4_led); }
}


void startM7(void)
{
	// set RCC->CGR:BOOT_C1 to true to signal the M7 core that it can run its initialization
	// igore option byte BCM7 since using that stops the core clock which interferes with debugging
	SET_BIT(RCC->GCR, RCC_GCR_BOOT_C1);
}


void waitForM7(void)
{
	// wait for the M7 core to set RCC->GCR:BOOT_C2 to true, indicating that the M7 is done initializing
	// igore option byte BCM4 since using that stops the core clock which interferes with debugging
	while (!READ_BIT(RCC->GCR, RCC_GCR_BOOT_C2)) {}
}


