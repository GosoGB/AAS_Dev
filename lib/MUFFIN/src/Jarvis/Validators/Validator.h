/**
 * @file Validator.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief JARVIS 설정 정보의 유효성을 검사하기 위한 모듈 클래스를 선언합니다.
 * 
 * @date 2024-10-06
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <ArduinoJson.h>
#include <map>
#include <set>
#include <vector>

#include "Common/Status.h"
#include "Jarvis/Include/Base.h"
#include "Jarvis/Include/TypeDefinitions.h"



namespace muffin { namespace jarvis {

    class Validator
    {
    public:
        Validator();
        virtual ~Validator();
    private:
        using cin_vector = std::vector<config::Base*>;
    public:
        Status Inspect(const JsonDocument& jsonDocument, std::map<cfg_key_e, cin_vector>* mapCIN);
    private:
        Status emplacePairsForCIN(std::map<cfg_key_e, cin_vector>* mapCIN);
        Status validateMetaData(const JsonObject json);
        Status validateSerialPort(const cfg_key_e key, const JsonArray json, cin_vector* outputVector);
    private:
        /*Metadata*/
        prtcl_ver_e mProtocolVersion;
        std::string mRequestIdentifier;
        std::set<cfg_key_e> mContainerKeySet;




    private:
        Status VailidateRs232(JsonPair& pair);
        Status VailidateRs485(JsonPair& pair);
        Status VailidateWIFI(JsonPair& pair);
        Status VailidateETH(JsonPair& pair);
        Status VailidateLTE(JsonPair& pair);
        Status VailidateModbusRTU(JsonPair& pair);
        Status VailidateModbusTCP(JsonPair& pair);
        Status VailidateOperation(JsonPair& pair);
        Status VailidateNode(JsonPair& pair);
        Status VailidateAlarm(JsonPair& pair);
        Status VailidateOptime(JsonPair& pair);
        Status VailidateProduction(JsonPair& pair);
    private:
        Status ValidateIPv4Address(const std::string& ipv4, const bool& isSubnetmask);
        Status ValidateModbusNodeCondition(JsonObject& node);
    
    private:/*Config Instance Container*/
        JsonObject mCnt;
    private:
        bool mHasPortNumber02 = false;
    #ifndef MODLINK_L
        bool mHasPortNumber03 = false;    
    #endif
    };
}}