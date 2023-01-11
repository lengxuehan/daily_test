//
// Created by weiliang.ye on 2023/1/5.
//

#ifndef TESTS_TIMER_TEST_H
#define TESTS_TIMER_TEST_H

#include "timer.h"
#include <iostream>

void trigger_check_cycle_timer(){
    std::cout << "trigger_check_cycle_timer" << std::endl;
}

void timer_test(){
    Timer trigger_check_timer{[](const boost::any& trigger_check_cycle) {trigger_check_cycle_timer();}};
    std::chrono::seconds trigger_check_cycle_s{1};
    Timer::Clock::duration timer_period{trigger_check_cycle_s};
    trigger_check_timer.start_periodic_delayed(timer_period, "");
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(60s);
    trigger_check_timer.stop();
}
#endif //TESTS_TIMER_TEST_H
