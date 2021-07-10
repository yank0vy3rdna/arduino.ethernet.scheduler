//
// Created by yank0vy3rdna on 09.07.2021.
//

#ifndef ETHERNET_SCHEDULER_TIME_CLOCK_H
#define ETHERNET_SCHEDULER_TIME_CLOCK_H

#include <DS3231.h>

DS3231 rtc(5, 3);

void setTimeClock(char hour, char minute) {
    rtc.setTime((int)hour, (int)minute, 0);
}

uint16_t getTimeClock() {
    Time time = rtc.getTime();
    return time.hour * 60 + time.min;
}

#endif //ETHERNET_SCHEDULER_TIME_CLOCK_H
