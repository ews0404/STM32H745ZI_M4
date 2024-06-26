#pragma once
#include <stdint.h>

namespace sys4
{
	void init(void);
	void update(void);
	
	uint32_t getMillis(void);
	uint32_t getMillisSince(uint32_t oldMillis);
}

// M4 parameters
#define M4_SYSCLOCK_HZ	200000000			// M4 core clock rate in Hz
#define M4_LED_MILLIS	500					// M4 led blink rate in milliseconds

// debug macros
#ifdef DEBUG
	#include <stdio.h>
	#define SYS_TRACE(...) do { printf(__VA_ARGS__); } while(0)
	#define SYS_LOG(...) do { printf("> %s\n", __VA_ARGS__); } while(0)
	#define SYS_WARN(...) do { printf("# %s\n", __VA_ARGS__); } while(0)
	#define SYS_ERROR(...) do { printf("! "); printf(__VA_ARGS__); printf(" (%s, line %d)\n", __FILE__, __LINE__); } while(0); do{}while(1) 
#elif 
	#define SYS_TRACE(...) 
	#define SYS_LOG(...) 
	#define SYS_WARN(...) 
	#define SYS_ERROR(...) 
#endif 

