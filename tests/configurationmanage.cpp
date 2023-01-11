#include "configurationmanage.h"

ConfigurationManage::ConfigurationManage(Token token)
        : db_("db_file.db") {
    create_tables();
}

void ConfigurationManage::update_strategy_set(const nlohmann::json &value) {
    db_ << "INSERT OR REPLACE INTO strategySet (id, data, timestamp) VALUES (?, ?, ?);"
        << 1
        << value.dump()
        << time(NULL);
}

std::string ConfigurationManage::get_strategy_set() {
    std::string ret;
    db_ << "SELECT data FROM strategySet LIMIT 1;"
        >> [&](std::string data) {
            ret = std::move(data);
        };
    return ret;
}

std::string ConfigurationManage::get_default_strategy_set() {
    std::string ret;
    db_ << "SELECT default_strategy FROM global LIMIT 1;"
        >> ret;
    return ret;
}

void ConfigurationManage::update_platform_sync(const nlohmann::json &value, int accId) {
    db_ << "INSERT OR REPLACE INTO platformSync (id, accountId, data, timestamp) VALUES (?, ?, ?, ?);"
        << 1
        << accId
        << value.dump()
        << time(NULL);
}

std::string ConfigurationManage::get_platform_sync() {
    std::string ret;
    db_ << "SELECT data FROM platformSync LIMIT 1;"
        >> [&](std::string data) {
            ret = std::move(data);
        };
    return ret;
}

uint16_t ConfigurationManage::get_seed() {
    uint16_t ret;
    db_ << "SELECT seed FROM global LIMIT 1;"
        >> [&](int seed) {
            ret = seed;
            db_ << "UPDATE global SET seed = ? where id = 1"
                << (++seed) % 1000;
        };
    // format seed like "000" "001" or "999"
    //for (int i = ret.length(); i < 3; ++i) {
    //    ret.insert(ret.begin(), '0');
    //}
    return ret;
}

void ConfigurationManage::update_activations_info(const std::string &activationID, const nlohmann::json &value) {
    db_ << "INSERT OR REPLACE INTO activation (trace, data, timestamp) VALUES (?, ?, ?);"
        << activationID
        << value.dump()
        << time(NULL);
}

std::string ConfigurationManage::get_activations_info() {
    nlohmann::json res_value;
    db_ << "SELECT data FROM activation WHERE res is null ORDER by timestamp DESC;"
        >> [&](std::string data) {
            nlohmann::json value;
            if(!parse_json(data, value)){
                //TODO LOG ERROR
                return;
            }
            res_value["activations"].push_back(value);
        };

    nlohmann::json keys;
    get_keys_ref(keys);
    res_value["keys"] = keys;
    return res_value.dump();
}

void ConfigurationManage::save_activations_result(nlohmann::json &value) {
    const std::string activationID = value["activationSubmit"]["activationID"];

    record_activation_info(activationID);

    db_ << "UPDATE activation SET res = ? where trace = ?"
        << value.dump()
        << activationID;

    if (!value["activationSubmit"]["manyGroups"].is_null()) {
        error_code_ = INPUT_FORMAT_ERROR;
        // TODO LOG ERROR
    }
    if (!value["activationSubmit"]["manyGroups"].is_array()) {
        error_code_ = INPUT_FORMAT_ERROR;
        // TODO LOG ERROR
    }

    const nlohmann::json &many_groups = value["activationSubmit"]["manyGroups"];
    // manyGroups
    for (int i = 0; i < many_groups.size(); ++i) {
        const nlohmann::json &group = many_groups[i];
        // funcsInGroup
        if (!group["funcsInGroup"].is_null() && !group["funcsInGroup"].is_array()) {
            // TODO LOG ERROR
            error_code_ = INPUT_FORMAT_ERROR;
            continue;
        }
        const nlohmann::json &funcs = group["funcsInGroup"];
        for (int j = 0; j < funcs.size(); ++j) {

            const nlohmann::json &func = funcs[i];
            int funcID = func["funcID"];
            std::string funcState = func["funcState"];

            update_func_state(funcID, funcState);

            // keys
            if (!func["keys"].is_null() && !func["keys"].is_array()) {
                // TODO LOG ERROR
                error_code_ = INPUT_FORMAT_ERROR;
                continue;
            }
            const nlohmann::json &keys = func["keys"];
            std::string action = func["action"];
            for (int m = 0; m < keys.size(); ++m) {

                const nlohmann::json &key = keys[m];
                std::string str_key = key["key"];
                std::string keyState = key["keyState"];

                update_key_state(action, str_key, keyState);
            }
        }
    }
}

// 个性化
void ConfigurationManage::update_preference(int account_id, const nlohmann::json &value) {
    db_ << "INSERT OR REPLACE INTO preference (account_id, data, timestamp) VALUES (?, ?, ?);"
        << account_id
        << value.dump()
        << time(NULL);
}

std::string ConfigurationManage::get_preference(int account_id) {
    std::string ret;
    db_ << "SELECT data FROM preference WHERE account_id = ?;"
        << account_id
        >> [&](std::string data) {
            ret = std::move(data);
        };
    return ret;
}

std::string ConfigurationManage::get_default_preference() {
    std::string ret;
    db_ << "SELECT default_preference FROM global limit 1;"
        >> ret;
    return ret;
}

std::string ConfigurationManage::get_last_account_preference() {
    int account_id = 0;
    // TODO 定义账号是否登录的四个状态
    db_ << "SELECT id FROM account WHERE state = ?;"
        << 1
        >> [&](int id) {
            account_id = id;
        };

    if (account_id == 0) {
        // TODO ERROR LOG
    }

    std::string res = get_preference(account_id);
    return res;
}

// 账号管理
void ConfigurationManage::add_account(const AccountInfo &accountInfo) {
    int account_id = 0;
    db_ << "SELECT id FROM account WHERE tinyID = ?;"
        << accountInfo.tinyId
        >> [&](int id) {
            account_id = id;
        };
    if (account_id > 0) {
        db_ << "INSERT OR REPLACE INTO account (id, tinyID, auID, state, timestamp, version) VALUES (?, ?, ?, ?, ?, ?);"
            << account_id
            << accountInfo.tinyId
            << accountInfo.auId
            << accountInfo.state
            << time(NULL)
            << accountInfo.version;
    } else {
        db_ << "INSERT INTO account (tinyID, auID, state, timestamp, version) VALUES (?, ?, ?, ?, ?);"
            << accountInfo.tinyId
            << accountInfo.auId
            << accountInfo.state
            << time(NULL)
            << accountInfo.version;
    }

}

void ConfigurationManage::update_auid(const std::string &tinyId, int auId) {
    db_ << "UPDATE account SET auId = ? WHERE tinyID  = ?"
        << auId
        << tinyId;
}

void ConfigurationManage::update_tinyid(const std::string &tinyId, int auId) {
    db_ << "UPDATE account SET tinyID  = ? WHERE auId  = ?"
        << tinyId
        << auId;
}

void ConfigurationManage::update_account_status(const std::string &tinyId, int state) {
    db_ << "UPDATE account SET state = ? WHERE tinyID  = ?"
        << state
        << tinyId;
}

AccountInfo ConfigurationManage::get_account_by_auid(int auId) {
    AccountInfo info;
    db_ << "SELECT id, tinyID, state, version FROM account WHERE auId = ?;"
        << auId
        >> [&](int id, std::string tinyID, int state, std::string version) {
            info.id = id;
            info.tinyId = std::move(tinyID);
            info.auId = auId;
            info.state = state;
            info.version = std::move(version);
        };
    return info;
}

AccountInfo ConfigurationManage::get_account_by_tinyid(const std::string &tinyId) {
    AccountInfo info;
    db_ << "SELECT id, auId, state, version FROM account WHERE tinyID = ?;"
        << tinyId
        >> [&](int id, int auId, int state, std::string version) {
            info.id = id;
            info.tinyId = tinyId;
            info.auId = auId;
            info.state = state;
            info.version = std::move(version);
        };
    return info;
}

void ConfigurationManage::update_account_version(const std::string &tinyId, const std::string &version) {
    db_ << "UPDATE account set  version = ? WHERE tinyID = ?;"
        << version
        << tinyId;
}

void ConfigurationManage::update_param_version(std::string &version) {
    db_ << "INSERT OR REPLACE INTO param_version (id, version, timestamp) VALUES (?, ?, ?);"
        << 1
        << version
        << time(NULL);
}

std::string ConfigurationManage::get_param_version() {
    std::string ret;
    db_ << "SELECT version FROM param_version limit 1;"
        >> ret;
    return ret;
}

void ConfigurationManage::update_check_list(const nlohmann::json &value) {
    if(value["check"]["trace"].is_null()){
        // TODO LOG ERROR
        error_code_ = INPUT_FORMAT_ERROR;
        return;
    }
    std::string trace = value["check"]["trace"];
    try {
    db_ << "INSERT OR REPLACE INTO checks (trace, data, timestamp) VALUES (?, ?, ?);"
        << trace
        << value.dump()
        << time(NULL);
    } catch (std::exception &e) {
        // TODO LOG ERROR
        // std::cout << e.what() << std::endl;
    }
}

std::string ConfigurationManage::get_check_list() {
    std::string ret;
    db_ << "SELECT data FROM checks  WHERE res is null order by timestamp desc limit 1;"
        >> [&](std::string data){
        ret = std::move(data);
    };
    return ret;
}

void ConfigurationManage::save_check_list_result(const nlohmann::json &value) {
    const std::string trace = value["checkSubmit"]["trace"];
    db_ << "UPDATE checks SET res = ?, timestamp = ? WHERE trace = ?;"
        << value.dump()
        << time(NULL)
        << trace;
}

std::string ConfigurationManage::get_check_list_result() {
    nlohmann::json res_value;
    db_ << "SELECT res, reported, response, timestamp FROM checks WHERE res is not null order by timestamp desc limit 1;"
        >> [&](std::string res, bool reported, bool response, uint32_t timestamp){
            if (!parse_json(res, res_value)){
                // TODO LOG ERROR
            }
            res_value["reported"] = reported;
            res_value["response"] = response;
            res_value["timestamp"] = timestamp;
    };
    return res_value.dump();
}

std::string ConfigurationManage::get_to_fix_check_list(){
    nlohmann::json res_value;
    db_ << "SELECT res, reported, response, timestamp FROM checks WHERE reported = 0 or response = 0;"
        >> [&](std::string res, bool reported, bool response, uint32_t timestamp){
            if (res.empty()){
                return;
            }
            nlohmann::json value;
            if (parse_json(res, value)){
                // TODO LOG ERROR
            }
            value["reported"] = reported;
            value["response"] = response;
            value["timestamp"] = timestamp;

            res_value["checkResults"].push_back(value);
        };
    return res_value.dump();
}

std::string ConfigurationManage::get_key_status(const nlohmann::json &value) {
    if (!value["keys"].is_null()) {
        error_code_ = INPUT_FORMAT_ERROR;
        // TODO LOG ERROR
    }
    if (!value["keys"].is_array()) {
        error_code_ = INPUT_FORMAT_ERROR;
        // TODO LOG ERROR
    }
    nlohmann::json res;
    const nlohmann::json &keys = value["keys"];
    for (int i = 0; i < keys.size(); ++i) {
        nlohmann::json res_key;
        KeyInfo key_info;
        key_info.key = keys[i]["key"];

        get_key_info(key_info);

        res_key["key"]  = key_info.key;
        if (key_info.state != "notExist"){
            res_key["result"] = key_info.value;
        }else{
            res_key["result"] = "notExist";
        }
        res["keys"].push_back(res_key);
    }
    return res.dump();
}

std::string ConfigurationManage::get_func_status(const nlohmann::json &value) {
    if (value.is_null() || !value.is_array()) {
        // TODO LOG ERROR
        error_code_ = INPUT_FORMAT_ERROR;
        return "";
    }

    nlohmann::json res;
    for (int i = 0; i < value.size(); ++i) {
        nlohmann::json res_func;
        FuncInfo func_info;
        func_info.funcID = value[i]["funcID"];

        get_func_info(func_info);

        res_func["funcID"] = func_info.funcID;
        res_func["state"] = func_info.state;
        res_func["expired"] = func_info.expired;
        res.push_back(res_func);
    }
    return res.dump();
}

std::string ConfigurationManage::get_conditions_value() {
    std::string ret;
    db_ << "SELECT data FROM conditions limit 1;"
        >> [&](std::string data){
        ret = std::move(data);
    };
    return ret;
}

void ConfigurationManage::update_conditions_value(const nlohmann::json &value) {
    db_ << "UPDATE conditions SET data = ? WHERE id = 1"
        << value.dump();
}

std::string ConfigurationManage::get_activate_conditions() {
    std::string strategy_set = get_strategy_set();
    nlohmann::json value;
    if (!parse_json(strategy_set, value)) {
        // TODO LOG ERROR
        return "";
    }
    if (value["activationCheck"].is_null()){
        // TODO LOG ERROR
        return "";
    }
    return value["activationCheck"].dump();
}

std::string ConfigurationManage::exist_key_test(const nlohmann::json &value) {
    if (value.is_null() || !value.is_array()) {
        // TODO LOG ERROR
        error_code_ = INPUT_FORMAT_ERROR;
        return "";
    }
    nlohmann::json res;
    for (int i = 0; i < value.size(); ++i) {
        nlohmann::json res_key;
        KeyInfo key_info;
        key_info.key = value[i]["key"];

        get_key_info(key_info);

        res_key["key"]  = key_info.key;
        if (key_info.state != "notExist"){
            res_key["exist"] = "live";
        }else{
            res_key["exist"] = "notExist";
        }
        res["keys"].push_back(res_key);
    }
    return res.dump();
}

void ConfigurationManage::update_check_reported() {
//    db_ << "UPDATE checks SET reported = true and timestamp = ? WHERE id = 1;"
//        << time(NULL);
}

void ConfigurationManage::update_check_response() {
//    db_ << "UPDATE checks SET response = true and timestamp = ? WHERE id = 1;"
//        << time(NULL);
}

std::string ConfigurationManage::get_abnormal_funcs() {
    nlohmann::json res_value;
    db_ << "SELECT funcID, state, expired  FROM funcsInGroup "
        >>[&](int funcID, std::string state, uint32_t expired){
            if (state.empty()){
                state = "abnormal";
            }else{
                if(state == "activated"){
                    if(expired < time(NULL)){
                        state = "expired";
                    }else{
                        state = "normal";
                    }
                }else{
                    state = "abnormal";
                }
            }
            if (state == "abnormal"){
                nlohmann::json value;
                value["funcID"] = funcID;
                value["state"]  = state;
                value["expired"] = expired;

                std::vector<std::string> key_list;
                get_keys_by_func_id(funcID, key_list);

                for(auto& key : key_list){
                    nlohmann::json res_key;
                    KeyInfo key_info;
                    key_info.key = key;

                    get_key_info(key_info);

                    res_key["key"]  = key_info.key;
                    res_key["result"] = key_info.value;

                    value["keys"].push_back(res_key);
                }
                res_value["abnormalFunc"].push_back(value);
            }
        };
    return res_value.is_null()? "" : res_value.dump();
}

std::string ConfigurationManage::get_expired_funcs() {
    nlohmann::json res_value;
    db_ << "SELECT funcID, state, expired  FROM funcsInGroup "
        >>[&](int funcID, std::string state, uint32_t expired){
            if (state.empty()){
                state = "abnormal";
            }else{
                if(state == "activated"){
                    if(expired < time(NULL)){
                        state = "expired";
                    }else{
                        state = "normal";
                    }
                }else{
                    state = "abnormal";
                }
            }
            if (state == "expired"){
                nlohmann::json value;
                value["funcID"] = funcID;
                value["state"]  = "abnormal";
                value["expired"] = expired;

                res_value["expiredFunc"].push_back(value);
            }
        };
    return res_value.is_null()? "" : res_value.dump();
}

void ConfigurationManage::create_tables() {
    static const char *tables[] ={
            /* 表 global */
            //|----------------------------------------------------------------------------|
            //|    id   |  default_preference  |  default_strategy  |   seed  | poweron_tm |
            //|---------------------------------------------------------------|------------|
            //| integer |           json       |           json     | integer |   integer  |
            //|---------------------------------------------------------------|------------|
            "create table if not exists global(id INTEGER primary key autoincrement not null,"
            "default_preference json not null, default_strategy json not null, seed INTEGER default 0,"
            "poweron_tm INTEGER default 0);",
            /* 表 strategySet */
            //|------------------------------|
            //|    id   |  data  | timestamp |
            //|------------------------------|
            //| integer |  json  |  integer  |
            //|------------------------------|
            "create table if not exists strategySet(id INTEGER primary key autoincrement not null,"
            "data json not null, timestamp INTEGER not null);",
            /* 表 platformSync */
            //|------------------------------------------|
            //|    id   | accountId |  data  | timestamp |
            //-------------------------------------------|
            //| integer |  integer  |  json  |  integer  |
            //|------------------------------------------|
            "create table if not exists platformSync(id INTEGER primary key autoincrement not null,"
            "accountId INTEGER not null, data json not null, timestamp INTEGER not null);",
            /* 表 activation */
            //|---------------------------------------------------------|
            //|    id   |       trace    |  data  |   res   | timestamp |
            //|---------------------------------------------------------|
            //| integer |   varchar(50)  |  json  |   json  |  integer  |
            //|---------------------------------------------------------|
            "create table if not exists activation(id INTEGER primary key autoincrement not null,"
            "trace varchar(50) not null, data json not null,  res json, timestamp INTEGER not null);",
            "CREATE UNIQUE INDEX IF NOT EXISTS activation_trace ON activation(trace);",
            /* 表 funcsInGroup */
            //|--------------------------------------------------------------------------------------------|
            //|    id   | funcID  |  action   | state | expired | counts |privateConditions|timestamp|
            //|--------------------------------------------------------------------------------------------|
            //| integer | integer |varchar(50)|varchar(50)| integer | integer |   varchar(50)  | integer |
            //|--------------------------------------------------------------------------------------------|
            "create table if not exists funcsInGroup(id INTEGER primary key autoincrement not null,"
            "funcID INTEGER not null, action varchar(50) not null, state varchar(50) not null, "
            "expired INTEGER not null, counts INTEGER not null, privateConditions varchar(50) not null,"
            "timestamp INTEGER not null);",
            /* 表 keys */
            //|----------------------------------------------------------------------|
            //|      key    |  data  |   state   |   ref   |   key_value | timestamp |
            //|----------------------------------------------------------------------|
            //| varchar(50) |  json  |varchar(50)| integer | varchar(50) |  integer  |
            //|----------------------------------------------------------------------|
            "create table if not exists keys(key varchar(50) primary key not null,"
            "data json not null, state varchar(50) not null, "
            "ref INTEGER default 0, key_value varchar(50),timestamp INTEGER not null);",
            /* 表 funcsInGroup_key */
            //|----------------------------------------------------|
            //|    id   | funcsInGroupID |     key     | timestamp |
            //|----------------------------------------------------|
            //| integer |     integer    | varchar(50) |  integer  |
            //|----------------------------------------------------|
            "create table if not exists funcsInGroup_key(id INTEGER primary key autoincrement not null,"
            "funcsInGroupID INTEGER not null,  key varchar(50) not null, timestamp INTEGER not null);",
            /* 表 depends */
            //|----------------------------------------------|
            //|    id   | funcInGroupID |  data | timestamp |
            //|----------------------------------------------|
            //| integer |     integer    |  json |  integer  |
            //|----------------------------------------------|
            "create table if not exists depends(id INTEGER primary key autoincrement not null,"
            "funcsInGroupID INTEGER not null,  data json not null, timestamp INTEGER not null);",
            "CREATE UNIQUE INDEX IF NOT EXISTS depends_funcsInGroupID ON depends(funcsInGroupID);",
            /* 表 account */
            //|---------------------------------------------------------------------|
            //|    id   |  tinyID   |   auID  |   state   | timestamp |   version   |
            //|---------------------------------------------------------------------|
            //| integer |varchar(50)| integer |  integer  |  integer  | varchar(50) |
            //|---------------------------------------------------------------------|
            "create table if not exists account(id INTEGER primary key autoincrement not null,"
            "tinyID varchar(50) not null, auID INTEGER,   state INTEGER not null, "
            "timestamp INTEGER not null, version varchar(50) not null);",
            /* 表 preference */
            //|-------------------------------------------|
            //|    id   | account_id |  data  | timestamp |
            //|-------------------------------------------|
            //| integer |   integer  |  json  |  integer  |
            //|-------------------------------------------|
            "create table if not exists preference(id INTEGER primary key autoincrement not null,"
            "account_id integer not null, data json not null,timestamp INTEGER not null);",
            "CREATE UNIQUE INDEX IF NOT EXISTS preference_account_id ON preference (account_id);",
            /* 表 param_version */
            //|--------------------------------------|
            //|    id   |    version    |  timestamp |
            //|--------------------------------------|
            //| integer |   varchar(50) |  integer   |
            //|--------------------------------------|
            "create table if not exists param_version(id INTEGER primary key autoincrement not null,"
            "version varchar(50) not null, timestamp INTEGER not null);",
            /* 表 checks */
            //|---------------------------------------------------------------------------|
            //|    id   |    trace    |  data  |  res   | reported | response | timestamp |
            //|---------------------------------------------------------------------------|
            //| integer | varchar(50) |  json  |  json  |   bool   |    bool  |  integer  |
            //|---------------------------------------------------------------------------|
            "create table if not exists checks(id INTEGER primary key autoincrement not null,"
            "trace varchar(50) not null, data json not null, res json, reported bool default 0, "
            "response bool default 0,timestamp INTEGER not null);",
            "CREATE UNIQUE INDEX IF NOT EXISTS checks_trace ON checks (trace);",
            /* 表 byFuncIDInfo */
            //|-------------------------------------------------------------|
            //|    id   | funcsInGroupID |    state   | expired | timestamp |
            //|-------------------------------------------------------------|
            //| integer |     integer    | varchar(50)| integer |  integer  |
            //|-------------------------------------------------------------|
            "create table if not exists byFuncIDInfo(id INTEGER primary key autoincrement not null,"
            "funcsInGroupID INTEGER not null, state varchar(50) not null, expired INTEGER not null,"
            " timestamp INTEGER not null);",
            /* 表 conditions */
            //|-------------------------------|
            //|    id   |  data  |  timestamp |
            //|-------------------------------|
            //| integer |  json  |   integer  |
            //|-------------------------------|
            "create table if not exists conditions(id INTEGER primary key autoincrement not null,"
            "data json not null, timestamp INTEGER not null);",
    };
    int count = sizeof(tables) / sizeof(tables[0]);
    for (int i = 0; i < count; ++i) {
        db_ << tables[i];
    }
    static const std::string default_pre = R"({
    "Params": [
        {
            "Domain":[
                {
                    "name":"CCU",
                    "version": "String",
                    "params":[
                        {
                            "name": "MR",
                            "version": "String",
                            "SystemParam":
                            {
                                "MR_MirrDipFunCfg": "0x3",
                                "MRReserved0": "0x3F",
                                "MR_LeftMirrorMemoryPosition_Y": "0xFF",
                                "MR_LeftMirrorMemoryPosition_X": "0xFF",
                                "MR_RightMirrorMemoryPosition_Y": "0xFF",
                                "MR_RightMirrorMemoryPosition_X": "0xFF",
                                "MR_LeftMirrorDripDownPosition_Y": "0x32",
                                "MR_LeftMirrorDripDownposition_X": "0xFF",
                                "MR_RightMirrorDripDownposition_Y": "0x32",
                                "MR_RightMirrorDripDownposition_X": "0xFF"
                            }
                        },
                        {
                            "name": "SSW",
                            "version": "String",
                            "SystemParam":
                            {
                                "SSW_FLSeatBackrestMotorPositionStatus": "0xFF",
                                "SSW_FLSeatHeightMotorPositionStatus": "0xFF",
                                "SSW_FLSeatSlide Motor PositionStatus": "0xFF",
                                "SSW_FLSeatTilt Motor Position Status": "0xFF",
                                "SSW_FLSeatCushionMotorPositionStatus": "0xFF",
                                "SSW_FLSeatOttomanMotorPositionStatus": "0xFF",
                                "SSWReserved0": "0xFF",
                                "SSWReserved1": "0xFF",
                                "SSW_SteeringAngleMotorPositionStatus": "0xFF",
                                "SSW_SteeringExtentMotorPositionStatus": "0xFF",
                                "SSW_FLSeatEasyEntryCfg": "0x0",
                                "SSWReserved2": "0x0"
                            }
                        },
                        {
                            "name": "IL",
                            "version": "String",
                            "SystemParam":
                            {
                                "IALUserEnCfg": "0x1",
                                "IALBreathingEnCfg": "0x0",
                                "IALFollowMusicEnCfg": "0x0",
                                "IALFollowDrivingModeEnCfg": "0x0",
                                "IALUserMode": "0x1",
                                "IALUserDefColor": "29",
                                "IALBrightnessLevel": "4",
                                "IALReserved1": "0x0"
                            }
                        }
                    ]
                }
            ]
        }
    ]
})";
    static const std::string strategySet = R"({
    "strategySet":{
        "trace":"DLL66GC5C6MB0000041656295595000",
        "timeout":-1,
        "checkFailedBeforeAcivation":"reportOnly",
        "checkFailedWhileActivating":"reportOnly",
        "afterFailInActivaiton":"reportOnly",
        "foundAbnormalInRoutinCheck":"reportOnly",
        "foundFuncExpired":"reportOnly",
        "activationCheck":[
            {"generalLocalSet":true},
            {"speed":[0,5]},
            {"gear":["P","D"]},
            {"vmm":["driving","standby","comfort"]},
            {"umm":["user","factory","transport"]}
        ],
        "routineCheck":[
                        {"period":3600},
                        {"powerFirstOn":true},
                        {"udsEvent":false},
                        {"autoBy":true}
                    ],
        "reportTiming":[
                        {"autoBy":true},
                        {"heartBeat":36000},
                        {"engineStart":true}
                    ],
        "fileURLPath":{
                         "offerTo":[
                            {"checkSubmit":"URL"},
                            {"activation":"URL"} ,
                            {"activationSubmit":"URL"}
                        ],
                         "demandFor":["checkSubmit","activationSubmit"]
                      },
        "securitySync":{
                    "secret":"for encryption"
                    },
        "checkAllScope":false,
        "timeoutGlobal": 600
    }
})";
    db_ << "INSERT OR REPLACE INTO global (id, default_preference, default_strategy) VALUES (?, ?, ?);"
        << 1
        << default_pre
        << strategySet;
}

bool ConfigurationManage::parse_json(const std::string &jsonStr, nlohmann::json &value) {
    value = nlohmann::json::parse(jsonStr);
    if (!value.is_null()) {
        return true;
    } else {
        return false;
    }
}

void ConfigurationManage::record_activation_info(const std::string &activationID) {
    std::string data;
    db_ << "SELECT data FROM activation WHERE trace = ?;"
        << activationID
        >> data;

    nlohmann::json value;
    if (!parse_json(data, value)) {
        //TODO LOG ERROR
        error_code_ = FATAL_ERROR;
        return;
    }

    if (value["activation"]["manyGroups"].is_null()) {
        error_code_ = FATAL_ERROR;
        // TODO LOG ERROR
        return;
    }

    if (!value["activation"]["manyGroups"].is_array()) {
        // TODO LOG ERROR
        return;
    }

    const nlohmann::json &many_groups = value["activation"]["manyGroups"];
    // manyGroups
    for (int i = 0; i < many_groups.size(); ++i) {
        const nlohmann::json &group = many_groups[i];
        // funcsInGroup
        if (!group["funcsInGroup"].is_null() && !group["funcsInGroup"].is_array()) {
            // TODO LOG ERROR
            continue;
        }

        const nlohmann::json &funcs = group["funcsInGroup"];
        for (int j = 0; j < funcs.size(); ++j) {
            const nlohmann::json &func = funcs[i];
            record_activation_func(func);

            // keys
            if (func["keys"].is_null()) {
                // TODO LOG ERROR
                continue;
            }
            if (!func["keys"].is_array()) {
                // TODO LOG ERROR
                continue;
            }

            int funcInGroupID = 0;
            db_ << "SELECT id FROM funcsInGroup WHERE funcID = ?;"
                << func["funcID"].get<int>()
                >> funcInGroupID;
            if (funcInGroupID <= 0) {
                //TODO LOG ERROR
            }

            const nlohmann::json &keys = func["keys"];
            for (int m = 0; m < keys.size(); ++m) {
                const nlohmann::json &key = keys[m];
                record_activation_key(funcInGroupID, key);
            }
        }
    }
}

void ConfigurationManage::record_activation_func(const nlohmann::json &func) {
    uint32_t funcID = func["funcID"];
    int count = 0;
    db_ << "SELECT count(*) FROM funcsInGroup WHERE funcID = ?;"
        << funcID
        >> count;

    std::string action = func["action"];
    uint64_t expired = func["expired"];
    std::string func_state = "inLine";
    uint32_t counts = func["counts"];
    std::string privateConditions = func["privateConditions"];
    if (count > 0) {
        db_ << "UPDATE funcsInGroup set  expired = ? WHERE funcID = ?;"
            << expired
            << funcID;
    } else {
        db_ << "INSERT INTO funcsInGroup (funcID, action, state, expired, counts, privateConditions, timestamp) "
               "VALUES (?, ?, ?, ?, ?, ?, ?);"
            << funcID
            << action
            << func_state
            << expired
            << counts
            << privateConditions
            << time(NULL);
    }

    int funcsInGroupID = 0;
    db_ << "SELECT id FROM funcsInGroup WHERE funcID = ?;"
        << funcID
        >> funcsInGroupID;
    if (funcsInGroupID <= 0) {
        //TODO LOG ERROR
        error_code_ = FATAL_ERROR;
        return;
    }
    // record depends
    db_ << "INSERT OR REPLACE INTO depends (funcsInGroupID, data, timestamp) VALUES (?, ?, ?);"
        << funcsInGroupID
        << func["depends"].dump()
        << time(NULL);
}

void ConfigurationManage::record_activation_key(int funcsInGroupID, const nlohmann::json &key) {
    std::string str_key = key["key"];
    int count = 0;
    db_ << "SELECT count(*) FROM keys WHERE key = ?;"
        << str_key
        >> count;

    std::string did = key["did"];
    std::string key_state = "inLine";

    if (count == 0) {
        db_ << "INSERT INTO keys (key, data, state ,timestamp) "
               "VALUES (?, ?, ?, ?);"
            << str_key
            << key.dump()
            << key_state
            << time(NULL);
    }

    count = 0;
    db_ << "SELECT count(*) FROM funcsInGroup_key WHERE funcsInGroupID = ? and key = ?;"
        << funcsInGroupID
        << str_key
        >> count;
    if (count == 0) {
        db_ << "INSERT INTO funcsInGroup_key (funcsInGroupID, key, timestamp) "
               "VALUES (?, ?, ?);"
            << funcsInGroupID
            << str_key
            << time(NULL);
    }
}

void ConfigurationManage::update_func_state(int funcId, const std::string &state) {
    db_ << "UPDATE funcsInGroup SET  state = ? WHERE funcID = ?;"
        << state
        << funcId;
}

void ConfigurationManage::update_key_state(const std::string& action, const std::string &key, const std::string &state) {
    if (state == "ok") {
        db_ << "SELECT ref FROM keys WHERE key = ?;"
            << key
            >>[&](int ref){
            if (action == "activate"){
                ref += 1;
            }else{
                ref -= 1;
            }
            std::string state = "activated";
            if (ref <= 0){
                ref = 0;
                state = "deactivate";
            }
            db_ << "UPDATE keys SET state = ? , ref = ? WHERE key = ?;"
                << state
                << ref
                << key;
        };
    }
}

void ConfigurationManage::get_key_info(KeyInfo& key_info){
    db_ << "SELECT state, did_value FROM keys WHERE key = ?"
        <<  key_info.key
        >>[&](std::string state, std::string did_value){
            key_info.state = std::move(state);
            key_info.value = std::move(did_value);
        };
    if( !key_info.state.empty()){
        key_info.state = "readSuccess";
    }else{
        key_info.state = "notExist";
    }
}

void ConfigurationManage::get_func_info(FuncInfo& func_info){
    db_ << "SELECT state, expired  FROM funcsInGroup WHERE funcID  = ?"
        <<  func_info.funcID
        >>[&](std::string state, uint32_t expired){
            func_info.state = std::move(state);
            func_info.expired = expired;
        };
    if( func_info.state.empty()){
        func_info.state = "abnormal";
    }else{
        if(func_info.state == "activated"){
            if (func_info.expired < time(NULL)){
                func_info.state = "expired";
            }else{
                func_info.state = "normal";
            }

        }else{
            func_info.state = "abnormal";
        }
    }
}

void ConfigurationManage::get_keys_ref(nlohmann::json& keys){
    db_ << "SELECT key, ref FROM keys;"
        >> [&](std::string key, int ref) {
            nlohmann::json value;
            value["key"] = std::move(key);
            value["ref"] = ref;

            keys.push_back(value);
        };
}

void ConfigurationManage::get_keys_by_func_id(int funcID, std::vector<std::string>& keys){
    int funcsInGroupID = 0;
    db_ << "SELECT id FROM funcsInGroup WHERE funcID = ?;"
        << funcID
        >> funcsInGroupID;
    if (funcsInGroupID <= 0) {
        //TODO LOG ERROR;
        return;
    }
    db_ << "SELECT key FROM funcsInGroup_key WHERE funcsInGroupID  = ?"
        <<  funcsInGroupID
        >>[&](std::string key){
            keys.push_back(std::move(key));
        };
}

void ConfigurationManage::set_func_info(int funcID, const char* state, uint32_t expired){
    db_ << "UPDATE funcsInGroup set state = ?, expired = ? WHERE funcID = ?;"
        << state
        << expired
        << funcID;
}