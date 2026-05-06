/* ================================================================
   TM1637 + Blue Pill (STM32F103C8T6)
   Complete main.c with explicit pin declarations
   ================================================================
   WIRING:
     PB6  ──────── CLK  (TM1637)
     PB7  ──────── DIO  (TM1637)
     3.3V ──────── VCC  (TM1637)
     GND  ──────── GND  (TM1637)

   Both lines need a pull-up resistor (4.7kΩ to 3.3V) if your
   TM1637 module doesn't already have them on-board.
   ================================================================ */

#include "stm32f1xx.h"
#include "tm1637.h"

/* ================================================================
   PIN DECLARATIONS
   These match the defines in tm1637.h — declared here explicitly
   so the wiring is visible at the top of main.
   ================================================================ */

/* GPIO Port used for both CLK and DIO */
#define TM_GPIO_PORT        GPIOB

/* CLK pin — PB6 */
#define TM_CLK_PIN          6U
#define TM_CLK_PIN_MASK     (1U << TM_CLK_PIN)

/* DIO pin — PB7 */
#define TM_DIO_PIN          7U
#define TM_DIO_PIN_MASK     (1U << TM_DIO_PIN)

/* CRL register bit positions (each pin = 4 bits in CRL for pins 0-7) */
#define TM_CLK_CRL_SHIFT    (TM_CLK_PIN * 4U)   /* = 24 */
#define TM_DIO_CRL_SHIFT    (TM_DIO_PIN * 4U)   /* = 28 */

/* GPIO mode values (written into CRL 4-bit fields)
   0x0 = input analog
   0x4 = input floating     <- used for HIGH (open-drain release)
   0x1 = output 10MHz PP    <- used for LOW  (actively pull low)
   0x3 = output 50MHz PP                                         */
#define GPIO_MODE_INPUT_FLOAT   0x4U
#define GPIO_MODE_OUTPUT_10MHZ  0x1U

/* ================================================================
   STATIC PIN CONTROL FUNCTIONS
   Declared here in main.c so pin behaviour is fully transparent.
   The driver (tm1637.c) calls these same operations internally,
   but here we show them explicitly for learning purposes.
   ================================================================ */

/* Release CLK -> line floats HIGH via pull-up resistor */
static inline void PIN_CLK_HIGH(void)
{
    /* Clear 4-bit field for PB6 then set input-floating mode */
    TM_GPIO_PORT->CRL &= ~(0xFU << TM_CLK_CRL_SHIFT);
    TM_GPIO_PORT->CRL |=  (GPIO_MODE_INPUT_FLOAT << TM_CLK_CRL_SHIFT);
}

/* Drive CLK -> actively pull LOW */
static inline void PIN_CLK_LOW(void)
{
    /* Set output mode, then write 0 to ODR */
    TM_GPIO_PORT->CRL &= ~(0xFU << TM_CLK_CRL_SHIFT);
    TM_GPIO_PORT->CRL |=  (GPIO_MODE_OUTPUT_10MHZ << TM_CLK_CRL_SHIFT);
    TM_GPIO_PORT->ODR &= ~TM_CLK_PIN_MASK;
}

/* Release DIO -> line floats HIGH via pull-up resistor */
static inline void PIN_DIO_HIGH(void)
{
    TM_GPIO_PORT->CRL &= ~(0xFU << TM_DIO_CRL_SHIFT);
    TM_GPIO_PORT->CRL |=  (GPIO_MODE_INPUT_FLOAT << TM_DIO_CRL_SHIFT);
}

/* Drive DIO -> actively pull LOW */
static inline void PIN_DIO_LOW(void)
{
    TM_GPIO_PORT->CRL &= ~(0xFU << TM_DIO_CRL_SHIFT);
    TM_GPIO_PORT->CRL |=  (GPIO_MODE_OUTPUT_10MHZ << TM_DIO_CRL_SHIFT);
    TM_GPIO_PORT->ODR &= ~TM_DIO_PIN_MASK;
}

/* Read DIO state — used for ACK detection */
static inline uint8_t PIN_DIO_READ(void)
{
    /* Switch to input first, then sample IDR */
    TM_GPIO_PORT->CRL &= ~(0xFU << TM_DIO_CRL_SHIFT);
    TM_GPIO_PORT->CRL |=  (GPIO_MODE_INPUT_FLOAT << TM_DIO_CRL_SHIFT);
    return (uint8_t)((TM_GPIO_PORT->IDR >> TM_DIO_PIN) & 0x1U);
}

/* ================================================================
   PIN INITIALIZATION FUNCTION
   Call this once before TM1637_Init().
   Sets up GPIOB clock and puts both pins in idle-high state.
   ================================================================ */
static void TM_Pins_Init(void)
{
    /* Step 1: Enable GPIOB peripheral clock on APB2 bus
               Bit 3 of APB2ENR = IOPBEN (GPIOB clock enable)    */
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

    /* Step 2: Small delay to let clock stabilize
               (mandatory on some STM32 revisions)                */
    for (volatile uint32_t i = 0; i < 10; i++);

    /* Step 3: Configure PB6 (CLK) as input floating
               Bits [27:24] of CRL — clear then set 0100          */
    TM_GPIO_PORT->CRL &= ~(0xFU << TM_CLK_CRL_SHIFT);
    TM_GPIO_PORT->CRL |=  (GPIO_MODE_INPUT_FLOAT << TM_CLK_CRL_SHIFT);

    /* Step 4: Configure PB7 (DIO) as input floating
               Bits [31:28] of CRL — clear then set 0100          */
    TM_GPIO_PORT->CRL &= ~(0xFU << TM_DIO_CRL_SHIFT);
    TM_GPIO_PORT->CRL |=  (GPIO_MODE_INPUT_FLOAT << TM_DIO_CRL_SHIFT);

    /* Step 5: Both lines now idle-HIGH via external pull-ups
               Input mode ignores ODR, so no ODR write needed     */
}

/* ================================================================
   SIMPLE DELAY  (~1 ms per count at 8 MHz HSI)
   ================================================================ */
static void delay_ms(uint32_t ms)
{
    for (volatile uint32_t i = 0; i < (ms * 800UL); i++);
}

/* ================================================================
   MAIN
   ================================================================ */
int main(void)
{
    /* ── 1. Initialize GPIO pins for CLK (PB6) and DIO (PB7) ─── */
    TM_Pins_Init();
    /*
       After TM_Pins_Init():
         PB6 (CLK) → input floating → pulled HIGH by 4.7k resistor
         PB7 (DIO) → input floating → pulled HIGH by 4.7k resistor
         GPIOB clock is running on APB2 bus
    */

    /* ── 2. Initialize TM1637 driver ────────────────────────── */
    TM1637_Init();
    TM1637_DisplayOn(TM_BRIGHT_5);   /* brightness 0 (dim) to 7 (max) */

    /* ── 3. Startup animation ───────────────────────────────── */
    uint8_t dashes[4] = { SEG_MINUS, SEG_MINUS, SEG_MINUS, SEG_MINUS };
    TM1637_WriteRaw(dashes);          /* show "----"  */
    delay_ms(800);
    TM1637_Clear();
    delay_ms(400);

    /* ── 4. Main demo loop ───────────────────────────────────── */
    while (1)
    {
        /* ── A) Plain integers ──────────────────────────────── */
        TM1637_DisplayInt(0,    1);   delay_ms(500);   /* 0000 */
        TM1637_DisplayInt(1234, 1);   delay_ms(800);   /* 1234 */
        TM1637_DisplayInt(9999, 1);   delay_ms(800);   /* 9999 */
        TM1637_DisplayInt(42,   0);   delay_ms(800);   /*   42 */
        TM1637_DisplayInt(-7,   0);   delay_ms(800);   /*  -7  */
        TM1637_DisplayInt(-999, 0);   delay_ms(800);   /* -999 */

        /* ── B) Hexadecimal ─────────────────────────────────── */
        TM1637_DisplayHex(0xABCD);    delay_ms(800);   /* AbCd */
        TM1637_DisplayHex(0xDEAD);    delay_ms(800);   /* dEAd */
        TM1637_DisplayHex(0xBEEF);    delay_ms(800);   /* bEEF */

        /* ── C) Alphabet strings ────────────────────────────── */
        TM1637_DisplayString("HELP"); delay_ms(800);
        TM1637_DisplayString("PASS"); delay_ms(800);
        TM1637_DisplayString("FAIL"); delay_ms(800);
        TM1637_DisplayString("Err "); delay_ms(800);
        TM1637_DisplayString("GOOD"); delay_ms(800);
        TM1637_DisplayString("COOL"); delay_ms(800);
        TM1637_DisplayString("PLAY"); delay_ms(800);
        TM1637_DisplayString("STOP"); delay_ms(800);
        TM1637_DisplayString("boot"); delay_ms(800);
        TM1637_DisplayString("donE"); delay_ms(800);

        /* ── D) Mixed alphanumeric ──────────────────────────── */
        TM1637_DisplayString("A5b3"); delay_ms(800);
        TM1637_DisplayString("C2H5"); delay_ms(800);
        TM1637_DisplayString("P1n4"); delay_ms(800);
        TM1637_DisplayString("H2O "); delay_ms(800);

        /* ── E) Temperature ─────────────────────────────────── */
        TM1637_DisplayTemp(0);        delay_ms(800);   /*  0C  */
        TM1637_DisplayTemp(25);       delay_ms(800);   /* 25C  */
        TM1637_DisplayTemp(-7);       delay_ms(800);   /* -7C  */
        TM1637_DisplayTemp(99);       delay_ms(800);   /* 99C  */

        /* ── F) Clock / time with blinking colon ────────────── */
        TM1637_DisplayTime(0,  0, 1); delay_ms(600);   /* 00:00 */
        TM1637_DisplayTime(9, 30, 1); delay_ms(600);   /* 09:30 */
        TM1637_DisplayTime(12, 0, 1); delay_ms(600);   /* 12:00 */
        TM1637_DisplayTime(23,59, 1); delay_ms(600);   /* 23:59 */

        /* Blink colon 6 times at 12:00 */
        for (uint8_t blink = 0; blink < 6; blink++) {
            TM1637_DisplayTime(12, 0, blink & 1);
            delay_ms(500);
        }

        /* ── G) Float values ────────────────────────────────── */
        TM1637_DisplayFloat(3.14f, 2); delay_ms(800);  /* 3.14 */
        TM1637_DisplayFloat(9.81f, 2); delay_ms(800);  /* 9.81 */
        TM1637_DisplayFloat(27.5f, 1); delay_ms(800);  /* 27.5 */
        TM1637_DisplayFloat(-1.5f, 1); delay_ms(800);  /* -1.5 */

        /* ── H) Brightness sweep ────────────────────────────── */
        TM1637_DisplayString("8888");
        for (uint8_t b = 0; b <= 7; b++) {
            TM1637_DisplayOn(b);
            delay_ms(250);
        }
        for (int8_t b = 6; b >= 0; b--) {
            TM1637_DisplayOn(b);
            delay_ms(250);
        }
        TM1637_DisplayOn(TM_BRIGHT_5);  /* restore */

        /* ── I) Blink alerts ────────────────────────────────── */
        TM1637_Blink("WARN", 4, 350000);
        TM1637_Blink("Err ", 4, 350000);

        /* ── J) Scroll long strings ─────────────────────────── */
        TM1637_Scroll("HELLO WORLD",        280000);
        TM1637_Scroll("STM32 TM1637 READY", 280000);

        /* ── K) Fast count up ───────────────────────────────── */
        for (uint16_t n = 0; n <= 9999; n += 7) {
            TM1637_DisplayInt(n, 1);
            delay_ms(5);
        }

        /* ── L) Display off / on ────────────────────────────── */
        TM1637_DisplayString("OFF ");
        delay_ms(800);
        TM1637_DisplayOff();
        delay_ms(1500);
        TM1637_DisplayOn(TM_BRIGHT_5);
        TM1637_DisplayString(" On ");
        delay_ms(800);

        TM1637_Clear();
        delay_ms(500);
    }
}
