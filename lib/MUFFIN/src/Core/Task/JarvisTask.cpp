/**
 * @file JarvisTask.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 수신한 JARIVS 설정 정보를 검증하여 유효하다면 적용하는 태스크를 정의합니다.
 * 
 * @date 2024-10-18
 * @version 0.0.1
 * 
 * @copyright Copyright (c) EdgecrBoss Inc. 2024
 */





#include "Common/Assert.h"
#include "Common/Status.h"
#include "Common/Time/TimeUtils.h"
#include "Common/Convert/ConvertClass.h"
#include "Core/Task/NetworkTask.h"
#include "DataFormat/JSON/JSON.h"
#include "Interfaces/RS485/RS485.h"
#include "Jarvis/Jarvis.h"
#include "JarvisTask.h"
#include "Protocol/HTTP/CatHTTP/CatHTTP.h"
#include "Protocol/HTTP/Include/TypeDefinitions.h"
#include "Protocol/MQTT/CIA.h"
#include "Protocol/MQTT/CatMQTT/CatMQTT.h"
#include "IM/MacAddress/MacAddress.h"



namespace muffin {

    static bool s_IsJarvisTaskRunning = false;
    static std::string s_JarvisApiPayload;

    /**
     * @todo LTE Cat.M1 모뎀을 쓰지 않을 때의 기능을 구현해야 합니다.
     */
    void ProcessJarvisRequestTask(void* pvParameters)
    {
        void (*callback)(jarvis::ValidationResult*) = (void (*)(jarvis::ValidationResult*))((uintptr_t*)pvParameters)[0];
        ASSERT((callback != nullptr), "INVALID CALLBACK: FUNCTION POINTER CANNOT BE A NULL POINTER");

        std::string* payload = (std::string*)((uintptr_t *)pvParameters)[1];
        ASSERT((payload != nullptr), "INVALID INPUT PARAMETER: JARVIS PAYLOAD CANNOT BE A NULL POINTER");

        jarvis::ValidationResult* validationResult = (jarvis::ValidationResult*)((uintptr_t *)pvParameters)[2];
        ASSERT((validationResult != nullptr), "INVALID OUTPUT PARAMETER: JARVIS VALIDATION RESULT CANNOT BE A NULL POINTER");


        {/* JARVIS 설정 태스크는 한 번에 하나의 요청만 처리하도록 설계되어 있습니다. */
            if (s_IsJarvisTaskRunning == true)
            {
                validationResult->SetRSC(jarvis::rsc_e::BAD_TEMPORARY_UNAVAILABLE);
                validationResult->SetDescription("UNAVAILABLE DUE TO JARVIS TASK BEING ALREADY RUNNING OR BLOCKED");

                callback(validationResult);
                vTaskDelete(NULL);
            }
            ASSERT((s_IsJarvisTaskRunning == false), "JARVIS TASK CANNOT BE HANDLED AT THE SAME TIME");
        }
        s_IsJarvisTaskRunning = true;


        {/* JARVIS 설정 요청 메시지가 JSON 형식인 경우에만 태스크를 이어가도록 설계되어 있습니다. */
            JSON json;
            JsonDocument doc;
            Status retJSON = json.Deserialize(*payload, &doc);
            if (retJSON != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO DESERIALIZE JSON: %s", retJSON.c_str());

                /**
                 * @todo retJSON 상태 코드에 따라서 서버로 상태 코드를 포함한 메시지를 생성하는 작업이 필요합니다.
                 * @note 아래는 예시 코드로 나머지는 구현을 해야 합니다.
                 * @author 김주성
                 */
                switch (retJSON.ToCode())
                {
                case Status::Code::BAD_END_OF_STREAM:
                    validationResult->SetRSC(jarvis::rsc_e::BAD_COMMUNICATION);
                    validationResult->SetDescription("PAYLOAD INSUFFICIENT OR INCOMPLETE");
                    break;
                default:
                    break;
                }
                
                callback(validationResult);
                s_IsJarvisTaskRunning = false;
                vTaskDelete(NULL);
            }
            ASSERT((retJSON == Status::Code::GOOD), "JARVIS REQUEST MESSAGE MUST BE A VALID JSON FORMAT");


            const auto retVersion = Convert.ToJarvisVersion(doc["ver"].as<std::string>());
            if ((retVersion.first.ToCode() != Status::Code::GOOD) || (retVersion.second > jarvis::prtcl_ver_e::VERSEOIN_1))
            {
                validationResult->SetRSC(jarvis::rsc_e::BAD_INVALID_VERSION);
                validationResult->SetDescription("INVALID OR UNSUPPORTED PROTOCOL VERSION");
                
                callback(validationResult);
                s_IsJarvisTaskRunning = false;
                vTaskDelete(NULL);
            }
            ASSERT((retVersion.second == jarvis::prtcl_ver_e::VERSEOIN_1), "ONLY JARVIS PROTOCOL VERSION 1 IS SUPPORTED");


            const uint64_t timeDifference = GetTimestampInMillis() - doc["ts"].as<uint64_t>();
            if (timeDifference > 60)
            {
                validationResult->SetRSC(jarvis::rsc_e::BAD);
                validationResult->SetDescription("INVALID TIMESTAMP: DIFFERS BY MORE THAN 60 SECONDS");
                
                callback(validationResult);
                s_IsJarvisTaskRunning = false;
                vTaskDelete(NULL);
            }
            ASSERT((timeDifference <= 60), "TIMESTAMPS MUST DIFFER BY LESS THAN OR EQUAL TO 60 SECONDS");


            if (doc.containsKey("rqi") == true)
            {
                const char* rqi = doc["rqi"].as<const char*>();
                if (rqi == nullptr || strlen(rqi) == 0)
                {
                    validationResult->SetRSC(jarvis::rsc_e::BAD);
                    validationResult->SetDescription("INVALID REQUEST ID: CANNOT BE NULL OR EMPTY");
                    
                    callback(validationResult);
                    s_IsJarvisTaskRunning = false;
                    vTaskDelete(NULL);
                }
                ASSERT((rqi != nullptr || strlen(rqi) != 0), "REQUEST ID CANNOT BE NULL OR EMPTY");
            }
        }
        ASSERT((s_IsJarvisTaskRunning == true), "JARVIS TASK RUNNING FLAG MUST BE SET TO TRUE");
        
        
        {/* API 서버로부터 JARVIS 설정 정보를 가져오는 데 성공한 경우에만 태스크를 이어가도록 설계되어 있습니다.*/
            JSON json;
            JsonDocument doc;

            http::CatHTTP& catHttp = http::CatHTTP::GetInstance();
            http::RequestHeader header(rest_method_e::GET, http_scheme_e::HTTPS, "api.mfm.edgecross.dev", 443, "/api/mfm/device/write", "MODLINK-L/0.0.1");
            http::RequestParameter parameters;
            parameters.Add("mac", MacAddress::GetEthernet());

            Status ret = catHttp.GET(header, parameters);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO FETCH JARVIS FROM SERVER: %s", ret.c_str());
                
                switch (ret.ToCode())
                {
                case Status::Code::BAD_TIMEOUT:
                    validationResult->SetRSC(jarvis::rsc_e::BAD_COMMUNICATION_TIMEOUT);
                    validationResult->SetDescription("FAILED TO FETCH FROM API SERVER: TIMEOUT");
                    break;
                case Status::Code::BAD_NO_COMMUNICATION:
                    validationResult->SetRSC(jarvis::rsc_e::BAD_COMMUNICATION);
                    validationResult->SetDescription("FAILED TO FETCH FROM API SERVER: COMMUNICATION FAILED");
                    break;
                case Status::Code::BAD_OUT_OF_MEMORY:
                    validationResult->SetRSC(jarvis::rsc_e::BAD_COMMUNICATION_CAPACITY_EXCEEDED);
                    validationResult->SetDescription("FAILED TO FETCH FROM API SERVER: OUT OF MEMORY");
                    break;
                default:
                    validationResult->SetRSC(jarvis::rsc_e::BAD_COMMUNICATION);
                    validationResult->SetDescription("FAILED TO FETCH FROM API SERVER");
                    break;
                }
                
                callback(validationResult);
                s_IsJarvisTaskRunning = false;
                vTaskDelete(NULL);
            }
            

            ret = catHttp.Retrieve(&s_JarvisApiPayload);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO RETRIEVE PAYLOAD FROM MODEM: %s", ret.c_str());
                
                switch (ret.ToCode())
                {
                case Status::Code::BAD_TIMEOUT:
                    validationResult->SetRSC(jarvis::rsc_e::BAD_INTERNAL_ERROR);
                    validationResult->SetDescription("FAILED TO READ FROM LTE MODEM: TIMEOUT");
                    break;
                case Status::Code::BAD_NO_COMMUNICATION:
                    validationResult->SetRSC(jarvis::rsc_e::BAD_TEMPORARY_UNAVAILABLE);
                    validationResult->SetDescription("FAILED TO READ FROM LTE MODEM: NO COMMUNICATION");
                    break;
                case Status::Code::BAD_OUT_OF_MEMORY:
                    validationResult->SetRSC(jarvis::rsc_e::BAD_OUT_OF_MEMORY);
                    validationResult->SetDescription("FAILED TO READ FROM LTE MODEM: OUT OF MEMORY");
                    break;
                default:
                    validationResult->SetRSC(jarvis::rsc_e::BAD_INTERNAL_ERROR);
                    validationResult->SetDescription("FAILED TO READ FROM LTE MODEM");
                    break;
                }
                
                callback(validationResult);
                s_IsJarvisTaskRunning = false;
                vTaskDelete(NULL);
            }
        }
        ASSERT((s_IsJarvisTaskRunning == true), "JARVIS TASK RUNNING FLAG MUST BE SET TO TRUE");


        JsonDocument jsonDocument;
        {/* JARVIS 설정 정보가 JSON 형식인 경우에만 태스크를 이어가도록 설계되어 있습니다. */
            JSON json;

            Status retJSON = json.Deserialize(s_JarvisApiPayload, &jsonDocument);
            if (retJSON != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO DESERIALIZE JSON: %s", retJSON.c_str());

                /**
                 * @todo retJSON 상태 코드에 따라서 서버로 상태 코드를 포함한 메시지를 생성하는 작업이 필요합니다.
                 * @note 아래는 예시 코드로 나머지는 구현을 해야 합니다.
                 * @author 김주성
                 */
                switch (retJSON.ToCode())
                {
                case Status::Code::BAD_END_OF_STREAM:
                    validationResult->SetRSC(jarvis::rsc_e::BAD_COMMUNICATION);
                    validationResult->SetDescription("PAYLOAD INSUFFICIENT OR INCOMPLETE");
                    break;
                default:
                    break;
                }
                
                callback(validationResult);
                s_IsJarvisTaskRunning = false;
                vTaskDelete(NULL);
            }
            ASSERT((retJSON == Status::Code::GOOD), "JARVIS REQUEST MESSAGE MUST BE A VALID JSON FORMAT");
        }
    

        {/* JARVIS 설정 정보의 유효성을 검증한 다음 그 결과를 호출자에게 전달합니다. */
            Jarvis& jarvis = Jarvis::GetInstance();
            *validationResult = std::move(jarvis.Validate(jsonDocument));

            callback(validationResult);
            s_IsJarvisTaskRunning = false;
            vTaskDelete(NULL);
        }
    }


    /**
     * @todo Core 클래스에서 payload를 받아가기 위한 더 좋은 방법을 생각해봐야 합니다.
     */
    void RetrieveJarvisRequestPayload(std::string* outputpayload)
    {
        ASSERT((outputpayload != nullptr), "OUTPUT PARAMETER CANNOT BE A NULL POINTER");

        *outputpayload = s_JarvisApiPayload;
        s_JarvisApiPayload.clear();
    }



    void ApplyJarvisTask()
    {
        Jarvis& jarvis = Jarvis::GetInstance();

        for (auto& pair : jarvis)
        {
            const jarvis::cfg_key_e key = pair.first;

            switch (key)
            {
            case jarvis::cfg_key_e::ALARM:
                applyAlarmCIN(pair.second);
                break;
            case jarvis::cfg_key_e::NODE:
                applyNodeCIN(pair.second);
                break;
            case jarvis::cfg_key_e::OPERATION_TIME:
                applyOperationTimeCIN(pair.second);
                break;
            case jarvis::cfg_key_e::PRODUCTION_INFO:
                applyProductionInfoCIN(pair.second);
                break;
            case jarvis::cfg_key_e::RS232:
            /*** @todo ATmega2560 MCU 버전의 MUFFIN을 개발할 때 RS-232 설정 개체에 대한 구현이 필요합니다. */
                LOG_ERROR(logger, "UNSUPPORTED CONFIG INSTANCE: RS-232");
                break;
            case jarvis::cfg_key_e::RS485:
                applyRS485CIN(pair.second);
                break;

            case jarvis::cfg_key_e::LTE_CatM1:
                applyLteCatM1CIN(pair.second);
                break;

            case jarvis::cfg_key_e::OPERATION:
                break;
            case jarvis::cfg_key_e::MODBUS_RTU:
                break;

            case jarvis::cfg_key_e::ETHERNET:
            case jarvis::cfg_key_e::WIFI4:
            case jarvis::cfg_key_e::MODBUS_TCP:
            default:
                ASSERT(false, "UNIMPLEMENTED CONFIGURATION SERVICES");
                break;
            }
        }
    }

    void applyAlarmCIN(std::vector<jarvis::config::Base*>& vectorAlarmCIN)
    {
        ASSERT(false, "APPLYING ALARM CIN IS NOT IMPLEMENTED");
    }
    
    void applyNodeCIN(std::vector<jarvis::config::Base*>& vectorNodeCIN)
    {
        ASSERT(false, "APPLYING NODE CIN IS NOT IMPLEMENTED");
    }
    
    void applyOperationTimeCIN(std::vector<jarvis::config::Base*>& vectorOperationTimeCIN)
    {
        ASSERT(false, "APPLYING OPERATION TIME CIN IS NOT IMPLEMENTED");
    }
    
    void applyProductionInfoCIN(std::vector<jarvis::config::Base*>& vectorProductionInfoCIN)
    {
        ASSERT(false, "APPLYING PRODUCTION INFO CIN IS NOT IMPLEMENTED");
    }

    /**
     * @todo 상태 코드를 반환하도록 코드를 수정해야 합니다.
     */
    void applyRS485CIN(std::vector<jarvis::config::Base*>& vectorRS485CIN)
    {
    #if defined(MODLINK_L) || defined(MODLINK_ML10)
        ASSERT((vectorRS485CIN.size() == 1), "THERE MUST BE ONLY ONE RS-485 CIN FOR MODLINK-L AND MODLINK-ML10");
    #endif

        jarvis::config::Rs485* cin = Convert.ToRS485CIN(vectorRS485CIN[0]);

        RS485* rs485 = RS485::GetInstanceOrNull();
        if (rs485 == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR RS-485 INTERFACE");
            // return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }

        rs485->Config(cin);
        // return Status(Status::Code::GOOD);
    }
    
    void applyLteCatM1CIN(std::vector<jarvis::config::Base*>& vectorLteCatM1CIN)
    {
        ASSERT((vectorLteCatM1CIN.size() == 1), "THERE MUST BE ONLY ONE LTE Cat.M1 CIN");

        jarvis::config::CatM1* cin = Convert.ToCatM1CIN(vectorLteCatM1CIN[0]);

        CatM1* catM1 = CatM1::GetInstanceOrNULL();
        if (catM1 == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR LTE Cat.M1 NETWORK INTERFACE");
            // return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }

        catM1->Config(cin);
        /**
         * @todo 상태 코드에 따라 적절한 처리를 수행하도록 코드를 수정해야 합니다.
         */
        StartCatM1Task();
    }
}