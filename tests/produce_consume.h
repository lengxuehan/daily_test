//
// Created by wuting.xu on 2023/1/4.
//

#ifndef TESTS_PRODUCE_CONSUME_H
#define TESTS_PRODUCE_CONSUME_H

#include <iostream>
#include <thread>
#include <condition_variable>
std::mutex mutex_recv_event;
std::condition_variable cv_recv_event;

void consume(){
    int i = 0;
    do{
        {
            std::unique_lock<std::mutex> lock{mutex_recv_event};
            cv_recv_event.wait(lock);
            lock.unlock();
        }
        i++;
        if(i > 10){
            break;
        }
        std::cout << "consume:" << i << std::endl;
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(500ms);
    } while (true);
}

void test_produce_consume(){
    std::thread t = std::thread{[](){consume();}};
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);
    for(int i = 0; i < 10; i++){
        std::unique_lock<std::mutex> lock{mutex_recv_event};
        cv_recv_event.notify_one();
        std::cout << "produce:" << i << std::endl;
        std::this_thread::sleep_for(100ms);
        lock.unlock();
    }
    if(t.joinable()) {
        t.join();
    }
}

#endif //TESTS_PRODUCE_CONSUME_H
