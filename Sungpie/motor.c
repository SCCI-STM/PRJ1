#include "device_driver.h"

MotorState prev_state = MOTOR_STOP;
MotorState curr_state = MOTOR_STOP;

void Motor_Init(void)
{
	// Port-A Clock Enable
	Macro_Set_Bit(RCC->AHB1ENR, 0); 

	// motor를 출력으로 설정하고 초기 OFF
	Macro_Write_Block(GPIOA->MODER, 0xf, 0x5, 0);
	Macro_Clear_Area(GPIOA->OTYPER, 0x3, 0);
	Macro_Clear_Area(GPIOA->ODR, 0x3, 0); 
}

void Motor_ProcessKeyState(void)
{
	if(curr_state == MOTOR_STOP)
	{
		if(prev_state == MOTOR_STOP)
		{
			curr_state = MOTOR_CW;
			prev_state = MOTOR_STOP;
		}
		else if(prev_state == MOTOR_CW)
		{
			curr_state = MOTOR_CW;
			prev_state = MOTOR_STOP;
		}
		else if(prev_state == MOTOR_CCW)
		{
			curr_state = MOTOR_CCW;
			prev_state = MOTOR_STOP;
		}
	}
	else if(curr_state == MOTOR_CW)
	{
		curr_state = MOTOR_CCW;
		prev_state = MOTOR_CW;
	}
	else if(curr_state == MOTOR_CCW)
	{
		curr_state = MOTOR_CW;
		prev_state = MOTOR_CCW;
	}
}

void Motor_Main(void)
{
	if((curr_state == MOTOR_CW) && (prev_state == MOTOR_STOP) && lock)
	{
		Motor_RotateCW();
		lock = 0;
	}
	else if((curr_state == MOTOR_CW) && (prev_state == MOTOR_CCW) && lock)
	{
		Motor_RotateCCW();
		lock = 0;
	}
	else if((curr_state == MOTOR_CCW) && (prev_state == MOTOR_STOP) && lock)
	{
	 	Motor_RotateCCW();
		lock = 0;
	}

}
void Motor_Stop(void)
{
	Macro_Clear_Area(GPIOA->ODR, 0x3, 0);
}
void Motor_Left(int duty)
{
	Macro_Set_Bit(GPIOA->ODR, 0);
	Macro_Clear_Bit(GPIOA->ODR, 1);
}
void Motor_Right(int duty)
{
	Macro_Clear_Bit(GPIOA->ODR, 0);
	Macro_Set_Bit(GPIOA->ODR, 1);
	
}
void Motor_RotateCW(void)
{
	Motor_Stop();
	Timer2_Delay(100);
	Macro_Set_Bit(GPIOA->ODR, 0);
	Macro_Clear_Bit(GPIOA->ODR, 1);
}
void Motor_RotateCCW(void)
{
	Motor_Stop();
	// Timer2_Delay(100);
	// Macro_Clear_Bit(GPIOA->ODR, 0);
	// Macro_Set_Bit(GPIOA->ODR, 1);
}
