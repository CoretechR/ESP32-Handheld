#include "soc/rtc_cntl_reg.h"
#include "soc/rtc_io_reg.h"
#include "soc/soc_ulp.h"

  /* Define variables, which go into .bss section (zero-initialized data) */
  .bss

  .global toggle_counter
toggle_counter:
  .long 0

  /* Code goes into .text section */
  .text
  .global entry
entry:

  /* Read toggle counter */
  move r3, toggle_counter
  ld r0, r3, 0
  /* Increment */
  add r0, r0, 1
  /* Save counter in memory */
  st r0, r3, 0
  /* Save counter in r3 to use it later */
  move r3, r0

  /* Disable hold of RTC_GPIO12 output */
  WRITE_RTC_REG(RTC_IO_TOUCH_PAD2_REG,RTC_IO_TOUCH_PAD2_HOLD_S,1,0)

  /* Toggle RTC_GPIO12 output */
  and r0, r0, 0x01
  jump toggle_clear, eq

  /* Set the RTC_GPIO12 output HIGH */
  WRITE_RTC_REG(RTC_GPIO_OUT_W1TS_REG,RTC_GPIO_OUT_DATA_W1TS_S+12,1,1)
  jump toggle_complete

  .global toggle_clear
toggle_clear:
  /* Set the RTC_GPIO12 output LOW (clear output) */
  WRITE_RTC_REG(RTC_GPIO_OUT_W1TC_REG,RTC_GPIO_OUT_DATA_W1TC_S+12,1,1)

  .global toggle_complete
toggle_complete:

  /* Enable hold on RTC_GPIO12 output */
  WRITE_RTC_REG(RTC_IO_TOUCH_PAD2_REG,RTC_IO_TOUCH_PAD2_HOLD_S,1,1)

  halt
