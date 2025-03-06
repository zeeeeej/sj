#ifndef _PWM_CTRL_H_
#define _PWM_CTRL_H_

int run_pwm();
void set_pwm_duty_cycle_encapsulated(int new_duty_cycle);
int get_heating_state();
int start_heating();
int stop_heating();

#endif