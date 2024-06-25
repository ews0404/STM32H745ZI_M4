#include "sys/system.h"
#include "../Common/inc/foo.h"

int main(void)
{
	sys::init();
	
	while (1) {
		sys::update();
	}
}