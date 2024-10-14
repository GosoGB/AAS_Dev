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
        std::pair<Status, bool> convertToDHCP(JsonVariant dhcp);
        std::pair<Status, bool> convertToEAP(JsonVariant eap);
        std::pair<Status, wifi_auth_mode_t> convertToAuth(JsonVariant auth);
        std::pair<Status, wpa2_auth_method_t> convertToWpaAuth(JsonVariant wpaAuth);
        std::pair<Status, std::string> convertToSSID(JsonVariant ssid);
        std::pair<Status, std::string> convertToPSK(JsonVariant psk);
        std::pair<Status, std::string> convertToEapID(JsonVariant id);
        std::pair<Status, std::string> convertToEapUser(JsonVariant user);
        std::pair<Status, std::string> convertToEapPassword(JsonVariant pass);
        std::pair<Status, std::string> convertToCaCertificate(JsonVariant caCert);
        std::pair<Status, std::string> convertToClientCertificate(JsonVariant crt);
        std::pair<Status, std::string> convertToClientKey(JsonVariant key);
        std::pair<Status, IPAddress> convertToIPv4(JsonVariant ip, const bool& isSubnetmask);
   
     
    };
}}