#include "sys/system.h"

int __attribute__((naked, noreturn)) main(void)
{
	sys4::init();
	
	while (1) {
		sys4::update();
	}
}