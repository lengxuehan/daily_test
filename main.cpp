#include <iostream>
#include "sqlite_modern_cpp.h"
#include "sqlite_modern_cpp/log.h"
#include "tests/configurationmanage.h"
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unistd.h>
#include <atomic>
#include <csignal>
#include "thread_pool.h"
#include "activate/uds.h"
#include "tests/jsons.h"

# undef MODERN_SQLITE_STD_OPTIONAL_SUPPORT

class  TestHandler {
public:
    TestHandler(){

    }

    void consume(){
        do {
            std::unique_lock<std::mutex> lock(mtx_);
            cv_.wait(lock,[=](){
                return !queue_.empty() || exit_requested_;
            });
            if (exit_requested_){
                return;
            }
            std::string str = queue_.front();
            std::cout << str << " ptr queue =" << &queue_ << std::endl;
            queue_.pop();
        }while(true);
    }

    void produce(){
        sigset_t signals;
        sigfillset(&signals);
        pthread_sigmask(SIG_SETMASK, &signals, nullptr);
        int count = 0;
        while (count++ < 10){
            std::unique_lock<std::mutex> lock(mtx_);
            queue_.push("hello world" + std::to_string(count));
            cv_.notify_one();
            lock.unlock();
            sleep(1);
        }
        exit_requested_ = true;
        cv_.notify_one();
    }

    void start(){
        std::cout << " ptr queue =" << &queue_ << std::endl;
        std::thread t_consume = std::thread{&TestHandler::consume, this};
        auto t = std::thread{&TestHandler::produce, this};
        t_consume.join();
    }

    std::mutex mtx_;
    std::condition_variable cv_;
    std::queue<std::string> queue_;
    std::atomic_bool exit_requested_{false};
};

int returnStaticInt(){
    static int count = 1;
    return count++;
}

void ForEach(const std::vector<int> &values, void (*func)(int)){
    for(int value : values)
        func(value);
}

int main() {
    std::cout << "Hello World!" << std::endl;

    //test_uds_activate();
    jsons_tests();

    auto json_value1 = R"..({"true": 1}).."_json;
    std::cout << json_value1.is_object() << std::endl;

    exit(0);
    std::vector<int> values = {1, 2, 3, 2, 1};
    ForEach(values, [](int value) { std::cout << __func__ << "value: " << value << std::endl; });
    auto json_value = R"..({
    "keys":
    [
      {"key":"key1","status":"Y"},
      {"key":"key2","status":"N"}
   ]
}).."_json;
    std::string str_context;
    for(auto item : json_value["keys"]){
        str_context.append(item["key"].get<std::string>());
        str_context.append(item["status"].get<std::string>());
    }
    std::cout<< str_context<<std::endl;

//    nlohmann::json value;
//    value["a"].push_back("");
//    std::cout<<value.dump()<<std::endl;
//    std::cout<< haha() << std::endl;
    std::cout<< std::stoi("0x11",0,16)<<std::endl;

//    std::string check_keys =R"({"keys":[{"key": "key120940319"},{"key": "key120940320"},{"key": "key120940321"}]})";
//    nlohmann::json json;
//    json.parse(check_keys);
//
//    if (!json.is_null()){
//        int size =  json["keys"].size();
//    }

    struct tasks t;
    t.start();

//    auto r1 = t.queue(returnStaticInt);
//    auto r2 = t.queue(returnStaticInt);
//    int res1 = r1.get();
//    printf("%d\n", res1);
//    //fill_did_data_bits();
//    t.abort();
//
//    TestHandler test;
//    test.start();

    bool error_detected = false;
    sqlite::error_log(
            [&](sqlite::errors::constraint) {
                std::cout << "Wrong error detected!" << std::endl;
            },
            [&](sqlite::errors::constraint_primarykey e) {
                std::cout << e.get_code() << '/' << e.get_extended_code() << ": " << e.what() << std::endl;
                error_detected = true;
            }
    );

    if (error_detected) {
        exit(EXIT_FAILURE);
    }
    return 0;
}