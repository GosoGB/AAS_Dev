/**
 * @file NetworkValidator.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 네트워크에 대한 설정 정보가 유효한지 검사하는 클래스를 정의합니다.
 * 
 * @date 2024-10-07
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */



#include <regex>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Jarvis/Config/Network/CatM1.h"
#include "Jarvis/Config/Network/Ethernet.h"
#include "Jarvis/Config/Network/WiFi4.h"
#include "Jarvis/Include/Helper.h"
#include "NetworkValidator.h"



namespace muffin { namespace jarvis {

    NetworkValidator::NetworkValidator()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    NetworkValidator::~NetworkValidator()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    Status NetworkValidator::Inspect(const cfg_key_e key, const JsonArray arrayCIN, cin_vector* outVector)
    {
        ASSERT((outVector != nullptr), "OUTPUT PARAMETER <outVector> CANNOT BE A NULL POINTER");
        ASSERT((arrayCIN.isNull() == false), "OUTPUT PARAMETER <arrayCIN> CANNOT BE NULL");

        Status ret(Status::Code::UNCERTAIN);

        switch (key)
        {
        case cfg_key_e::ETHERNET:
            ret = validateEthernet(arrayCIN, outVector);
            break;
        case cfg_key_e::WIFI4:
            ret = validateWiFi4(arrayCIN, outVector);
            break;
        default:
            ASSERT(false, "UNDEFINED NETWORK CONFIGURATION");
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        };

        return ret;
    }

    Status NetworkValidator::validateWiFi4(const JsonArray array, cin_vector* outVector)
    {
        if (array.size() != 1)
        {
            LOG_ERROR(logger, "INVALID WIFI CONFIG: ONLY ONE WIFI MODULE CAN BE CONFIGURED");
            return Status(Status::Code::BAD_NOT_SUPPORTED);
        }

        JsonObject cin = array[0];
        Status ret = validateMandatoryKeysWiFi4(cin);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "INVALID WIFI: MANDATORY KEY CANNOT BE MISSING");
            return ret;
        }

        ret = validateMandatoryValuesWiFi4(cin);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "INVALID WIFI: MANDATORY KEY'S VALUE CANNOT BE NULL");
            return ret;
        }

        const bool DHCP = cin["dhcp"].as<bool>();
        const bool EAP  = cin["eap"].as<bool>();

        const auto retAUTH = convertToAuth(cin["auth"].as<JsonVariant>());
        if (retAUTH.first.ToCode() != Status::Code::GOOD &&
            retAUTH.first.ToCode() != Status::Code::GOOD_NO_DATA)
        {
            LOG_ERROR(logger, "INVALID WIFI AUTH: %s", retAUTH.first.c_str());
            return retAUTH.first;
        }

        const auto retWpaAUTH = convertToWpaAuth(cin["wpa2auth"].as<JsonVariant>());
        if (retWpaAUTH.first.ToCode() != Status::Code::GOOD &&
            retWpaAUTH.first.ToCode() != Status::Code::GOOD_NO_DATA)
        {
            LOG_ERROR(logger, "INVALID WIFI EAP AUTH: %s", retWpaAUTH.first.c_str());
            return retWpaAUTH.first;
        }

        const auto retSSID = convertToSSID(cin["ssid"].as<JsonVariant>());
        if (retSSID.first.ToCode() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "INVALID WIFI SSID: %s", retAUTH.first.c_str());
            return retAUTH.first;
        }

        const auto retPSK = convertToPSK(cin["psk"].as<JsonVariant>());
        if (retPSK.first.ToCode() != Status::Code::GOOD &&
            retPSK.first.ToCode() != Status::Code::GOOD_NO_DATA)
        {
            LOG_ERROR(logger, "INVALID WIFI PSK: %s", retPSK.first.c_str());
            return retPSK.first;
        }

        const auto retEapID = convertToEapID(cin["id"].as<JsonVariant>());
        if (retEapID.first.ToCode() != Status::Code::GOOD &&
            retEapID.first.ToCode() != Status::Code::GOOD_NO_DATA)
        {
            LOG_ERROR(logger, "INVALID WIFI EAP ID: %s", retEapID.first.c_str());
            return retEapID.first;
        }

        const auto retEapUser = convertToEapUser(cin["user"].as<JsonVariant>());
        if (retEapUser.first.ToCode() != Status::Code::GOOD &&
            retEapUser.first.ToCode() != Status::Code::GOOD_NO_DATA)
        {
            LOG_ERROR(logger, "INVALID WIFI EAP USER: %s", retEapUser.first.c_str());
            return retEapUser.first;
        }

        const auto retEapPassword = convertToEapPassword(cin["pass"].as<JsonVariant>());
        if (retEapPassword.first.ToCode() != Status::Code::GOOD &&
            retEapPassword.first.ToCode() != Status::Code::GOOD_NO_DATA)
        {
            LOG_ERROR(logger, "INVALID WIFI EAP PASSWORD %s", retEapPassword.first.c_str());
            return retEapPassword.first;
        }

        const auto retCaCertificate = convertToCaCertificate(cin["ca_cert"].as<JsonVariant>());
        if (retCaCertificate.first.ToCode() != Status::Code::GOOD &&
            retCaCertificate.first.ToCode() != Status::Code::GOOD_NO_DATA)
        {
            LOG_ERROR(logger, "INVALID WIFI CA CERTIFICATE %s", retCaCertificate.first.c_str());
            return retCaCertificate.first;
        }

        const auto retClientCertificate = convertToClientCertificate(cin["crt"].as<JsonVariant>());
        if (retClientCertificate.first.ToCode() != Status::Code::GOOD &&
            retClientCertificate.first.ToCode() != Status::Code::GOOD_NO_DATA)
        {
            LOG_ERROR(logger, "INVALID WIFI CLIENT CERTIFICATE %s", retClientCertificate.first.c_str());
            return retClientCertificate.first;
        }

        const auto retClientKey = convertToClientKey(cin["key"].as<JsonVariant>());
        if (retClientKey.first.ToCode() != Status::Code::GOOD &&
            retClientKey.first.ToCode() != Status::Code::GOOD_NO_DATA)
        {
            LOG_ERROR(logger, "INVALID WIFI CLIENT KEY %s", retClientKey.first.c_str());
            return retClientKey.first;
        }

        config::WiFi4* wifi4 = new (std::nothrow) config::WiFi4();
        if (wifi4 == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR WIFI REFERENCES");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }
        
        if (DHCP == false)
        {
            const auto retIP    = convertToIPv4(cin["ip"].as<JsonVariant>(),false);
            const auto retSVM   = convertToIPv4(cin["snm"].as<JsonVariant>(),true);
            const auto retGTW   = convertToIPv4(cin["gtw"].as<JsonVariant>(),false);
            const auto retDNS1  = convertToIPv4(cin["dns1"].as<JsonVariant>(),false);
            const auto retDNS2  = convertToIPv4(cin["dns2"].as<JsonVariant>(),false);

            if (retIP.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID WIFI IP %s", retIP.first.c_str());
                return retIP.first;
            }

            if (retSVM.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID WIFI Subnetmask: %s",  retSVM.first.c_str());
                return retSVM.first;
            }

            if (retGTW.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID WIFI Gateway: %s",  retGTW.first.c_str());
                return retGTW.first;
            }

            if (retDNS1.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID WIFI DNS1 %s",  retDNS1.first.c_str());
                return retDNS1.first;
            }

            if (retDNS2.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID WIFI DNS2 %s", retDNS2.first.c_str());
                return retDNS2.first;
            }

            wifi4->SetStaticIPv4(retIP.second);
            wifi4->SetSubnetmask(retSVM.second);
            wifi4->SetGateway(retGTW.second);
            wifi4->SetDNS1(retDNS1.second);
            wifi4->SetDNS2(retDNS2.second);
        }
        
        wifi4->SetDHCP(DHCP);
        wifi4->SetEAP(EAP);
        wifi4->SetSSID(retSSID.second);

        if (retAUTH.first.ToCode() == Status::Code::GOOD)
        {
            wifi4->SetAuthMode(retAUTH.second);
        }

        if (retWpaAUTH.first.ToCode() == Status::Code::GOOD)
        {
            wifi4->SetEapAuthMode(retWpaAUTH.second);
        }

        if (retPSK.first.ToCode() == Status::Code::GOOD)
        {
            wifi4->SetPSK(retPSK.second);
        }

        if (retEapID.first.ToCode() == Status::Code::GOOD)
        {
            wifi4->SetEapID(retEapID.second);
        }

        if (retEapUser.first.ToCode() == Status::Code::GOOD)
        {
            wifi4->SetEapUserName(retEapUser.second);
        }
        
        if (retEapPassword.first.ToCode() == Status::Code::GOOD)
        {
            wifi4->SetEapPassword(retEapPassword.second);
        }

        if (retCaCertificate.first.ToCode() == Status::Code::GOOD)
        {
            wifi4->SetEapCaCertificate(retCaCertificate.second);
        }

        if (retClientCertificate.first.ToCode() == Status::Code::GOOD)
        {
            wifi4->SetEapClientCertificate(retClientCertificate.second);
        }

        if (retClientKey.first.ToCode() == Status::Code::GOOD)
        {
            wifi4->SetEapClientKey(retClientKey.second);
        }

        ret = emplaceCIN(static_cast<config::Base*>(wifi4), outVector);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO EMPLACE CONFIG INSTANCE: %s", ret.c_str());
            return ret;
        }

        LOG_VERBOSE(logger, "Valid WiFi4 config instance")
        return Status(Status::Code::GOOD);
    }

    Status NetworkValidator::validateMandatoryKeysWiFi4(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json.containsKey("dhcp");
        isValid &= json.containsKey("ip");
        isValid &= json.containsKey("snm");     
        isValid &= json.containsKey("gtw");    
        isValid &= json.containsKey("dns1");    
        isValid &= json.containsKey("dns2");   
        isValid &= json.containsKey("ssid");    
        isValid &= json.containsKey("psk");     
        isValid &= json.containsKey("auth");    
        isValid &= json.containsKey("eap");     
        isValid &= json.containsKey("wpa2auth");
        isValid &= json.containsKey("id");      
        isValid &= json.containsKey("user");    
        isValid &= json.containsKey("pass");    
        isValid &= json.containsKey("ca_cert"); 
        isValid &= json.containsKey("crt");     
        isValid &= json.containsKey("key");     

        if (isValid == true)
        {
            return Status(Status::Code::GOOD);
        }
        else
        {
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }
    }

    Status NetworkValidator::validateMandatoryValuesWiFi4(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json["dhcp"].isNull() == false;
        isValid &= json["ssid"].isNull() == false;      
        isValid &= json["eap"].isNull()  == false;
        isValid &= json["dhcp"].is<bool>();
        isValid &= json["ssid"].is<std::string>();
        isValid &= json["eap"].is<bool>();

        if (isValid == false)
        {
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }
    
        bool DHCP = json["dhcp"].as<bool>();

        // DHCP가 아닌 경우에는 IP정보가 필수로 입력 되어야 함
        if (!DHCP) 
        {
            isValid &= json["ip"].isNull() == false;
            isValid &= json["snm"].isNull() == false;
            isValid &= json["gtw"].isNull() == false;
            isValid &= json["dns1"].isNull() == false;
            isValid &= json["dns2"].isNull() == false;
            isValid &= json["ip"].is<std::string>();
            isValid &= json["snm"].is<std::string>();
            isValid &= json["gtw"].is<std::string>();
            isValid &= json["dns1"].is<std::string>();
            isValid &= json["dns2"].is<std::string>();

            if (isValid == false)
            {
                LOG_ERROR(logger, "WIFI : STATIC IP SETTING ERROR");
                return Status(Status::Code::BAD_ENCODING_ERROR);
            }

        }

          // auth가 null이 아니면 uint8_t 타입인지 검사
        if (json["auth"].isNull() == false)
        {
            if (json["auth"].is<uint8_t>() == false)
            {
                LOG_ERROR(logger, "'AUTH' IS NOT OF TYPE UNSIGNED INT 8");
                return Status(Status::Code::BAD_ENCODING_ERROR);
            }

            bool EAP = json["eap"].as<bool>();
            uint8_t Auth = json["auth"].as<uint8_t>();

            // AUTH값이 0이 아니고 EAP 값이 FALSE일때만 PSK 입력 필요// EAP가 TURE이면 PSK가 들어와도 사용하지 않기 때문에 우선 ERROR 처리하지 않음
            if (Auth != 0 && EAP == false)
            {
                isValid &= json["psk"].isNull() == false;
                isValid &= json["psk"].is<std::string>();
            }

            if (isValid == false)
            {
                LOG_ERROR(logger, "AUTH SETTING ERROR, CHECK PSK");
                return Status(Status::Code::BAD_ENCODING_ERROR);
            }
        }

        if (json["wpa2auth"].isNull() == false)
        {
            if (json["wpa2auth"].is<uint8_t>() == false)
            {
                LOG_ERROR(logger, "'EAP AUTH' IS NOT OF TYPE UNSIGNED INT 8");
                return Status(Status::Code::BAD_ENCODING_ERROR);
            }

            // WPA2AUTH 값이 0이 아닌 경우 EAP ID, EAP USERNAME, EAP PASSWORD 입력 필요
            uint8_t Wpa2Auth = json["wpa2auth"].as<uint8_t>();
            if (Wpa2Auth == 0)
            {
                isValid &= json["crt"].isNull() == false;
                isValid &= json["key"].isNull() == false;
                isValid &= json["crt"].is<std::string>();
                isValid &= json["key"].is<std::string>();

                if (isValid == false)
                {
                    LOG_ERROR(logger, "EAP AUTH SETTING ERROR, CHECK CLIENT CERTIFICATE,KEY");
                    return Status(Status::Code::BAD_ENCODING_ERROR);
                }
            }
            else
            {
                isValid &= json["id"].isNull() == false;
                isValid &= json["user"].isNull() == false;
                isValid &= json["pass"].isNull() == false;
                isValid &= json["id"].is<std::string>();
                isValid &= json["user"].is<std::string>();
                isValid &= json["pass"].is<std::string>();

                if (isValid == false)
                {
                    LOG_ERROR(logger, "EAP AUTH SETTING ERROR, CHECK EAP ID, USER, PASSWORD");
                    return Status(Status::Code::BAD_ENCODING_ERROR);
                }
            }
        }
        
        if (isValid == true)
        {
            return Status(Status::Code::GOOD);
        }
        else
        {
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }
    }

    Status NetworkValidator::validateEthernet(const JsonArray array, cin_vector* outVector)
    {
        if (array.size() != 1)
        {
            LOG_ERROR(logger, "INVALID ETHERNET CONFIG: ONLY ONE ETHERNET MODULE CAN BE CONFIGURED");
            ASSERT((array.size() == 1), "ETHERNET CONFIG CANNOT BE GREATER THAN 1");
            return Status(Status::Code::BAD_NOT_SUPPORTED);
        }

        JsonObject cin = array[0];
        Status ret = validateMandatoryKeysEthernet(cin);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "INVALID ETHERNET: MANDATORY KEY CANNOT BE MISSING");
            return ret;
        }

        ret = validateMandatoryValuesEthernet(cin);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "INVALID ETHERNET: MANDATORY KEY'S VALUE CANNOT BE NULL");
            return ret;
        }
        
        const bool DHCP = cin["dhcp"].as<bool>();
        
        config::Ethernet* ethernet = new(std::nothrow) config::Ethernet();
        if (ethernet == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR CIN: ETHERNET");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }
        
        ethernet->SetDHCP(DHCP);

        if (DHCP == false)
        {
            const auto retIP    = convertToIPv4(cin["ip"].as<JsonVariant>(),false);
            const auto retSVM   = convertToIPv4(cin["snm"].as<JsonVariant>(),true);
            const auto retGTW   = convertToIPv4(cin["gtw"].as<JsonVariant>(),false);
            const auto retDNS1  = convertToIPv4(cin["dns1"].as<JsonVariant>(),false);
            const auto retDNS2  = convertToIPv4(cin["dns2"].as<JsonVariant>(),false);

            if (retIP.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID ETHERNET IP %s", retIP.first.c_str());
                return retIP.first;
            }

            if (retSVM.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID ETHERNET SUBNETMASK: %s",  retSVM.first.c_str());
                return retSVM.first;
            }

            if (retGTW.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID ETHERNET GATEWAY: %s",  retGTW.first.c_str());
                return retGTW.first;
            }

            if (retDNS1.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID ETHERNET DNS1 %s",  retDNS1.first.c_str());
                return retDNS1.first;
            }

            if (retDNS2.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID ETHERNET DNS2 %s", retDNS2.first.c_str());
                return retDNS2.first;
            }

            ethernet->SetStaticIPv4(retIP.second);
            ethernet->SetSubnetmask(retSVM.second);
            ethernet->SetGateway(retGTW.second);
            ethernet->SetDNS1(retDNS1.second);
            ethernet->SetDNS2(retDNS2.second);
        }
    

        ret = emplaceCIN(static_cast<config::Base*>(ethernet), outVector);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO EMPLACE CONFIG INSTANCE: %s", ret.c_str());
            return ret;
        }

        LOG_VERBOSE(logger, "Valid ethernet config instance")
        return Status(Status::Code::GOOD);
    }

    Status NetworkValidator::validateMandatoryKeysEthernet(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json.containsKey("dhcp");
        isValid &= json.containsKey("ip");
        isValid &= json.containsKey("snm");     
        isValid &= json.containsKey("gtw");    
        isValid &= json.containsKey("dns1");    
        isValid &= json.containsKey("dns2");  

        if (isValid == true)
        {
            return Status(Status::Code::GOOD);
        }
        else
        {
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }
    }

    Status NetworkValidator::validateMandatoryValuesEthernet(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json["dhcp"].isNull() == false;
        isValid &= json["dhcp"].is<bool>();

        if (isValid == false)
        {
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }
        
        bool DHCP = json["dhcp"].as<bool>();

        // DHCP가 아닌 경우에는 IP정보가 필수로 입력 되어야 함
        if (!DHCP) 
        {
            isValid &= json["ip"].isNull() == false;
            isValid &= json["snm"].isNull() == false;
            isValid &= json["gtw"].isNull() == false;
            isValid &= json["dns1"].isNull() == false;
            isValid &= json["dns2"].isNull() == false;
            isValid &= json["ip"].is<std::string>();
            isValid &= json["snm"].is<std::string>();
            isValid &= json["gtw"].is<std::string>();
            isValid &= json["dns1"].is<std::string>();
            isValid &= json["dns2"].is<std::string>();
        }

        if (isValid == true)
        {
            return Status(Status::Code::GOOD);
        }
        else
        {
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }
    }

    Status NetworkValidator::emplaceCIN(config::Base* cin, cin_vector* outVector)
    {
        ASSERT((cin != nullptr), "OUTPUT PARAMETER <cin> CANNOT BE A NULL POINTER");

        try
        {
            outVector->emplace_back(cin);
            return Status(Status::Code::GOOD);
        }
        catch(const std::bad_alloc& e)
        {
            LOG_ERROR(logger, "%s: CIN class: Network, CIN address: %p", e.what(), cin);
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(logger, "%s: CIN class: Network, CIN address: %p", e.what(), cin);
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        }
    }

    std::pair<Status, wifi_auth_mode_t> NetworkValidator::convertToAuth(JsonVariant auth)
    {
        if (auth.isNull() == true)
        {
            LOG_VERBOSE(logger, "WiFi AUTH values were not provied");
            return std::make_pair(Status(Status::Code::GOOD_NO_DATA), WIFI_AUTH_OPEN);
        }

        if (auth.is<uint8_t>() == false)
        {
            LOG_ERROR(logger, "WiFi AUTH MUST BE A 8-BIT UNSIGNED INTEGER");
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), WIFI_AUTH_OPEN);
        }

        const uint8_t authValue = auth.as<uint8_t>();
        switch (authValue)
        {
        case 0: 
            return std::make_pair(Status(Status::Code::GOOD), WIFI_AUTH_OPEN);
        case 1:
            return std::make_pair(Status(Status::Code::GOOD), WIFI_AUTH_WEP);
        case 2:
            return std::make_pair(Status(Status::Code::GOOD), WIFI_AUTH_WPA_PSK);
        case 3:
            return std::make_pair(Status(Status::Code::GOOD), WIFI_AUTH_WPA2_PSK);
        case 4:
            return std::make_pair(Status(Status::Code::GOOD), WIFI_AUTH_WPA_WPA2_PSK);
        default:
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), WIFI_AUTH_OPEN);
        }
    }

    std::pair<Status, wpa2_auth_method_t> NetworkValidator::convertToWpaAuth(JsonVariant wpaAuth)
    {
        if (wpaAuth.isNull() == true)
        {
            LOG_VERBOSE(logger, "WiFi EAP AUTH values were not provied");
            return std::make_pair(Status(Status::Code::GOOD_NO_DATA), WPA2_AUTH_TLS);
        }

        if (wpaAuth.is<uint8_t>() == false)
        {
            LOG_ERROR(logger, "WiFi EAP AUTH MUST BE A 8-BIT UNSIGNED INTEGER");
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), WPA2_AUTH_TLS);
        }

        const uint8_t WpaAuthValue = wpaAuth.as<uint8_t>();

        switch (WpaAuthValue)
        {
        case 0: 
            return std::make_pair(Status(Status::Code::GOOD), WPA2_AUTH_TLS);
        case 1:
            return std::make_pair(Status(Status::Code::GOOD), WPA2_AUTH_PEAP);
        case 2:
            return std::make_pair(Status(Status::Code::GOOD), WPA2_AUTH_TTLS);
        default:
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), WPA2_AUTH_TLS);
        }
    }

    std::pair<Status, std::string> NetworkValidator::convertToSSID(JsonVariant ssid)
    {
        const std::string ssidValue = ssid.as<std::string>();

        if (ssidValue.length() > 32)
        {
            LOG_ERROR(logger, "WiFi SSID LENGTH IS TOO LONG");
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), ssidValue);
        }
       
        return std::make_pair(Status(Status::Code::GOOD), ssidValue);
    }

    std::pair<Status, std::string> NetworkValidator::convertToPSK(JsonVariant psk)
    {
        if (psk.isNull() == true)
        {
            LOG_VERBOSE(logger, "WiFi PSK values were not provied");
            return std::make_pair(Status(Status::Code::GOOD_NO_DATA), "");
        }

        if (psk.is<std::string>() == false)
        {
            LOG_ERROR(logger, "WiFi PSK MUST BE STRING");
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), "");
        }

        const std::string pskValue = psk.as<std::string>();

        if (pskValue.length() > 63)
        {
            LOG_ERROR(logger, "WiFi PSK LENGTH IS TOO LONG");
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), pskValue);
        }
       
        return std::make_pair(Status(Status::Code::GOOD), pskValue);
    }

    std::pair<Status, std::string> NetworkValidator::convertToEapID(JsonVariant id)
    {
        if (id.isNull() == true)
        {
            LOG_VERBOSE(logger, "WiFi EAP Identity values were not provied");
            return std::make_pair(Status(Status::Code::GOOD_NO_DATA), "");
        }

        if (id.is<std::string>() == false)
        {
            LOG_ERROR(logger, "WiFi EAP IDENTITY MUST BE STRING");
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), "");
        }

        const std::string idValue = id.as<std::string>();

        if (idValue.length() > 64)
        {
            LOG_ERROR(logger, "WiFi EAP IDENTITY LENGTH IS TOO LONG");
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), idValue);
        }
       
        return std::make_pair(Status(Status::Code::GOOD), idValue);
    }

    std::pair<Status, std::string> NetworkValidator::convertToEapUser(JsonVariant user)
    {
        if (user.isNull() == true)
        {
            LOG_VERBOSE(logger, "WiFi EAP username values were not provied");
            return std::make_pair(Status(Status::Code::GOOD_NO_DATA), "");
        }

        if (user.is<std::string>() == false)
        {
            LOG_ERROR(logger, "WiFi EAP USERNAME MUST BE STRING");
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), "");
        }

        const std::string userValue = user.as<std::string>();

        if (userValue.length() > 64)
        {
            LOG_ERROR(logger, "WiFi EAP USERNAME LENGTH IS TOO LONG");
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), userValue);
        }
       
        return std::make_pair(Status(Status::Code::GOOD), userValue);
    }

    std::pair<Status, std::string> NetworkValidator::convertToEapPassword(JsonVariant pass)
    {
        if (pass.isNull() == true)
        {
            LOG_VERBOSE(logger, "WiFi EAP password values were not provied");
            return std::make_pair(Status(Status::Code::GOOD_NO_DATA), "");
        }

        if (pass.is<std::string>() == false)
        {
            LOG_ERROR(logger, "WiFi EAP PASSWORD MUST BE STRING");
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), "");
        }

        const std::string passValue = pass.as<std::string>();

        if (passValue.length() > 64)
        {
            LOG_ERROR(logger, "WiFi EAP PASSWORD LENGTH IS TOO LONG");
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), passValue);
        }
       
        return std::make_pair(Status(Status::Code::GOOD), passValue);
    }

    std::pair<Status, std::string> NetworkValidator::convertToCaCertificate(JsonVariant caCert)
    {
        if (caCert.isNull() == true)
        {
            LOG_VERBOSE(logger, "WiFi CA Certificate values were not provied");
            return std::make_pair(Status(Status::Code::GOOD_NO_DATA), "");
        }

        if (caCert.is<std::string>() == false)
        {
            LOG_ERROR(logger, "WiFi CA CERTIFICATE MUST BE STRING");
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), "");
        }
        
        const std::string caCertValue = caCert.as<std::string>();
        return std::make_pair(Status(Status::Code::GOOD), caCertValue);
    }

    std::pair<Status, std::string> NetworkValidator::convertToClientCertificate(JsonVariant crt)
    {
        if (crt.isNull() == true)
        {
            LOG_VERBOSE(logger, "WiFi Client Certificate values were not provied");
            return std::make_pair(Status(Status::Code::GOOD_NO_DATA), "");
        }

        if (crt.is<std::string>() == false)
        {
            LOG_ERROR(logger, "WiFi Client CERTIFICATE MUST BE STRING");
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), "");
        }

        const std::string clientCertValue = crt.as<std::string>();
        return std::make_pair(Status(Status::Code::GOOD), clientCertValue);
    }

    std::pair<Status, std::string> NetworkValidator::convertToClientKey(JsonVariant key)
    {
        if (key.isNull() == true)
        {
            LOG_VERBOSE(logger, "WiFi Client key values were not provied");
            return std::make_pair(Status(Status::Code::GOOD_NO_DATA), "");
        }

        if (key.is<std::string>() == false)
        {
            LOG_ERROR(logger, "WiFi Client KEY MUST BE STRING");
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), "");
        }

        const std::string clientKeyValue = key.as<std::string>();
        return std::make_pair(Status(Status::Code::GOOD), clientKeyValue);
    }

    std::pair<Status, IPAddress> NetworkValidator::convertToIPv4(JsonVariant ip, const bool& isSubnetmask)
    {
        IPAddress IPv4;
        const std::string IpValue = ip.as<std::string>();

        std::regex validationRegex;
        if (isSubnetmask == true)
        {
            validationRegex.assign("^((255|254|252|248|240|224|192|128|0)\\.){3}(255|254|252|248|240|224|192|128|0)$");
        }
        else
        {
            validationRegex.assign("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
        }

        // validating IPv4 address using regular expression
        if (std::regex_match(IpValue, validationRegex))
        {
            if (IPv4.fromString(IpValue.c_str()))  // fromString 함수가 IP 변환에 성공했는지 확인
            {
                return std::make_pair(Status(Status::Code::GOOD), IPv4);
            }
            else
            {
                return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), IPAddress());
            }
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), IPAddress());
        }
    }
}}