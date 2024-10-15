/**
 * @file SerialPortValidator.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 시리얼 포트에 대한 설정 정보가 유효한지 검사하는 클래스를 정의합니다.
 * 
 * @date 2024-10-15
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "SerialPortValidator.h"
#include "Jarvis/Config/Interfaces/Rs232.h"
#include "Jarvis/Config/Interfaces/Rs485.h"
#include "Jarvis/Include/Helper.h"



namespace muffin { namespace jarvis {

    SerialPortValidator::SerialPortValidator()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    SerialPortValidator::~SerialPortValidator()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    Status SerialPortValidator::Inspect(const cfg_key_e key, const JsonArray arrayCIN, cin_vector* outVector)
    {
        ASSERT((outVector != nullptr), "OUTPUT PARAMETER <outVector> CANNOT BE A NULL POINTER");
        ASSERT((arrayCIN.isNull() == false), "OUTPUT PARAMETER <arrayCIN> CANNOT BE NULL");

        Status ret(Status::Code::UNCERTAIN);

        switch (key)
        {
        case cfg_key_e::RS232:
            ret = validateRS232(arrayCIN, outVector);
            break;
        case cfg_key_e::RS485:
            ret = validateRS485(arrayCIN, outVector);
            break;
        default:
            ASSERT(false, "UNDEFINED SERIAL PORT CONFIGURATION");
            return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
        };

        return ret;
    }

    /**
     * @return rsc_e 
     *      @li GOOD
     *      @li BAD_OUT_OF_MEMORY
     *      @li BAD_UNEXPECTED_ERROR
     *      @li BAD_DATA_ENCODING_INVALID
     *      @li BAD_UNSUPPORTED_CONFIGURATION
     *      @li BAD_INVALID_FORMAT_CONFIG_INSTANCE
     */
    std::pair<rsc_e, std::string> SerialPortValidator::validateRS232(const JsonArray array, cin_vector* outVector)
    {
    // #if defined(MODLINK_L) || defined(MODLINK_ML10)
    //     const std::string message = "RS-232 IS NOT SUPPORTED ON MODLINK-L OR MODLINK-ML10";
    //     return std::make_pair(rsc_e::BAD_UNSUPPORTED_CONFIGURATION, message);
    // #else
        for (JsonObject cin : array)
        {
            rsc_e rsc = validateMandatoryKeys(cin);
            if (rsc != rsc_e::GOOD)
            {
                return std::make_pair(rsc, "INVALID RS-232: MANDATORY KEY CANNOT BE MISSING");
            }

            rsc = validateMandatoryValues(cin);
            if (rsc != rsc_e::GOOD)
            {
                return std::make_pair(rsc, "INVALID RS-232: MANDATORY KEY'S VALUE CANNOT BE NULL");
            }

            const uint8_t  prt    = cin["prt"].as<uint8_t>();
            const uint32_t bdr    = cin["bdr"].as<uint32_t>();
            const uint8_t  dbit   = cin["dbit"].as<uint8_t>();
            const uint8_t  pbit   = cin["pbit"].as<uint8_t>();
            const uint8_t  sbit   = cin["sbit"].as<uint8_t>();

            const auto retPRT     = convertToPortIndex(prt);
            const auto retBDR     = convertToBaudRate(bdr);
            const auto retDBIT    = convertToDataBit(dbit);
            const auto retPBIT    = convertToParityBit(pbit);
            const auto retSBIT    = convertToStopBit(sbit);

            if (retPRT.first != rsc_e::GOOD)
            {
                const std::string message = "INVALID SERIAL PORT INDEX: " + std::to_string(prt);
                return std::make_pair(rsc, message);
            }
            
            if (retBDR.first != rsc_e::GOOD)
            {
                const std::string message = "INVALID BAUD RATE: " + std::to_string(bdr);
                return std::make_pair(rsc, message);
            }

            if (retDBIT.first != rsc_e::GOOD)
            {
                const std::string message = "INVALID DATA BIT: " + std::to_string(dbit);
                return std::make_pair(rsc, message);
            }
            
            if (retPBIT.first != rsc_e::GOOD)
            {
                const std::string message = "INVALID PARITY BIT: " + std::to_string(pbit);
                return std::make_pair(rsc, message);
            }
            
            if (retSBIT.first != rsc_e::GOOD)
            {
                const std::string message = "INVALID STOP BIT: " + std::to_string(sbit);
                return std::make_pair(rsc, message);
            }

            config::Rs232* rs232 = new(std::nothrow) config::Rs232();
            if (rs232 == nullptr)
            {
                return std::make_pair(rsc_e::BAD_OUT_OF_MEMORY, "FAILED TO ALLOCATE MEMORY FOR RS-232 CONFIG");
            }

            rs232->SetPortIndex(retPRT.second);
            rs232->SetBaudRate(retBDR.second);
            rs232->SetDataBit(retDBIT.second);
            rs232->SetParityBit(retPBIT.second);
            rs232->SetStopBit(retSBIT.second);

            rsc = emplaceCIN(static_cast<config::Base*>(rs232), outVector);
            if (rsc != rsc_e::GOOD)
            {
                if (rs232 != nullptr)
                {
                    delete rs232;
                    rs232 = nullptr;
                }
                return std::make_pair(rsc, "FAILED TO EMPLACE: RS-232 CONFIG INSTANCE");
            }
        }

        return std::make_pair(rsc_e::GOOD, "GOOD");
    #endif
    }

    /**
     * @return rsc_e 
     *      @li GOOD
     *      @li BAD_OUT_OF_MEMORY
     *      @li BAD_UNEXPECTED_ERROR
     *      @li BAD_DATA_ENCODING_INVALID
     *      @li BAD_UNSUPPORTED_CONFIGURATION
     *      @li BAD_INVALID_FORMAT_CONFIG_INSTANCE
     */
    rsc_e SerialPortValidator::validateRS485(const JsonArray array, cin_vector* outVector)
    {
        /**
         * @todo MODLINK-L의 경우에는 RS-485 포트가 한 개 뿐이기 때문에 
         *       만약 두 개의 CIN이 들어온다면 ERROR 코드 대신 WARNING
         *       코드를 반환하도록 코드를 수정해야 합니다.
         */
        for (JsonObject cin : array)
        {
            Status ret = validateMandatoryKeys(cin);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID RS-485: MANDATORY KEY CANNOT BE MISSING");
                return ret;
            }

            ret = validateMandatoryValues(cin);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID RS-485: MANDATORY KEY'S VALUE CANNOT BE NULL");
                return ret;
            }

            const uint8_t  prt    = cin["prt"].as<uint8_t>();
            const uint32_t bdr    = cin["bdr"].as<uint32_t>();
            const uint8_t  dbit   = cin["dbit"].as<uint8_t>();
            const uint8_t  pbit   = cin["pbit"].as<uint8_t>();
            const uint8_t  sbit   = cin["sbit"].as<uint8_t>();

            const auto retPRT     = convertToPortIndex(prt);
            const auto retBDR     = convertToBaudRate(bdr);
            const auto retDBIT    = convertToDataBit(dbit);
            const auto retPBIT    = convertToParityBit(pbit);
            const auto retSBIT    = convertToStopBit(sbit);

            if (retPRT.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID SERIAL PORT INDEX: %u", prt);
                goto INVALID_RS485;
            }
            
            if (retBDR.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID BAUD RATE: %u", bdr);
                goto INVALID_RS485;
            }

            if (retDBIT.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID DATA BIT: %u", dbit);
                goto INVALID_RS485;
            }
            
            if (retPBIT.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID PARITY BIT: %u", pbit);
                goto INVALID_RS485;
            }
            
            if (retSBIT.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID STOP BIT: %u", sbit);
                goto INVALID_RS485;
            }

            config::Rs485* rs485 = new(std::nothrow) config::Rs485();
            if (rs485 == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR CIN: RS-485");
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            }
            
            rs485->SetPortIndex(retPRT.second);
            rs485->SetBaudRate(retBDR.second);
            rs485->SetDataBit(retDBIT.second);
            rs485->SetParityBit(retPBIT.second);
            rs485->SetStopBit(retSBIT.second);

            ret = emplaceCIN(static_cast<config::Base*>(rs485), outVector);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO EMPLACE CONFIG INSTANCE: %s", ret.c_str());
                delete rs485;
                return ret;
            }
        }

        LOG_VERBOSE(logger, "Valid RS-485 config instance")
        return Status(Status::Code::GOOD);
    
    INVALID_RS485:
        return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
    }

    /**
     * @return rsc_e 
     *      @li GOOD
     *      @li BAD_INVALID_FORMAT_CONFIG_INSTANCE
     */
    rsc_e SerialPortValidator::validateMandatoryKeys(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json.containsKey("prt");
        isValid &= json.containsKey("bdr");
        isValid &= json.containsKey("dbit");
        isValid &= json.containsKey("pbit");
        isValid &= json.containsKey("sbit");

        if (isValid == true)
        {
            return rsc_e::GOOD;
        }
        else
        {
            return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
        }
    }

    /**
     * @return rsc_e 
     *      @li GOOD
     *      @li BAD_INVALID_FORMAT_CONFIG_INSTANCE
     */
    rsc_e SerialPortValidator::validateMandatoryValues(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json["prt"].isNull()  == false;
        isValid &= json["bdr"].isNull()  == false;
        isValid &= json["dbit"].isNull() == false;
        isValid &= json["pbit"].isNull() == false;
        isValid &= json["sbit"].isNull() == false;
        isValid &= json["prt"].is<uint8_t>()  == false;
        isValid &= json["bdr"].is<uint32_t>()  == false;
        isValid &= json["dbit"].is<uint8_t>() == false;
        isValid &= json["pbit"].is<uint8_t>() == false;
        isValid &= json["sbit"].is<uint8_t>() == false;

        if (isValid == true)
        {
            return rsc_e::GOOD;
        }
        else
        {
            return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
        }
    }

    /**
     * @return rsc_e 
     *      @li GOOD
     *      @li BAD_OUT_OF_MEMORY
     *      @li BAD_UNEXPECTED_ERROR
     */
    rsc_e SerialPortValidator::emplaceCIN(config::Base* cin, cin_vector* outVector)
    {
        ASSERT((cin != nullptr), "INPUT PARAMETER <cin> CANNOT BE A NULL POINTER");

        try
        {
            outVector->emplace_back(cin);
            return rsc_e::GOOD;
        }
        catch(const std::bad_alloc& e)
        {
            LOG_ERROR(logger, "%s", e.what());
            return rsc_e::BAD_OUT_OF_MEMORY;
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(logger, "%s", e.what());
            return rsc_e::BAD_UNEXPECTED_ERROR;
        }
    }

    /**
     * @return rsc_e 
     *      @li GOOD
     *      @li BAD_INVALID_FORMAT_CONFIG_INSTANCE
     */
    std::pair<rsc_e, prt_e> SerialPortValidator::convertToPortIndex(const uint8_t portIndex)
    {
        switch (portIndex)
        {
        case 2:
            return std::make_pair(rsc_e::GOOD, prt_e::PORT_2);
        
        #if !defined(MODLINK_L) && !defined(MODLINK_ML10)
        case 3:
            return std::make_pair(rsc_e::GOOD, prt_e::PORT_3);
        #endif

        default:
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, prt_e::PORT_2);
        }
    }

    /**
     * @return rsc_e 
     *      @li GOOD
     *      @li BAD_INVALID_FORMAT_CONFIG_INSTANCE
     */
    std::pair<rsc_e, bdr_e> SerialPortValidator::convertToBaudRate(const uint32_t baudRate)
    {
        switch (baudRate)
        {
        case 9600:
        case 19200:
        case 38400:
        case 115200:
            return std::make_pair(rsc_e::GOOD, static_cast<bdr_e>(baudRate));
        default:
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, bdr_e::BDR_9600);
        }
    }

    /**
     * @return rsc_e 
     *      @li GOOD
     *      @li BAD_INVALID_FORMAT_CONFIG_INSTANCE
     */
    std::pair<rsc_e, dbit_e> SerialPortValidator::convertToDataBit(const uint8_t dataBit)
    {
        switch (dataBit)
        {
        case 5:
        case 6:
        case 7:
        case 8:
            return std::make_pair(rsc_e::GOOD, static_cast<dbit_e>(dataBit));
        default:
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, dbit_e::DBIT_8);
        }
    }

    /**
     * @return rsc_e 
     *      @li GOOD
     *      @li BAD_INVALID_FORMAT_CONFIG_INSTANCE
     */
    std::pair<rsc_e, pbit_e> SerialPortValidator::convertToParityBit(const uint8_t parityBit)
    {
        switch (parityBit)
        {
        case 0:
        case 1:
        case 2:
            return std::make_pair(rsc_e::GOOD, static_cast<pbit_e>(parityBit));
        default:
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, pbit_e::NONE);
        }
    }

    /**
     * @return rsc_e 
     *      @li GOOD
     *      @li BAD_INVALID_FORMAT_CONFIG_INSTANCE
     */
    std::pair<rsc_e, sbit_e> SerialPortValidator::convertToStopBit(const uint8_t stopBit)
    {
        switch (stopBit)
        {
        case 1:
        case 2:
            return std::make_pair(rsc_e::GOOD, static_cast<sbit_e>(stopBit));
        default:
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, sbit_e::SBIT_1);
        }
    }
}}