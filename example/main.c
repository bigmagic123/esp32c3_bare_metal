#include <stdio.h>
#include <wdt.h>

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
  us = us * 1000;
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
    int cnt = 0;
    wdt_disable();
    printf("build timer: %s %s\n", __DATE__, __TIME__);
    time_init();
    while(1)
    {
        cnt = cnt + 1;
        printf("cur tick is %d!\n", cnt);
        delay_us(1000);
    }
    return 0;
}
