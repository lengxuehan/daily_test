//
// Created by weiliang.ye on 2023/1/30.
//

#ifndef DAILY_TEST_JSONS_H
#define DAILY_TEST_JSONS_H

#include "nlohmann/json.hpp"
#include <iostream>

bool parse_json(const std::string &jsonStr, nlohmann::json &value) {
    value = nlohmann::json::parse(jsonStr, nullptr, false);
    if (!value.is_discarded()) {
        return true;
    } else {
        return false;
    }
}

std::string to_good_json_string(const std::string& str){
    std::string res;
    for (size_t i=0; i<str.length(); i++) {
        char c = str[i];
        switch (c) {
            case '\"':
                res.append("\\\"");
                break;
            case '\\':
                res.append("\\\\");
                break;
            case '/':
                res.append("\\/");
                break;
            case '\b':
                res.append("\\b");
                break;
            case '\f':
                res.append("\\f");
                break;
            case '\n':
                res.append("\\n");
                break;
            case '\r':
                res.append("\\r");
                break;
            case '\t':
                res.append("\\t");
                break;
            case '\'':
                res.append("\\\'");
                break;
            default:
                res.push_back(c);
        }
    }
    return res;
}

bool compare_str_json(const std::string & expect, const std::string &src){
    nlohmann::json expect_value;
    nlohmann::json src_value;
    if (!parse_json(expect, expect_value)){
        std::cout << "compare_str_json parse expect error " << std::endl;
        return false;
    }
    if (!parse_json(src, src_value)){
        std::cout << "compare_str_json parse src error " << std::endl;
        return false;
    }

    return src_value["keys"] == expect_value["keys"];
}

bool is_good_json_string(const std::string& str){
    return nlohmann::json::accept("\"" + str + "\"");
   //nlohmann::json::accept("{\"key\": \"" + str + "\"}");
}

void jsons_tests(){
    nlohmann::json json_obj;
    //std::vector<uint8_t> data{38,30,34,36,30,30,33,43, 52, 44, 30, 30, 30, 30, 0xe3, 0xe3, 0xe3};
    std::vector<uint8_t> data{0xe3};
    //std::string str("123\t");//
    std::string str(data.begin(),data.end());
    std::cout << str << std::endl;
    std::cout << is_good_json_string(str) << std::endl;
}
#endif //DAILY_TEST_JSONS_H
