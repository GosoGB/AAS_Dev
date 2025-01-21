/**
 * @file NetworkValidator.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 네트워크에 대한 설정 정보가 유효한지 검사하는 클래스를 선언합니다.
 * 
 * @date 2024-10-07
 * @version 1.0.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <ArduinoJson.h>
#include <esp_wifi_types.h>
#include <IPAddress.h>
#include <IPv6Address.h>
#include <vector>
#include <WiFiSTA.h>

#include "Common/Status.h"
#include "JARVIS/Include/Base.h"
#include "JARVIS/Include/TypeDefinitions.h"



namespace muffin { namespace jvs {

    class NetworkValidator
    {
    public:
        NetworkValidator();
        virtual ~NetworkValidator();
    private:
        using cin_vector = std::vector<config::Base*>;
    public:
        std::pair<rsc_e, std::string> Inspect(const cfg_key_e key, const JsonArray arrayCIN, cin_vector* outVector);
    private:
        std::pair<rsc_e, std::string> validateEthernet(const JsonArray array, cin_vector* outVector);
        rsc_e validateMandatoryKeysEthernet(const JsonObject json);
        rsc_e validateMandatoryValuesEthernet(const JsonObject json);
    private:
        std::pair<rsc_e, std::string> validateWiFi4(const JsonArray array, cin_vector* outVector);
        rsc_e validateMandatoryKeysWiFi4(const JsonObject json);
        rsc_e validateMandatoryValuesWiFi4(const JsonObject json);
    private:
        rsc_e emplaceCIN(config::Base* cin, cin_vector* outVector);
    private:
        std::pair<rsc_e, bool> convertToDHCP(JsonVariant dhcp);
        std::pair<rsc_e, bool> convertToEAP(JsonVariant eap);
        std::pair<rsc_e, wifi_auth_mode_t> convertToAuth(JsonVariant auth);
        std::pair<rsc_e, wpa2_auth_method_t> convertToWpaAuth(JsonVariant wpaAuth);
        std::pair<rsc_e, std::string> convertToSSID(JsonVariant ssid);
        std::pair<rsc_e, std::string> convertToPSK(JsonVariant psk);
        std::pair<rsc_e, std::string> convertToEapID(JsonVariant id);
        std::pair<rsc_e, std::string> convertToEapUser(JsonVariant user);
        std::pair<rsc_e, std::string> convertToEapPassword(JsonVariant pass);
        std::pair<rsc_e, std::string> convertToCaCertificate(JsonVariant caCert);
        std::pair<rsc_e, std::string> convertToClientCertificate(JsonVariant crt);
        std::pair<rsc_e, std::string> convertToClientKey(JsonVariant key);
        std::pair<rsc_e, IPAddress> convertToIPv4(JsonVariant ip, const bool& isSubnetmask);
   
     
    };
}}