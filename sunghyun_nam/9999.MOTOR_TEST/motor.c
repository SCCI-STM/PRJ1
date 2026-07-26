#include "device_driver.h"
#include <stdio.h>

MotorState prev_state = MOTOR_STOP;
MotorState curr_state = MOTOR_STOP;
static MotorState applied_state = MOTOR_STOP;

volatile int TIM3_Expired = 0;
static int motor_wait = 0;

void Motor_Init(void)
{
	Macro_Set_Bit(RCC->AHB1ENR, 0); 
	Macro_Write_Block(GPIOA->MODER, 0xf, 0x5, 0);
	Macro_Clear_Area(GPIOA->OTYPER, 0x3, 0);
	Macro_Clear_Area(GPIOA->ODR, 0x3, 0); 
}

void Motor_Main(void)
{
	if (curr_state == MOTOR_STOP)
	{
		if (applied_state != MOTOR_STOP || motor_wait)
		{
			Timer3_Stop();
			Motor_Stop();

			motor_wait = 0;
			applied_state = MOTOR_STOP;
		}
		return;
	}

	if ((curr_state != applied_state) && !motor_wait)
    {
        Motor_Stop();
		TIM3_Expired = 0;
        motor_wait = 1;
        Timer3_Start(1000);

        return;
    }

	if (motor_wait && TIM3_Expired)
    {
        TIM3_Expired = 0;
        motor_wait = 0;
        if (curr_state == MOTOR_CW)
        {
			Motor_RotateCW();
        }
        else if (curr_state == MOTOR_CCW)
        {
			Motor_RotateCCW();
        }

        applied_state = curr_state;
    }
}

void Motor_Stop(void)
{
	Macro_Write_Block(GPIOA->MODER, 0xf, 0x5, 0);
	Macro_Write_Block(GPIOA->ODR, 0x3, 0x3, 0);
}

void Motor_RotateCW(void)
{
	Macro_Clear_Bit(GPIOA->ODR, 1);
	Macro_Write_Block(GPIOA->MODER, 0xf, 0x6, 0);
	Macro_Write_Block(GPIOA->AFR[0], 0xf, 0x2, 0);
}

void Motor_RotateCCW(void)
{
	Macro_Clear_Bit(GPIOA->ODR, 0);
	Macro_Write_Block(GPIOA->MODER, 0xf, 0x9, 0);
	Macro_Write_Block(GPIOA->AFR[0], 0xf, 0x2, 4);
}

void Motor_ProcessStopState(void)
{
    if (curr_state != MOTOR_STOP)
    {
        prev_state = curr_state;
    }

    curr_state = MOTOR_STOP;
}

void Motor_ProcessKeyState(void)
{
	MotorState tmp_state;

	if (curr_state == MOTOR_STOP)
	{
		if (prev_state == MOTOR_STOP)
		{
			curr_state = MOTOR_CW;
			prev_state = MOTOR_CW;
		
		}
		else 
		{
			tmp_state = prev_state;
			prev_state = curr_state;
			curr_state = tmp_state;
		}	
	}
	else if (curr_state == MOTOR_CW)
    {
        prev_state = MOTOR_CW;
        curr_state = MOTOR_CCW;
    }
    else if (curr_state == MOTOR_CCW)
    {
        prev_state = MOTOR_CCW;
        curr_state = MOTOR_CW;
    }
}

void Motor_ProcessUartState(unsigned char input_data)
{
	prev_state = curr_state;

	if (input_data == 'F' || input_data == 'f')
	{	
		curr_state = MOTOR_CW;
	}
	else if (input_data == 'S' || input_data == 's')
    {
        curr_state = MOTOR_STOP;
    }
    else if (input_data == 'R' || input_data == 'r')
    {
        curr_state = MOTOR_CCW;
    }
}