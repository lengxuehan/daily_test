//
// Created by weiliang.ye on 2023/4/25.
//

#ifndef DAILY_TEST_TEST_WAIT_FOR_H
#define DAILY_TEST_TEST_WAIT_FOR_H

#include <thread>
#include <iostream>
#include <condition_variable>
#include <future>

std::mutex mtx;
std::condition_variable cv;

int64_t get_timestamp() {
    const auto start = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(
            start.time_since_epoch()).count();
}

void handle_test(){
    std::unique_lock<std::mutex> lock{mtx};
    std::chrono::seconds timeout_duration{ 10 };
    std::cv_status status = cv.wait_for(lock,timeout_duration);
    if(status == std::cv_status::timeout){
        std::cout << "handle_test timeout:" << get_timestamp() << std::endl;
        return;
    }
    std::cout << "handle_test finish:" << get_timestamp() << std::endl;
}
void test_wait_for(){
    std::chrono::seconds check_timeout_duration{ 5 };
    std::future<void> future_handle_test = std::async(std::launch::async, []() {
        handle_test();
    });
    std::cout << "Begin:" << get_timestamp() << std::endl;
    const std::future_status future_handle_test_status = future_handle_test.wait_for(check_timeout_duration);
    if (future_handle_test_status == std::future_status::ready) {
        std::cout << "Finish handle_test:" << get_timestamp() << std::endl;
    }
    if (future_handle_test_status == std::future_status::timeout) {
        std::cout << "Finish handle_test timeout:" << get_timestamp() << std::endl;
    }
    std::cout << "End:" << get_timestamp() << std::endl;

    cv.notify_one();
    std::this_thread::sleep_for(check_timeout_duration);
}

#endif //DAILY_TEST_TEST_WAIT_FOR_H
