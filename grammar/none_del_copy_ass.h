//
// Created by wuting.xu on 2023/3/27.
//
#ifndef DAILY_TEST_NONE_DEL_COPY_ASS_H
#define DAILY_TEST_NONE_DEL_COPY_ASS_H
#include <iostream>
class C {  // base class with public copy/move special member functions
public:
    C(){
        std::cout << "C()" << std::endl;
    }
    C(C const&) = delete;
    C& operator =(C const&) = delete;
    C(C&&) = delete;
    C& operator=(C&&) = delete;
};
class D : public C{
public:
    D(){
        std::cout << "D()" << std::endl;
    }
};

class A {  // base class with protected copy/move special member functions
public:
    A(){

    }
protected:
    A(A const&) = delete;
    A& operator =(A const&) = delete;
    A(A&&) = delete;
    A& operator=(A&&) = delete;
};

class B : public A{

};

void Bar(C c){
    std::cout << "Bar is called: " << &c << std::endl;
}

void Foo(D d){
    std::cout << "call Bar: " << &d << std::endl;
    //Bar(d);
}

void test_none_del_ass(){
    D d;
    //Foo(d);
}
#endif //DAILY_TEST_NONE_DEL_COPY_ASS_H
