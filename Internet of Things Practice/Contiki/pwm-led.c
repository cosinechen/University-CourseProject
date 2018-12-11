/**
 * \file
 *         PWM example application,
 *          Demonstrates using the PWM API to dim an LED.
 *          expected result from running it:
 *          red and green LEDs fading in and out, out of phase from eachother.
 * \author
 *         Marcus Linderoth <linderoth.marcus@gmail.com>
 */

#include <stdio.h>
#include "contiki.h"
#include "dev/leds.h"
#include "dev/pwm.h"
#include "dev/button.h"

/* pins to PWM */
#define LEDRED_PIN       (0)   // P1.0
#define LEDGRN_PIN       (6)   // P1.6

/* the PWM duty cycle will step back and forth between these limits, with this step */
#define PWM_MIN           0
#define PWM_MAX           100
#define PWM_STEP          1

/* wait this long between setting a new PWM setting */
#define INTERVAL          CLOCK_SECOND/64

/*
 * lookup-table of sin(x) where x == [0 .. 1.5 .. 90], 63 elements but I added
 * one extra 255 to get 64 elements.
 */
const uint8_t sin_lut[] = {0, 2, 4, 6, 13, 20, 26, 33, 40, 46, 53, 59, 66, 72, 79, 85, 91, 97,
    104, 110, 116, 122, 127, 133, 139, 144, 150, 155, 161, 166, 171, 176, 181,
    185, 190, 194, 198, 203, 207, 210, 214, 218, 221, 224, 228, 231, 233, 236,
    238, 241, 243, 245, 247, 248, 250, 251, 252, 253, 254, 255, 255, 255, 255,
    255};

#define SIN_MAXELEMENTS    45
/*---------------------------------------------------------------------------*/
PROCESS(button_process, "Button reader");
PROCESS(pwmled_process, "PWM LED process");
AUTOSTART_PROCESSES(&button_process, &pwmled_process);
/*---------------------------------------------------------------------------*/
/* set the PWM duty cycle on the pins (ie the LEDs) out of phase from eachother */

static struct etimer etr;
PROCESS_THREAD(pwmled_process, ev, data)
{
  PROCESS_POLLHANDLER();
  PROCESS_EXITHANDLER(pwm_all_off(); etimer_stop(&etr););
  PROCESS_BEGIN();
  static uint8_t i = 1;     /* counter */
  static uint8_t up = 1;    /* counting up or down? */

  pwm_confpin(LEDGRN_PIN);
  pwm_confpin(LEDRED_PIN);

  while(1) {
    /* set PWM; there are two possible PWM-devices/-channels (the 0 and 1) and
      this is how they are set */
    pwm_on(0, LEDGRN_PIN, i);
    pwm_on(1, LEDRED_PIN, i);

    /* find next PWM setting */
    if(up) {
      if(i < PWM_MAX - PWM_STEP) {
        i += PWM_STEP;
      } else {
        i = PWM_MAX;
        up = 0;
      }
    } else {
      if(i > PWM_MIN + PWM_STEP) {
        i -= PWM_STEP;
      } else {
        i = PWM_MIN ;
        up = 1;
      }
    }
    etimer_set(&etr, INTERVAL);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&etr));
  }
  PROCESS_END();
}
/*--------------------------------------------------------------------------.*/
/* Kill the PWM'ing with the push of the button. */
PROCESS_THREAD(button_process, ev, data)
{
  PROCESS_POLLHANDLER();
  PROCESS_EXITHANDLER();
  PROCESS_BEGIN();

  /* wait for a button press */
  PROCESS_WAIT_EVENT_UNTIL(ev == button_event);

  /* kill the PWM process */
  process_exit(&pwmled_process);
  PROCESS_END();
}
/* -------------------------------------------------------------------------- */
