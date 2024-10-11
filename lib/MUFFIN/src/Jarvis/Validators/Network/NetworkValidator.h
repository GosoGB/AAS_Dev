/**
 * @file NetworkValidator.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 네트워크에 대한 설정 정보가 유효한지 검사하는 클래스를 선언합니다.
 * 
 * @date 2024-10-07
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <ArduinoJson.h>
#include <vector>

#include "Common/Status.h"
#include "Jarvis/Include/Base.h"
#include "Jarvis/Include/TypeDefinitions.h"



namespace muffin { namespace jarvis {

    class NetworkValidator
    {
    public:
        NetworkValidator();
        virtual ~NetworkValidator();
    private:
        using cin_vector = std::vector<config::Base*>;
    public:
        Status Inspect(const cfg_key_e key, const JsonArray arrayCIN, cin_vector* outVector);
    private:
        Status validateLteCatM1(const JsonArray array, cin_vector* outVector);
        Status validateMandatoryKeysLteCatM1(const JsonObject json);
        Status validateMandatoryValuesLteCatM1(const JsonObject json);
    private:
        Status validateEthernet(const JsonArray array, cin_vector* outVector);
        Status validateMandatoryKeysEthernet(const JsonObject json);
        Status validateMandatoryValuesEthernet(const JsonObject json);
    private:
        Status validateWiFi4(const JsonArray array, cin_vector* outVector);
        Status validateMandatoryKeysWiFi4(const JsonObject json);
        Status validateMandatoryValuesWiFi4(const JsonObject json);
    private:
        Status emplaceCIN(config::Base* cin, cin_vector* outVector);
    private:
        std::pair<Status, md_e> convertToLteModel(const std::string model);
        std::pair<Status, ctry_e> convertToLteCountry(const std::string country);
    private:
        std::pair<Status, wifi_auth_mode_t> convertToAuth(const uint8_t auth);
        std::pair<Status, wpa2_auth_method_t> convertToEapAuth(const uint8_t eapAuth);
        std::pair<Status, IPAddress> convertToIPv4(const std::string ip, const bool& isSubnetmask);
   
     
    };
}}