#include "../inc/gpio.h"

void gpio::configurePin(pinDef pin)
{
	// turn on port clock
	     if (pin.port == GPIOA) { RCC->AHB4ENR |= RCC_AHB4ENR_GPIOAEN; }
	else if (pin.port == GPIOB) { RCC->AHB4ENR |= RCC_AHB4ENR_GPIOBEN; }
	else if (pin.port == GPIOC) { RCC->AHB4ENR |= RCC_AHB4ENR_GPIOCEN; }
	else if (pin.port == GPIOD) { RCC->AHB4ENR |= RCC_AHB4ENR_GPIODEN; }
	else if (pin.port == GPIOE) { RCC->AHB4ENR |= RCC_AHB4ENR_GPIOEEN; }
	else if (pin.port == GPIOF) { RCC->AHB4ENR |= RCC_AHB4ENR_GPIOFEN; }
	else if (pin.port == GPIOG) { RCC->AHB4ENR |= RCC_AHB4ENR_GPIOGEN; }
	else if (pin.port == GPIOH) { RCC->AHB4ENR |= RCC_AHB4ENR_GPIOHEN; }
	else if (pin.port == GPIOI) { RCC->AHB4ENR |= RCC_AHB4ENR_GPIOIEN; }
	else if (pin.port == GPIOJ) { RCC->AHB4ENR |= RCC_AHB4ENR_GPIOJEN; }
	else if (pin.port == GPIOK) { RCC->AHB4ENR |= RCC_AHB4ENR_GPIOKEN; }
		
	// MODER	pin mode: input/output/alternate/analog	
	pin.port->MODER &= ~(3 << (pin.pin * 2)); 							
	pin.port->MODER |= (pin.mode << (pin.pin * 2));		
	
	// OTYPER	output type: push-pull/open drain						
	pin.port->OTYPER &= ~(1 << pin.pin);								
	pin.port->OTYPER |= (pin.type << pin.pin);			
	
	// OSPEEDR	output speed: low/med/high/very high
	pin.port->OSPEEDR &= ~(3 << (pin.pin * 2));
	pin.port->OSPEEDR |= (pin.speed << (pin.pin * 2));
	
	// PUPDR	pin resistors: none/pull-up/pull-down
	pin.port->PUPDR &= ~(3 << (pin.pin * 2));
	pin.port->PUPDR |= (pin.pull << (pin.pin * 2));
	
	// AFRL, AFRH	alternate function (AF0-AF15)
	if(pin.pin < 8) {
		pin.port->AFR[0] &= ~(0xF << (pin.pin * 4));
		pin.port->AFR[0] |= (pin.alternate << (pin.pin * 4));
	} else {
		pin.port->AFR[1] &= ~(0xF << ((pin.pin - 8) * 4));
		pin.port->AFR[1] |= (pin.alternate << ((pin.pin - 8) * 4));
	}
}


void gpio::digitalWrite(pinDef gpio, bool value)
{
	// BSRR: write 1 to bits 0:15 to set, write 1 to bits 16:31 to clear
	gpio.port->BSRR = (1 << (gpio.pin + (value ? 0 : 16)));
}


bool gpio::digitalRead(pinDef gpio)
{
	// IDR: bits 0:15 hold input values for port pins 0:15
	return gpio.port->IDR & (1 << gpio.pin);
}


void gpio::toggle(pinDef gpio)
{
	digitalWrite(gpio, !digitalRead(gpio));
}
