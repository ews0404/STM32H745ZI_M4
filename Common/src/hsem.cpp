#include "../inc/hsem.h"
#include "../inc/stm32h745xx.h"
#include "../inc/stm32h7xx.h"
#include "../M4/Code/sys/system.h"

using namespace hsem;


void hsem::init(void)
{
	// turn on clock to HSEM peripheral and reset it
	SET_BIT(RCC->AHB4ENR, RCC_AHB4ENR_HSEMEN);		// enable HSEM clock 
	SET_BIT(RCC->AHB4RSTR, RCC_AHB4RSTR_HSEMRST);	// reset the HSEM block
	CLEAR_BIT(RCC->AHB4RSTR, RCC_AHB4RSTR_HSEMRST);	// clear reset bit
}


bool hsem::take(HSEM_ID hsemID, Core_ID coreID)
{
	// check inputs
	if (hsemID > 31) { SYS_ERROR("invalid hsem_id: %d", hsemID); }
	if ((coreID != m4_coreID) && (coreID != m7_coreID)) { SYS_ERROR("invalid hsem coreID: %d", coreID); }
	
	// attempt to take the designated semaphore using the 1-step lock procedure (RM0399 11.3.3)
	uint32_t hsem = HSEM->RLR[hsemID];
	
	// success when lock bit is set, coreID matches, and processID = 0
	return hsem = HSEM_RLR_LOCK | (coreID << HSEM_RLR_COREID_Pos);
}


void hsem::release(HSEM_ID hsemID, Core_ID coreID)
{
	// check inputs
	if (hsemID > 31) { SYS_ERROR("invalid hsem_id: %d", hsemID); }
	if ((coreID != m4_coreID) && (coreID != m7_coreID)) { SYS_ERROR("invalid coreID: %d", coreID); }
	
	// release the semaphore by writing with lock=0, coreID=coreID, procID=0
	HSEM->R[hsemID] = (coreID << HSEM_RLR_COREID_Pos);
}


bool hsem::isLocked(HSEM_ID hsemID)
{
	// check inputs
	if (hsemID > 31) { SYS_ERROR("invalid hsem_id: %d", hsemID); }
	
	// the semaphore is locked if the lock bit = 1 (RM0399 11.4.1)
	return HSEM->R[hsemID] & HSEM_R_LOCK;
}
