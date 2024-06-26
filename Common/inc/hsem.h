#pragma once
#include <stdint.h>

/* This file contains hardware semaphore functions (RM0399 Ch 11) that allow the M4 and M7 cores to safely access
 * shared resources.
 * 
 * Reference code: https://github.com/STMicroelectronics/stm32h7xx_hal_driver/blob/master/Src/stm32h7xx_hal_hsem.c
 * 
 * Note: The hardware semaphores can change state just by reading the location of their memory. Displaying their value
 * via hardware debugger counts as a read and will change their state!
 */

namespace hsem
{
	// define the hardware semaphore processor core IDs (RM0399 11.3.8)
	enum Core_ID : uint8_t
	{
		m4_coreID = 1,
		m7_coreID = 3
	};
	
	// the hardware semaphores are numbered 0-31, assign them to controlled resources
	enum HSEM_ID : uint8_t
	{
		hsem_M7msgQueue = 0,
		hsem_M4msgQueue = 1
	};
	
	void init(void);								// turn on HSEM clock, clear all semaphores
	bool take(HSEM_ID hsemID, Core_ID coreID);		// 1-step semaphore take, returns true if successful
	void release(HSEM_ID hsemID, Core_ID coreID);	// release the semaphore
	bool isLocked(HSEM_ID hsemID);					// returns lock status regardless of core 
}
