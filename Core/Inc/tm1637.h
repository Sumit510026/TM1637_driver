#ifndef TM1637_H
#define TM1637_H

#include "stm32f1xx.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

/* ─────────────────────────────────────────────
   PIN CONFIGURATION  — change these to your wiring
   ───────────────────────────────────────────── */
#define TM_PORT         GPIOB
#define TM_CLK_PIN      6          // PB6
#define TM_DIO_PIN      7          // PB7
#define TM_GPIO_CLK_EN  (RCC->APB2ENR |= RCC_APB2ENR_IOPBEN)

/* ─────────────────────────────────────────────
   TM1637 COMMAND BYTES
   ───────────────────────────────────────────── */
#define TM_CMD_DATA_WRITE   0x40   // write data to display
#define TM_CMD_ADDR_START   0xC0   // auto-increment from digit 0
#define TM_CMD_DISPLAY_ON   0x88   // display ON, brightness OR'd in
#define TM_CMD_DISPLAY_OFF  0x80   // display OFF

/* brightness levels (OR with TM_CMD_DISPLAY_ON) */
#define TM_BRIGHT_0   0x00
#define TM_BRIGHT_1   0x01
#define TM_BRIGHT_2   0x02
#define TM_BRIGHT_3   0x03
#define TM_BRIGHT_4   0x04
#define TM_BRIGHT_5   0x05
#define TM_BRIGHT_6   0x06
#define TM_BRIGHT_7   0x07   // maximum

/* special segment patterns */
#define SEG_BLANK   0x00
#define SEG_MINUS   0x40
#define SEG_DEGREE  0x63
#define SEG_UNDER   0x08
#define SEG_EQUAL   0x48
#define SEG_DOT     0x80   // OR into any digit byte

/* ─────────────────────────────────────────────
   PUBLIC API
   ───────────────────────────────────────────── */
void    TM1637_Init          (void);
void    TM1637_DisplayOn     (uint8_t brightness);   // brightness 0–7
void    TM1637_DisplayOff    (void);
void    TM1637_Clear         (void);

void    TM1637_WriteRaw      (uint8_t seg[4]);
void    TM1637_DisplayString (const char *str);
void    TM1637_DisplayInt    (int16_t num, uint8_t leading_zeros);
void    TM1637_DisplayHex    (uint16_t num);
void    TM1637_DisplayTemp   (int8_t celsius);
void    TM1637_DisplayTime   (uint8_t hours, uint8_t minutes, uint8_t colon);
void    TM1637_DisplayFloat  (float val, uint8_t decimals);
void    TM1637_Scroll        (const char *str, uint32_t delay_ticks);
void    TM1637_Blink         (const char *str, uint8_t times, uint32_t period);
uint8_t TM1637_CharToSeg     (char c);

#endif /* TM1637_H */
