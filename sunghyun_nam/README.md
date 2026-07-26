## 상위 설계서와 다른 구현 사항

### 1. 모터 정지 출력

- 상위 설계: PA0과 PA1을 모두 Low로 출력
- 현재 구현: PA0과 PA1을 모두 High로 출력

두 방식 모두 L293 입력을 같은 레벨로 만들어 모터를 정지시킨다. 현재
구현은 실제 회로에서 확인한 `11` 정지 방식을 사용한다.

### 2. CW/CCW의 MODER 값

| 상태 | 상위 설계 | 현재 구현 |
| :--- | :---: | :---: |
| `MOTOR_CW` | `0x9` | `0x6` |
| `MOTOR_CCW` | `0x6` | `0x9` |

차이는 실제 배선 방향을 반영한 결과이다.

### 3. UART 속도 입력 범위

- 상위 설계: 문자 `1`~`9`
- 현재 구현: 문자 `0`~`9`

현재 속도 정책은 50%부터 95%까지 총 10단계이므로 `0`~`9` 입력을
사용한다.

### 4. USART2 수신 데이터 분류 위치

- 상위 설계: `USART2_IRQHandler()`에서 방향 명령과 속도 명령을 분류
- 현재 구현: ISR에서는 수신 데이터 저장과 이벤트 발생만 수행하고,
  `main.c`의 `state_handler()`에서 명령을 분류


### 5. TIM3 동작 모드

- 상위 설계: Repeat Timer
- 현재 구현: One-pulse mode

```c
TIM3->CR1 = (1 << 4) | (1 << 3);
```


### 6. 1초 안전 지연의 적용 범위

상위 설계의 핵심 요구사항은 CW와 CCW 사이의 방향 전환 전에 1초간
정지하는 것이다. 현재 구현은 `curr_state != applied_state`이면 항상
안전 지연을 적용하므로 다음 상황에도 1초 지연이 발생한다.

- 부팅 후 최초 구동
- 정지 후 재구동
- CW와 CCW 사이의 방향 전환

즉, 현재 구현은 방향 전환뿐 아니라 모든 정지 상태에서 구동 상태로
진입할 때 안전 지연을 적용한다. 

### 7. Short Press 후 PWM Duty

- 상위 설계: Short Press 시 방향을 토글하고 Duty를 초기값으로 설정
- 현재 구현: 방향만 토글하고 기존 TIM5 CCR 값을 유지

현재 `EVENT_KEY_RELEASED` 처리에서는 다음 함수만 호출한다.

```c
Motor_ProcessKeyState();
```

따라서 UART로 설정한 속도는 키로 방향을 바꿔도 유지된다. Short Press 때
항상 50%로 초기화하려면 아래 호출을 추가해야 한다.

```c
Timer5_Out_Pwm_Generator(MOTOR_SPEED_STEP(0));
```

### 8. TIM5 초기 Duty 및 시작 시점

`Timer5_Init()`은 PSC, ARR, CCMR1, CCER를 설정하지만 `CCR1`, `CCR2`에
초기 Duty를 넣거나 `CEN`을 활성화하지 않는다. TIM5의 실제 PWM 시작은
`Timer5_Out_Pwm_Generator()`가 처음 호출될 때 이루어진다.

```text
Timer5_Init()
    → TIM5 기본 설정만 수행

UART 숫자 입력
    → CCR1/CCR2 설정
    → EGR.UG 발생
    → TIM5.CEN 활성화
```

따라서 방향 명령이나 키 입력 전에 속도 명령을 한 번도 받지 않았다면,
초기 PWM 상태가 입력 순서에 따라 달라질 수 있다. 입력 순서와 관계없이
항상 같은 초기 속도로 시작하려면 시스템 초기화 후 다음 함수를 호출하는
방식을 사용할 수 있다.

```c
Timer5_Out_Pwm_Generator(MOTOR_SPEED_INIT);
```

## 트러블슈팅

### 1. 버튼을 누르자마자 Long Press로 처리되는 문제

#### 증상

버튼을 누르면 `KEY_PRESSED`가 먼저 처리되지 않고 TIM2 인터럽트가 즉시
발생해 Timeout 상태로 처리되었다.

#### 원인

TIM2의 PSC와 ARR 값을 반영하기 위해 `EGR.UG`로 강제 Update Event를
발생시켰다. 이 과정에서 `SR.UIF`가 설정될 수 있으며, Update Interrupt가
활성화된 상태라면 곧바로 TIM2 인터럽트 요청이 발생할 수 있다.

#### 해결

타이머 시작 전에 다음 순서로 기존 상태를 정리했다.

```c
Macro_Set_Bit(TIM2->EGR, 0);        // UG 발생
Macro_Clear_Bit(TIM2->SR, 0);       // UIF Clear
NVIC_ClearPendingIRQ(TIM2_IRQn);    // NVIC Pending Clear
Macro_Set_Bit(TIM2->DIER, 0);       // Update Interrupt Enable
Macro_Set_Bit(TIM2->CR1, 0);        // Timer Start
```

`Timer2_Stop()`에서는 `DIER.UIE`도 비활성화하여 정지 상태에서 불필요한
Update Interrupt 요청이 발생하지 않도록 했다.

### 2. 속도 단계가 정상적으로 증가하지 않는 문제

#### 증상

- 속도 0단계에서 모터가 동작하지 않음
- 0단계에서 1단계로 변경할 때 속도가 급격히 증가함
- 1단계 이후에는 단계별 속도 차이를 체감하기 어려움

#### 원인

속도 단계 계산식의 초기 속도와 증가 폭 위치가 서로 뒤바뀌어 있었다.

수정 전:

```c
#define MOTOR_SPEED_STEP(x) \
    ((MOTOR_SPEED_UP) + ((MOTOR_SPEED_INIT) * (x)))
```

이 식은 `x=0`일 때 5%, `x=1`일 때 55%, `x=2`일 때 105%가 되어
정상적인 Duty 단계가 만들어지지 않는다.

#### 해결

다음과 같이 초기 속도에 단계별 증가량을 더하도록 수정했다.

```c
#define MOTOR_SPEED_STEP(x) \
    ((MOTOR_SPEED_INIT) + ((MOTOR_SPEED_UP) * (x)))
```

수정 후 입력 `0`~`9`가 Duty 50%~95%에 대응한다.

## 회고

- 설계 단계에서 함수와 변수의 세부 구현까지 모두 확정하려다 보니 예상보다
  많은 시간이 소요되었다.
- 큰 흐름과 모듈의 책임, 입출력 조건을 먼저 정의하고 세부 구현은 개발
  과정에서 결정하는 방식이 더 효율적이라는 점을 배웠다.
- 기획 단계에서는 문서화 과정의 필요성이 잘 느껴지지 않았지만, 실제 구현
  과정에서 설계 내용을 기준으로 작업하니 설계서 없이 개발할 때보다 구현
  속도가 빨라지는 것을 체감했다.
- 설계와 실제 배선 또는 구현이 달라질 경우, 차이를 단순히 남겨두는 것이
  아니라 변경 이유와 최종 기준을 문서에 함께 기록해야 협업 과정의 혼란을
  줄일 수 있다는 점을 배웠다.
