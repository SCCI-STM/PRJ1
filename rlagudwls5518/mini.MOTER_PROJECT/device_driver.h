#include "stm32f4xx.h"
#include "option.h"
#include "macro.h"
#include "malloc.h"
#include <stdio.h>
#define PWM_INPUT_FREQ       10000
#define MOTOR_SPEED_INIT     50
#define MOTOR_SPEED_UP       5
#define MOTOR_SPEED_STEP(x)  ((MOTOR_SPEED_UP) + ((MOTOR_SPEED_INIT) * (x)))

typedef enum
{
    MOTOR_STOP,
    MOTOR_CW,
    MOTOR_CCW
} MotorState;

typedef enum
{
    NONE,
    KEY_PRESSED,
    KEY_RELEASED,
    UART_INPUT,
    TIMER2_OUT
} eventState;

extern MotorState prev_state;
extern MotorState curr_state;
extern eventState event;


// Uart.c

extern void Uart2_Init(int baud);
extern void Uart2_Send_Byte(char data);
extern void Uart2_RX_Interrupt_Enable(int en);
extern void Uart1_Init(int baud);
extern void Uart1_Send_Byte(char data);
extern void Uart1_Send_String(char *pt);
extern void Uart1_Printf(char *fmt,...);
extern char Uart1_Get_Char(void);
extern char Uart1_Get_Pressed(void);

// SysTick.c

extern void SysTick_Run(unsigned int msec);
extern int SysTick_Check_Timeout(void);
extern unsigned int SysTick_Get_Time(void);
extern unsigned int SysTick_Get_Load_Time(void);
extern void SysTick_Stop(void);

// Led.c

extern void LED_Init(void);
extern void LED_On(void);
extern void LED_Off(void);

// Clock.c

extern void Clock_Init(void);

// Key.c

extern void Key_Init();
extern void Key_ISR_Enable(int en);

// Timer.c
extern void Timer2_Init(void);
extern void Timer2_Start();
extern void Timer2_Stop(void);
extern void TIM2_Interrupt_Enable(int en);
extern void TIM3_Repeat_Interrupt_Enable(int en);
extern void Timer5_Init(void);
extern void Timer5_Out_Pwm_Generator(int duty);

//motor.c
extern void Motor_Init(void);
extern void Motor_Main();
extern void Long_key_motor_state();
extern void Motor_ProcessKeyState();
extern void Motor_ProcessUartState(char data);
extern void Motor_Stop();
extern void Motor_RotateCW(void);
extern void Motor_RotateCCW(void);


//exception.c
extern void _Invalid_ISR();
extern void TIM3_IRQHandler();
extern void EXTI15_10_IRQHandler();
extern void TIM2_IRQHandler();
extern void USART2_IRQHandler();