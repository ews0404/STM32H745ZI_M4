#include "sys/system.h"

int main(void)
{
	sys::init();
	
	while (1) {
		sys::update();
	}
}