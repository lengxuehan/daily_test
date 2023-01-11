/**
 * @brief Singleton template definition
 * @data 2022-07-21
 * @copyright Copyright (c) 2022 Megatronix
 */

#pragma once

template<typename T>
class Singleton {
public:
    static T &instance() {
        static T instance{Token{}};
        return instance;
    }

    // disable copy and assign
    Singleton(const Singleton &) = delete;

    Singleton &operator=(const Singleton) = delete;

protected:
    struct Token {
    };

    Singleton() {}
};