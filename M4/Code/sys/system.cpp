#include "system.h"
#include "inc/stm32h7xx.h"
#include "inc/gpio.h"

using namespace gpio;

static pinDef m4_led = { .port = GPIOB, .pin = PIN_0, .mode = Output, .type = PushPull, .speed = Low, .pull = None, .alternate = AF0 };
static uint32_t m4_ledCounter = 0;

static void led_init(void);
static void led_update(void);
static void pwr_init(void);
static void flash_init(void);
static void lse_clock_init(void);
static void hse_clock_init(void);
static void startM7(void);
static void waitForM7(void);


void sys::init(void)
{
	// let the M4 do its conifiguration while the M7 waits
	led_init();
	pwr_init();
	flash_init();
	lse_clock_init();
	hse_clock_init();
	
	// make the M4 wait while the M7 does its configuration
	startM7();
	waitForM7();
}


void sys::update(void)
{
	led_update();
}


void led_init(void)
{
	configurePin(m4_led);
	digitalWrite(m4_led, 1);
}


void led_update(void)
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
	/* The nucleo board is connected with a direct SMPS step down converter supply (option 2 in RM0399 Figure 22), and this 
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


void lse_clock_init(void)
{
	uint32_t timeout = 0xFFFF;
	
	// turn on the Low Speed External (LSE) 32.768kHz crystal oscillator for the real time clock, wait for it to stabilize
	SET_BIT(RCC->CSR, RCC_CSR_LSION);													// turn on LSE drive
	while ((!READ_BIT(RCC->CSR, RCC_CSR_LSIRDY)) && (timeout>0)) { timeout--; }			// wait for LSE to start
	if (timeout == 0) { SYS_ERROR("LSIRDY timeout"); }
}

void hse_clock_init(void)
{
	/* initialize High Speed External (HSE) clock tree, see RM0399 Figure 51 
	 * (8MHz HSE clock input) * (/1 DIVM1) * (x100 DIVN1) * (/2 DIVP1) = (400MHz PLLCLK output) */
	
	uint32_t timeout = 0xFFFF;
	
	// turn on the High Speed External (HSE) 8MHz crystal oscillator for the main system clock, wait for it to stabilize
	SET_BIT(RCC->CR, RCC_CR_HSEON);														// turn on HSE drive
	while ((!READ_BIT(RCC->CR, RCC_CR_HSERDY)) && (timeout>0)) { timeout--; }			// wait for HSE to start
	if (timeout == 0) { SYS_ERROR("HSE timeout"); }
	
	// configure PLL1 to convert 8MHz HSE input to 400MHz DIVP1 output (RM0399 figure 54)
	MODIFY_REG(RCC->PLLCKSELR, RCC_PLLCKSELR_PLLSRC_Msk, RCC_PLLCKSELR_PLLSRC_HSE);		// configure PLL source mux to use HSE
	MODIFY_REG(RCC->PLLCKSELR, RCC_PLLCKSELR_DIVM1_Msk, 1 << RCC_PLLCKSELR_DIVM1_Pos);	// DIVM1, divide by 1
	MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLL1RGE, 0b01 << RCC_PLLCFGR_PLL1RGE_Pos);		// PLL1 input frequency range 4-8MHz
	CLEAR_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLL1VCOSEL); 									// PLL1 wide-range VCO (192-960MHz)
	CLEAR_BIT(RCC->PLL1FRACR, RCC_PLL1FRACR_FRACN1);									// do not use fractional mode
	MODIFY_REG(RCC->PLL1DIVR, RCC_PLL1DIVR_N1_Msk, 99 << RCC_PLL1DIVR_N1_Pos);			// DIVN1, multiply by 100
	MODIFY_REG(RCC->PLL1DIVR, RCC_PLL1DIVR_P1_Msk, 1 << RCC_PLL1DIVR_P1_Pos);			// DIVP1, divide by 2
	SET_BIT(RCC->PLLCFGR, RCC_PLLCFGR_DIVP1EN);											// enable DIVP1 output (400MHz SYSCLK)
	CLEAR_BIT(RCC->PLLCFGR, RCC_PLLCFGR_DIVQ1EN);										// disable DIVQ1 output
	CLEAR_BIT(RCC->PLLCFGR, RCC_PLLCFGR_DIVR1EN);										// disable DIVR1 output
	SET_BIT(RCC->CR, RCC_CR_PLL1ON);													// enable PLL1
	while ((!READ_BIT(RCC->CR, RCC_CR_PLL1RDY)) && (timeout>0)) { timeout--; }			// wait for PLL1 to start
	if (timeout == 0) { SYS_ERROR("PLL1 timeout"); }
	
	// configure system clock prescalers (RM0399 Figure 55)
	MODIFY_REG(RCC->D1CFGR, RCC_D1CFGR_D1CPRE_Msk, 0b0000 << RCC_D1CFGR_D1CPRE_Pos);	// D1 domain core prescaler = divide by 1 (400MHz, D1CPRE)
	MODIFY_REG(RCC->D1CFGR, RCC_D1CFGR_HPRE_Msk, 0b1000 << RCC_D1CFGR_HPRE_Pos);		// D1 domain AHB  prescaler = divide by 2 (200MHz, HPRE)
	MODIFY_REG(RCC->D1CFGR, RCC_D1CFGR_D1PPRE_Msk, 0b100 << RCC_D1CFGR_D1PPRE_Pos);		// D1 domain APB3 prescaler = divide by 2 (100MHz, D1PPRE)
	MODIFY_REG(RCC->D2CFGR, RCC_D2CFGR_D2PPRE1_Msk, 0b100 << RCC_D2CFGR_D2PPRE1_Pos);	// D2 domain APB1 prescaler = divide by 2 (100MHz, D2PPRE1)
	MODIFY_REG(RCC->D2CFGR, RCC_D2CFGR_D2PPRE2_Msk, 0b100 << RCC_D2CFGR_D2PPRE2_Pos);	// D2 domain APB2 prescaler = divide by 2 (120MHz, D2PPRE2)
	MODIFY_REG(RCC->D3CFGR, RCC_D3CFGR_D3PPRE_Msk, 0b100 << RCC_D3CFGR_D3PPRE_Pos);		// D3 domain APB4 prescaler = divide by 2 (120MHz, D3PPRE)
	
	// set system clock mux input to use PLL1, DIVP1 as a source (400MHz) (RM0399 9.7.6)
	MODIFY_REG(RCC->CFGR, RCC_CFGR_SW_Msk, RCC_CFGR_SW_PLL1 << RCC_CFGR_SW_Pos);		// set system clock mux input to PLL1, DIVP1 
	while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != (RCC_CFGR_SW_PLL1 << RCC_CFGR_SWS_Pos) && (timeout > 0)) { timeout--; } 
	if (timeout == 0) { SYS_ERROR("system clock mux timeout"); }
	
//	// test code - route SYSCLK/8 out of MCO2 pin (RM0399 9.7.6) to verify the frequency (expected 50MHz, measured 50.05MHz)
//	static pinDef mco2Pin	= { .port = GPIOC, .pin = PIN_9,  .mode = Alternate, .type = PushPull, .speed = High, .pull = None, .alternate = AF0 };	
//	MODIFY_REG(RCC->CFGR, RCC_CFGR_MCO2_Msk, 0b000 << RCC_CFGR_MCO2_Pos);				// select SYSCLK as input 
//	MODIFY_REG(RCC->CFGR, RCC_CFGR_MCO2PRE_Msk, 8 << RCC_CFGR_MCO2PRE_Pos);				// divide input by 8 to get 50MHz on MCO2 pin
//	configurePin(mco2Pin);																// configure and enable pin as MCO2 output
}