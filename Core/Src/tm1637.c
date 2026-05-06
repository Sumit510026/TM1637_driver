#include "tm1637.h"

/* ═══════════════════════════════════════════════════════════════
   SEGMENT LOOKUP TABLE
   Direct ASCII indexing: seg_map['A'], seg_map['5'] etc.
   Bit layout:  bit0=a(top)  bit1=b(top-right)  bit2=c(bot-right)
                bit3=d(bot)  bit4=e(bot-left)   bit5=f(top-left)
                bit6=g(mid)  bit7=dp(dot)
   ═══════════════════════════════════════════════════════════════ */
static const uint8_t seg_map[128] = {

    /* ASCII 0–31 control chars — all blank */
    [0 ... 31] = 0x00,

    /* ── Symbols ────────────────────────────── */
    [' '] = 0x00,   /*  (blank)       */
    ['!'] = 0x86,   /*  !             */
    ['"'] = 0x22,   /*  "             */
    ['#'] = 0x00,   /*  unrepresentable */
    ['$'] = 0x6D,   /*  same as S     */
    ['%'] = 0x00,   /*  unrepresentable */
    ['&'] = 0x7B,   /*  approximation */
    ['\'']= 0x02,   /*  apostrophe    */
    ['('] = 0x39,   /*  same as C/[   */
    [')'] = 0x0F,   /*  ]             */
    ['*'] = 0x00,   /*  unrepresentable */
    ['+'] = 0x70,   /*  +  (limited)  */
    [','] = 0x04,   /*  comma (bot-right) */
    ['-'] = 0x40,   /*  minus         */
    ['.'] = 0x80,   /*  dot only      */
    ['/'] = 0x52,   /*  /             */
    [':'] = 0x00,   /*  (use colon bit on digit 1 instead) */
    [';'] = 0x00,
    ['<'] = 0x58,   /*  <             */
    ['='] = 0x48,   /*  =  (mid+bot)  */
    ['>'] = 0x4C,   /*  >             */
    ['?'] = 0x53,   /*  ?             */
    ['@'] = 0x5F,   /*  @  approx     */
    ['['] = 0x39,   /*  [             */
    ['\\']= 0x64,   /*  backslash     */
    [']'] = 0x0F,   /*  ]             */
    ['^'] = 0x23,   /*  ^             */
    ['_'] = 0x08,   /*  _             */
    ['`'] = 0x20,   /*  `             */
    ['{'] = 0x39,
    ['|'] = 0x06,   /*  |             */
    ['}'] = 0x0F,
    ['~'] = 0x40,   /*  ~  same as -  */

    /* ── Digits 0–9 ─────────────────────────── */
    ['0'] = 0x3F,   /*  0  */
    ['1'] = 0x06,   /*  1  */
    ['2'] = 0x5B,   /*  2  */
    ['3'] = 0x4F,   /*  3  */
    ['4'] = 0x66,   /*  4  */
    ['5'] = 0x6D,   /*  5  */
    ['6'] = 0x7D,   /*  6  */
    ['7'] = 0x07,   /*  7  */
    ['8'] = 0x7F,   /*  8  */
    ['9'] = 0x6F,   /*  9  */

    /* ── Uppercase A–Z ──────────────────────── */
    ['A'] = 0x77,   /*  A  */
    ['B'] = 0x7C,   /*  b  */
    ['C'] = 0x39,   /*  C  */
    ['D'] = 0x5E,   /*  d  */
    ['E'] = 0x79,   /*  E  */
    ['F'] = 0x71,   /*  F  */
    ['G'] = 0x3D,   /*  G  */
    ['H'] = 0x76,   /*  H  */
    ['I'] = 0x06,   /*  I  */
    ['J'] = 0x1E,   /*  J  */
    ['K'] = 0x76,   /*  K  (looks like H, best possible) */
    ['L'] = 0x38,   /*  L  */
    ['M'] = 0x00,   /*  M  (unrepresentable — blank) */
    ['N'] = 0x54,   /*  n  */
    ['O'] = 0x3F,   /*  O  (same as 0) */
    ['P'] = 0x73,   /*  P  */
    ['Q'] = 0x67,   /*  q  */
    ['R'] = 0x50,   /*  r  */
    ['S'] = 0x6D,   /*  S  (same as 5) */
    ['T'] = 0x78,   /*  t  */
    ['U'] = 0x3E,   /*  U  */
    ['V'] = 0x1C,   /*  v  (lower half) */
    ['W'] = 0x00,   /*  W  (unrepresentable — blank) */
    ['X'] = 0x76,   /*  X  (looks like H) */
    ['Y'] = 0x6E,   /*  y  */
    ['Z'] = 0x5B,   /*  Z  (same as 2) */

    /* ── Lowercase a–z ──────────────────────── */
    ['a'] = 0x5F,   /*  a  (small, different from A) */
    ['b'] = 0x7C,   /*  b  */
    ['c'] = 0x58,   /*  c  (small, no top) */
    ['d'] = 0x5E,   /*  d  */
    ['e'] = 0x79,   /*  e  */
    ['f'] = 0x71,   /*  F  */
    ['g'] = 0x6F,   /*  g  (same as 9) */
    ['h'] = 0x74,   /*  h  (no top bar) */
    ['i'] = 0x04,   /*  i  (single segment) */
    ['j'] = 0x1E,   /*  J  */
    ['k'] = 0x76,   /*  k  (looks like H) */
    ['l'] = 0x38,   /*  L  */
    ['m'] = 0x00,   /*  m  (unrepresentable — blank) */
    ['n'] = 0x54,   /*  n  */
    ['o'] = 0x5C,   /*  o  (small, no top) */
    ['p'] = 0x73,   /*  P  */
    ['q'] = 0x67,   /*  q  */
    ['r'] = 0x50,   /*  r  */
    ['s'] = 0x6D,   /*  S  */
    ['t'] = 0x78,   /*  t  */
    ['u'] = 0x1C,   /*  u  (small, no top) */
    ['v'] = 0x1C,   /*  v  */
    ['w'] = 0x00,   /*  w  (unrepresentable — blank) */
    ['x'] = 0x76,   /*  x  (looks like H) */
    ['y'] = 0x6E,   /*  y  */
    ['z'] = 0x5B,   /*  Z  */
};

/* hex digits A–F (uppercase, distinct from letters above) */
static const uint8_t hex_map[16] = {
    0x3F, 0x06, 0x5B, 0x4F,   /* 0 1 2 3 */
    0x66, 0x6D, 0x7D, 0x07,   /* 4 5 6 7 */
    0x7F, 0x6F, 0x77, 0x7C,   /* 8 9 A b */
    0x39, 0x5E, 0x79, 0x71    /* C d E F */
};


/* ═══════════════════════════════════════════════════════════════
   LOW-LEVEL GPIO BIT-BANG
   Both pins are operated as open-drain:
     HIGH = switch to input (pull-up resistor floats line high)
     LOW  = switch to output + write 0 (actively pull low)
   ═══════════════════════════════════════════════════════════════ */

/* ~5 µs at 8 MHz HSI — adjust multiplier for other clock speeds */
static void tm_delay(void)
{
    for (volatile uint32_t i = 0; i < 40; i++);
}

/* Helper: set a CRL pin field (pins 0–7) */
static void _set_crl(uint32_t pin, uint32_t mode_cnf)
{
    uint32_t shift = pin * 4;
    TM_PORT->CRL = (TM_PORT->CRL & ~(0xFU << shift)) | (mode_cnf << shift);
}

static void CLK_HIGH(void) { _set_crl(TM_CLK_PIN, 0x4); }  /* input floating */
static void CLK_LOW(void)
{
    _set_crl(TM_CLK_PIN, 0x1);                   /* output 10 MHz push-pull */
    TM_PORT->ODR &= ~(1U << TM_CLK_PIN);         /* drive low               */
}

static void DIO_HIGH(void) { _set_crl(TM_DIO_PIN, 0x4); }
static void DIO_LOW(void)
{
    _set_crl(TM_DIO_PIN, 0x1);
    TM_PORT->ODR &= ~(1U << TM_DIO_PIN);
}

static uint8_t DIO_READ(void)
{
    _set_crl(TM_DIO_PIN, 0x4);                   /* ensure input mode */
    return (TM_PORT->IDR >> TM_DIO_PIN) & 0x1U;
}


/* ═══════════════════════════════════════════════════════════════
   PROTOCOL PRIMITIVES
   ═══════════════════════════════════════════════════════════════ */

static void tm_start(void)
{
    DIO_HIGH(); CLK_HIGH(); tm_delay();
    DIO_LOW();              tm_delay();   /* DIO falls while CLK high = START */
    CLK_LOW();              tm_delay();
}

static void tm_stop(void)
{
    CLK_LOW();  tm_delay();
    DIO_LOW();  tm_delay();
    CLK_HIGH(); tm_delay();
    DIO_HIGH(); tm_delay();               /* DIO rises while CLK high = STOP  */
}

/* Send one byte LSB-first, then clock in ACK bit.
   Returns 0 if chip ACK'd, 1 if NACK (useful for error checking). */
static uint8_t tm_write_byte(uint8_t byte)
{
    for (uint8_t i = 0; i < 8; i++) {
        CLK_LOW();
        tm_delay();

        if (byte & 0x01) DIO_HIGH();
        else             DIO_LOW();
        byte >>= 1;

        tm_delay();
        CLK_HIGH();
        tm_delay();
    }

    /* 9th clock: release DIO, read ACK */
    CLK_LOW();
    DIO_HIGH();        /* release — chip pulls low to ACK */
    tm_delay();
    CLK_HIGH();
    tm_delay();

    uint8_t ack = DIO_READ();   /* 0 = ACK, 1 = NACK */

    CLK_LOW();
    tm_delay();

    return ack;
}


/* ═══════════════════════════════════════════════════════════════
   PUBLIC API IMPLEMENTATION
   ═══════════════════════════════════════════════════════════════ */

void TM1637_Init(void)
{
    TM_GPIO_CLK_EN;

    /* Start both lines idle-high */
    CLK_HIGH();
    DIO_HIGH();
    tm_delay();
}

void TM1637_DisplayOn(uint8_t brightness)
{
    tm_start();
    tm_write_byte(TM_CMD_DISPLAY_ON | (brightness & 0x07));
    tm_stop();
}

void TM1637_DisplayOff(void)
{
    tm_start();
    tm_write_byte(TM_CMD_DISPLAY_OFF);
    tm_stop();
}

void TM1637_Clear(void)
{
    uint8_t blank[4] = {0x00, 0x00, 0x00, 0x00};
    TM1637_WriteRaw(blank);
}

/* ── Core write: pushes 4 raw segment bytes to display ────────── */
void TM1637_WriteRaw(uint8_t seg[4])
{
    /* Step 1: data command — write mode, auto-increment address */
    tm_start();
    tm_write_byte(TM_CMD_DATA_WRITE);
    tm_stop();

    /* Step 2: set start address + send all 4 digit bytes */
    tm_start();
    tm_write_byte(TM_CMD_ADDR_START);
    for (uint8_t i = 0; i < 4; i++)
        tm_write_byte(seg[i]);
    tm_stop();

    /* Step 3: display control — turn on at current brightness */
    tm_start();
    tm_write_byte(TM_CMD_DISPLAY_ON | TM_BRIGHT_7);
    tm_stop();
}

/* ── Convert single ASCII char to segment byte ────────────────── */
uint8_t TM1637_CharToSeg(char c)
{
    if ((uint8_t)c > 127) return 0x00;
    return seg_map[(uint8_t)c];
}

/* ── Display up to 4 characters from a string ────────────────── */
void TM1637_DisplayString(const char *str)
{
    uint8_t seg[4] = {0x00, 0x00, 0x00, 0x00};

    for (uint8_t i = 0; i < 4 && str[i] != '\0'; i++)
        seg[i] = TM1637_CharToSeg(str[i]);

    TM1637_WriteRaw(seg);
}

/* ── Display integer -999 to 9999 ────────────────────────────── */
void TM1637_DisplayInt(int16_t num, uint8_t leading_zeros)
{
    uint8_t seg[4] = {0x00, 0x00, 0x00, 0x00};
    uint8_t negative = 0;

    if (num < 0) {
        negative = 1;
        num = -num;
        if (num > 999) num = 999;   /* max "-999" */
    } else {
        if (num > 9999) num = 9999;
    }

    /* Fill digits right to left */
    for (int8_t i = 3; i >= 0; i--) {
        if (num == 0 && i < 3 && !leading_zeros)
            seg[i] = SEG_BLANK;
        else
            seg[i] = seg_map['0' + (num % 10)];
        num /= 10;
    }

    /* Place minus sign in the leftmost blank position */
    if (negative) {
        for (uint8_t i = 0; i < 4; i++) {
            if (seg[i] == SEG_BLANK) {
                seg[i] = SEG_MINUS;
                break;
            }
        }
    }

    TM1637_WriteRaw(seg);
}

/* ── Display 16-bit hex value e.g. 0x1A2B → "1A2B" ──────────── */
void TM1637_DisplayHex(uint16_t num)
{
    uint8_t seg[4];
    seg[3] = hex_map[(num >>  0) & 0xF];
    seg[2] = hex_map[(num >>  4) & 0xF];
    seg[1] = hex_map[(num >>  8) & 0xF];
    seg[0] = hex_map[(num >> 12) & 0xF];
    TM1637_WriteRaw(seg);
}

/* ── Display temperature  e.g. 25 → "25°C" ───────────────────── */
void TM1637_DisplayTemp(int8_t celsius)
{
    uint8_t seg[4];
    seg[2] = SEG_DEGREE;
    seg[3] = seg_map['C'];

    if (celsius < 0) {
        seg[0] = SEG_MINUS;
        seg[1] = seg_map['0' + (uint8_t)(-celsius % 10)];
    } else if (celsius < 10) {
        seg[0] = SEG_BLANK;
        seg[1] = seg_map['0' + celsius];
    } else {
        seg[0] = seg_map['0' + (celsius / 10)];
        seg[1] = seg_map['0' + (celsius % 10)];
    }

    TM1637_WriteRaw(seg);
}

/* ── Display time  e.g. 09:35, colon=1 shows colon ──────────── */
void TM1637_DisplayTime(uint8_t hours, uint8_t minutes, uint8_t colon)
{
    if (hours   > 23) hours   = 23;
    if (minutes > 59) minutes = 59;

    uint8_t seg[4];
    seg[0] = seg_map['0' + (hours   / 10)];
    seg[1] = seg_map['0' + (hours   % 10)];
    seg[2] = seg_map['0' + (minutes / 10)];
    seg[3] = seg_map['0' + (minutes % 10)];

    if (colon)
        seg[1] |= 0x80;   /* bit 7 of digit 1 drives the colon LED */

    TM1637_WriteRaw(seg);
}

/* ── Display float with 1 or 2 decimal places ────────────────── */
/*    decimals=1 → "3.14x" shows "3.14"  (4 sig digits)          */
/*    decimals=2 → " 3.14" shows " 3.1"                          */
void TM1637_DisplayFloat(float val, uint8_t decimals)
{
    /* Convert to integer scaled by 10^decimals */
    int16_t scaled;
    uint8_t dot_pos;

    if (decimals == 2) {
        scaled  = (int16_t)(val * 100.0f);
        dot_pos = 1;   /* X.XX */
    } else {
        scaled  = (int16_t)(val * 10.0f);
        dot_pos = 2;   /* XX.X */
    }

    uint8_t seg[4] = {0x00, 0x00, 0x00, 0x00};
    uint8_t negative = 0;

    if (scaled < 0) { negative = 1; scaled = -scaled; }

    for (int8_t i = 3; i >= 0; i--) {
        seg[i] = seg_map['0' + (scaled % 10)];
        scaled /= 10;
    }

    seg[dot_pos] |= 0x80;   /* insert decimal point */

    if (negative) seg[0] = SEG_MINUS;

    TM1637_WriteRaw(seg);
}

/* ── Scroll a long string across the display ─────────────────── */
void TM1637_Scroll(const char *str, uint32_t delay_ticks)
{
    /* Pad with leading and trailing spaces so text scrolls fully */
    char padded[72];
    snprintf(padded, sizeof(padded), "    %s    ", str);
    uint8_t plen = (uint8_t)strlen(padded);

    for (uint8_t i = 0; (i + 4) <= plen; i++) {
        uint8_t seg[4];
        for (uint8_t j = 0; j < 4; j++)
            seg[j] = TM1637_CharToSeg(padded[i + j]);

        TM1637_WriteRaw(seg);

        for (volatile uint32_t t = 0; t < delay_ticks; t++);
    }
}

/* ── Blink a string N times ──────────────────────────────────── */
void TM1637_Blink(const char *str, uint8_t times, uint32_t period)
{
    uint8_t seg[4] = {0x00, 0x00, 0x00, 0x00};
    for (uint8_t i = 0; i < 4 && str[i] != '\0'; i++)
        seg[i] = TM1637_CharToSeg(str[i]);

    for (uint8_t n = 0; n < times; n++) {
        TM1637_WriteRaw(seg);
        for (volatile uint32_t t = 0; t < period; t++);
        TM1637_Clear();
        for (volatile uint32_t t = 0; t < period; t++);
    }
}
