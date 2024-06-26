#include "sys/system.h"

int main(void)
{
	sys4::init();
	
	while (1) {
		sys4::update();
	}
}