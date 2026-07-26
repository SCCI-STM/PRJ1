## 🐛 Trouble Shooting: 타이머 시작 시 의도치 않은 인터럽트 발생 문제

### 📌 문제 증상
타이머를 시작할 때(`Timer2_Start`), 타이머 카운팅이 정식으로 시작되기도 전에 **시작과 동시에 인터럽트가 즉시 발생하는 현상**이 발생.

---

### 🔍 원인 분석

MCU 타이머에서 `EGR(Event Generation Register)`의 0번 비트(UG, Update Generation)를 `1`로 세트하면 레지스터 적재가 일어나면서 **`SR(Status Register)`의 0번 비트(UIF, Update Interrupt Flag)가 자동으로 `1`로 세트**.

* **기존 코드의 문제 발생 순서:**
  1. `SR` 비트를 먼저 0으로 지움 (`Macro_Clear_Bit(TIM2->SR, 0)`)
  2. `EGR`을 세트함 → **이 순간 `SR`의 0번 비트가 다시 `1`로 켜짐.**
  3. `DIER`을 설정해 인터럽트를 허용함 → `SR`의 플래그가 `1`인 상태이므로 **즉시 인터럽트 실행**.

> 💡 **비유로 이해하기**  
> `EGR`을 건드리는 것은 **"새 설정을 적용하면서 공사 먼지를 만드는 작업"**과 같음.  
> 먼지(`SR` 플래그)를 먼저 닦아낸 다음 공사(`EGR` 세트)를 시작하면 다시 먼지가 쌓임. 따라서 **공사(`EGR`)를 먼저 마친 후 먼지(`SR`)를 닦아내고, 마지막에 앰프(`DIER`)를 켜야** 소음 없이 깨끗하게 작동.

---

### 💻 코드 비교

#### 문제 코드 (Before)
`EGR`을 세트하기 전에 `SR`을 먼저 지우는 바람에, `EGR` 실행 시 발생한 플래그가 그대로 남아 인터럽트가 즉시 터지는 구조.

```c
void Timer2_Start(void)
{
    // ❌ EGR 세트 전에 SR을 지움 (뒤에서 다시 1로 켜지므로 의미 없음)
    Macro_Clear_Bit(TIM2->SR, 0);

    // EGR 세트 시 Update Event 발생 -> SR의 UIF 비트가 다시 1로 세트됨!
    Macro_Set_Bit(TIM2->EGR, 0);

    NVIC_ClearPendingIRQ(28);

    // SR에 플래그가 남아있어 인터럽트 허용 직후 즉시 인터럽트가 터짐
    Macro_Set_Bit(TIM2->DIER, 0);
    Macro_Set_Bit(TIM2->CR1, 0);
}

void Timer2_Stop(void)
{
    Macro_Clear_Bit(TIM2->CR1, 0);
    Macro_Clear_Bit(TIM2->DIER, 0);
}
```

####  해결 코드
`EGR`을 세트하고 난 다음, `SR`을 지움 `EGR` 실행 시 발생한 플래그가 지워져 인터럽트가 즉시 터지지 않게 됨.
```c
void Timer2_Start(void)
{
    // 1. EGR 세트 (레지스터 적재 & Update Event 발생)
    Macro_Set_Bit(TIM2->EGR, 0);

    // 2. EGR 실행 여파로 켜진 SR의 UIF 플래그를 지움
    Macro_Clear_Bit(TIM2->SR, 0);

    // 3. NVIC의 대기 중인 인터럽트 클리어
    NVIC_ClearPendingIRQ(28);

    // 4. 안전해진 상태에서 인터럽트 허용 및 타이머 구동
    Macro_Set_Bit(TIM2->DIER, 0);
    Macro_Set_Bit(TIM2->CR1, 0);
}

void Timer2_Stop(void)
{
    Macro_Clear_Bit(TIM2->CR1, 0);
    Macro_Clear_Bit(TIM2->DIER, 0);
}
```