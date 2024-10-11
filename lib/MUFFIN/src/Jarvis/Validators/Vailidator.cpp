/**
 * @file Validator.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief JARVIS 설정 정보의 유효성을 검사하기 위한 모듈 클래스를 정의합니다.
 * 
 * @date 2024-10-06
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include <WString.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Jarvis/Config/Interfaces/Rs232.h"
#include "Jarvis/Config/Interfaces/Rs485.h"
#include "Jarvis/Include/Helper.h"
#include "Validator.h"
#include "Jarvis/Validators/MetaDataValidator.h"
#include "Jarvis/Validators/SerialPortValidator.h"
#include "Jarvis/Validators/NetworkValidator.h"


 
namespace muffin { namespace jarvis {

    Validator::Validator()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }

    Validator::~Validator()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    Status Validator::Inspect(const JsonDocument& jsonDocument, std::map<cfg_key_e, cin_vector>* mapCIN)
    {
        ASSERT((mapCIN != nullptr), "OUTPUT PARAMETER <mapCIN> CANNOT BE A NULL POINTER");
        ASSERT((mapCIN->size() == 0), "OUTPUT PARAMETER <mapCIN> MUST BE EMPTY");

        if (jsonDocument.is<JsonObject>() == false)
        {
            LOG_ERROR(logger, "JSON FOR JARVIS MUST HAVE KEY-VALUE PAIRS");
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }
        
        JsonObject json = jsonDocument.as<JsonObject>();
        Status ret = validateMetaData(json);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "INVALID JARVIS METADATA: %s", ret.c_str());
            return ret;
        }
        /*컨테이너 내부 키(key)를 비롯하여 모든 메타데이터는 유효합니다.*/

        ret = emplacePairsForCIN(mapCIN);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO VALIDATE: %s", ret.c_str());
            return Status(Status::Code::BAD_INTERNAL_ERROR);
        }
        /*NULL 값이 아닌 모든 키(key)에 대한 키-값 쌍이 <*mapCIN>에 생성되었습니다.*/

        for (auto pair : json)
        {
            const char* strKey = pair.key().c_str();
            const auto result  = ConvertToConfigKey(mProtocolVersion, strKey);
            ASSERT((result.first.ToCode() == Status::Code::GOOD), "METADATA VALIDATION PROCESS HAS BEEN COMPROMISED");

            const cfg_key_e cinKey   = result.second;
            const JsonArray cinArray = pair.value().as<JsonArray>();
            cin_vector& outputVector  = mapCIN->at(cinKey);

            switch (cinKey)
            {
                case cfg_key_e::RS232:
                case cfg_key_e::RS485:
                    ret = validateSerialPort(cinKey, cinArray, &outputVector);
                    break;
                case cfg_key_e::WIFI4:
                case cfg_key_e::ETHERNET:
                case cfg_key_e::LTE_CatM1:
                    ret = validateNetwork(cinKey, cinArray, &outputVector);
                    break;
                case cfg_key_e::MODBUS_RTU:
                case cfg_key_e::MODBUS_TCP:
                    break;
                case cfg_key_e::OPERATION:
                    break;
                case cfg_key_e::NODE:
                    break;
                case cfg_key_e::ALARM:
                    break;
                case cfg_key_e::OPERATION_TIME:
                    break;
                case cfg_key_e::PRODUCTION_INFO:
                    break;
                default:
                    ASSERT(false, "UNDEFINED CIN KEY");
                    break;
            }

            if (ret != Status::Code::GOOD)
            {
                JsonDocument doc;
                doc.add(pair);

                std::string payload;
                serializeJson(doc, payload);
                
                LOG_ERROR(logger, "INVALID CONFIG INSTANCE: %s: %s", strKey, payload.c_str());
                mapCIN->clear();

                return ret;
            }
        }
    }

    Status Validator::emplacePairsForCIN(std::map<cfg_key_e, cin_vector>* mapCIN)
    {
        Status ret(Status::Code::UNCERTAIN);
        std::exception exception;
        cfg_key_e cinKey;

        for (const auto key : mContainerKeySet)
        {
            try
            {
                auto result = mapCIN->emplace(key, std::vector<config::Base*>());
                ASSERT((result.second == true), "FAILED TO EMPLACE NEW PAIR SINCE IT ALREADY EXISTS WHICH DOESN'T MAKE ANY SENSE");
                cinKey = key;
            }
            catch(const std::bad_alloc& e)
            {
                ret = Status::Code::BAD_OUT_OF_MEMORY;
                exception = e;
                goto ON_FAIL;
            }
            catch(const std::exception& e)
            {
                ret = Status::Code::BAD_UNEXPECTED_ERROR;
                exception = e;
                goto ON_FAIL;
            }
        }

        LOG_VERBOSE(logger, "Emplaced pairs for CIN map");
        return Status(Status::Code::GOOD);
    
    ON_FAIL:
        LOG_ERROR(logger, "%s: CIN class: %s", exception.what(), ConverKeyToString(cinKey));
        mapCIN->clear();
        return ret;
    }

    Status Validator::validateMetaData(const JsonObject json)
    {
        MetaDataValidator validator;
        Status ret = validator.Inspect(json);
        if (ret != Status(Status::Code::GOOD))
        {
            return ret;
        }

        mProtocolVersion   = validator.RetrieveProtocolVersion();
        mRequestIdentifier = validator.RetrieveRequestID();
        mContainerKeySet   = validator.RetrieveContainerKeys();
    
    #if defined(DEBUG)
        LOG_DEBUG(logger, "Protocol Version: v%u", static_cast<uint8_t>(mProtocolVersion));
        LOG_DEBUG(logger, "Request Identifier: %s", mRequestIdentifier);
        for (const auto& key : mContainerKeySet)
        {
            LOG_DEBUG(logger, "Key: %s", ConverKeyToString(key));
        }
    #endif

        return ret;
    }

    Status Validator::validateSerialPort(const cfg_key_e key, const JsonArray json, cin_vector* outputVector)
    {
        ASSERT((outputVector != nullptr), "OUTPUT PARAMETER <outputVector> CANNOT BE A NULL POINTER");
        ASSERT((outputVector->size() == 0), "OUTPUT PARAMETER <outputVector> MUST BE EMPTY");
        
        SerialPortValidator validator;
        Status ret = validator.Inspect(key, json, outputVector);
        if (ret != Status(Status::Code::GOOD))
        {
            LOG_ERROR(logger, "INVALID SERIAL PORT CONFIG: %s", ret.c_str());
        }
        else
        {
            LOG_INFO(logger, "Serial Port Config: %s", ret.c_str());
        }
        return ret;
    }

    Status Validator::validateNetwork(const cfg_key_e key, const JsonArray json, cin_vector* outputVector)
    {
        ASSERT((outputVector != nullptr), "OUTPUT PARAMETER <outputVector> CANNOT BE A NULL POINTER");
        ASSERT((outputVector->size() == 0), "OUTPUT PARAMETER <outputVector> MUST BE EMPTY");
        
        NetworkValidator validator;
        Status ret = validator.Inspect(key, json, outputVector);
        if (ret != Status(Status::Code::GOOD))
        {
            LOG_ERROR(logger, "INVALID NETWORK CONFIG: %s", ret.c_str());
        }
        else
        {
            LOG_INFO(logger, "Network Config: %s", ret.c_str());
        }
        return ret;
    }



    Status Validator::VailidateRs485(JsonPair& pair)
    {
        JsonArray params = pair.value().as<JsonArray>();
        
        for (JsonObject param : params)
        {
            const bool hasKeys = 
            param.containsKey("prt")  == true &&
            param.containsKey("bdr")  == true &&
            param.containsKey("dbit") == true &&
            param.containsKey("pbit") == true &&
            param.containsKey("sbit") == true;

            if (hasKeys == false)
            {
                LOG_ERROR(logger, "[DECODING ERROR] INVALID MESSAGE STRUCTURE");

                assert(hasKeys == true);
                return Status(Status::Code::BAD_DECODING_ERROR);
            }

            uint8_t PortName   = param["prt"].as<uint8_t>();
            uint32_t BaudRate  = param["bdr"].as<uint32_t>();
            uint8_t DataBit    = param["dbit"].as<uint8_t>();
            uint8_t ParityBit  = param["pbit"].as<uint8_t>();
            uint8_t StopBit    = param["sbit"].as<uint8_t>();

            const bool isValidPortname = PortName == 2 || PortName == 3 ;

            // 시리얼포트 설정이 중복해서 들어왔는지 확인
            #ifdef MODLINK_L
                if(PortName == 2)
                {
                    if(mHasPortNumber02 == true)
                    {
                        LOG_ERROR(logger, "[DECODING ERROR] SERIAL PORT #1 IS ALREADY OCCUPIED");

                        assert(mHasPortNumber02 == false);
                        return Status(Status::Code::BAD_ALREADY_EXISTS);
                    }
                    else
                    {
                        mHasPortNumber02 = true;
                    }
                }
            #else
                if(PortName == 2)
                {
                    if(mHasPortNumber02 == true)
                    {
                        LOG_ERROR(logger, "[DECODING ERROR] SERIAL PORT #2 IS ALREADY OCCUPIED");

                        assert(mHasPortNumber02 == false);
                        return Status(Status::Code::BAD_ALREADY_EXISTS);
                    }
                    else
                    {
                        mHasPortNumber02 = true;
                    }
                }
                else if(PortName == 3)
                {
                    if(mHasPortNumber03 == true)
                    {
                        LOG_ERROR(logger, "[DECODING ERROR] SERIAL PORT #3 IS ALREADY OCCUPIED");

                        assert(mHasPortNumber03 == false);
                        return Status(Status::Code::BAD_ALREADY_EXISTS);
                    }
                    else
                    {
                        mHasPortNumber03 = true;
                    }
                }
            #endif

            if (isValidPortname == false)
            {
                LOG_ERROR(logger, "[DECODING ERROR] INVALID PORTNAME: {%d}", PortName);

                assert(isValidPortname == true);
                return Status(Status::Code::BAD_DECODING_ERROR);
            }

            const bool isValidBaudRate = 
            BaudRate == 9600    ||
            BaudRate == 19200   ||
            BaudRate == 38400   ||
            BaudRate == 115200;

            if (isValidBaudRate == false)
            {
                LOG_ERROR(logger, "[DECODING ERROR] INVALID DATA BAUDRATE: {%lu}", BaudRate);

                assert(isValidPortname == true);
                return Status(Status::Code::BAD_DECODING_ERROR);
            }

            const bool isValidDataBit = 
            DataBit == 5   ||
            DataBit == 6   ||
            DataBit == 7   ||
            DataBit == 8;

            if (isValidDataBit == false)
            {
                LOG_ERROR(logger, "[DECODING ERROR] INVALID DATA DATABIT: {%d}", DataBit);

                assert(isValidDataBit == true);
                return Status(Status::Code::BAD_DECODING_ERROR);
            }

            const bool isValidParityBit = 
            ParityBit == 0   ||
            ParityBit == 1   ||
            ParityBit == 2;

            if (isValidParityBit == false)
            {
                LOG_ERROR(logger, "[DECODING ERROR] INVALID DATA PARITYBIT: {%d}", ParityBit);

                assert(isValidParityBit == true);
                return Status(Status::Code::BAD_DECODING_ERROR);
            }

            const bool isValidStopBit = 
            StopBit == 1   ||
            StopBit == 2;

            if (isValidStopBit == false)
            {
                LOG_ERROR(logger, "[DECODING ERROR] INVALID DATA STOPBIT: {%d}", StopBit);

                assert(isValidStopBit == true);
                return Status(Status::Code::BAD_DECODING_ERROR);
            }
        }

        return Status(Status::Code::GOOD);
    }

    Status Validator::VailidateWIFI(JsonPair& pair)
    {
        JsonArray params = pair.value().as<JsonArray>();

        // WIFI는 오직 1개의 설정만 가능, 사이즈는 무조건1이어야 함
        if(params.size() == 1 )
        {
             LOG_ERROR(logger, "[DECODING ERROR] WIFI ALLOWS ONLY ONE SETTING , BUT FOUND {%d} SETTINGS", params.size());

            assert(params.size() != 1);
            return Status(Status::Code::BAD_DECODING_ERROR);
        }

        JsonObject param = params[0];

        const bool hasKeys = (
        param.containsKey("dhcp")       == true &&
        param.containsKey("ip")         == true &&
        param.containsKey("snm")        == true &&
        param.containsKey("gtw")        == true &&
        param.containsKey("dns1")       == true &&
        param.containsKey("dns2")       == true &&
        param.containsKey("ssid")       == true &&
        param.containsKey("psk")        == true &&
        param.containsKey("auth")       == true &&
        param.containsKey("eap")        == true &&
        param.containsKey("wpa2auth")   == true &&
        param.containsKey("id")         == true &&
        param.containsKey("user")       == true &&
        param.containsKey("pass")       == true &&
        param.containsKey("ca_cert")    == true &&
        param.containsKey("crt")        == true &&
        param.containsKey("key")        == true
        );

        if (hasKeys == false)
        {
            LOG_ERROR(logger, "[DECODING ERROR] INVALID MESSAGE STRUCTURE");

            assert(hasKeys == true);
            return Status(Status::Code::BAD_DECODING_ERROR);
        }

        std::string SSID = param["ssid"].as<std::string>();
        //SSID는 필수
        if(SSID.empty())
        {
            LOG_ERROR(logger, "[DECODING ERROR] SSID CANNOT BE EMPTY");

            assert(!(SSID.empty()));
            return Status(Status::Code::BAD_DECODING_ERROR);
        }


        bool DHCP = param["dhcp"].as<bool>();

        std::string Ipv4 = param["ip"].as<std::string>();
        std::string Subnet = param["snm"].as<std::string>();
        std::string Gateway = param["gtw"].as<std::string>();
        std::string DNS1 = param["dns1"].as<std::string>();
        std::string DNS2 = param["dns2"].as<std::string>();

        // DHCP가 아닌 경우에는 IP정보가 필수로 입력 되어야 함
        if(DHCP == false && (Ipv4.empty() || Subnet.empty() || Gateway.empty() || DNS1.empty() || DNS2.empty()))
        {
            LOG_ERROR(logger, "[DECODING ERROR] IF DHCP IS DISABLED, ALL IP INFORMATION MUST BE PROVIDED");

            assert(!(DHCP == false && (Ipv4.empty() || Subnet.empty() || Gateway.empty() || DNS1.empty() || DNS2.empty())));
            return Status(Status::Code::BAD_DECODING_ERROR);
        }

        // check if the ipv4 addresses are valid
        std::map<std::string, Status> mapRet;
        mapRet.emplace("IP ADDRESS",   ValidateIPv4Address(Ipv4 , false));
        mapRet.emplace("SUBNETMASK",   ValidateIPv4Address(Subnet , true));
        mapRet.emplace("GATEWAY",      ValidateIPv4Address(Gateway, false));
        mapRet.emplace("DNS SERVER 1", ValidateIPv4Address(DNS1 ,  false));
        mapRet.emplace("DNS SERVER 2", ValidateIPv4Address(DNS2 ,  false));

        for (auto& ret : mapRet)
        {
            if (ret.second != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "[DECODING ERROR] INVALID IPv4 Address {%s}", ret.first.c_str());

                assert(ret.second == Status::Code::GOOD);
                return ret.second;
            }
        }

        
        std::string PSK = param["psk"].as<std::string>();
        uint8_t Auth = param["auth"].as<uint8_t>();
        bool EAP = param["eap"].as<bool>();
        uint8_t Wpa2Auth = param["wpa2auth"].as<uint8_t>();
        

        const bool isValidAuth = (
            Auth == 0    ||
            Auth == 1    ||
            Auth == 2    ||
            Auth == 3    ||
            Auth == 4);

        if (isValidAuth == false)
        {
            LOG_ERROR(logger, "[DECODING ERROR] INVALID DATA BAUDRATE: {%d}", isValidAuth);

            assert(isValidAuth == true);
            return Status(Status::Code::BAD_DECODING_ERROR);
        }

        const bool isValidWpa2Auth =(
            Wpa2Auth == 0    ||
            Wpa2Auth == 1    ||
            Wpa2Auth == 2);

        if (isValidWpa2Auth == false)
        {
            LOG_ERROR(logger, "[DECODING ERROR] INVALID DATA BAUDRATE: {%d}", isValidWpa2Auth);

            assert(isValidWpa2Auth == true);
            return Status(Status::Code::BAD_DECODING_ERROR);
        }

        // AUTH값이 0이 아니고 EAP 값이 FALSE일때만 PSK 입력 필요// EAP가 TURE이면 PSK가 들어와도 사용하지 않기 때문에 우선 ERROR 처리하지 않음
        if((Auth != 0 && EAP == false) && PSK.empty()) 
        {
            LOG_ERROR(logger, "[DECODING ERROR] IF AUTHENTICATION IS REQUIRED (Auth != 0) AND EAP IS DISABLED, PSK MUST BE PROVIDED");

            assert(!((Auth != 0 && EAP == false) && PSK.empty()));
            return Status(Status::Code::BAD_DECODING_ERROR);
        }

        std::string EapID = param["id"].as<std::string>();
        std::string EapUser = param["user"].as<std::string>();
        std::string EapPassword = param["pass"].as<std::string>();
        std::string CaCertificate = param["ca_cert"].as<std::string>();
        std::string ClientCertificate = param["crt"].as<std::string>();
        std::string ClientKey = param["key"].as<std::string>();

        // WPA2AUTH 값이 0이 아닌 경우 EAP ID, EAP USERNAME, EAP PASSWORD 입력 필요
        if(Wpa2Auth != 0 && (EapID.empty() || EapUser.empty() || EapPassword.empty()))
        {
            LOG_ERROR(logger, "[DECODING ERROR] IF AUTHENTICATION IS REQUIRED (Wpa2Auth != 0), EapID, EapUSER, EapPASSWORD MUST BE PROVIDED");

            assert(!(Wpa2Auth != 0 && (EapID.empty() || EapUser.empty() || EapPassword.empty())));
            return Status(Status::Code::BAD_DECODING_ERROR);
        }

        // WPA2AUTH 값이 0인 경우, CLIENT CERTIFICATE, CLIENT KEY 입력 필요
        if(Wpa2Auth == 0 && (ClientCertificate.empty() || ClientKey.empty()))
        {
            LOG_ERROR(logger, "[DECODING ERROR] IF AUTHENTICATION IS NOT REQUIRED (Wpa2Auth == 0), CLIENT CERTIFICATE AND CLIENT KEY MUST BE PROVIDED");

            assert(!(Wpa2Auth == 0 && (ClientCertificate.empty() || ClientKey.empty())));
            return Status(Status::Code::BAD_DECODING_ERROR);
        }

        return Status(Status::Code::GOOD);
    }

    Status Validator::VailidateETH(JsonPair& pair)
    {
        JsonArray params = pair.value().as<JsonArray>();

        // ETH는 오직 1개의 설정만 가능, 사이즈는 무조건1이어야 함
        if(params.size() == 1 )
        {
             LOG_ERROR(logger, "[DECODING ERROR] ETHERNET ALLOWS ONLY ONE SETTING , BUT FOUND {%d} SETTINGS", params.size());

            assert(params.size() != 1);
            return Status(Status::Code::BAD_DECODING_ERROR);
        }

        JsonObject param = params[0];

        const bool hasKeys = (
        param.containsKey("dhcp")       == true &&
        param.containsKey("ip")         == true &&
        param.containsKey("snm")        == true &&
        param.containsKey("gtw")        == true &&
        param.containsKey("dns1")       == true &&
        param.containsKey("dns2")       == true 
        );

        if (hasKeys == false)
        {
            LOG_ERROR(logger, "[DECODING ERROR] INVALID MESSAGE STRUCTURE");

            assert(hasKeys == true);
            return Status(Status::Code::BAD_DECODING_ERROR);
        }

        bool DHCP = param["dhcp"].as<bool>();

        std::string Ipv4 = param["ip"].as<std::string>();
        std::string Subnet = param["snm"].as<std::string>();
        std::string Gateway = param["gtw"].as<std::string>();
        std::string DNS1 = param["dns1"].as<std::string>();
        std::string DNS2 = param["dns2"].as<std::string>();

        // DHCP가 아닌 경우에는 IP정보가 필수로 입력 되어야 함
        if(DHCP == false && (Ipv4.empty() || Subnet.empty() || Gateway.empty() || DNS1.empty() || DNS2.empty()))
        {
            LOG_ERROR(logger, "[DECODING ERROR] IF DHCP IS DISABLED, ALL IP INFORMATION MUST BE PROVIDED");

            assert(!(DHCP == false && (Ipv4.empty() || Subnet.empty() || Gateway.empty() || DNS1.empty() || DNS2.empty())));
            return Status(Status::Code::BAD_DECODING_ERROR);
        }

        // check if the ipv4 addresses are valid
        std::map<std::string, Status> mapRet;
        mapRet.emplace("IP ADDRESS",   ValidateIPv4Address(Ipv4 , false));
        mapRet.emplace("SUBNETMASK",   ValidateIPv4Address(Subnet , true));
        mapRet.emplace("GATEWAY",      ValidateIPv4Address(Gateway, false));
        mapRet.emplace("DNS SERVER 1", ValidateIPv4Address(DNS1 ,  false));
        mapRet.emplace("DNS SERVER 2", ValidateIPv4Address(DNS2 ,  false));

        for (auto& ret : mapRet)
        {
            if (ret.second != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "[DECODING ERROR] INVALID IPv4 Address {%s}", ret.first.c_str());

                assert(ret.second == Status::Code::GOOD);
                return ret.second;
            }
        }

        return Status(Status::Code::GOOD);
    }

    Status Validator::VailidateLTE(JsonPair& pair)
    {
        JsonArray params = pair.value().as<JsonArray>();

        // LTE는 오직 1개의 설정만 가능, 사이즈는 무조건1이어야 함
        if(params.size() == 1 )
        {
             LOG_ERROR(logger, "[DECODING ERROR] ETHERNET ALLOWS ONLY ONE SETTING , BUT FOUND {%d} SETTINGS", params.size());

            assert(params.size() != 1);
            return Status(Status::Code::BAD_DECODING_ERROR);
        }

        JsonObject param = params[0];

        const bool hasKeys = (
        param.containsKey("md")       == true &&
        param.containsKey("ctry")     == true
        );

        if (hasKeys == false)
        {
            LOG_ERROR(logger, "[DECODING ERROR] INVALID MESSAGE STRUCTURE");

            assert(hasKeys == true);
            return Status(Status::Code::BAD_DECODING_ERROR);
        }

        std::string Model = param["md"].as<std::string>();
        std::string Country = param["ctry"].as<std::string>();
     
        const bool isValidModel =(
            Model == "LM5"      ||
            Model == "LCM300"
        );

        if (isValidModel == false)
        {
            LOG_ERROR(logger, "[DECODING ERROR] INVALID DATA MODEL: {%s}", Model.c_str());

            assert(isValidModel == true);
            return Status(Status::Code::BAD_DECODING_ERROR);
        }

        const bool isValidCountry =(
            Country == "LM5"      ||
            Country == "LCM300"
        );

        if (isValidCountry == false)
        {
            LOG_ERROR(logger, "[DECODING ERROR] INVALID DATA COUNTRY: {%s}", Country.c_str());

            assert(isValidCountry == true);
            return Status(Status::Code::BAD_DECODING_ERROR);
        }


        return Status(Status::Code::GOOD);
    }

    Status Validator::VailidateModbusRTU(JsonPair& pair)
    {
        JsonArray params = pair.value().as<JsonArray>();
        
        for (JsonObject param : params)
        {
            const bool hasKeys = 
            param.containsKey("prt")  == true &&
            param.containsKey("sid")  == true &&
            param.containsKey("nodes") == true ;

            if (hasKeys == false)
            {
                LOG_ERROR(logger, "[DECODING ERROR] INVALID MESSAGE STRUCTURE");

                assert(hasKeys == true);
                return Status(Status::Code::BAD_DECODING_ERROR);
            }

            uint8_t PortName    = param["prt"].as<uint8_t>();
            uint32_t SlaveID    = param["sid"].as<uint8_t>();
            JsonArray nodes     = param["nodes"].as<JsonArray>();
      

            const bool isValidPortname = PortName == 2 || PortName == 3 ;

            // 시리얼포트 설정이 중복해서 들어왔는지 확인
            #ifdef MODLINK_L
                if(PortName == 2)   
                {
                    if(mHasPortNumber02 == true)
                    {
                        LOG_ERROR(logger, "[DECODING ERROR] SERIAL PORT #1 IS ALREADY OCCUPIED");

                        assert(mHasPortNumber02 == false);
                        return Status(Status::Code::BAD_ALREADY_EXISTS);
                    }
                    else
                    {
                        mHasPortNumber02 = true;
                    }
                }
            #else
                if(PortName == 2)
                {
                    if(mHasPortNumber02 == true)
                    {
                        LOG_ERROR(logger, "[DECODING ERROR] SERIAL PORT #2 IS ALREADY OCCUPIED");

                        assert(mHasPortNumber02 == false);
                        return Status(Status::Code::BAD_ALREADY_EXISTS);
                    }
                    else
                    {
                        mHasPortNumber02 = true;
                    }
                }
                else if(PortName == 3)
                {
                    if(mHasPortNumber03 == true)
                    {
                        LOG_ERROR(logger, "[DECODING ERROR] SERIAL PORT #3 IS ALREADY OCCUPIED");

                        assert(mHasPortNumber03 == false);
                        return Status(Status::Code::BAD_ALREADY_EXISTS);
                    }
                    else
                    {
                        mHasPortNumber03 = true;
                    }
                }
            #endif

            if (isValidPortname == false)
            {
                LOG_ERROR(logger, "[DECODING ERROR] INVALID PORTNAME: {%d}", PortName);

                assert(isValidPortname == true);
                return Status(Status::Code::BAD_DECODING_ERROR);
            }

            const bool isValidSlaveID = 
            SlaveID > 0 && SlaveID < 248;

            if (isValidSlaveID == false)
            {
                LOG_ERROR(logger, "[DECODING ERROR] INVALID DATA SLAVE ID: {%d}", SlaveID);

                assert(isValidSlaveID == true);
                return Status(Status::Code::BAD_DECODING_ERROR);
            }

            for (JsonVariant node : nodes)
            {
                std::string nodeID = node.as<std::string>();
                if (nodeID.size() != 4)
                {
                    LOG_ERROR(logger, "DECODING ERROR: INVALID OR CORRUPTED NODE ID: {%s}", nodeID.c_str());

                    assert(nodeID.size() == 4);
                    return Status(Status::Code::BAD_DECODING_ERROR);
                }
            }    
        }


        return Status(Status::Code::GOOD);
    }

    Status Validator::VailidateModbusTCP(JsonPair& pair)
    {
        JsonArray params = pair.value().as<JsonArray>();
        
        for (JsonObject param : params)
        {
            const bool hasKeys = 
            param.containsKey("ip")     == true &&
            param.containsKey("prt")    == true &&
            param.containsKey("ifaces") == true &&
            param.containsKey("sid")    == true &&
            param.containsKey("nodes")  == true;

            if (hasKeys == false)
            {
                LOG_ERROR(logger, "[DECODING ERROR] INVALID MESSAGE STRUCTURE");

                assert(hasKeys == true);
                return Status(Status::Code::BAD_DECODING_ERROR);
            }

            
            std::string Ipv4 = param["ip"].as<std::string>();
            uint32_t PortNumber    = param["prt"].as<uint32_t>();
            std::string Interface = param["ifaces"].as<std::string>();
            uint32_t SlaveID    = param["sid"].as<uint8_t>();
            JsonArray nodes     = param["nodes"].as<JsonArray>();

            Status result =  ValidateIPv4Address(Ipv4 , false);
            if(result != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "[DECODING ERROR] INVALID IPv4 Address {%s}", result.c_str());

                assert(result == Status::Code::GOOD);
                return result;
            }
            

            const bool isValidPortname = PortNumber > 0 && PortNumber <  65536 ;

            if (isValidPortname == false)
            {
                LOG_ERROR(logger, "[DECODING ERROR] INVALID PORTNAME: {%lu}", PortNumber);

                assert(isValidPortname == true);
                return Status(Status::Code::BAD_DECODING_ERROR);
            }


            const bool isValidInterface = 
            Interface == "wifi" || Interface == "eth";

            if (isValidInterface == false)
            {
                LOG_ERROR(logger, "[DECODING ERROR] INVALID DATA SLAVE ID: {%s}", Interface.c_str());

                assert(isValidInterface == true);
                return Status(Status::Code::BAD_DECODING_ERROR);
            }

            const bool isValidSlaveID = 
            SlaveID > 0 && SlaveID < 248;

            if (isValidSlaveID == false)
            {
                LOG_ERROR(logger, "[DECODING ERROR] INVALID DATA SLAVE ID: {%d}", SlaveID);

                assert(isValidSlaveID == true);
                return Status(Status::Code::BAD_DECODING_ERROR);
            }

            for (JsonVariant node : nodes)
            {
                std::string nodeID = node.as<std::string>();
                if (nodeID.size() != 4)
                {
                    LOG_ERROR(logger, "DECODING ERROR: INVALID OR CORRUPTED NODE ID: {%s}", nodeID.c_str());

                    assert(nodeID.size() == 4);
                    return Status(Status::Code::BAD_DECODING_ERROR);
                }
            }    
        }


        return Status(Status::Code::GOOD);
    }

    Status Validator::VailidateNode(JsonPair& pair)
    {
        JsonArray params = pair.value().as<JsonArray>();
        // NodeID 중복 검사를 위한 변수
        std::set<std::string> NodeIdSet;

        for (JsonObject param : params)
        {
            const bool hasKeys = (
            param.containsKey("id")     == true &&
            param.containsKey("adtp")   == true &&
            param.containsKey("addr")   == true &&
            param.containsKey("area")   == true &&
            param.containsKey("bit")    == true &&
            param.containsKey("qty")    == true &&
            param.containsKey("scl")    == true &&
            param.containsKey("ofst")   == true &&
            param.containsKey("map")    == true &&
            param.containsKey("ord")    == true &&
            param.containsKey("dt")     == true &&
            param.containsKey("fmt")    == true &&
            param.containsKey("UID")    == true &&
            param.containsKey("Name")   == true &&
            param.containsKey("Unit")   == true &&
            param.containsKey("event")  == true
            );

            if (hasKeys == false)
            {
                LOG_ERROR(logger, "[DECODING ERROR] INVALID MESSAGE STRUCTURE");

                assert(hasKeys == true);
                return Status(Status::Code::BAD_DECODING_ERROR);
            }

            std::string NodeID = param["id"].as<std::string>();
            // NodeID 사이즈가 4 인지 확인
            if (NodeID.size() != 4)
            {
                LOG_ERROR(logger, "DECODING ERROR: INVALID OR CORRUPTED NODE ID: {%s}", NodeID.c_str());

                assert(NodeID.size() == 4);
                return Status(Status::Code::BAD_DECODING_ERROR);
            }
            // 중복된 NodeID가 있는지 확인
            if(NodeIdSet.find(NodeID) != NodeIdSet.end())
            {
                LOG_ERROR(logger, "[DECODING ERROR] DUPLICATE NODE ID FOUND: %s", NodeID.c_str());

                assert(!(NodeIdSet.find(NodeID) != NodeIdSet.end()));
                return Status(Status::Code::BAD_DECODING_ERROR);
            }

            uint16_t AddressType = param["adtp"].as<uint16_t>();
            // 기계 데이터 수집 주소 타입 확인
            const bool isValidAddressType = 
            AddressType == 0    ||
            AddressType == 1    ||
            AddressType == 2    ||
            AddressType == 3;

            if (isValidAddressType == false)
            {
                LOG_ERROR(logger, "[DECODING ERROR] INVALID DATA ADDRESS TYPE: {%d}", AddressType);

                assert(isValidAddressType == true);
                return Status(Status::Code::BAD_DECODING_ERROR);
            }
            
            // 기계 데이터 수집 주소 타입에 따른 수집 주소 확인 및 타입별 조건 확인
            if(AddressType == 0)
            {
                if(param["addr"].is<uint16_t>() == false)
                {
                    LOG_ERROR(logger, "[DECODING ERROR] INVALID DATA ADDRESS TYPE [UINT16] ");

                    assert(param["addr"].is<uint16_t>() == true);
                    return Status(Status::Code::BAD_DECODING_ERROR);
                }

                Status result = ValidateModbusNodeCondition(param);
                if(result != Status::Code::GOOD)
                {
                    LOG_ERROR(logger, "[DECODING ERROR] INVALID Node Condition {%s}", result.c_str());

                    assert(result == Status::Code::GOOD);
                    return result; 
                }

            }
            else if(AddressType == 1)
            {
                if(param["addr"].is<std::string>() == false)
                {
                    LOG_ERROR(logger, "[DECODING ERROR] INVALID DATA ADDRESS TYPE [STRING] ");

                    assert(param["addr"].is<std::string>() == true);
                    return Status(Status::Code::BAD_DECODING_ERROR);
                }
            }
            else if(AddressType == 2)
            {
                // BYTE STRING 검사 로직 추가 예정 (const char* ?)
            }
            else
            {
                // GUID 검사 로직 추가 예정
            }
        

            
         

          
        }


        return Status(Status::Code::GOOD);
    }

    Status Validator::ValidateIPv4Address(const std::string& ipv4, const bool& isSubnetmask)
    {
        // if (ipv4.length() < 7)
        // {
        //     LOG_ERROR(logger, "[DECODING ERROR] INVALID IPv4 ADDRESS: {%s}", ipv4.c_str());
            
        //     assert(ipv4.length() > 6);
        //     return Status(Status::Code::BAD_DECODING_ERROR);
        // }
        // // end of preconditions


        // // create regular expression for the validation
        // std::regex validationRegex;
        // if (isSubnetmask == true)
        // {
        //     validationRegex.assign("^((255|254|252|248|240|224|192|128|0)\\.){3}(255|254|252|248|240|224|192|128|0)$");
        // }
        // else
        // {
        //     validationRegex.assign("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
        // }


        // // validating IPv4 address using regular expression
        // const bool isValid = std::regex_match(ipv4.c_str(), validationRegex);


        // // returning corresponding status code
        // if (isValid == true)
        // {
        //     return Status(Status::Code::GOOD);
        // }
        // else
        // {
        //     if (isSubnetmask == true)
        //     {
        //         LOG_ERROR(logger, "[DECODING ERROR] INVALID SUBNETMASK ADDRESS: {%s}", ipv4.c_str());
        //     }
        //     else
        //     {
        //         LOG_ERROR(logger, "[DECODING ERROR] INVALID IPv4 ADDRESS: {%s}", ipv4.c_str());
        //     }

        //     assert(isValid == true);
        //     return Status(Status::Code::BAD_DECODING_ERROR);
        // }
    }

    Status Validator::ValidateModbusNodeCondition(JsonObject& node)
    {
        std::string NodeID = node["id"].as<std::string>();
        uint8_t ModbusArea = node["area"].as<uint8_t>();
        uint8_t DataType   = node["dt"].as<uint8_t>();
    
        const bool isValidModbusArea = 
        ModbusArea == 1    ||
        ModbusArea == 2    ||
        ModbusArea == 3    ||
        ModbusArea == 4;

        if (isValidModbusArea == false)
        {
            LOG_ERROR(logger, "[DECODING ERROR] INVALID DATA MODBUS AREA: {%d}", ModbusArea);

            assert(isValidModbusArea == true);
            return Status(Status::Code::BAD_DECODING_ERROR);
        }

        const bool isValidDataType = DataType < 12;
        if (isValidDataType == false)
        {
            LOG_ERROR(logger, "[DECODING ERROR] INVALID DATA MODBUS AREA: {%d}", DataType);

            assert(isValidDataType == true);
            return Status(Status::Code::BAD_DECODING_ERROR);
        }

        // BIT 가 null면 유효성 검사 x
        if(node["bit"].isNull() == false)
        {
            if(ModbusArea == 1 || ModbusArea == 2)
            {
                LOG_ERROR(logger, "[DECODING ERROR] BIT NOT SET IN COIL/DISCRETE INPUT AREA, AREA :{%s}", 
                ModbusArea == 1 ? "COILS" : "DISCRETE INPUTS");

                assert(false);
                return Status(Status::Code::BAD_DECODING_ERROR);
            }

            uint8_t ModbusBit = node["bit"].as<uint8_t>();
            const bool isValidModbusBit = ModbusBit < 16;

            if (isValidModbusBit == false)
            {
                LOG_ERROR(logger, "[DECODING ERROR] INVALID DATA MODBUS AREA: {%d}", ModbusBit);

                assert(isValidModbusBit == true);
                return Status(Status::Code::BAD_DECODING_ERROR);
            }
        }
        
        // QTY 가 null면 유효성 검사 x
        if(node["qty"].isNull() == false)
        {
            uint8_t Quantity = node["qty"].as<uint8_t>();   
            
            if(ModbusArea == 1 || ModbusArea == 2)
            {
                LOG_ERROR(logger, "[DECODING ERROR] QUANTITY NOT SET IN COIL/DISCRETE INPUT AREA, AREA :{%s}", 
                ModbusArea == 1 ? "COILS" : "DISCRETE INPUTS");

                assert(false);
                return Status(Status::Code::BAD_DECODING_ERROR);
            }   
        }

        // SCALE이 null면 유효성 검사 x
        if(node["scl"].isNull() == false)
        {
            if(ModbusArea == 1 || ModbusArea == 2)
            {
                LOG_ERROR(logger, "[DECODING ERROR] SCALE NOT SET IN COIL/DISCRETE INPUT AREA, AREA :{%s}", 
                ModbusArea == 1 ? "COILS" : "DISCRETE INPUTS");

                assert(false);
                return Status(Status::Code::BAD_DECODING_ERROR);
            }

            if( DataType == 11)
            {
                LOG_ERROR(logger, "[DECODING ERROR] SCALE NOT SET IN DataType :{%d}", ModbusArea);

                assert(false);
                return Status(Status::Code::BAD_DECODING_ERROR);
            }

            int8_t Scale = node["scl"].as<int8_t>();   
            // 기계 데이터 수집 주소 타입 확인
            const bool isValidScale = 
            Scale > -4 && Scale < 4 && Scale != 0;

            if (isValidScale == false)
            {
                LOG_ERROR(logger, "[DECODING ERROR] INVALID DATA ADDRESS TYPE: {%d}", Scale);

                assert(isValidScale == true);
                return Status(Status::Code::BAD_DECODING_ERROR);
            }
        }

        // OFFSET이 null면 유효성 검사 x
        if(node["ofst"].isNull() == false)
        {
            if(ModbusArea == 1 || ModbusArea == 2)
            {
                LOG_ERROR(logger, "[DECODING ERROR] OFFSET NOT SET IN COIL/DISCRETE INPUT AREA, AREA :{%s}", 
                ModbusArea == 1 ? "COILS" : "DISCRETE INPUTS");

                assert(false);
                return Status(Status::Code::BAD_DECODING_ERROR);
            }   

            if( DataType == 11)
            {
                LOG_ERROR(logger, "[DECODING ERROR] OFFSET NOT SET IN DataType :{%d}", ModbusArea);

                assert(false);
                return Status(Status::Code::BAD_DECODING_ERROR);
            }

            float Scale = node["ofst"].as<float>();   
        
            const bool isValidScale = 
            Scale > -4 && Scale < 4 && Scale != 0;

            if (isValidScale == false)
            {
                LOG_ERROR(logger, "[DECODING ERROR] INVALID DATA ADDRESS TYPE: {%d}", Scale);

                assert(isValidScale == true);
                return Status(Status::Code::BAD_DECODING_ERROR);
            }
        }
        
        // MAP이 null면 유효성 검사 x
        if(node["map"].isNull() == false)
        {
            std::map<uint16_t, String> mapRules;
            JsonObject rules = node["map"].as<JsonObject>();
            for (JsonPair rule : rules)
            {
                uint16_t key = 0;
                try
                {
                    key = std::stoi(rule.key().c_str());
                }
                catch(const std::invalid_argument& e)
                {
                    LOG_ERROR(logger, "DECODING ERROR: INVALID KEY VALUE: {%s}, {%s}", rule.key(), e.what());

                    assert(false);
                    return Status(Status::Code::BAD_DECODING_ERROR);
                }
                const String value = rules[rule.key().c_str()].as<const char*>();

                const bool isEmplaced = mapRules.emplace(key, value).second;
                if (isEmplaced == false)
                {
                    LOG_ERROR(logger, "INTERNAL ERROR: MAPPING RULES CANNOT BE DUPLICATED");

                    assert(isEmplaced == true);
                    return Status(Status::Code::BAD_INTERNAL_ERROR);
                }
            }
        }
        
        // ORDER가 null면 유효성 검사 x
        if(node["ord"].isNull() == false)
        {
            if(ModbusArea == 1 || ModbusArea == 2)
            {
                LOG_ERROR(logger, "[DECODING ERROR] OORDER NOT SET IN COIL/DISCRETE INPUT AREA, AREA :{%s}", 
                ModbusArea == 1 ? "COILS" : "DISCRETE INPUTS");

                assert(false);
                return Status(Status::Code::BAD_DECODING_ERROR);
            }   

            // create vector to store order data
            JsonArray arrayOrder = node["ord"].as<JsonArray>();
            std::vector<std::string> vOrder;
            try
            {
                vOrder.reserve(arrayOrder.size());
            }
            catch (const std::bad_alloc& e)
            {
                LOG_ERROR(logger, "MEMORY ERROR: [%s] FAILED TO ALLOCATE MEMORY FOR THE ORDER DATA", NodeID.c_str());

                assert(false);
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            }


            // configuring 'ord' attribute of the abject
            for (JsonArray order : arrayOrder)
            {
                std::string dataOrder("");

                for (size_t idx = 0; idx < order.size(); idx++)
                {
                    const char* strOrder = order[idx].as<const char*>();
                    if (*strOrder == 'W')
                    {
                        ++strOrder;

                        while (*strOrder != false)
                        {
                            if (isdigit(*strOrder) == false)
                            {
                                LOG_ERROR(logger, "DECODING ERROR: [%s] INVALID INDEX FOR ORDER: {%c}", 
                                    NodeID.c_str(), *strOrder);

                                assert(isdigit(*strOrder) == true);
                                return Status(Status::Code::BAD_DECODING_ERROR);
                            }
                            ++strOrder;
                        }
                    }
                    else if (*strOrder == 'B')
                    {
                        ++strOrder;

                        if (*strOrder == 'H' || *strOrder == 'L')
                        {
                            ++strOrder;
                        }
                        else
                        {
                            LOG_ERROR(logger, "DECODING ERROR: [%s] BYTE ORDER IS NOT PROVIDED: {%c}", 
                                NodeID.c_str(), *strOrder);

                            assert(*strOrder == 'H' || *strOrder == 'L');
                            return Status(Status::Code::BAD_DECODING_ERROR);
                        }
                        
                        while (*strOrder != false)
                        {
                            if (isdigit(*strOrder) == false)
                            {
                                LOG_ERROR(logger, "DECODING ERROR: [%s] INVALID INDEX FOR ORDER: {%c}", 
                                    NodeID.c_str(), *strOrder);

                                assert(isdigit(*strOrder) == true);
                                return Status(Status::Code::BAD_DECODING_ERROR);
                            }
                            ++strOrder;
                        }
                    }
                    else
                    {
                        LOG_ERROR(logger, "DECODING ERROR: [%s] INVALID ORDER STRING: {%s}", 
                            NodeID.c_str(), strOrder);

                        assert(*strOrder == 'W' || *strOrder == 'B');
                        return Status(Status::Code::BAD_DECODING_ERROR);
                    }

                    const bool isDelimiterNeeded   = idx == order.size() - 1 ? false : true;
                    try 
                    {
                        // Concatenate order[idx] to dataOrder
                        dataOrder += order[idx].as<const char*>();

                        // Concatenate delimiter if needed
                        if (isDelimiterNeeded) {
                            dataOrder += ",";
                        }
                    } 
                    catch (const std::exception& e) 
                    {
                        // Handle concatenation errors
                        LOG_ERROR(logger, "DECODING ERROR: [%s] CONCATENATION STRINGS HAVE FAILED. Error: %s", NodeID.c_str(), e.what());
                        
                        assert(false); // Replace with an appropriate assertion if needed
                        return Status(Status::Code::BAD_DECODING_ERROR);
                    }

                }

                vOrder.emplace_back(dataOrder);
            }


            // check postconditions
            const bool isIdenticalSize = vOrder.size() == arrayOrder.size();
            if (isIdenticalSize == false)
            {
                LOG_ERROR(logger, "INTERNAL ERROR: FAILED TO EMPLACE THE DATA ORDER");
                
                assert(isIdenticalSize == true);
                return Status(Status::Code::BAD_INTERNAL_ERROR);
            }
        }     
     
        return Status(Status::Code::GOOD);
    }
}}