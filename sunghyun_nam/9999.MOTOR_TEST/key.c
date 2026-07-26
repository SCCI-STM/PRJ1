#include "device_driver.h"

void Key_Init(void)
{
	Macro_Set_Bit(RCC->AHB1ENR, 2); 
	Macro_Write_Block(GPIOC->MODER, 0x3, 0x0, 26);
}

int Key_IsPressed(void)
{
	// Key가 눌렸으면 1, 안 눌렸으면 0 리턴
	return Macro_Extract_Area(~GPIOC->IDR, 0x1, 13);
}

void Key_ISR_Enable(void)
{
	Macro_Set_Bit(RCC->AHB1ENR, 2); 
	Macro_Write_Block(GPIOC->MODER, 0x3, 0x0, 26);

	Macro_Set_Bit(RCC->APB2ENR, 14); 
	Macro_Write_Block(SYSCFG->EXTICR[3], 0xf, 0x2, 4);

	Macro_Set_Bit(EXTI->RTSR, 13);
	Macro_Set_Bit(EXTI->FTSR, 13);
	EXTI->PR = 0x1 << 13;
	
	NVIC_ClearPendingIRQ((IRQn_Type)40);
	Macro_Set_Bit(EXTI->IMR, 13);
	NVIC_EnableIRQ((IRQn_Type)40);
}