#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
typedef struct {
    volatile uint32_t CRL, CRH, IDR, ODR, BSRR, BRR, LCKR;
} GPIO_TypeDef;

typedef struct {
    volatile uint32_t CR, CFGR, CIR, APB2RSTR, APB1RSTR, AHBENR, APB2ENR, APB1ENR, BDCR, CSR;
} RCC_TypeDef;

typedef struct {
    volatile uint32_t CTRL, LOAD, VAL, CALIB;
} SysTick_TypeDef;

typedef struct {
    volatile uint32_t SR, DR, BRR, CR1, CR2, CR3, GTPR;
} USART_TypeDef;

#define GPIOA   ((GPIO_TypeDef*)0x40010800)
#define RCC     ((RCC_TypeDef*)0x40021000)
#define SysTick ((SysTick_TypeDef*)0xE000E010)
#define USART2  ((USART_TypeDef*)0x40004400)

#define GPIO_PIN_1          (1 << 1)
#define DEBOUNCE_MS         20

static volatile bool btn_pressed = false;
static inline bool btn_raw(void) { return btn_pressed; }

typedef enum { IDLE, BOUNCE_ON, PRESSED, BOUNCE_OFF } btn_state_t;
typedef struct {
    btn_state_t state;
    uint16_t timer;
    void (*on_press)(void);
} button_t;

button_t my_button;
static bool led_state = false;


void uart_puts(const char *s);  

void led_toggle(void) {
    if (led_state) {
        GPIOA->BSRR = (GPIO_PIN_1 << 16);  // OFF
        uart_puts("LED OFF\r\n");
    } else {
        GPIOA->BSRR = GPIO_PIN_1;          // ON
        uart_puts("LED ON\r\n");
    }
    led_state = !led_state;
    btn_pressed = false;  // TỰ ĐỘNG RESET
}

// NON-BLOCKING UART
void uart_puts(const char *s) {
    static const char *ptr = NULL;
    static uint8_t busy = 0;

    if (s) {
        if (busy) return;
        ptr = s;
        busy = 1;
        return;
    }

    if (!busy || !ptr) {
        busy = 0;
        return;
    }

    if (USART2->SR & (1 << 7)) {
        USART2->DR = *ptr;
        if (*ptr == '\0') {
            ptr = NULL;
            busy = 0;
        } else {
            ptr++;
        }
    }
}

void uart_poll(void) {
    uart_puts(NULL);
}

// FSM 
void btn_init(button_t *btn) {
    btn->state = IDLE;
    btn->timer = 0;
    btn->on_press = led_toggle;
}

void btn_debounce_tick(button_t *btn) {
    bool current = btn_raw();
    switch (btn->state) {
        case IDLE:
            if (current) {
                btn->state = BOUNCE_ON;
                btn->timer = DEBOUNCE_MS;
            }
            break;
        case BOUNCE_ON:
            if (--btn->timer == 0) {
                if (current) {
                    btn->state = PRESSED;
                    if (btn->on_press) btn->on_press();
                } else {
                    btn->state = IDLE;
                }
            }
            break;
        case PRESSED:
            if (!current) {
                btn->state = BOUNCE_OFF;
                btn->timer = DEBOUNCE_MS;
            }
            break;
        case BOUNCE_OFF:
            if (--btn->timer == 0) {
                if (!current) {
                    btn->state = IDLE;
                } else {
                    btn->state = PRESSED;
                }
            }
            break;
    }
}

void SysTick_Handler(void) {
    btn_debounce_tick(&my_button);
    uart_poll();
}

// UART INIT 
void uart_init(void) {
    RCC->APB2ENR |= (1 << 2);
    RCC->APB1ENR |= (1 << 17);
    GPIOA->CRL &= ~(0xF << 8); GPIOA->CRL |= (0xB << 8);
    USART2->BRR = 69;
    USART2->CR1 = (1<<13)|(1<<3)|(1<<2);
}
void uart_rx_handler(void) {
    if (USART2->SR & (1 << 5)) {
        char c = USART2->DR;
        if ((c == 'P' || c == 'p') && !btn_pressed) {
            btn_pressed = true;
            uart_puts(">>> TOGGLE (P)\r\n");
        }
    }
}

// VECTOR TABLE 
void Reset_Handler(void);
void Default_Handler(void) { while(1); }

void (* const vectors[])(void) __attribute__((section(".isr_vector"))) = {
    (void(*)(void))(0x20005000), Reset_Handler, Default_Handler, Default_Handler,
    Default_Handler, Default_Handler, Default_Handler, 0,0,0,0,
    Default_Handler, Default_Handler, 0, Default_Handler, SysTick_Handler,
    Default_Handler
};

void Reset_Handler(void) {
    extern uint32_t _sidata, _sdata, _edata, _sbss, _ebss;
    uint32_t *src = &_sidata, *dst = &_sdata;
    while (dst < &_edata) *dst++ = *src++;
    for (dst = &_sbss; dst < &_ebss; dst++) *dst = 0;

    RCC->APB2ENR |= (1 << 2);
    uart_init();
    GPIOA->CRL &= ~(0xF << 4); GPIOA->CRL |= (0x2 << 4);

    SysTick->LOAD = 8000 - 1;
    SysTick->VAL = 0;
    SysTick->CTRL = 0x7;

    btn_init(&my_button);

    uart_puts("=== DEBOUNCE TEST ===\r\n");
    uart_puts("GÕ 'P' → TOGGLE LED \r\n");

    while (1) {
        uart_rx_handler();
        uart_poll();
    }
}