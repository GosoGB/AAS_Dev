/**
 * @file NetworkValidator.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
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
#include "NetworkValidator.h"
#include "Jarvis/Config/Network/CatM1.h"
#include "Jarvis/Config/Network/Ethernet.h"
#include "Jarvis/Config/Network/WiFi4.h"
#include "Jarvis/Include/Helper.h"



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
        case cfg_key_e::LTE_CatM1:
            ret = validateLteCatM1(arrayCIN, outVector);
            break;
        case cfg_key_e::ETHERNET:
            ret = validateEthernet(arrayCIN, outVector);
            break;
        case cfg_key_e::WIFI4:
            ret = validateWiFi4(arrayCIN, outVector);
            break;
        default:
            ASSERT(false, "UNDEFINED SERIAL PORT CONFIGURATION");
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        };

        return ret;
    }

    Status NetworkValidator::validateLteCatM1(const JsonArray array, cin_vector* outVector)
    {
        if (array.size() != 1)
        {
            LOG_ERROR(logger, "INVALID LTE CONFIG: ONLY ONE LTE MODULE CAN BE CONFIGURED");
            ASSERT((array.size() == 1), "LTE CONFIG CANNOT BE GREATER THAN 1");
            return Status(Status::Code::BAD_NOT_SUPPORTED);
        }

        JsonObject cin = array[0];
        Status ret = validateMandatoryKeysLteCatM1(cin);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "INVALID LTE Cat.M1: MANDATORY KEY CANNOT BE MISSING");
            return ret;
        }

        ret = validateMandatoryValuesLteCatM1(cin);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "INVALID LTE Cat.M1: MANDATORY KEY'S VALUE CANNOT BE NULL");
            return ret;
        }

        const std::string md    = cin["md"].as<std::string>();
        const std::string ctry  = cin["ctry"].as<std::string>();

        const auto retMD     = convertToLteModel(md);
        const auto retCtry   = convertToLteCountry(ctry);

        if (retMD.first.ToCode() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "INVALID LTE Cat.M1 Model: %s", md.c_str());
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        }

        if (retCtry.first.ToCode() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "INVALID LTE Cat.M1 Country: %s", ctry.c_str());
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        }

        config::CatM1* catM1 = new config::CatM1(cfg_key_e::LTE_CatM1);
        catM1->SetModel(retMD.second);
        catM1->SetCounty(retCtry.second);

        ret = emplaceCIN(static_cast<config::Base*>(catM1), outVector);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO EMPLACE CONFIG INSTANCE: %s", ret.c_str());
            return ret;
        }

        LOG_VERBOSE(logger, "Valid LTE Cat.M1 config instance")
        return Status(Status::Code::GOOD);
        
    }

    Status NetworkValidator::validateMandatoryKeysLteCatM1(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json.containsKey("md");
        isValid &= json.containsKey("ctry");

        if (isValid == true)
        {
            return Status(Status::Code::GOOD);
        }
        else
        {
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }
    }

    Status NetworkValidator::validateMandatoryValuesLteCatM1(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json["md"].isNull()  == false;
        isValid &= json["ctry"].isNull()  == false;

        if (isValid == true)
        {
            return Status(Status::Code::GOOD);
        }
        else
        {
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }
    }


    Status NetworkValidator::validateWiFi4(const JsonArray array, cin_vector* outVector)
    {
        if (array.size() != 1)
        {
            LOG_ERROR(logger, "INVALID LTE CONFIG: ONLY ONE WIFI MODULE CAN BE CONFIGURED");
            ASSERT((array.size() == 1), "LTE CONFIG CANNOT BE GREATER THAN 1");
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
        
        const bool DHCP                     = cin["dhcp"].as<bool>();
        const std::string IP                = cin["ip"].as<std::string>();
        const std::string SVM               = cin["snm"].as<std::string>();
        const std::string GTW               = cin["gtw"].as<std::string>();
        const std::string DNS1              = cin["dns1"].as<std::string>();
        const std::string DNS2              = cin["dns2"].as<std::string>();
        const std::string SSID              = cin["ssid"].as<std::string>();
        const std::string PSK               = cin["psk"].as<std::string>();
        const uint8_t AUTH                  = cin["auth"].as<uint8_t>();
        const bool EAP                      = cin["eap"].as<bool>(); 
        const uint8_t WPA2AUTH              = cin["wpa2auth"].as<uint8_t>();
        const std::string EapID             = cin["id"].as<std::string>();
        const std::string EapUser           = cin["user"].as<std::string>();
        const std::string EapPassword       = cin["pass"].as<std::string>();
        const std::string CaCertifiacte     = cin["ca_cert"].as<std::string>();
        const std::string ClientCertificate = cin["crt"].as<std::string>();
        const std::string ClientKey         = cin["key"].as<std::string>();

        config::WiFi4* wifi4 = new (std::nothrow) config::WiFi4(cfg_key_e::WIFI4);
        if (wifi4 == nullptr)
        {
            LOG_ERROR(logger, );
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }
        
        

        const auto retAUTH      = convertToAuth(AUTH);
        const auto retWPA2AUTH  = convertToEapAuth(WPA2AUTH);

        if(DHCP == false)
        {
            const auto retIP        = convertToIPv4(IP,false);
            const auto retSVM       = convertToIPv4(SVM,true);
            const auto retGTW       = convertToIPv4(GTW,false);
            const auto retDNS1      = convertToIPv4(DNS1,false);
            const auto retDNS2      = convertToIPv4(DNS2,false);

            if (retIP.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID WIFI IPv4: %s", IP.c_str());
                goto INVALID_WIFI4;
            }

            if (retSVM.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID WIFI Subnetmask: %s", SVM.c_str());
                goto INVALID_WIFI4;
            }

            if (retGTW.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID WIFI Gateway: %s", GTW.c_str());
                goto INVALID_WIFI4;
            }

            if (retDNS1.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID WIFI IPv4: %s", IP.c_str());
                goto INVALID_WIFI4;
            }

            if (retDNS2.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID WIFI IPv4: %s", IP.c_str());
                goto INVALID_WIFI4;
            }
            wifi4->SetStaticIPv4(retIP.second);
            wifi4->SetSubnetmask(retSVM.second);
            wifi4->SetGateway(retGTW.second);
            wifi4->SetDNS1(retDNS1.second);
            wifi4->SetDNS2(retDNS2.second);
        }
        

        if (retAUTH.first.ToCode() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "INVALID WIFI AUTH: %d", AUTH);
            goto INVALID_WIFI4;
        }

        if (retWPA2AUTH.first.ToCode() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "INVALID WIFI WPA AUTH: %d", WPA2AUTH);
            goto INVALID_WIFI4;
        }


        wifi4->SetDHCP(DHCP);
        wifi4->SetAuthMode(retAUTH.second);
        wifi4->SetEapAuthMode(retWPA2AUTH.second);
        wifi4->SetSSID(SSID);

        
        if(cin["psk"].isNull() == false)
        {
            wifi4->SetPSK(PSK);
        }

        if(cin["eap"].isNull() == false)
        {
            wifi4->SetEAP(EAP);
        }

        if(cin["id"].isNull() == false)
        {
            wifi4->SetEapID(EapID);
        }

        if(cin["user"].isNull() == false)
        {
            wifi4->SetEapUserName(EapUser);
        }

        if(cin["pass"].isNull() == false)
        {
            wifi4->SetEapPassword(EapPassword);
        }

        if(cin["ca_cert"].isNull() == false)
        {
            wifi4->SetEapCaCertificate(CaCertifiacte);
        }

        if(cin["crt"].isNull() == false)
        {
            wifi4->SetEapClientCertificate(ClientCertificate);
        }

        if(cin["key"].isNull() == false)
        {
            wifi4->SetEapClientKey(ClientKey);
        }

        ret = emplaceCIN(static_cast<config::Base*>(wifi4), outVector);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO EMPLACE CONFIG INSTANCE: %s", ret.c_str());
            return ret;
        }

    LOG_VERBOSE(logger, "Valid WiFi4 config instance")
    return Status(Status::Code::GOOD);
    
INVALID_WIFI4:
    return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
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
    
        bool DHCP = json["dhcp"].as<bool>();

        // DHCP가 아닌 경우에는 IP정보가 필수로 입력 되어야 함
        if (!DHCP) 
        {
            isValid &= json["ip"].isNull() == false;
            isValid &= json["snm"].isNull() == false;
            isValid &= json["gtw"].isNull() == false;
            isValid &= json["dns1"].isNull() == false;
            isValid &= json["dns2"].isNull() == false;
        }

        // AUTH값이 0이 아니고 EAP 값이 FALSE일때만 PSK 입력 필요// EAP가 TURE이면 PSK가 들어와도 사용하지 않기 때문에 우선 ERROR 처리하지 않음
        uint8_t Auth = json["auth"].as<uint8_t>();
        bool EAP = json["eap"].as<bool>();

        if (Auth != 0 && EAP == false)
        {
            isValid &= json["psk"].isNull() == false;
        } 

        // WPA2AUTH 값이 0이 아닌 경우 EAP ID, EAP USERNAME, EAP PASSWORD 입력 필요
        uint8_t Wpa2Auth = json["wpa2auth"].as<uint8_t>();
        if (Wpa2Auth == 0)
        {
            isValid &= json["crt"].isNull() == false;
            isValid &= json["key"].isNull() == false;
        }
        else
        {
            isValid &= json["id"].isNull() == false;
            isValid &= json["user"].isNull() == false;
            isValid &= json["pass"].isNull() == false;
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
        
        const bool DHCP                     = cin["dhcp"].as<bool>();
        const std::string IP                = cin["ip"].as<std::string>();
        const std::string SVM               = cin["snm"].as<std::string>();
        const std::string GTW               = cin["gtw"].as<std::string>();
        const std::string DNS1              = cin["dns1"].as<std::string>();
        const std::string DNS2              = cin["dns2"].as<std::string>();
    
        config::Ethernet* ethernet = new config::Ethernet(cfg_key_e::ETHERNET);
        
        ethernet->SetDHCP(DHCP);
        if(DHCP == false)
        {
            const auto retIP        = convertToIPv4(IP,false);
            const auto retSVM       = convertToIPv4(SVM,true);
            const auto retGTW       = convertToIPv4(GTW,false);
            const auto retDNS1      = convertToIPv4(DNS1,false);
            const auto retDNS2      = convertToIPv4(DNS2,false);

            if (retIP.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID ETHERNET IPv4: %s", IP.c_str());
                goto INVALID_ETHERNET;
            }

            if (retSVM.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID ETHERNET Subnetmask: %s", SVM.c_str());
                goto INVALID_ETHERNET;
            }

            if (retGTW.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID ETHERNET Gateway: %s", GTW.c_str());
                goto INVALID_ETHERNET;
            }

            if (retDNS1.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID ETHERNET IPv4: %s", IP.c_str());
                goto INVALID_ETHERNET;
            }

            if (retDNS2.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID ETHERNET IPv4: %s", IP.c_str());
                goto INVALID_ETHERNET;
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
    
INVALID_ETHERNET:
    return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
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
    
        bool DHCP = json["dhcp"].as<bool>();

        // DHCP가 아닌 경우에는 IP정보가 필수로 입력 되어야 함
        if (!DHCP) 
        {
            isValid &= json["ip"].isNull() == false;
            isValid &= json["snm"].isNull() == false;
            isValid &= json["gtw"].isNull() == false;
            isValid &= json["dns1"].isNull() == false;
            isValid &= json["dns2"].isNull() == false;
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
            LOG_ERROR(logger, "%s: CIN class: RS-232, CIN address: %p", e.what(), cin);
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(logger, "%s: CIN class: RS-232, CIN address: %p", e.what(), cin);
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        }
    }

    std::pair<Status, md_e> NetworkValidator::convertToLteModel(const std::string model)
    {
        if (model == "LM5") 
        {
            return std::make_pair(Status(Status::Code::GOOD), md_e::LM5);
        } 
        else if (model == "LCM300") 
        {
            return std::make_pair(Status(Status::Code::GOOD), md_e::LCM300);
        } 
        else 
        {
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), md_e::LM5);
        }
    }

    std::pair<Status, ctry_e> NetworkValidator::convertToLteCountry(const std::string country)
    {
        if (country == "KR") 
        {
            return std::make_pair(Status(Status::Code::GOOD), ctry_e::KOREA);
        } 
        else if (country == "USA") 
        {
            return std::make_pair(Status(Status::Code::GOOD), ctry_e::USA);
        } 
        else 
        {
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), ctry_e::KOREA);
        }
    }

    std::pair<Status, wifi_auth_mode_t> NetworkValidator::convertToAuth(const uint8_t auth)
    {
        switch (auth)
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

    std::pair<Status, wpa2_auth_method_t> NetworkValidator::convertToEapAuth(const uint8_t eapAuth)
    {
        switch (eapAuth)
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

    std::pair<Status, IPAddress> NetworkValidator::convertToIPv4(const std::string ip, const bool& isSubnetmask)
    {
        IPAddress IPv4;

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
        if (std::regex_match(ip, validationRegex))
        {
            if (IPv4.fromString(ip.c_str()))  // fromString 함수가 IP 변환에 성공했는지 확인
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