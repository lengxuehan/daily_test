//
// Created by weiliang.ye on 2023/1/29.
//

#ifndef DAILY_TEST_SOA_H
#define DAILY_TEST_SOA_H

#include "nlohmann/json.hpp"
#include <iostream>

template<class UnaryFunction>
void recursive_iterate(const nlohmann::json& j, UnaryFunction f){
    for(auto it = j.begin(); it != j.end(); ++it){
        if (it.value().is_structured()){
            recursive_iterate(*it, f);
        }else{
            f(it);
        }
    }
}

bool compare_json_response(const nlohmann::json& soa_response, const nlohmann::json& respCompare){
    std::map<nlohmann::json, bool> map_checked;
    std::map<std::string, bool> map_keys;
    if (respCompare.is_array()){
        for (const auto& checked : respCompare) {
            map_checked.emplace(std::make_pair(checked, false));
        }
    }else{
        for(auto iter = respCompare.begin(); iter != respCompare.end(); ++iter){
            map_keys.emplace(std::make_pair(iter.key(), false));
        }
    }

    recursive_iterate(soa_response, [&respCompare, &map_checked, &map_keys](nlohmann::json::const_iterator it){
        if (respCompare.is_array()){
            for (auto checked : respCompare) {
                for(auto pos = checked.begin(); pos != checked.end(); ++pos){
                    if (pos.key() == it.key() && pos.value() == it.value()){
                        std::cout << "find key:" << it.key() << std::endl;
                        map_checked[checked] = true;
                    }
                }
            }
        }else{
            for(auto pos = respCompare.begin(); pos != respCompare.end(); ++pos){
                if (pos.key() == it.key() && pos.value() == it.value()){
                    std::cout << "find key:" << it.key() << std::endl;
                    map_keys[it.key()] = true;
                }
            }
        }
    });

    for (auto result : map_checked) {
        if (!result.second){
            std::cout << "activate failed\n";
            return false;
        }
    }
    for (auto result : map_keys) {
        if (!result.second){
            std::cout << "activate failed" << std::endl;
            return false;
        }
    }
    std::cout << "activate succeed" << std::endl;
    return true;
}

bool test_soa_activate_resp_compare(){
    nlohmann::json j = R"({
        "code": 0,
        "result": [{
            "productid": "TestCb1666836184772",
            "status": 1
        }]
    })"_json;
    nlohmann::json j2 = R"({
	"activate": {
		"productid": "TestCb1666836184772",
		"status": 1
	},
	"deactivate": {
		"productid": "TestCb1666836184772",
		"status": 0
	}
})"_json;
    return compare_json_response(j,j2["activate"]);
}

#endif //DAILY_TEST_SOA_H
