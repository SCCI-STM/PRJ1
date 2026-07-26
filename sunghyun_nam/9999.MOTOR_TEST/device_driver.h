#include "stm32f4xx.h"
#include "option.h"
#include "macro.h"
#include "malloc.h"

#define PWM_INPUT_FREQ       10000
#define MOTOR_SPEED_INIT     50
#define MOTOR_SPEED_UP       5
#define MOTOR_SPEED_STEP(x)  ((MOTOR_SPEED_INIT) + ((MOTOR_SPEED_UP) * (x)))

typedef enum 
{
    MOTOR_STOP,
    MOTOR_CW,
    MOTOR_CCW
} MotorState;

extern MotorState prev_state;
extern MotorState curr_state;

typedef enum
{
    EVENT_NONE,
    EVENT_KEY_PRESSED,
    EVENT_KEY_RELEASED,
    EVENT_TIMER_TIMEOUT,
    EVENT_UART_INPUT
} Event;

extern volatile Event event;

// Uart.c

extern void Uart2_Init(int baud);
extern void Uart2_Send_Byte(char data);
extern char Uart2_Get_Pressed(void);
extern void Uart2_RX_Interrupt_Enable(void);

// Motor.c

extern void Motor_Init(void);
extern void Motor_Main(void);
extern void Motor_Stop(void);
extern void Motor_RotateCW(void);
extern void Motor_RotateCCW(void);
extern void Motor_ProcessStopState(void);
extern void Motor_ProcessKeyState(void);
extern void Motor_ProcessUartState(unsigned char input_data);

// Clock.c

extern void Clock_Init(void);

// Key.c

extern void Key_Init(void);
extern int Key_IsPressed(void);
extern void Key_ISR_Enable(void);

// Timer.c

extern void Timer2_Init(void);
extern void Timer2_Start(int time);
extern void Timer2_Stop(void);

extern void Timer3_Init(void);
extern void Timer3_Start(int time);
extern void Timer3_Stop(void);

extern void Timer5_Init(void);
extern void Timer5_Out_Pwm_Generator(int duty);