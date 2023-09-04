//
// Created by weiliang.ye on 2023/4/25.
//

#ifndef DAILY_TEST_STD_FORWARD_H
#define DAILY_TEST_STD_FORWARD_H

#include <iostream>

template<typename T>
void print(T& t){
    std::cout << "lvalue" << std::endl;
}
template<typename T>
void print(T&& t){
    std::cout << "rvalue" << std::endl;
}

template<typename T>
void TestForward(T && v){
    print(v);
    print(std::forward<T>(v));
    print(std::move(v));
}

int std_forward_main(){
    //TestForward(1);
    int x = 1;
    //TestForward(x);
    TestForward(std::forward<int>(x));
    return 0;
}
#endif //DAILY_TEST_STD_FORWARD_H
