#include <stdio.h>
#include <wdt.h>
#include <stdbool.h>

#define OK 1
// On the ESP32C3 dev boards, the WS2812 LED is connected to GPIO 8
static int ws_2812_pin = 8;

// Simple hue function for generation of smooth rainbow.
static uint8_t hueval(int value) {
  value = value % 1536;
  if (value < 256) {
    return (uint8_t) value;
  } else if (value < 768) {
    return 255;
  } else if (value < 1024) {
    return (uint8_t)(1023 - value);
  } else {
    return 0;
  }
}

extern int uart_tx_one_char(uint8_t c);
extern void uart_tx_flush(uint8_t uart_no);
extern unsigned char uart_rx_one_char(unsigned char *ch);

void gpio_output_set(uint32_t set_mask, uint32_t clear_mask, uint32_t enable_mask, uint32_t disable_mask);

ssize_t _write(int fildes, const void *buf, size_t nbyte)
{
    const uint8_t *cbuf = (const uint8_t *) buf;
    for (size_t i = 0; i < nbyte; ++i) {
        uart_tx_one_char(cbuf[i]);
        uart_tx_flush(0);
    }
    return nbyte;
}

static inline void nop(volatile unsigned long count) {
  while (count--) asm volatile("nop");
}

#define TIMER_G0     REG(C3_TIMERGROUP0)
#define TIMER_G0_CFG REG(C3_TIMERGROUP0 + 0xFC)

static inline unsigned long time_us(void) 
{
  TIMER_G0[3] = BIT(31);             // TRM 8.3.1, request to fetch timer value
  nop(1);                            // Is there a better way to wait?
  return (TIMER_G0[2] << 31 |  TIMER_G0[1]);                // Read low bits, ignore high - don't bother
}

static inline void delay_us(unsigned long us) 
{
  unsigned long until = time_us() + us;  // Handler counter wrap
  while (time_us() < until) nop(1);
}

void time_init(void)
{
  TIMER_G0[0] &= ~BIT(31); //disable timer

  TIMER_G0_CFG[0] |= BIT(30) | BIT(31);

  TIMER_G0[0] |=  BIT(12);
  nop(100);
  TIMER_G0[0] |= (19<<13) | BIT(30);  
  nop(100);
  TIMER_G0[0] |= BIT(31);
}

static inline void ws2812_show(int pin, const uint8_t *buf, size_t len) {
  int bit_value = 0;
  unsigned long delays[2] = {2, 6};
  for (size_t i = 0; i < len; i++) {
    for (uint8_t mask = 0x80; mask; mask >>= 1) {
      int i1 = buf[i] & mask ? 1 : 0, i2 = i1 ^ 1;  // This takes some cycles
      bit_value = 1;
      gpio_output_set(bit_value<<pin, (bit_value ? 0 : 1)<<pin, 1<<pin,0);
      //gpio_write(pin, 1);
      nop(delays[i1]);
      bit_value = 0;
      gpio_output_set(bit_value<<pin, (bit_value ? 0 : 1)<<pin, 1<<pin,0);
      nop(delays[i2]);
    }
  }
}


static void blink(int count, unsigned long delay_millis) {
  uint8_t green[3] = {100, 0, 0}, black[3] = {0, 0, 0};
  for (int i = 0; i < count; i++) {
    ws2812_show(ws_2812_pin, green, sizeof(green));
    delay_us(delay_millis * 1000);
    ws2812_show(ws_2812_pin, black, sizeof(black));
    delay_us(delay_millis * 1000);
  }
}

static void rainbow(int count) {
  for (int i = 0; i < count; i++) {
    uint8_t rgb[3] = {hueval(i), hueval(i + 512), hueval(i + 1024)};
    ws2812_show(ws_2812_pin, rgb, sizeof(rgb));
    delay_us(1000);
  }
}

int main(void)
{
    unsigned char state;
    unsigned char ch;
    wdt_disable();
    printf("build timer: %s %s\n", __DATE__, __TIME__);
    time_init();
    while(1)
    {
      // state = uart_rx_one_char(&ch);
      // if(state != OK)
      // {
      //   uart_tx_one_char(ch);
      // }
      // delay_us(1000);

        for (;;) {
                blink(3, 300);
                rainbow(2500);
                delay_us(1000);
              }
    }
    return 0;
}
