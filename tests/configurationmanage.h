/**
 * @brief ConfigurationManage definition
 * @data 2022-07-21
 * @copyright Copyright (c) 2022 Megatronix
 */

#pragma once

#include "singleton.h"
#include "sqlite_modern_cpp.h"
#include "nlohmann/json.hpp"

typedef struct AccountInfo {
    std::string tinyId;
    std::string version;
    int auId;
    int state;
    int id;
} AccountInfo;

enum ErrorCode {
    INPUT_FORMAT_ERROR = 1, // 输入参数格式错误
    FATAL_ERROR        = 2,
    OK                 = 3,
};

class ConfigurationManage : public Singleton<ConfigurationManage>{
public:
    ConfigurationManage(Token token);

    // 控制策略消息 表strategySet
    void update_strategy_set(const nlohmann::json& value);
    std::string get_strategy_set();
    std::string get_default_strategy_set();

    // 云平台通用信息 表platformSync
    void update_platform_sync(const nlohmann::json& value, int accId = 0);
    std::string get_platform_sync();

    // 获取流水号ID 0-999, 依次递增
    uint16_t get_seed();

    // 激活列表
    void update_activations_info(const std::string& activationID, const nlohmann::json& value);
    std::string get_activations_info();
    void save_activations_result(nlohmann::json& value);

    // 个性化
    void update_preference(int account_id, const nlohmann::json& value);
    std::string get_preference(int account_id);
    std::string get_default_preference();
    std::string get_last_account_preference();

    // 账号管理
    void add_account(const AccountInfo& accountInfo);
    void update_auid(const std::string& tinyId, int auId);
    void update_tinyid(const std::string& tinyId, int auId);
    void update_account_status(const std::string& tinyId, int state);
    AccountInfo get_account_by_auid(int auId);
    AccountInfo get_account_by_tinyid(const std::string& tinyId);
    void update_account_version(const std::string& tinyId, const std::string& version);

    // 参数版本号
    void update_param_version(std::string& version);
    std::string get_param_version();

    /****  点检接口   ***/
    // 更新点检列表
    void update_check_list(const nlohmann::json &value);
    // 获取点检列表
    std::string get_check_list();
    // 保存点检结果
    void save_check_list_result(const nlohmann::json &value);
    // 获取上次的点检结果
    std::string get_check_list_result();
    // 获取等待修复得点检列表
    std::string get_to_fix_check_list();
    // 查询key的状态
    std::string get_key_status(const nlohmann::json &value);
    // 查询功能号的功能状态
    std::string get_func_status(const nlohmann::json &value);
    // 获取车端条件物理值
    std::string get_conditions_value();
    // 更新车端条件物理值
    void update_conditions_value(const nlohmann::json &value);
    // 获取检查车端的激活条件配置
    std::string get_activate_conditions();
    // 检查key是否存在
    std::string exist_key_test(const nlohmann::json &value);
    // 设置点检上报成功
    void update_check_reported();
    // 设置云端响应点检上报
    void update_check_response();
    // 查询本地异常功能
    std::string get_abnormal_funcs();
    // 查询本地过期功能
    std::string get_expired_funcs();

private:
    class  KeyInfo {
    public:
        std::string key;   // "key -> key"
        std::string state; // "state -> keyState"
        std::string value; // "did_value -> keyValue"
    };

    class FuncInfo {
    public:
        int funcID;
        std::string state; // "normal"、"abnormal"、"expired"
        uint32_t expired;
    };

    void create_tables();
    bool parse_json(const std::string &jsonStr, nlohmann::json &value);

    // 从表activation的data字段生成func、key、group等信息
    void record_activation_info(const std::string& activationID);
    void record_activation_func(const nlohmann::json& func);
    void record_activation_key(int funcsInGroupID, const nlohmann::json& key);

    void update_func_state(int funcId, const std::string& state);
    void update_key_state(const std::string& action, const std::string& key, const std::string& state);

    void get_key_info(KeyInfo& key_info);
    void get_func_info(FuncInfo& func_info);

    void get_keys_ref(nlohmann::json& keys);

    void get_keys_by_func_id(int funcID, std::vector<std::string>& keys);

    sqlite::database db_;
    ErrorCode        error_code_;
public:
    // used for test
    void set_func_info(int funcID, const char* state, uint32_t expired);
    ErrorCode get_last_error(){
        ErrorCode err = error_code_;
        error_code_ = OK;
        return err;
    }
};
