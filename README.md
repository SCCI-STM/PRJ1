# 🚗 STM32 기반 양방향 DC 모터 제어 시스템

> **STM32F4 레지스터 직접 제어(Bare-metal) 기반의 DC 모터 정-역방향 및 PWM 속도 제어 미니 프로젝트**
> 
> 버튼(Short/Long Press) 및 USART2 시계열 통신을 통한 이중 제어를 지원하며, 모터 정-역방향 직전환 시 발생하는 탈조 및 역전류 방지를 위해 **타이머 인터럽트 기반 비동기 지연 기법**을 적용하였습니다.

---



## 📋 목차 (Table of Contents)
1. [주요 기능 (Key Features)](#-주요-기능-key-features)
2. [하드웨어 구성 및 핀 맵 (Hardware Setup)](#-하드웨어-구성-및-핀-맵-hardware-setup)
3. [소프트웨어 모듈 및 핵심 로직 (Software Detail)](#-소프트웨어-모듈-및-핵심-로직-software-detail)
4. [팀원별 구현 및 트러블슈팅 (Trouble shooting)](#-팀원별-구현-및-트러블슈팅-Trouble-shooting)


---

## ✨ 주요 기능 (Key Features)

- **비동기 이벤트 핸들링**: 메인 루프 지연(Non-blocking) 없는 State Handler 구조
- **이중 제어 인터페이스**:
  - **Button (PC13 EXTI)**:
    - **Short Press**: 모터 방향 토글 (CW $\leftrightarrow$ CCW)
    - **Long Press (3초 이상)**: 모터 즉시 정지 (STOP)
  - **USART2 통신**:
    - 방향 명령 (`F`/`f`: CW, `S`/`s`: STOP, `R`/`r`: CCW 수신)
    - 속도 명령 (`1` ~ `9` 수신 시 PWM Duty Rate 변경)
- **하드웨어 보호 기법**: 방향 전환 시 TIM3 타이머를 이용한 1초 비동기 정지 구역 처리 (탈조 및 역전류 방지)
- **PWM 속도 제어**: TIM5 CH1/CH2 레지스터 직접 제어를 통한 9단계 속도 조절 (Duty 50% ~ 100%)

---

## 🛠 하드웨어 구성 및 핀 맵 (Hardware Setup)

### 1. 사용 부품
- **MCU**: STM32F4 Board
- **Motor Driver**: L293 Driver IC
- **Actuator**: DC Motor

### 2. Pin Assignment

| Peripheral | Pin / Channel | 역할 (Function) |
| :--- | :--- | :--- |
| **GPIO Pin** | PA0 | Motor Driver Line 1 (1A) / TIM5 CH1 |
| **GPIO Pin** | PA1 | Motor Driver Line 2 (2A) / TIM5 CH2 |
| **EXTI** | PC13 | Push Button (Falling / Rising Edge 감지) |
| **USART2** | PA2 (TX), PA3 (RX) | Baudrate 115200 외부 시리얼 명령어 수신 |
| **TIM2** | Internal | 3초 Long Press 감지용 One-shot Timer |
| **TIM3** | Internal | 방향 전환 시 1초 비동기 딜레이 체크용 Repeat Timer |
| **TIM5** | Internal | PA0, PA1 PWM Signal Generator (AF02) |

---


## 💻 소프트웨어 모듈 및 핵심 로직 (Software Detail)

### 1. `exception.c` (인터럽트 서비스 루틴)
- **EXTI15_10_ISR**: PC13 비트를 확인하여 Key Push 시 `TIM2` 스타트, Key Release 시 `TIM2` 스탑 및 Short Press 판단
- **TIM2_ISR**: 3초 이상 누르고 있을 경우 `is_long_pressed = 1` 및 `TIMER2_OUT` 이벤트 발생
- **TIM3_ISR**: 1초 지연 인터럽트 발생 시 `TIM3_Expired = 1` 플래그 세팅
- **USART2_ISR**: 수신 데이터 유형 분류 (`1`: 방향 명령어, `2`: 속도 단계)


### 2. `motor.c` PWM제어, GPIO 레지스터 제어 모드 
모터 구동 상태에 따라 PA0, PA1의 `MODER`, `AFR`, `ODR` 레지스터를 동적으로 전환합니다.

| 모터 상태 (`curr_state`) | PA0/PA1 MODER | AFR[0] 매핑 | ODR 출력 | 동작 설명 |
| :--- | :--- | :--- | :--- | :--- |
| **`MOTOR_STOP`** | `0x5` (GP Output) | - | `ODR &= ~0x3` | PA0(0), PA1(0) 출력으로 완전 정지 |
| **`MOTOR_CW`** | `0x9` (PA0: AF, PA1: GP Output) | PA0 = `0x2` (TIM5) | PA1 ODR = 0 | PA0에만 TIM5 PWM 출력, PA1은 Low |
| **`MOTOR_CCW`** | `0x6` (PA0: GP Output, PA1: AF) | PA1 = `0x2` (TIM5) | PA0 ODR = 0 | PA1에만 TIM5 PWM 출력, PA0은 Low |

### 3. 상태 머신 기반 방향 토글 및 예외 처리 (`motor.c`, `exception.c`)
- **이전 상태 기억 기반 토글 (`Motor_ProcessKeyState`)**:
  - `prev_state`와 `curr_state`를 함께 관리하여 정지(`STOP`) 후 재가동 시 직전 회전 방향의 반대 방향으로 안전하게 토글 전환
- **중복 제어 방지 (`running_state`)**:
  - 현재 실제 구동 중인 상태(`running_state`)와 현재 요청 상태(`curr_state`)를 비교하여 동일한 상태일 경우 레지스터 재설정을 건너뛰어 시스템 효율 향상
- **Short/Long Press 예외 처리**:
  - 3초 이상 길게 눌러 정지된 경우, 버튼을 뗄 때(`KEY_RELEASED`) `if(!is_long_pressed)` 조건을 통해 Short Press 토글 동작이 오작동하지 않도록 방지

---

## 👥 팀원별 구현 및 트러블슈팅 (Trouble shooting)
각 팀원이 상위설계서를 바탕으로 독립적으로 구현한 코드와 트러블슈팅 기록입니다.

| 구성 | 담당 폴더 | 개인 README |
| :--- | :--- | :--- |
| 남성현(팀장) | `sunghyun_nam/` | [👉 바로가기](./sunghyun_nam/README.md) |
| 조성수 | `Sungpie/` | [👉 바로가기](./Sungpie/README.md) |
| 김형진 | `rlagudwls5518/` | [👉 바로가기](./rlagudwls5518/README.md) |

