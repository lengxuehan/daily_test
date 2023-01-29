//
// Created by wuting.xu on 2023/1/11.
//

#ifndef CONFIGURATIONMANAGE_UDS_H
#define CONFIGURATIONMANAGE_UDS_H

#include <unistd.h>
#include <netinet/in.h>

/*
 "keys": [{
            "activateValue": "0x01",
            "deactivateValue": "0x00",
            "did": "0x0101",
            "ecuid": "0x70E",
            "key": "d000004",
            "ref": 0,
            "spbit": 43,
            "spbyte": 6,
            "stbit": 43,
            "stbyte": 6,
            "type": "did"
 }]
 */

//按照16比特处理的，给的bit标记也刚好是16个，然后activateValue转成一个uint16，填充过去
int fill_did_data_bits() {
    uint8_t  st_byte = 5;
    uint8_t  st_bit  = 3;
    uint8_t  sp_byte = 7;
    uint8_t  sp_bit  = 4;
    uint16_t u_value   = 1;


    std::vector<uint8_t> did_data = {1, 1, 1 ,1, 1, 1, 1};
    --st_byte;
    --sp_byte;
    // copy 'activateValue' or 'deactivateValue' to the specified bits of did_data
    uint8_t  tmp = did_data[st_byte];
    uint8_t  left  = tmp >> (8 - st_bit);
    uint8_t  right =  u_value >> (16 - st_bit);
    tmp = left << (8 - st_bit) | right;
    did_data[st_byte++] = tmp;
    u_value = u_value << (st_bit + 1);
    for (; st_byte < sp_byte; ++st_byte){
        tmp =  u_value >> 8;
        did_data[st_byte] = tmp;
        u_value = u_value << 8;
    }
    tmp = did_data[sp_byte];
    right = (tmp << sp_bit) >> sp_bit;
    left  = (u_value >> (16  - sp_bit));
    tmp =  left << sp_bit | right;
    did_data[sp_byte] = tmp;

    st_byte = 5;
    sp_byte = 7;
    --st_byte;
    --sp_byte;
    // cannot compare the total bytes in case other key bits changed in concurrence
    std::vector<uint8_t>& src = did_data;
    uint16_t did_value = 0;
    tmp = src[st_byte++];
    tmp = (tmp << st_bit) >> st_bit;
    did_value = tmp << (8 + 8 - st_bit);
    for (; st_byte < sp_byte; ++st_byte){
        tmp = src[st_byte];
        uint16_t  value = tmp;
        did_value = did_value  | (value << (8 - st_bit)) ;
    }
    tmp = src[sp_byte];
    tmp = tmp >> (8 - sp_bit);
    did_value = did_value | tmp;

    std::cout << did_value << std::endl;
    return 0;
}

bool check_did_bits_mark(const std::vector<uint8_t>& did_record, uint8_t  st_bit, uint8_t  sp_bit){

    uint8_t  st_byte = 0, sp_byte = 0;
    st_byte = st_bit / 8;
    sp_byte = sp_bit / 8;
    st_bit = st_bit % 8;
    sp_bit = sp_bit % 8;

    std::cout << "stbit:" << static_cast<unsigned int>(st_bit) << " spbit:" << static_cast<unsigned int>(sp_bit)
              << " st_byte:" << static_cast<unsigned int>(st_byte) << " sp_byte:" << static_cast<unsigned int>(sp_byte) << std::endl;

    if (st_bit > 7 || sp_bit > 7 ){
        std::cout <<" did bit index bigger than 7." << std::endl;
        return false;
    }

    if (st_byte > sp_byte){
        std::cout << __func__ <<" did start byte must little than stop byte." << std::endl;
        return false;
    }

    if (did_record.size() < sp_byte){
        std::cout << __func__ <<" did record size too short record. size:"<< did_record.size()
                            << "sp_byte:" << sp_byte;
        return false;
    }

    return true;
}

int compare_did_data_bits( std::vector<uint8_t>& src, uint8_t  st_bit, uint8_t  sp_bit, uint32_t expect_did_value){
    for(uint index = st_bit; index <= sp_bit; ++index){
        uint8_t expect_bit_value = expect_did_value & 1;
        expect_did_value = expect_did_value >> 1;

        uint8_t  byte = 0, bit = 0;
        byte = index / 8;
        bit = index % 8;
        uint8_t u_num = src[byte];
        uint8_t bit_value = u_num & (1 << bit);
        bit_value = (bit_value >> bit) & 1;
        std::cout <<"expect_bit_value:" <<  static_cast<unsigned int>(expect_bit_value)
            << " bit_value:" <<  static_cast<unsigned int>(bit_value) << std::endl;
        if(bit_value != expect_bit_value){
            return -1;
        }
    }

    return 0;
}


void fill_did_data_bits1(uint8_t  st_bit, uint8_t  sp_bit, uint32_t u_action_value) {
    uint32_t u_tmp_action_value = u_action_value;
    union test {
        int a;
        char b;
    } c;
    c.a = 1;
    bool is_little = c.b & 1;
    std::cout << ( is_little? "little-endian" : "big-endian") << std::endl;

    std::vector<uint8_t> did_data = {1, 1, 1 ,1, 1, 1, 1};

    std::cout << "init: ";
    for(auto u : did_data){
        std::cout << static_cast<unsigned int>(u) << " , ";
    }
    std::cout << std::endl;

    bool b_res = check_did_bits_mark(did_data, st_bit, sp_bit);
    if (!b_res){
        return;
    }
    if(is_little){
        //u_action_value = htonl(u_action_value);
        //std::cout << "little-endian u_action_value:"<< u_action_value << std::endl;
    }

    // 填充 00001
    // 47（0）46（0）45（0）44（0）43（1）
    for(uint index = st_bit; index <= sp_bit; ++index){
        uint8_t bit_value = u_action_value & 1;
        u_action_value = u_action_value >> 1;


        uint8_t  byte = 0, bit = 0;
        byte = index / 8;
        bit = index % 8;
        uint8_t u_num = did_data[byte];
        if(bit_value){ // 比特位赋值1
            u_num = u_num | (1 << bit);
        }else{ // 比特位赋值0
            u_num = u_num & ~(1 << bit);
        }
        did_data[byte] = u_num;
    }

    std::cout << "filed: ";
    for(auto u : did_data){
        std::cout << static_cast<unsigned int>(u) << " , ";
    }
    //std::cout << std::endl;
//    did_data[5] = 1;
//    for(auto u : did_data){
//        std::cout << static_cast<unsigned int>(u) << " , ";
//    }
    //std::cout << std::endl;
    int res = compare_did_data_bits(did_data, st_bit, sp_bit, u_tmp_action_value);
    std::cout << "compare_did_data_bits:" << res << std::endl;
}

void test_uds_activate(){
    // 填充 00001
    // 47（0）46（0）45（0）44（0）43（1）
    //std::cout << "sample 1: st_bit 43  sp_bit 43  action_value 1" << std::endl;
    //fill_did_data_bits1(43,43,1);

    // st_bit 40 到 sp_bit 50 填充 0x133  ->  100110011
    //-----st_byte 5-----------|-------st_byte 7---- ---|
    // 47 46 45 44 43 42 41 40 | 55 54 53 52 51 50 49 48|
    // 0   0  1  1  0  0  1  1 |                 0  0  1|
    std::cout  << std::endl << "sample 1: st_bit 40  sp_bit 50  action_value 0x133" << std::endl;
    fill_did_data_bits1(40,50,0x233);
}
#endif //CONFIGURATIONMANAGE_UDS_H
