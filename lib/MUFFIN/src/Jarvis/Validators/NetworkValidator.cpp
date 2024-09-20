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
            // ret = validateRS485(arrayCIN, outVector);
            break;
        case cfg_key_e::WIFI4:
            // ret = validateRS485(arrayCIN, outVector);
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

        if (md != "LM5" && md != "LCM300")
        {
            LOG_ERROR(logger, "INVALID LTE Cat.M1 Model: %s", md.c_str());
            goto INVALID_LTE_CatM1;
        }

        if (ctry != "KR" && ctry != "USA")
        {
            LOG_ERROR(logger, "INVALID LTE Cat.M1 Country: %s", ctry.c_str());
            goto INVALID_LTE_CatM1;
        }

        config::CatM1* catM1 = new config::CatM1("catm1");
        catM1->SetModel(md);
        catM1->SetCounty(ctry);

        Status ret = emplaceCIN(static_cast<config::Base*>(rs232), outVector);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO EMPLACE CONFIG INSTANCE: %s", ret.c_str());
            return ret;
        }

    LOG_VERBOSE(logger, "Valid RS-232 config instance")
    return Status(Status::Code::GOOD);
    
INVALID_LTE_CatM1:
    return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
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

    std::pair<Status, prt_e> NetworkValidator::convertToPortIndex(const uint8_t portIndex)
    {
        switch (portIndex)
        {
        case 2:
            return std::make_pair(Status(Status::Code::GOOD), prt_e::PORT_2);
        
        #if !defined(MODLINK_L) && !defined(MODLINK_ML10)
        case 3:
            return std::make_pair(Status(Status::Code::GOOD), prt_e::PORT_3);
        #endif

        default:
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), prt_e::PORT_2);
        }
    }

    std::pair<Status, bdr_e> NetworkValidator::convertToBaudRate(const uint32_t baudRate)
    {
        switch (baudRate)
        {
        case 9600:
        case 19200:
        case 38400:
        case 115200:
            return std::make_pair(Status(Status::Code::GOOD), static_cast<bdr_e>(baudRate));
        default:
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), bdr_e::BDR_9600);
        }
    }

    std::pair<Status, dbit_e> NetworkValidator::convertToDataBit(const uint8_t dataBit)
    {
        switch (dataBit)
        {
        case 5:
        case 6:
        case 7:
        case 8:
            return std::make_pair(Status(Status::Code::GOOD), static_cast<dbit_e>(dataBit));
        default:
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), dbit_e::DBIT_8);
        }
    }

    std::pair<Status, pbit_e> NetworkValidator::convertToParityBit(const uint8_t parityBit)
    {
        switch (parityBit)
        {
        case 0:
        case 1:
        case 2:
            return std::make_pair(Status(Status::Code::GOOD), static_cast<pbit_e>(parityBit));
        default:
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), pbit_e::NONE);
        }
    }

    std::pair<Status, sbit_e> NetworkValidator::convertToStopBit(const uint8_t stopBit)
    {
        switch (stopBit)
        {
        case 1:
        case 2:
            return std::make_pair(Status(Status::Code::GOOD), static_cast<sbit_e>(stopBit));
        default:
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), sbit_e::SBIT_1);
        }
    }
}}