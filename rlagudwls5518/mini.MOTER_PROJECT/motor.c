#include "device_driver.h"


MotorState prev_state = MOTOR_STOP;
MotorState curr_state = MOTOR_STOP;
static MotorState running_state = MOTOR_STOP;
volatile int TIM3_Expired = 0;

void Motor_Init(void){
	GPIOA->MODER = ((GPIOA->MODER & ~0xF) | 0x05);
	GPIOA->OTYPER &= ~0x3;
}

void Motor_Main(){

    if(running_state == curr_state){
        return;
    }
    
    if(curr_state == MOTOR_STOP){
        Motor_Stop();
        running_state = MOTOR_STOP;
        
    }
    else if(curr_state== MOTOR_CW){
        
        if(TIM3_Expired){
            Motor_RotateCW();
            TIM3_Expired = 0;
            running_state = MOTOR_CW;
        }
        
    }
    else{
        if(TIM3_Expired){
            Motor_RotateCCW();
            TIM3_Expired = 0;
            running_state = MOTOR_CCW;
        }
    }
    
}

void Long_key_motor_state(){
    prev_state = curr_state;
    curr_state = MOTOR_STOP;
}

void Motor_ProcessKeyState(){
    if(curr_state == MOTOR_STOP){

        if(prev_state == MOTOR_STOP){
            curr_state = MOTOR_CW;
            prev_state = MOTOR_CW;
        }
        else if(prev_state == MOTOR_CW){
            curr_state = MOTOR_CW;
            prev_state = MOTOR_STOP;
        }
        else if(prev_state == MOTOR_CCW){
            curr_state = MOTOR_CCW;
            prev_state = MOTOR_STOP;
        }

    }

    else if(curr_state == MOTOR_CW){
        
        prev_state = MOTOR_CW;
        curr_state = MOTOR_CCW;
    }
    else if(curr_state == MOTOR_CCW){
    
        prev_state = MOTOR_CCW;
        curr_state = MOTOR_CW;
    }
}

void Motor_ProcessUartState(char data){
    prev_state = curr_state;
    if(data =='F' || data =='f'){
        curr_state = MOTOR_CW;
    }
    else if(data =='S' || data =='s'){
        curr_state = MOTOR_STOP;
    }
    else if(data =='R' || data =='r'){
        curr_state = MOTOR_CCW;
    }
}


void Motor_Stop(){
    Macro_Write_Block(GPIOA->MODER, 0xf, 0x5, 0);
    GPIOA->ODR &= ~0x3; 
}

void Motor_RotateCW(){
    Macro_Write_Block(GPIOA->MODER, 0xf, 0x9, 0);
    Macro_Write_Block(GPIOA->AFR[0], 0xf, 0x2, 4);
    Macro_Clear_Bit(GPIOA->ODR, 0);
    
}

void Motor_RotateCCW(){
    Macro_Write_Block(GPIOA->MODER, 0xf, 0x6, 0);
    Macro_Write_Block(GPIOA->AFR[0], 0xf, 0x2, 0);
    Macro_Clear_Bit(GPIOA->ODR, 1);
    
}



	