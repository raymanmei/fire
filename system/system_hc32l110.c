#include "base_types.h"
#include "hc32l110.h"
#include "system_hc32l110.h"
#include "ddl.h"

uint32_t SystemCoreClock = 4000000;


void SystemCoreClockUpdate(void)
{
    SystemCoreClock = Clk_GetHClkFreq();
}

void SystemInit(void)
{
    stc_clk_systickcfg_t stcCfg;

    Clk_SetRCHFreq(ClkFreq4Mhz);
    Clk_Enable(ClkRCH, TRUE);    
                                                
    SystemCoreClockUpdate();
	  
    DDL_ZERO_STRUCT(stcCfg);
    stcCfg.bNoRef = TRUE;
    stcCfg.u32LoadVal = 0xFFFFFF;
    Clk_SysTickConfig(&stcCfg);
}
