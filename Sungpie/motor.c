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
		if(is_long_pressed == 1)
		{
			curr_state = MOTOR_STOP;
			prev_state = MOTOR_CW;	
		}
		else
		{
			curr_state = MOTOR_CCW;
			prev_state = MOTOR_CW;
		}
		
	}
	else if(curr_state == MOTOR_CCW)
	{
		if(is_long_pressed == 1)
		{
			curr_state = MOTOR_STOP;
			prev_state = MOTOR_CCW;
		}
		else
		{
			curr_state = MOTOR_CW;
			prev_state = MOTOR_CCW;
		}
	}
}

void Motor_Main(void)
{
	if((curr_state == MOTOR_CW) && (prev_state == MOTOR_STOP))
	{
		if(lock == 1)
		{
			lock = 0;
			Motor_Stop();
			Timer3_Delay(100);
			Motor_RotateCW();
		}	
	}
	else if((curr_state == MOTOR_CCW) && (prev_state == MOTOR_CW))
	{
		if(lock == 1)
		{
			lock = 0;
			Motor_Stop();
			Timer3_Delay(100);
			Motor_RotateCCW();
		}	
	}
	else if((curr_state == MOTOR_CW) && (prev_state == MOTOR_CCW))
	{
	 	if(lock == 1)
		{
			lock = 0;
			Motor_Stop();
			Timer3_Delay(100);
			Motor_RotateCW();
		}
	}
	else if(curr_state == MOTOR_STOP)
	{
		lock = 0;
		Motor_Stop();
		
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
	Macro_Set_Bit(GPIOA->ODR, 0);
	Macro_Clear_Bit(GPIOA->ODR, 1);
}
void Motor_RotateCCW(void)
{
	Macro_Clear_Bit(GPIOA->ODR, 0);
	Macro_Set_Bit(GPIOA->ODR, 1);
}
