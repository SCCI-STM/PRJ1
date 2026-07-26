#include "stm32f4xx.h"
#include "option.h"
#include "macro.h"
#include "malloc.h"

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
} Event;

extern MotorState prev_state;
extern MotorState curr_state;

extern volatile unsigned char is_key_pressed;
extern volatile unsigned char is_long_pressed;

extern Event event;

extern int lock;

// Clock.c
extern void Clock_Init(void);

// Key.c
extern void Key_Init(void);
extern int Key_Is_Pressed(void);
extern void Key_ISR_Enable(int en);

// Uart.c
extern void Uart2_Init(int baud);
extern void Uart2_Send_Byte(char data);
extern void Uart2_RX_Interrupt_Enable(int en);
extern char Uart2_Get_Pressed(void);

// Timer.c
extern void Timer2_Init(void);
extern void Timer2_Start(void);
extern void Timer2_Stop(void);
extern int Timer2_IsTimeout(void);
extern void TIM2_Interrupt_Enable(int en);

extern void Timer3_Init(void);
extern void Timer3_Delay(int time);

extern void Timer5_Init(void);
extern void Timer5_Out_Pwm_Generator(unsigned short freq);
extern void Timer5_Stop(void);

// Motor.c
extern void Motor_Init(void);
extern void Motor_ProcessKeyState(void);

extern void Motor_Main(void);
extern void Motor_Stop(void);
extern void Motor_Left(int duty);
extern void Motor_Right(int duty);
extern void Motor_RotateCW(void);
extern void Motor_RotateCCW(void);

// State.c
extern void State_Handler(void);

//exception.c
extern void _Invalid_ISR();
extern void TIM3_IRQHandler();
extern void EXTI15_10_IRQHandler();
extern void TIM2_IRQHandler();
extern void USART2_IRQHandler();

// ------- 사용 X -----------
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

// Key.c
extern void Key_Wait_Key_Released(void);
extern void Key_Wait_Key_Pressed(void);

// Uart.c
extern void Uart1_Init(int baud);
extern void Uart1_Send_Byte(char data);
extern void Uart1_Send_String(char *pt);
extern void Uart1_Printf(char *fmt,...);
extern char Uart1_Get_Char(void);

// timer.c
#if 0
extern void TIM4_Repeat(int time);
extern int TIM4_Check_Timeout(void);
extern void TIM4_Stop(void);
extern void TIM4_Change_Value(int time);


#endif