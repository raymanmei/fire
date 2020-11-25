#include "main.h"
#include "com.h"


#define     T1_PORT                 (3)
#define     T1_PIN                  (3)

uint8_t u8RxData[2]={0x55,0x00};
uint8_t u8RxFlg=0;
uint8_t CheckFlg=0;

void RxIntCallback(void)
{
    u8RxData[1]=M0P_LPUART->SBUF;
	u8RxFlg = 1;
}

void ErrIntCallback(void)
{
    
}

int32_t COM_Init(void)
{    
    uint16_t u16timer;
    uint32_t u32sclk;

    stc_lpuart_config_t stcConfig;
    stc_lpuart_irq_cb_t stcLPUartIrqCb;
    stc_lpuart_multimode_t stcMulti;
    stc_lpuart_sclk_sel_t stcLpuart_clk;
    stc_lpuart_mode_t stcRunMode;
    stc_lpuart_baud_config_t stcBaud;
    stc_bt_config_t stcBtConfig;
    
    DDL_ZERO_STRUCT(stcConfig);
    DDL_ZERO_STRUCT(stcLPUartIrqCb);
    DDL_ZERO_STRUCT(stcMulti);
    DDL_ZERO_STRUCT(stcBtConfig);
    
    Clk_SetPeripheralGate(ClkPeripheralLpUart, TRUE);//使能LPUART时钟
    Clk_SetPeripheralGate(ClkPeripheralBt, TRUE);
    
    //通道端口配置
    Gpio_InitIOExt(2, 5, GpioDirOut, TRUE,FALSE,FALSE,FALSE);
    Gpio_InitIOExt(2, 6, GpioDirOut,TRUE,FALSE,FALSE,FALSE);

    Gpio_SetFunc_UART2RX_P25();
    Gpio_SetFunc_UART2TX_P26();

    Gpio_InitIO(T1_PORT,T1_PIN,GpioDirIn);
    
   
    stcLpuart_clk.enSclk_sel = LPUart_Pclk;//LPUart_Rcl;
    
    stcLpuart_clk.enSclk_Prs = LPUartDiv1;
    stcConfig.pstcLpuart_clk = &stcLpuart_clk;

    stcRunMode.enLpMode = LPUartNoLPMode;//正常工作模式或低功耗工作模式配置
    stcRunMode.enMode   = LPUartMode3;
    stcConfig.pstcRunMode = &stcRunMode;

    stcLPUartIrqCb.pfnRxIrqCb = RxIntCallback;
    stcLPUartIrqCb.pfnTxIrqCb = NULL;
    stcLPUartIrqCb.pfnRxErrIrqCb = ErrIntCallback;
    stcConfig.pstcIrqCb = &stcLPUartIrqCb;
    stcConfig.bTouchNvic = TRUE;

    stcMulti.enMulti_mode = LPUartNormal;//只有模式2/3才有多主机模式

    stcConfig.pstcMultiMode = &stcMulti;
   
    LPUart_EnableIrq(LPUartRxIrq);

    LPUart_Init(&stcConfig);

    if(LPUart_Pclk == stcLpuart_clk.enSclk_sel)
        u32sclk = Clk_GetPClkFreq();
    else if(LPUart_Rcl == stcLpuart_clk.enSclk_sel)
        u32sclk = 38400;//此处建议用户使用内部38.4K时钟，如果用户使用32.768K时钟的，此处更新成32768
    else
        u32sclk = 32768;
      
    stcBaud.u32Baud = 2400;
    stcBaud.bDbaud = 0;
    stcBaud.u8LpMode = LPUartNoLPMode;
    stcBaud.u8Mode = LPUartMode3;
    u16timer = LPUart_SetBaudRate(u32sclk,stcLpuart_clk.enSclk_Prs,&stcBaud);
    stcBtConfig.enMD = BtMode2;
    stcBtConfig.enCT = BtTimer;
    stcBtConfig.enTog = BtTogEnable;
    Bt_Init(TIM2, &stcBtConfig);//调用basetimer2设置函数产生波特率
    Bt_ARRSet(TIM2,u16timer);
    Bt_Cnt16Set(TIM2,u16timer);
    Bt_Run(TIM2);

    LPUart_EnableFunc(LPUartRx);    
    
    while (1)
    {
		CheckFlg = 0;
		if(u8RxFlg)
		{
			u8RxFlg = 0;
			if(LPUart_CheckEvenOrOdd(Even,u8RxData[1])!=Ok)
			{
				CheckFlg = 1;//奇偶校验出错
			}
			else
			{
				LPUart_SetTb8(Even,u8RxData[0]);
				LPUart_SendData(u8RxData[0]);				
				LPUart_SetTb8(Even,u8RxData[1]);
				LPUart_SendData(u8RxData[1]);
			}
		}
	}
}
