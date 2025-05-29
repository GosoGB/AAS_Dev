/**
 * @file NetworkValidator.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief 네트워크에 대한 설정 정보가 유효한지 검사하는 클래스를 정의합니다.
 * 
 * @date 2025-01-24
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */



#include <regex>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "JARVIS/Config/Network/CatM1.h"
#include "JARVIS/Config/Network/Ethernet.h"
#include "JARVIS/Config/Network/WiFi4.h"
#include "NetworkValidator.h"



namespace muffin { namespace jvs {

    std::pair<rsc_e, std::string> NetworkValidator::Inspect(const cfg_key_e key, const JsonArray arrayCIN,  prtcl_ver_e ProtocolVersion, cin_vector* outVector)
    {
        ASSERT((arrayCIN.isNull() == false), "OUTPUT PARAMETER <arrayCIN> CANNOT BE NULL");
        ASSERT((arrayCIN.size() != 0), "INPUT PARAMETER <arrayCIN> CANNOT BE 0 IN LENGTH");
        mProtocolVersion = ProtocolVersion;
        switch (key)
        {
        case cfg_key_e::ETHERNET:
            return validateEthernet(arrayCIN, outVector);
        case cfg_key_e::WIFI4:
            return std::make_pair(rsc_e::BAD_UNSUPPORTED_CONFIGURATION, "Wi-Fi IS NOT SUPPORTED IN THE CURRENT VERSION");
        default:
            return std::make_pair(rsc_e::BAD_INTERNAL_ERROR, "UNDEFINED CONFIG KEY FOR NETWORK INTERFACE");
        };
    }

    /**
     * @todo 상태 코드로 오류가 아닌 경고를 반환하도록 수정해야 합니다.
     * @brief 크기가 1보다 클 때 첫번째 설정만 적용되고 나머지는 버려집니다.
     *        이는 설정 형식 자체의 오류는 아니며, 일부는 적용되기 때문에
     *        상태 코드로 오류가 아닌 경고를 반환하는 것이 적합합니다.
     */
    std::pair<rsc_e, std::string> NetworkValidator::validateWiFi4(const JsonArray array, cin_vector* outVector)
    {
        if (array.size() != 1)
        {
            ASSERT((array.size() == 1), "WIFI CONFIG CANNOT BE GREATER THAN 1");
            return std::make_pair(rsc_e::BAD_UNSUPPORTED_CONFIGURATION, "INVALID WIFI CONFIG: ONLY ONE WIFI MODULE CAN BE CONFIGURED");
        }

        JsonObject cin = array[0];
        rsc_e rsc = validateMandatoryKeysWiFi4(cin);
        if (rsc != rsc_e::GOOD)
        {
            return std::make_pair(rsc, "INVALID WIFI: MANDATORY KEY CANNOT BE MISSING");
        }

        rsc = validateMandatoryValuesWiFi4(cin);
        
        if (rsc != rsc_e::GOOD)
        {
            return std::make_pair(rsc, "INVALID WIFI: MANDATORY KEY'S VALUE CANNOT BE NULL");
        }

        const bool DHCP = cin["dhcp"].as<bool>();
        const bool EAP  = cin["eap"].as<bool>();

        const auto retAUTH = convertToAuth(cin["auth"].as<JsonVariant>());
        if (retAUTH.first != rsc_e::GOOD &&
            retAUTH.first != rsc_e::GOOD_NO_DATA)
        {
            return std::make_pair(rsc, "INVALID WIFI AUTH");
        }

        const auto retWpaAUTH = convertToWpaAuth(cin["wpa2auth"].as<JsonVariant>());
        if (retWpaAUTH.first != rsc_e::GOOD &&
            retWpaAUTH.first != rsc_e::GOOD_NO_DATA)
        {
            return std::make_pair(rsc, "INVALID WIFI EAP AUTH");
        }

        const auto retSSID = convertToSSID(cin["ssid"].as<JsonVariant>());
        if (retSSID.first != rsc_e::GOOD)
        {
            const std::string message = "INVALID WIFI SSID: " + cin["ssid"].as<std::string>();
            return std::make_pair(rsc, message);
        }

        const auto retPSK = convertToPSK(cin["psk"].as<JsonVariant>());
        if (retPSK.first != rsc_e::GOOD &&
            retPSK.first != rsc_e::GOOD_NO_DATA)
        {
            return std::make_pair(rsc, "INVALID WIFI PSK");
        }

        const auto retEapID = convertToEapID(cin["id"].as<JsonVariant>());
        if (retEapID.first != rsc_e::GOOD &&
            retEapID.first != rsc_e::GOOD_NO_DATA)
        {
            return std::make_pair(rsc, "INVALID WIFI EAP ID");
        }

        const auto retEapUser = convertToEapUser(cin["user"].as<JsonVariant>());
        if (retEapUser.first != rsc_e::GOOD &&
            retEapUser.first != rsc_e::GOOD_NO_DATA)
        {
            return std::make_pair(rsc, "INVALID WIFI EAP USER");
        }

        const auto retEapPassword = convertToEapPassword(cin["pass"].as<JsonVariant>());
        if (retEapPassword.first != rsc_e::GOOD &&
            retEapPassword.first != rsc_e::GOOD_NO_DATA)
        {
            return std::make_pair(rsc, "INVALID WIFI EAP PASSWORD");
        }

        const auto retCaCertificate = convertToCaCertificate(cin["ca_cert"].as<JsonVariant>());
        if (retCaCertificate.first != rsc_e::GOOD &&
            retCaCertificate.first != rsc_e::GOOD_NO_DATA)
        {
            return std::make_pair(rsc, "INVALID WIFI CA CERTIFICATE");
        }

        const auto retClientCertificate = convertToClientCertificate(cin["crt"].as<JsonVariant>());
        if (retClientCertificate.first != rsc_e::GOOD &&
            retClientCertificate.first != rsc_e::GOOD_NO_DATA)
        {
            return std::make_pair(rsc, "INVALID WIFI CLIENT CERTIFICATE");
        }

        const auto retClientKey = convertToClientKey(cin["key"].as<JsonVariant>());
        if (retClientKey.first != rsc_e::GOOD &&
            retClientKey.first != rsc_e::GOOD_NO_DATA)
        {
            return std::make_pair(rsc, "INVALID WIFI CLIENT KEY");
        }

        config::WiFi4* wifi4 = new (std::nothrow) config::WiFi4();
        if (wifi4 == nullptr)
        {
            return std::make_pair(rsc_e::BAD_OUT_OF_MEMORY, "FAILED TO ALLOCATE MEMORY WIFI CONFIG");
        }
        
        if (DHCP == false)
        {
            const auto retIP    = convertToIPv4(cin["ip"].as<JsonVariant>(),false);
            const auto retSVM   = convertToIPv4(cin["snm"].as<JsonVariant>(),true);
            const auto retGTW   = convertToIPv4(cin["gtw"].as<JsonVariant>(),false);
            const auto retDNS1  = convertToIPv4(cin["dns1"].as<JsonVariant>(),false);
            const auto retDNS2  = convertToIPv4(cin["dns2"].as<JsonVariant>(),false);

            if (retIP.first != rsc_e::GOOD)
            {
                const std::string message = "INVALID WIFI IP: " + cin["ip"].as<std::string>();
                return std::make_pair(rsc, message);
            }

            if (retSVM.first != rsc_e::GOOD)
            {
                const std::string message = "INVALID WIFI SUBNETMASK: " + cin["snm"].as<std::string>();
                return std::make_pair(rsc, message);
            }

            if (retGTW.first != rsc_e::GOOD)
            {
                const std::string message = "INVALID WIFI GATEWAY: " + cin["gtw"].as<std::string>();
                return std::make_pair(rsc, message);
            }

            if (retDNS1.first != rsc_e::GOOD)
            {
                const std::string message = "INVALID WIFI DNS1: " + cin["dns1"].as<std::string>();
                return std::make_pair(rsc, message);
            }

            if (retDNS2.first != rsc_e::GOOD)
            {
                const std::string message = "INVALID WIFI DNS2: " + cin["dns2"].as<std::string>();
                return std::make_pair(rsc, message);
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

        if (retAUTH.first == rsc_e::GOOD)
        {
            wifi4->SetAuthMode(retAUTH.second);
        }

        if (retWpaAUTH.first == rsc_e::GOOD)
        {
            wifi4->SetEapAuthMode(retWpaAUTH.second);
        }

        if (retPSK.first == rsc_e::GOOD)
        {
            wifi4->SetPSK(retPSK.second);
        }

        if (retEapID.first == rsc_e::GOOD)
        {
            wifi4->SetEapID(retEapID.second);
        }

        if (retEapUser.first == rsc_e::GOOD)
        {
            wifi4->SetEapUserName(retEapUser.second);
        }
        
        if (retEapPassword.first == rsc_e::GOOD)
        {
            wifi4->SetEapPassword(retEapPassword.second);
        }

        if (retCaCertificate.first == rsc_e::GOOD)
        {
            wifi4->SetEapCaCertificate(retCaCertificate.second);
        }

        if (retClientCertificate.first == rsc_e::GOOD)
        {
            wifi4->SetEapClientCertificate(retClientCertificate.second);
        }

        if (retClientKey.first == rsc_e::GOOD)
        {
            wifi4->SetEapClientKey(retClientKey.second);
        }

        rsc = emplaceCIN(static_cast<config::Base*>(wifi4), outVector);
        if (rsc != rsc_e::GOOD)
        {
            if (wifi4 != nullptr)
            {
                delete wifi4;
                wifi4 = nullptr;
            }
            return std::make_pair(rsc, "FAILED TO EMPLACE: WIFI CONFIG INSTANCE");
        }

        return std::make_pair(rsc_e::GOOD, "GOOD"); 
    }

    rsc_e NetworkValidator::validateMandatoryKeysWiFi4(const JsonObject json)
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
            return rsc_e::GOOD;
        }
        else
        {
            return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
        }
    }

    rsc_e NetworkValidator::validateMandatoryValuesWiFi4(const JsonObject json)
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
            return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
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
                return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
            }

        }

          // auth가 null이 아니면 uint8_t 타입인지 검사
        if (json["auth"].isNull() == false)
        {
            if (json["auth"].is<uint8_t>() == false)
            {
                LOG_ERROR(logger, "'AUTH' IS NOT OF TYPE UNSIGNED INT 8");
                return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
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
                return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
            }
        }

        if (json["wpa2auth"].isNull() == false)
        {
            if (json["wpa2auth"].is<uint8_t>() == false)
            {
                LOG_ERROR(logger, "'EAP AUTH' IS NOT OF TYPE UNSIGNED INT 8");
                return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
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
                    return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
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
                    return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
                }
            }
        }
        
        if (isValid == true)
        {
            return rsc_e::GOOD;
        }
        else
        {
            return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
        }
    }

    std::pair<rsc_e, std::string> NetworkValidator::setEthernetConfig(const JsonObject obj, config::Ethernet* eth)
    {
        const bool DHCP = obj["dhcp"].as<bool>();
        eth->SetDHCP(DHCP);
        
        if (DHCP == false)
        {
            const auto retIP    = convertToIPv4(obj["ip"].as<JsonVariant>(),   false);
            const auto retSVM   = convertToIPv4(obj["snm"].as<JsonVariant>(),  true);
            const auto retGTW   = convertToIPv4(obj["gtw"].as<JsonVariant>(),  false);
            const auto retDNS1  = convertToIPv4(obj["dns1"].as<JsonVariant>(), false);
            const auto retDNS2  = convertToIPv4(obj["dns2"].as<JsonVariant>(), false);

            if (retIP.first != rsc_e::GOOD)
            {
                return std::make_pair(retIP.first, "INVALID ETHERNET IP");
            }

            if (retSVM.first != rsc_e::GOOD)
            {
                return std::make_pair(retSVM.first, "INVALID ETHERNET SUBNETMASK");
            }

            if (retGTW.first != rsc_e::GOOD)
            {
                return std::make_pair(retGTW.first, "INVALID ETHERNET GATEWAY");
            }

            if (retDNS1.first != rsc_e::GOOD)
            {
                return std::make_pair(retDNS1.first, "INVALID ETHERNET DNS1");
            }

            if (retDNS2.first != rsc_e::GOOD)
            {
                return std::make_pair(retDNS2.first, "INVALID ETHERNET DNS2");
            }

            eth->SetStaticIPv4(retIP.second);
            eth->SetSubnetmask(retSVM.second);
            eth->SetGateway(retGTW.second);
            eth->SetDNS1(retDNS1.second);
            eth->SetDNS2(retDNS2.second);
        }

        return std::make_pair(rsc_e::GOOD, "GOOD");
    }

    /**
     * @todo 상태 코드로 오류가 아닌 경고를 반환하도록 수정해야 합니다.
     * @brief 크기가 1보다 클 때 첫번째 설정만 적용되고 나머지는 버려집니다.
     *        이는 설정 형식 자체의 오류는 아니며, 일부는 적용되기 때문에
     *        상태 코드로 오류가 아닌 경고를 반환하는 것이 적합합니다.
     */
    std::pair<rsc_e, std::string> NetworkValidator::validateEthernet(const JsonArray array, cin_vector* outVector)
    {

    #if defined(MT11)
        for (JsonObject cin : array)
        {
    #else
            const bool hasMultipleCIN = array.size() > 1 ? true : false;
            const JsonObject cin = array[0];
    #endif
            rsc_e rsc = validateMandatoryKeysEthernet(cin);
            if (rsc != rsc_e::GOOD)
            {
                return std::make_pair(rsc, "INVALID ETHERNET: MANDATORY KEY CANNOT BE MISSING");
            }

            rsc = validateMandatoryValuesEthernet(cin);
            if (rsc != rsc_e::GOOD)
            {
                return std::make_pair(rsc, "INVALID ETHERNET: MANDATORY KEY'S VALUE CANNOT BE NULL");
            }
            
            uint8_t EthernetInterfaces = 0;
            if (mProtocolVersion > prtcl_ver_e::VERSEOIN_3)
            {
                EthernetInterfaces = cin["eths"].as<uint8_t>();
                const auto retEths = convertToEthernetInterfaces(EthernetInterfaces);
                if (retEths.first != rsc_e::GOOD)
                {
                    return std::make_pair(retEths.first, "INVALID ETHERNET INTERFACES");
                }
            }
            std::pair<rsc_e, std::string> result;
            
            switch (EthernetInterfaces)
            {
            case static_cast<uint8_t>(if_e::EMBEDDED):

                config::embeddedEthernet = new(std::nothrow) config::Ethernet();
                if (config::embeddedEthernet == nullptr)
                {
                    return std::make_pair(rsc_e::BAD_OUT_OF_MEMORY, "FAILED TO ALLOCATE MEMORY FOR CIN: EMBEDDED ETHERNET");
                }
                result = setEthernetConfig(cin, config::embeddedEthernet);
                break;
        #if defined(MT11)
            case static_cast<uint8_t>(if_e::LINK_01):

                config::link1Ethernet = new(std::nothrow) config::Ethernet();
                if (config::link1Ethernet == nullptr)
                {
                    return std::make_pair(rsc_e::BAD_OUT_OF_MEMORY, "FAILED TO ALLOCATE MEMORY FOR CIN: LINK1 ETHERNET");
                }
                result = setEthernetConfig(cin, config::link1Ethernet);
                break;
            case static_cast<uint8_t>(if_e::LINK_02):

                config::link2Ethernet = new(std::nothrow) config::Ethernet();
                if (config::link2Ethernet == nullptr)
                {
                    return std::make_pair(rsc_e::BAD_OUT_OF_MEMORY, "FAILED TO ALLOCATE MEMORY FOR CIN: LINK2 ETHERNET");
                }
                result = setEthernetConfig(cin, config::link2Ethernet);
                break;
        #endif 
            default:
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, "INVALID ETHERNET INTERFACES");
            }
            
            if (result.first != rsc_e::GOOD)
            {
                return result;
            }
          
            
    #if defined(MT11)
        }
        
        return std::make_pair(rsc_e::GOOD, "GOOD");
    #else
        if (hasMultipleCIN == true)
        {
            return std::make_pair(rsc_e::UNCERTAIN_CONFIG_INSTANCE, "UNCERTIAN: APPLIED ONLY ONE ETHERTNET CONFIG");
        }
        else
        {
            return std::make_pair(rsc_e::GOOD, "GOOD");
        }
    #endif

    }

    rsc_e NetworkValidator::validateMandatoryKeysEthernet(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json.containsKey("dhcp");
        isValid &= json.containsKey("ip");
        isValid &= json.containsKey("snm");     
        isValid &= json.containsKey("gtw");    
        isValid &= json.containsKey("dns1");    
        isValid &= json.containsKey("dns2"); 
        if (mProtocolVersion > prtcl_ver_e::VERSEOIN_3)
        {
            isValid &= json.containsKey("eths");  
        }
         
        if (isValid == true)
        {
            return rsc_e::GOOD;
        }
        else
        {
            return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
        }
    }

    rsc_e NetworkValidator::validateMandatoryValuesEthernet(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json["dhcp"].isNull() == false;
        isValid &= json["dhcp"].is<bool>();
        
        if (mProtocolVersion > prtcl_ver_e::VERSEOIN_3)
        {
            isValid &= json["eths"].isNull() == false;
            isValid &= json["eths"].is<uint8_t>(); 
        }

        if (isValid == false)
        {
            return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
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
            return rsc_e::GOOD;
        }
        else
        {
            return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
        }
    }

    rsc_e NetworkValidator::emplaceCIN(config::Base* cin, cin_vector* outVector)
    {
        ASSERT((cin != nullptr), "OUTPUT PARAMETER <cin> CANNOT BE A NULL POINTER");

        try
        {
            outVector->emplace_back(cin);
            return rsc_e::GOOD;
        }
        catch(const std::bad_alloc& e)
        {
            LOG_ERROR(logger, "%s: CIN class: Network, CIN address: %p", e.what(), cin);
            return rsc_e::BAD_OUT_OF_MEMORY;
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(logger, "%s: CIN class: Network, CIN address: %p", e.what(), cin);
            return rsc_e::BAD_UNEXPECTED_ERROR;
        }
    }

    std::pair<rsc_e, wifi_auth_mode_t> NetworkValidator::convertToAuth(JsonVariant auth)
    {
        if (auth.isNull() == true)
        {
            return std::make_pair(rsc_e::GOOD_NO_DATA, WIFI_AUTH_OPEN);
        }

        if (auth.is<uint8_t>() == false)
        {
            LOG_ERROR(logger, "WiFi AUTH MUST BE A 8-BIT UNSIGNED INTEGER");
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, WIFI_AUTH_OPEN);
        }

        const uint8_t authValue = auth.as<uint8_t>();
        switch (authValue)
        {
        case 0: 
            return std::make_pair(rsc_e::GOOD, WIFI_AUTH_OPEN);
        case 1:
            return std::make_pair(rsc_e::GOOD, WIFI_AUTH_WEP);
        case 2:
            return std::make_pair(rsc_e::GOOD, WIFI_AUTH_WPA_PSK);
        case 3:
            return std::make_pair(rsc_e::GOOD, WIFI_AUTH_WPA2_PSK);
        case 4:
            return std::make_pair(rsc_e::GOOD, WIFI_AUTH_WPA_WPA2_PSK);
        default:
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, WIFI_AUTH_OPEN);
        }
    }

    std::pair<rsc_e, wpa2_auth_method_t> NetworkValidator::convertToWpaAuth(JsonVariant wpaAuth)
    {
        if (wpaAuth.isNull() == true)
        {
            return std::make_pair(rsc_e::GOOD_NO_DATA, WPA2_AUTH_TLS);
        }

        if (wpaAuth.is<uint8_t>() == false)
        {
            LOG_ERROR(logger, "WiFi EAP AUTH MUST BE A 8-BIT UNSIGNED INTEGER");
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, WPA2_AUTH_TLS);
        }

        const uint8_t WpaAuthValue = wpaAuth.as<uint8_t>();

        switch (WpaAuthValue)
        {
        case 0: 
            return std::make_pair(rsc_e::GOOD, WPA2_AUTH_TLS);
        case 1:
            return std::make_pair(rsc_e::GOOD, WPA2_AUTH_PEAP);
        case 2:
            return std::make_pair(rsc_e::GOOD, WPA2_AUTH_TTLS);
        default:
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, WPA2_AUTH_TLS);
        }
    }

    std::pair<rsc_e, std::string> NetworkValidator::convertToSSID(JsonVariant ssid)
    {
        const std::string ssidValue = ssid.as<std::string>();

        if (ssidValue.length() > 32)
        {
            LOG_ERROR(logger, "WiFi SSID LENGTH IS TOO LONG");
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, ssidValue);
        }
       
        return std::make_pair(rsc_e::GOOD, ssidValue);
    }

    std::pair<rsc_e, std::string> NetworkValidator::convertToPSK(JsonVariant psk)
    {
        if (psk.isNull() == true)
        {
            return std::make_pair(rsc_e::GOOD_NO_DATA, "WiFi PSK values were not provied");
        }

        if (psk.is<std::string>() == false)
        {
            LOG_ERROR(logger, "WiFi PSK MUST BE STRING");
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, "WiFi PSK MUST BE STRING");
        }

        const std::string pskValue = psk.as<std::string>();

        if (pskValue.length() > 63)
        {
            LOG_ERROR(logger, "WiFi PSK LENGTH IS TOO LONG");
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, pskValue);
        }
       
        return std::make_pair(rsc_e::GOOD, pskValue);
    }

    std::pair<rsc_e, std::string> NetworkValidator::convertToEapID(JsonVariant id)
    {
        if (id.isNull() == true)
        {
            return std::make_pair(rsc_e::GOOD_NO_DATA, "WiFi EAP Identity values were not provied");
        }

        if (id.is<std::string>() == false)
        {
            LOG_ERROR(logger, "WiFi EAP IDENTITY MUST BE STRING");
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, "WiFi EAP IDENTITY MUST BE STRING");
        }

        const std::string idValue = id.as<std::string>();

        if (idValue.length() > 64)
        {
            LOG_ERROR(logger, "WiFi EAP IDENTITY LENGTH IS TOO LONG");
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, idValue);
        }
       
        return std::make_pair(rsc_e::GOOD, idValue);
    }

    std::pair<rsc_e, std::string> NetworkValidator::convertToEapUser(JsonVariant user)
    {
        if (user.isNull() == true)
        {
            return std::make_pair(rsc_e::GOOD_NO_DATA, "WiFi EAP username values were not provied");
        }

        if (user.is<std::string>() == false)
        {
            LOG_ERROR(logger, "WiFi EAP USERNAME MUST BE STRING");
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, "WiFi EAP USERNAME MUST BE STRING");
        }

        const std::string userValue = user.as<std::string>();

        if (userValue.length() > 64)
        {
            LOG_ERROR(logger, "WiFi EAP USERNAME LENGTH IS TOO LONG");
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, userValue);
        }
       
        return std::make_pair(rsc_e::GOOD, userValue);
    }

    std::pair<rsc_e, std::string> NetworkValidator::convertToEapPassword(JsonVariant pass)
    {
        if (pass.isNull() == true)
        {
            return std::make_pair(rsc_e::GOOD_NO_DATA, "WiFi EAP password values were not provied");
        }

        if (pass.is<std::string>() == false)
        {
            LOG_ERROR(logger, "WiFi EAP PASSWORD MUST BE STRING");
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, "WiFi EAP PASSWORD MUST BE STRING");
        }

        const std::string passValue = pass.as<std::string>();

        if (passValue.length() > 64)
        {
            LOG_ERROR(logger, "WiFi EAP PASSWORD LENGTH IS TOO LONG");
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, passValue);
        }
       
        return std::make_pair(rsc_e::GOOD, passValue);
    }

    std::pair<rsc_e, std::string> NetworkValidator::convertToCaCertificate(JsonVariant caCert)
    {
        if (caCert.isNull() == true)
        {
            return std::make_pair(rsc_e::GOOD_NO_DATA, "WiFi CA Certificate values were not provied");
        }

        if (caCert.is<std::string>() == false)
        {
            LOG_ERROR(logger, "WiFi CA CERTIFICATE MUST BE STRING");
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, "WiFi CA CERTIFICATE MUST BE STRING");
        }
        
        const std::string caCertValue = caCert.as<std::string>();
        return std::make_pair(rsc_e::GOOD, caCertValue);
    }

    std::pair<rsc_e, std::string> NetworkValidator::convertToClientCertificate(JsonVariant crt)
    {
        if (crt.isNull() == true)
        {
            return std::make_pair(rsc_e::GOOD_NO_DATA, "WiFi Client Certificate values were not provied");
        }

        if (crt.is<std::string>() == false)
        {
            LOG_ERROR(logger, "WiFi Client CERTIFICATE MUST BE STRING");
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, "WiFi Client CERTIFICATE MUST BE STRING");
        }

        const std::string clientCertValue = crt.as<std::string>();
        return std::make_pair(rsc_e::GOOD, clientCertValue);
    }

    std::pair<rsc_e, std::string> NetworkValidator::convertToClientKey(JsonVariant key)
    {
        if (key.isNull() == true)
        {
            return std::make_pair(rsc_e::GOOD_NO_DATA, "WiFi Client key values were not provied");
        }

        if (key.is<std::string>() == false)
        {
            LOG_ERROR(logger, "WiFi Client KEY MUST BE STRING");
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, "WiFi Client KEY MUST BE STRING");
        }

        const std::string clientKeyValue = key.as<std::string>();
        return std::make_pair(rsc_e::GOOD, clientKeyValue);
    }

    std::pair<rsc_e, IPAddress> NetworkValidator::convertToIPv4(JsonVariant ip, const bool& isSubnetmask)
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
                return std::make_pair(rsc_e::GOOD, IPv4);
            }
            else
            {
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, IPAddress());
            }
        }
        else
        {
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, IPAddress());
        }
    }

    std::pair<rsc_e, if_e> NetworkValidator::convertToEthernetInterfaces(uint8_t eths)
    {
        switch (eths)
        {
        case 0:
            return std::make_pair(rsc_e::GOOD, if_e::EMBEDDED);
        case 1:
            return std::make_pair(rsc_e::GOOD, if_e::LINK_01);
        case 2:
            return std::make_pair(rsc_e::GOOD, if_e::LINK_02);
        default:
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, if_e::EMBEDDED);
        }
    }
}}