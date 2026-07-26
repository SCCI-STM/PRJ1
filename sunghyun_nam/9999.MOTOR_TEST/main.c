#include "device_driver.h"
#include <stdio.h>


static void Sys_Init(int baud) 
{
	SCB->CPACR |= (0x3 << 10*2)|(0x3 << 11*2); 
	Clock_Init();
	setvbuf(stdout, NULL, _IONBF, 0);
	Motor_Init();
	Uart2_Init(baud);
	Timer2_Init();
	Timer3_Init();
	Timer5_Init();
}

volatile Event event = EVENT_NONE;
volatile int is_key_pressed = 0;
volatile int is_long_pressed = 0;
volatile int Uart_Data_In = 0;
volatile unsigned char Uart_Data = 0;

void state_handler(void)
{
	if (event == EVENT_NONE)
	{
		return;
	}
	else if (event == EVENT_KEY_PRESSED)
	{
		event = EVENT_NONE;
	}
	else if (event == EVENT_TIMER_TIMEOUT)
    {
		Motor_ProcessStopState();
        event = EVENT_NONE;
    }
	else if (event == EVENT_KEY_RELEASED)
    {
        if (!is_long_pressed)
        {
           Motor_ProcessKeyState();
		   Timer5_Out_Pwm_Generator(MOTOR_SPEED_STEP(0));
        }

        is_long_pressed = 0;
        event = EVENT_NONE;
    }

	if (event == EVENT_UART_INPUT)
	{
		if ((Uart_Data >= 'A' && Uart_Data <= 'Z') ||
   	 		(Uart_Data >= 'a' && Uart_Data <= 'z'))
		{
			Motor_ProcessUartState(Uart_Data);
		}
		else if (Uart_Data >= '0' && Uart_Data <= '9')
		{
			Timer5_Out_Pwm_Generator(MOTOR_SPEED_STEP(Uart_Data - 0x30));
		}
		
		event = EVENT_NONE;
	}
}

void Main(void)
{
	Sys_Init(115200);
	printf("MOTOR Test\n");
	
	Key_ISR_Enable();
	Uart2_RX_Interrupt_Enable();

	for(;;)
	{	
		state_handler();
		Motor_Main();
	}
}