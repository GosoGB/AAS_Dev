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
#include "Core/Task/ModbusTask.h"
#include "DataFormat/JSON/JSON.h"
#include "IM/Node/NodeStore.h"
#include "IM/AC/Alarm/DeprecableAlarm.h"
#include "IM/EA/DeprecableProductionInfo.h"
#include "IM/EA/DeprecableOperationTime.h"
#include "Jarvis/Jarvis.h"
#include "JarvisTask.h"
#include "Protocol/HTTP/CatHTTP/CatHTTP.h"
#include "Protocol/HTTP/Include/TypeDefinitions.h"
#include "Protocol/Modbus/Include/ArduinoRS485/src/ArduinoRS485.h"
#include "Protocol/Modbus/ModbusRTU.h"
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
        jarvis_task_params* params = (jarvis_task_params*)pvParameters;
        void (*callback)(jarvis::ValidationResult) = params->Callback;
        ASSERT((callback != nullptr), "INVALID CALLBACK: FUNCTION POINTER CANNOT BE A NULL POINTER");
        std::string payload = params->RequestPayload;
        
        delete params;
        params = nullptr;

        jarvis::ValidationResult validationResult;


#ifdef DEBUG
    LOG_DEBUG(logger, "[TASK: JARVIS][PARAMS RECEIVED] Stack Remaind: %u Bytes", uxTaskGetStackHighWaterMark(NULL));
#endif

        {/* JARVIS 설정 태스크는 한 번에 하나의 요청만 처리하도록 설계되어 있습니다. */
            if (s_IsJarvisTaskRunning == true)
            {
                validationResult.SetRSC(jarvis::rsc_e::BAD_TEMPORARY_UNAVAILABLE);
                validationResult.SetDescription("UNAVAILABLE DUE TO JARVIS TASK BEING ALREADY RUNNING OR BLOCKED");

                callback(validationResult);
                vTaskDelete(NULL);
            }
            ASSERT((s_IsJarvisTaskRunning == false), "JARVIS TASK CANNOT BE HANDLED AT THE SAME TIME");
        }
        s_IsJarvisTaskRunning = true;

#ifdef DEBUG
    LOG_DEBUG(logger, "[TASK: JARVIS][SINGLE TASK CHECK] Stack Remaind: %u Bytes", uxTaskGetStackHighWaterMark(NULL));
#endif


        {/* JARVIS 설정 요청 메시지가 JSON 형식인 경우에만 태스크를 이어가도록 설계되어 있습니다. */
            JSON json;
            JsonDocument doc;
            Status retJSON = json.Deserialize(payload, &doc);
#ifdef DEBUG
    LOG_DEBUG(logger, "[TASK: JARVIS][DECODE MQTT] Stack Remaind: %u Bytes", uxTaskGetStackHighWaterMark(NULL));
#endif

            if (retJSON != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO DESERIALIZE JSON: %s", retJSON.c_str());

                switch (retJSON.ToCode())
                {
                case Status::Code::BAD_END_OF_STREAM:
                    validationResult.SetRSC(jarvis::rsc_e::BAD_COMMUNICATION);
                    validationResult.SetDescription("PAYLOAD INSUFFICIENT OR INCOMPLETE");
                    break;
                case Status::Code::BAD_NO_DATA:
                    validationResult.SetRSC(jarvis::rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE);
                    validationResult.SetDescription("PAYLOAD EMPTY");
                    break;
                case Status::Code::BAD_DATA_ENCODING_INVALID:
                    validationResult.SetRSC(jarvis::rsc_e::BAD_DECODING_ERROR);
                    validationResult.SetDescription("PAYLOAD INVALID ENCODING");
                    break;
                case Status::Code::BAD_OUT_OF_MEMORY:
                    validationResult.SetRSC(jarvis::rsc_e::BAD_OUT_OF_MEMORY);
                    validationResult.SetDescription("PAYLOAD OUT OF MEMORY");
                    break;
                case Status::Code::BAD_ENCODING_LIMITS_EXCEEDED:
                    validationResult.SetRSC(jarvis::rsc_e::BAD_DECODING_CAPACITY_EXCEEDED);
                    validationResult.SetDescription("PAYLOAD EXCEEDED NESTING LIMIT");
                    break;
                case Status::Code::BAD_UNEXPECTED_ERROR:
                    validationResult.SetRSC(jarvis::rsc_e::BAD_UNEXPECTED_ERROR);
                    validationResult.SetDescription("UNDEFINED CONDITION");
                    break;
                default:
                    validationResult.SetRSC(jarvis::rsc_e::BAD_UNEXPECTED_ERROR);
                    validationResult.SetDescription("UNDEFINED CONDITION");
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
                validationResult.SetRSC(jarvis::rsc_e::BAD_INVALID_VERSION);
                validationResult.SetDescription("INVALID OR UNSUPPORTED PROTOCOL VERSION");
                
                callback(validationResult);
                s_IsJarvisTaskRunning = false;
                vTaskDelete(NULL);
            }
            ASSERT((retVersion.second == jarvis::prtcl_ver_e::VERSEOIN_1), "ONLY JARVIS PROTOCOL VERSION 1 IS SUPPORTED");

            if (doc.containsKey("rqi") == true)
            {
                const char* rqi = doc["rqi"].as<const char*>();
                if (rqi == nullptr || strlen(rqi) == 0)
                {
                    validationResult.SetRSC(jarvis::rsc_e::BAD);
                    validationResult.SetDescription("INVALID REQUEST ID: CANNOT BE NULL OR EMPTY");
                    
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

        #ifdef DEBUG
            LOG_DEBUG(logger, "[TASK: JARVIS][REQUEST HTTP] Stack Remaind: %u Bytes", uxTaskGetStackHighWaterMark(NULL));
        #endif
            Status ret = catHttp.GET(header, parameters);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO FETCH JARVIS FROM SERVER: %s", ret.c_str());
                
                switch (ret.ToCode())
                {
                case Status::Code::BAD_TIMEOUT:
                    validationResult.SetRSC(jarvis::rsc_e::BAD_COMMUNICATION_TIMEOUT);
                    validationResult.SetDescription("FAILED TO FETCH FROM API SERVER: TIMEOUT");
                    break;
                case Status::Code::BAD_NO_COMMUNICATION:
                    validationResult.SetRSC(jarvis::rsc_e::BAD_COMMUNICATION);
                    validationResult.SetDescription("FAILED TO FETCH FROM API SERVER: COMMUNICATION FAILED");
                    break;
                case Status::Code::BAD_OUT_OF_MEMORY:
                    validationResult.SetRSC(jarvis::rsc_e::BAD_COMMUNICATION_CAPACITY_EXCEEDED);
                    validationResult.SetDescription("FAILED TO FETCH FROM API SERVER: OUT OF MEMORY");
                    break;
                default:
                    validationResult.SetRSC(jarvis::rsc_e::BAD_COMMUNICATION);
                    validationResult.SetDescription("FAILED TO FETCH FROM API SERVER");
                    break;
                }
                
                callback(validationResult);
                s_IsJarvisTaskRunning = false;
                vTaskDelete(NULL);
            }
#ifdef DEBUG
    LOG_DEBUG(logger, "[TASK: JARVIS][HTTP REQUESTED] Stack Remaind: %u Bytes", uxTaskGetStackHighWaterMark(NULL));
#endif

            s_JarvisApiPayload.clear();
            ret = catHttp.Retrieve(&s_JarvisApiPayload);
            LOG_INFO(logger, "RECEIVED JARVIS: %s", s_JarvisApiPayload.c_str());

            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO RETRIEVE PAYLOAD FROM MODEM: %s", ret.c_str());
                
                switch (ret.ToCode())
                {
                case Status::Code::BAD_TIMEOUT:
                    validationResult.SetRSC(jarvis::rsc_e::BAD_INTERNAL_ERROR);
                    validationResult.SetDescription("FAILED TO READ FROM LTE MODEM: TIMEOUT");
                    break;
                case Status::Code::BAD_NO_COMMUNICATION:
                    validationResult.SetRSC(jarvis::rsc_e::BAD_TEMPORARY_UNAVAILABLE);
                    validationResult.SetDescription("FAILED TO READ FROM LTE MODEM: NO COMMUNICATION");
                    break;
                case Status::Code::BAD_OUT_OF_MEMORY:
                    validationResult.SetRSC(jarvis::rsc_e::BAD_OUT_OF_MEMORY);
                    validationResult.SetDescription("FAILED TO READ FROM LTE MODEM: OUT OF MEMORY");
                    break;
                default:
                    validationResult.SetRSC(jarvis::rsc_e::BAD_INTERNAL_ERROR);
                    validationResult.SetDescription("FAILED TO READ FROM LTE MODEM");
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

                switch (retJSON.ToCode())
                {
                case Status::Code::BAD_END_OF_STREAM:
                    validationResult.SetRSC(jarvis::rsc_e::BAD_COMMUNICATION);
                    validationResult.SetDescription("PAYLOAD INSUFFICIENT OR INCOMPLETE");
                    break;
                case Status::Code::BAD_NO_DATA:
                    validationResult.SetRSC(jarvis::rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE);
                    validationResult.SetDescription("PAYLOAD EMPTY");
                    break;
                case Status::Code::BAD_DATA_ENCODING_INVALID:
                    validationResult.SetRSC(jarvis::rsc_e::BAD_DECODING_ERROR);
                    validationResult.SetDescription("PAYLOAD INVALID ENCODING");
                    break;
                case Status::Code::BAD_OUT_OF_MEMORY:
                    validationResult.SetRSC(jarvis::rsc_e::BAD_OUT_OF_MEMORY);
                    validationResult.SetDescription("PAYLOAD OUT OF MEMORY");
                    break;
                case Status::Code::BAD_ENCODING_LIMITS_EXCEEDED:
                    validationResult.SetRSC(jarvis::rsc_e::BAD_DECODING_CAPACITY_EXCEEDED);
                    validationResult.SetDescription("PAYLOAD EXCEEDED NESTING LIMIT");
                    break;
                case Status::Code::BAD_UNEXPECTED_ERROR:
                    validationResult.SetRSC(jarvis::rsc_e::BAD_UNEXPECTED_ERROR);
                    validationResult.SetDescription("UNDEFINED CONDITION");
                    break;
                default:
                    validationResult.SetRSC(jarvis::rsc_e::BAD_UNEXPECTED_ERROR);
                    validationResult.SetDescription("UNDEFINED CONDITION");
                    break;
                }
                
                callback(validationResult);
                s_IsJarvisTaskRunning = false;
                vTaskDelete(NULL);
            }
            ASSERT((retJSON == Status::Code::GOOD), "JARVIS REQUEST MESSAGE MUST BE A VALID JSON FORMAT");
        }

        {/* JARVIS 설정 정보의 유효성을 검증한 다음 그 결과를 호출자에게 전달합니다. */
            /**
             * @todo Initializer 쪽에서 create를 먼저 하는 걸 가정하고 만들었습니다만
             *       JARVIS 없이 설정하는 경우를 대비해서 GetInstanceOrCrash()룰 호출해야 합니다.
             * @todo CreateOrNULL()로 함수 명을 바꾸면 위의 투 두 없이도 가능합니다.
             */
            Jarvis* jarvis = Jarvis::CreateInstanceOrNULL();
            for (auto& pair : *jarvis)
            {
                for (auto& element : pair.second)
                {
                    delete element;
                }
                pair.second.clear();
            }
            jarvis->Clear();

            validationResult = jarvis->Validate(jsonDocument);
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


    /**
     * @todo 상태 코드를 반환하도록 코드를 수정해야 합니다.
     */
    void ApplyJarvisTask()
    {
        Jarvis& jarvis = Jarvis::GetInstance();
        for (auto& pair : jarvis)
        {
            const jarvis::cfg_key_e key = pair.first;
            if (key == jarvis::cfg_key_e::LTE_CatM1)
            {
                applyLteCatM1CIN(pair.second);
                break;
            }
        }

        for (auto& pair : jarvis)
        {
            const jarvis::cfg_key_e key = pair.first;
            if (key == jarvis::cfg_key_e::NODE)
            {
                applyNodeCIN(pair.second);
                break;
            }
        }
        

        for (auto& pair : jarvis)
        {
            const jarvis::cfg_key_e key = pair.first;

            switch (key)
            {
            case jarvis::cfg_key_e::ALARM:
                applyAlarmCIN(pair.second);
                break;
            case jarvis::cfg_key_e::NODE:
                break;
            case jarvis::cfg_key_e::OPERATION_TIME:
                applyOperationTimeCIN(pair.second);
                break;
            case jarvis::cfg_key_e::RS232:
            /**
             * @todo ATmega2560 MCU 버전의 MUFFIN을 개발할 때 RS-232 
             *       설정 개체에 대한 구현이 필요합니다.
             */
                LOG_ERROR(logger, "UNSUPPORTED CONFIG INSTANCE: RS-232");
                break;
            case jarvis::cfg_key_e::RS485:
                applyRS485CIN(pair.second);
                break;
            case jarvis::cfg_key_e::LTE_CatM1:
                break;
            case jarvis::cfg_key_e::PRODUCTION_INFO:
            case jarvis::cfg_key_e::OPERATION:
                break;
            case jarvis::cfg_key_e::MODBUS_RTU:
                {
                    for (auto cin : jarvis)
                    {
                        if (cin.first == jarvis::cfg_key_e::RS485)
                        {
                            jarvis::config::Rs485* rs485CIN = static_cast<jarvis::config::Rs485*>(cin.second[0]);
                            applyModbusRtuCIN(pair.second, rs485CIN);
                            break;
                        }
                    }
                }
                break;

            case jarvis::cfg_key_e::ETHERNET:
            case jarvis::cfg_key_e::WIFI4:
            case jarvis::cfg_key_e::MODBUS_TCP:
            default:
                ASSERT(false, "UNIMPLEMENTED CONFIGURATION SERVICES");
                break;
            }
        }

        for (auto& pair : jarvis)
        {
            for (auto& element : pair.second)
            {
                delete element;
            }
            pair.second.clear();
        }

        for (auto& pair : jarvis)
        {
            const jarvis::cfg_key_e key = pair.first;
            if (key == jarvis::cfg_key_e::PRODUCTION_INFO)
            {
                applyProductionInfoCIN(pair.second);
                break;
            }
        }
    }

    void applyAlarmCIN(std::vector<jarvis::config::Base*>& vectorAlarmCIN)
    {
        LOG_DEBUG(logger, "Start applying Alarm CIN");
        AlarmMonitor& alarmMonitor = AlarmMonitor::GetInstance();
        for (auto cin : vectorAlarmCIN)
        {
            alarmMonitor.Add(static_cast<jarvis::config::Alarm*>(cin));
        }
        alarmMonitor.StartTask();
    }
    
    void applyNodeCIN(std::vector<jarvis::config::Base*>& vectorNodeCIN)
    {
        im::NodeStore* nodeStore = im::NodeStore::CreateInstanceOrNULL();
        if (nodeStore == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO CRAETE NODE STORE");
            return;
        }        

        for (auto& baseCIN : vectorNodeCIN)
        {
            jarvis::config::Node* nodeCIN = static_cast<jarvis::config::Node*>(baseCIN);
            nodeStore->Create(nodeCIN);
        }
    }
    
    void applyOperationTimeCIN(std::vector<jarvis::config::Base*>& vectorOperationTimeCIN)
    {
        LOG_DEBUG(logger, "Start applying Operation time CIN");
        OperationTime& operationTime = OperationTime::GetInstance();
        for (auto cin : vectorOperationTimeCIN)
        {
            operationTime.Config(static_cast<jarvis::config::OperationTime*>(cin));
        }
        operationTime.StartTask();
    }
    
    void applyProductionInfoCIN(std::vector<jarvis::config::Base*>& vectorProductionInfoCIN)
    {
        LOG_DEBUG(logger, "Start applying Production CIN");
        ProductionInfo& productionInfo = ProductionInfo::GetInstance();
        for (auto cin : vectorProductionInfoCIN)
        {
            productionInfo.Config(static_cast<jarvis::config::Production*>(cin));
        }
        productionInfo.StartTask();
    }

    void applyRS485CIN(std::vector<jarvis::config::Base*>& vectorRS485CIN)
    {
    #if defined(MODLINK_L) || defined(MODLINK_ML10)
        ASSERT((vectorRS485CIN.size() == 1), "THERE MUST BE ONLY ONE RS-485 CIN FOR MODLINK-L AND MODLINK-ML10");
    #endif

        jarvis::config::Rs485* cin = Convert.ToRS485CIN(vectorRS485CIN[0]);
        if (cin->GetPortIndex().second == jarvis::prt_e::PORT_2)
        {
            RS485 = new(std::nothrow) RS485Class(Serial2, 17, -1, -1);
            if (RS485 == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR RS485 INTERFACE");
            }
        }
    }
    
    void applyLteCatM1CIN(std::vector<jarvis::config::Base*>& vectorLteCatM1CIN)
    {
        ASSERT((vectorLteCatM1CIN.size() == 1), "THERE MUST BE ONLY ONE LTE Cat.M1 CIN");
        LOG_INFO(logger, "Start to apply LTE Cat.M1 configuration");

        jarvis::config::CatM1* cin = Convert.ToCatM1CIN(vectorLteCatM1CIN[0]);
        InitCatM1(cin);
        InitCatHTTP();
        ConnectToBroker();
        StartCatM1Task();
    }

    void applyModbusRtuCIN(std::vector<jarvis::config::Base*>& vectorModbusRTUCIN, jarvis::config::Rs485* rs485CIN)
    {
        ModbusRTU* modbusRTU = ModbusRTU::CreateInstanceOrNULL();
        if (modbusRTU == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO CRAETE MODBUS RTU PROTOCOL");
            return;
        }

        modbusRTU->SetPort(rs485CIN);

        for (auto& modbusRTUCIN : vectorModbusRTUCIN)
        {
            jarvis::config::ModbusRTU* cin = static_cast<jarvis::config::ModbusRTU*>(modbusRTUCIN);
            Status ret = modbusRTU->Config(cin);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO CONFIGURE MODBUS RTU");
                return;
            }
        }
        LOG_INFO(logger, "Configured Modbus RTU protocol");
        
        StartModbusTask();
    }
}