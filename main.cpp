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
#include "grammar/none_del_copy_ass.h"
#include "activate/soa.h"
#include "grammar/std_forward.h"
#include "grammar/test_wait_for.h"

# undef MODERN_SQLITE_STD_OPTIONAL_SUPPORT

class  TestHandler {
public:
    TestHandler(){

    }

    void consume(){
        do {
            std::unique_lock<std::mutex> lock(mtx_);
            if(cv_.wait_for(lock, std::chrono::seconds(10), [=](){
                return !queue_.empty() || exit_requested_;
            })){
                if (exit_requested_){
                    return;
                }
                std::string str = queue_.front();
                std::cout << str << " ptr queue =" << &queue_ << std::endl;
                queue_.pop();

            }else{
                std::cout << "time out" << std::endl;
                if (exit_requested_){
                    return;
                }
            }
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
        t.join();
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
#include <unistd.h>
#include <syscall.h>
#include <regex>
#include <set>

int main() {
    std::cout << "Hello World!"  << gettid() << std::endl;
    std::vector<std::string> v_long_strings{{"11111111111"}};
    int max_print_size = 5;
    for (auto str : v_long_strings){
        size_t n_length = str.length();
        size_t n_begin_index{0};
        while (n_length >= max_print_size) {
            std::cout << str.substr(n_begin_index, max_print_size) << std::endl;
            n_begin_index +=max_print_size;
            n_length -= max_print_size;
        }
        if(n_length > 0){
            std::cout << str.substr(n_begin_index) << std::endl;
        }
    }

    test_uds_activate();
    exit(0);
    std::set<std::string> set_did{};
    std::string str_did{"0xf189"};
    std::cout << set_did.count(str_did) << std::endl;
    set_did.insert(str_did);
    std::cout << set_did.count(str_did) << std::endl;
    //test_wait_for();
    std::chrono::time_point<std::chrono::steady_clock> interval = std::chrono::steady_clock::now() + std::chrono::seconds(3);
    std::this_thread::sleep_until(interval);
    for(int i = 0 ; i < 3; i++){
        interval = std::chrono::steady_clock::now() + std::chrono::seconds(3);
        std::cout << "sleep_until:"  << i << std::endl;
        std::this_thread::sleep_until(interval);
    }
    exit(0);
    std::string str_cloud_did_version = "^878TCRD000  S.B0[0-9]$";
    std::regex regex_version{str_cloud_did_version};
    std::string str_did_from_local = "878TCRD000  S.B00";
    if(std::regex_match(str_did_from_local, regex_version)){
        std::cout << "match\n";
    }else{
        std::cout << "not match\n";
    }
    std_forward_main();
    char c = 'a';
    int8_t uc= 'a';

    if(c == uc){
        std::cout << "equal" << std::endl;
    }

    test_uds_activate();

    test_soa_activate_resp_compare();
    //jsons_tests();

    //TestHandler test;
    //test.start();

    auto json_value1 = R"..({"key": "\r\t"}).."_json;
    std::vector<uint8_t> data{0xe3};
    std::string  str(data.begin(), data.end());
    json_value1["key"] = str;
    std::cout << json_value1.is_object() << std::endl;

    test_none_del_ass();

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