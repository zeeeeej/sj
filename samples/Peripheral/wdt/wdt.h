//
// Created by v on 24-10-6.
//

#ifndef SAMPLES_WDT_H
#define SAMPLES_WDT_H

int wdt_keep_alive(void);

int wdt_enable();

int wdt_disable();

int wdt_set_timeout(int to);

int wdt_get_timeout();

#endif // SAMPLES_WDT_H
