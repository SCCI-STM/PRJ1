#include "device_driver.h"
#include <stdio.h>

volatile unsigned char is_key_pressed = 0;
volatile unsigned char is_long_pressed = 0;

volatile int Uart_Data_In = 0;
volatile unsigned char Uart_Data = 0;
volatile int TIM4_Expired = 0;

Event event = NONE;

int lock = 0;

void State_Handler(void);

static void Sys_Init(int baud) 
{
	SCB->CPACR |= (0x3 << 10*2)|(0x3 << 11*2); 
	Clock_Init();
	Uart2_Init(baud);
	setvbuf(stdout, NULL, _IONBF, 0);
	Motor_Init();
	Timer2_Init();
}

void Main(void)
{
	Sys_Init(115200);
	printf("\nTimer 2 Interrupt Test\n");

	Key_ISR_Enable(1);
	Uart2_RX_Interrupt_Enable(1);
	TIM2_Interrupt_Enable(1);

	for(;;)
	{
		State_Handler();
		Motor_Main();
	}
}

void State_Handler(void)
{
	if(event == NONE)
    {
        return;
    }
    else if(event == KEY_PRESSED)
    {
		printf("[ Key Pressed ] !!!\n");
        event = NONE;
    }
    else if(event == TIMER2_OUT)
    {
        printf("[ 3sec ] !!!\n");
        
        event = NONE;
    }
    else if(event == KEY_RELEASED)
    {
        if(!is_long_pressed)
        {
			printf("[ Key Released ] !!!\n");
            Motor_ProcessKeyState();
			is_long_pressed = 0;
			lock = 1;
        }
		event = NONE;
    }
}