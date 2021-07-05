#include <stdio.h>
#include <wdt.h>
#include <stdbool.h>

#define OK 1

extern int uart_tx_one_char(uint8_t c);
extern void uart_tx_flush(uint8_t uart_no);
extern unsigned char uart_rx_one_char(unsigned char *ch);

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

int main(void)
{
    unsigned char state;
    unsigned char ch;
    wdt_disable();
    printf("build timer: %s %s\n", __DATE__, __TIME__);
    time_init();
    while(1)
    {
      state = uart_rx_one_char(&ch);
      if(state != OK)
      {
        uart_tx_one_char(ch);
      }
      delay_us(1000);
    }
    return 0;
}
