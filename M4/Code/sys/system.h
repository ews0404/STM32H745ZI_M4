#pragma once

namespace sys
{
	void init(void);
	void update(void);
}


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

