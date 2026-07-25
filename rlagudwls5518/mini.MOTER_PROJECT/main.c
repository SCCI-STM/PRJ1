#include "device_driver.h"
#include <stdio.h>

static void Sys_Init(int baud) 
{
	SCB->CPACR |= (0x3 << 10*2)|(0x3 << 11*2); 
	Clock_Init();
	Uart2_Init(baud);
	setvbuf(stdout, NULL, _IONBF, 0);
	LED_Init();
	Timer2_Init();
	Timer5_Init();
	Key_Init();
	Motor_Init();
}

eventState event = NONE;

volatile int is_long_pressed = 0;
volatile int is_key_pressed = 0;

//uart 입력 플래그 변수
volatile int Uart_Data_In = 0;

volatile unsigned char input_motor_dir = 0;
volatile int input_motor_speed = 0;

void state_handler();


void Main(void)
{
	Sys_Init(115200);
	printf("\nMOTER_PROJECT\n");

	prev_state = MOTOR_STOP;
	curr_state = MOTOR_STOP;

	TIM2_Interrupt_Enable(1);
	TIM3_Repeat_Interrupt_Enable(1);
	Key_ISR_Enable(1);
	Uart2_RX_Interrupt_Enable(1);

	for(;;){

		state_handler();
		Motor_Main();
	}
}

void state_handler(){
	
	if(event == NONE){
		return;
	}
	else if(event == KEY_PRESSED){
		event = NONE;
	}
	else if(event == TIMER2_OUT){
		printf("main tim2 out\n");
		Long_key_motor_state();
		event = NONE;
	}
	else if(event == KEY_RELEASED){
		printf("main key release\n");
		if(!is_long_pressed){
			Motor_ProcessKeyState();
			Timer5_Out_Pwm_Generator(MOTOR_SPEED_STEP(50));
		}
		event = NONE;
	}

	if(event == UART_INPUT){
		
		if(Uart_Data_In == 1){
			
			Motor_ProcessUartState(input_motor_dir);
		}
		else if(Uart_Data_In == 2){
			
			Timer5_Out_Pwm_Generator(MOTOR_SPEED_STEP(input_motor_speed));
		}
		Uart_Data_In = 0;
		event = NONE;
	}
}