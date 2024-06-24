#include "system.h"
#include "inc/stm32h7xx.h"
#include "inc/gpio.h"

using namespace gpio;

static pinDef m4_led = { .port = GPIOB, .pin = PIN_0, .mode = Output, .type = PushPull, .speed = Low, .pull = None, .alternate = AF0 };
static uint32_t m4_ledCounter = 0;

static void startM7(void);
static void waitForM7(void);
static void pwr_init(void);
static void flash_init(void);

void sys::init(void)
{
	SYS_TRACE("m4 init\n");

	// let the M4 do its conifiguration while the M7 waits
	configurePin(m4_led);
	pwr_init();
	flash_init();
	
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


void pwr_init(void)
{
	/* The nucleo board is connected with a direct SMPS step down converter supply (option 2 in RM0399 Table 33), and this 
	 * causes the hardware to ignore attempts to set some register values, e.g. LDOEN. This has the effect of also limiting 
	 * the clock speed to 400MHz max at voltage VOS1 (DS12923, Table 23). */
	
	uint32_t timeout = 0xFFFF;

	CLEAR_BIT(PWR->CR3, PWR_CR3_LDOEN);												// turn off LDO, SMPS only
	MODIFY_REG(PWR->D3CR, PWR_D3CR_VOS_Msk, PWR_D3CR_VOS_1 | PWR_D3CR_VOS_0);		// set VOS scale 1
	while ((!READ_BIT(PWR->D3CR, PWR_D3CR_VOSRDY)) && (timeout>0)) { timeout--; }	// wait for the voltage to stabilize	
	if (timeout == 0) { SYS_ERROR("VOSRDY timeout"); }
}


void flash_init()
{
	/* This section configures the flash memory wait states and write/read times for the main
	 * AXI data bus. This bus runs at a max of 240MHz (usually half of the core clock speed) and 
	 * the settings depend on Vcore setpoint (VOS0-3) as shown in RM0399 Table 16.
	 * 
	 * Anticipate running at VOS1 with a 400MHz core clock when the clock chain setup is complete
	 * then the AXI bus will be at 200MHz and Table 16 says to use WRHIGHFREQ = 10 and 
	 * latency = 2 wait states */
	
	MODIFY_REG(FLASH->ACR, FLASH_ACR_WRHIGHFREQ_Msk, FLASH_ACR_WRHIGHFREQ_1);
	MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY_Msk, FLASH_ACR_LATENCY_2WS);		
}