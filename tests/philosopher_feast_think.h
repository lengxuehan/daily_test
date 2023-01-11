//
// Created by wuting.xu on 2023/1/4.
//

#ifndef TESTS_PHILOSOPHER_FEAST_THINK_H
#define TESTS_PHILOSOPHER_FEAST_THINK_H

#include <future>
#include <iostream>
#include <thread>
#include <utility>
#include <random>

std::random_device rd_device;
std::mt19937_64 rd_engine(rd_device());
//生成随机数
int generate_random_number(int min, int max)
{
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rd_engine);
}

void lock(std::atomic_flag& resource_flag)
{
    while (resource_flag.test_and_set());
    //忙等待
}

void unlock(std::atomic_flag& resource_flag)
{
    resource_flag.clear();
}

void feast_think(int n, std::atomic_flag& a, std::atomic_flag& b)
{
    while (true)
    {
        int duration = generate_random_number(100, 200);
        std::cout << n << " 思考" << duration << "ms \n";
        std::this_thread::sleep_for(std::chrono::milliseconds(duration)); // 思考

        lock(a);
        std::cout << n << " 获取a筷子\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        lock(b);
        std::cout << n <<" 获取b筷子\n";

        duration = generate_random_number(100, 200);
        std::cout << n << " 干饭时间:" << duration << " ms \n";

        std::this_thread::sleep_for(std::chrono::milliseconds(duration));

        unlock(b);
        unlock(a);
        std::cout << "\n\n";
    }
}

int test_philosopher_feast_think()
{
    std::atomic_flag m1, m2, m3, m4;

    std::thread t1{[&]() { feast_think(1, m1, m2); }};
    std::thread t2{[&]() { feast_think(2, m2, m3); }};
    std::thread t3{[&]() { feast_think(3, m3, m4); }};
    std::thread t4{[&]() { feast_think(4, m1, m4); }};

    t1.join();
    t2.join();
    t3.join();
    t4.join();
}


std::mutex mo;
void feast_think1(int n, std::mutex& a, std::mutex& b)
{
    while (true)
    {
        int duration = generate_random_number(1000, 2000);
        {
            std::lock_guard<std::mutex> g(mo);
            std::cout << n << " 思考" << duration << "ms \n";
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(duration)); // 思考

        std::unique_lock<std::mutex> ga(a, std::defer_lock);
        {
            std::lock_guard<std::mutex> g(mo);
            std::cout << n << " 获取a筷子\n";
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        std::unique_lock<std::mutex> gb(b, std::defer_lock);
        std::lock(ga, gb);
        {
            std::lock_guard<std::mutex> g(mo);
            std::cout << n <<" 获取b筷子\n";
        }

        duration = generate_random_number(1000, 2000);
        {
            std::lock_guard<std::mutex> g(mo);
            std::cout << n << " 干饭时间:" << duration << " ms \n";
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(duration));

        std::cout << "\n\n";
    }
}
int test_philosopher_feast_think1()
{
    std::mutex m1, m2, m3, m4;

    std::thread t1{[&]() { feast_think1(1, m1, m2); }};
    std::thread t2{[&]() { feast_think1(2, m2, m3); }};
    std::thread t3{[&]() { feast_think1(3, m3, m4); }};
    std::thread t4{[&]() { feast_think1(4, m1, m4); }};

    t1.join();
    t2.join();
    t3.join();
    t4.join();
}
#endif //TESTS_PHILOSOPHER_FEAST_THINK_H
