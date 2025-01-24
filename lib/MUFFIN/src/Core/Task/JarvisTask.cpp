/**
 * @file JarvisTask.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 수신한 JARIVS 설정 정보를 검증하여 유효하다면 적용하는 태스크를 정의합니다.
 * 
 * @date 2025-01-23
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include "Preferences.h"
#include "Common/Assert.h"
#include "Common/Status.h"
#include "Common/Time/TimeUtils.h"
#include "Common/Convert/ConvertClass.h"
#include "Core/Core.h"
#include "Core/Task/ModbusTask.h"
#include "Core/Task/CyclicalPubTask.h"
#include "DataFormat/JSON/JSON.h"

#include "IM/Custom/Constants.h"
#include "IM/Custom/MacAddress/MacAddress.h"
#include "IM/Node/NodeStore.h"
#include "IM/AC/Alarm/DeprecableAlarm.h"
#include "IM/EA/DeprecableProductionInfo.h"
#include "IM/EA/DeprecableOperationTime.h"

#include "JARVIS/JARVIS.h"
#include "JARVIS/Config/Operation/Operation.h"
#include "JarvisTask.h"
#include "Protocol/MQTT/CDO.h"
#include "Protocol/HTTP/CatHTTP/CatHTTP.h"
#include "Protocol/HTTP/Include/TypeDefinitions.h"
#include "Protocol/Modbus/Include/TypeDefinitions.h"
#include "Protocol/Modbus/Include/ArduinoRS485/src/ArduinoRS485.h"
#include "Protocol/Modbus/ModbusRTU.h"
#include "Protocol/Modbus/ModbusTCP.h"
#include "Protocol/MQTT/CIA.h"
#include "Protocol/MQTT/CatMQTT/CatMQTT.h"
#include "Protocol/MQTT/LwipMQTT/LwipMQTT.h"
#include "Protocol/MQTT/Include/Helper.h"
#include "Protocol/SPEAR/SPEAR.h"

#include "Storage/ESP32FS/ESP32FS.h"
#include "Network/Ethernet/Ethernet.h"
#include "Storage/CatFS/CatFS.h"
#include "Protocol/HTTP/LwipHTTP/LwipHTTP.h"

muffin::mqtt::IMQTT* mqttClient = nullptr;
muffin::http::IHTTP* httpClient = nullptr;



namespace muffin {

    static bool s_IsJarvisTaskRunning = false;
    static std::string s_JarvisApiPayload;
    static bool s_HasNode = false;


    /**
     * @todo LTE Cat.M1 모뎀을 쓰지 않을 때의 기능을 구현해야 합니다.
     */
    void ProcessJarvisRequestTask(void* pvParameters)
    {
        jarvis_task_params* params = (jarvis_task_params*)pvParameters;
        void (*callback)(jvs::ValidationResult&) = params->Callback;
        ASSERT((callback != nullptr), "CALLBACK FUNCTION CANNOT BE NULL");
        
        delete params;
        params = nullptr;

        jvs::ValidationResult validationResult;
        {/* JARVIS 설정 태스크는 한 번에 하나의 요청만 처리하도록 설계되어 있습니다. */
            if (s_IsJarvisTaskRunning == true)
            {
                validationResult.SetRSC(jvs::rsc_e::BAD_TEMPORARY_UNAVAILABLE);
                validationResult.SetDescription("UNAVAILABLE DUE TO JARVIS TASK BEING ALREADY RUNNING OR BLOCKED");

                callback(validationResult);
                vTaskDelete(NULL);
            }
            ASSERT((s_IsJarvisTaskRunning == false), "JARVIS TASK CANNOT BE HANDLED AT THE SAME TIME");
        }
        s_IsJarvisTaskRunning = true;
      
        {/* API 서버로부터 JARVIS 설정 정보를 가져오는 데 성공한 경우에만 태스크를 이어가도록 설계되어 있습니다.*/
            Status ret = Status(Status::Code::UNCERTAIN);
            switch (jvs::config::operation.GetServerNIC().second)
            {
            case jvs::snic_e::LTE_CatM1:
                ret = strategyCatHttp();
                break;
#if defined(MODLINK_T2)
            case jvs::snic_e::Ethernet:
                ret = strategyLwipHttp();
                break;
#endif
            default:
                break;
            }
            
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO FETCH JARVIS FROM SERVER: %s", ret.c_str());
                
                switch (ret.ToCode())
                {
                case Status::Code::BAD_TIMEOUT:
                    validationResult.SetRSC(jvs::rsc_e::BAD_COMMUNICATION_TIMEOUT);
                    validationResult.SetDescription("FAILED TO FETCH FROM API SERVER: TIMEOUT");
                    break;
                case Status::Code::BAD_NO_COMMUNICATION:
                    validationResult.SetRSC(jvs::rsc_e::BAD_COMMUNICATION);
                    validationResult.SetDescription("FAILED TO FETCH FROM API SERVER: COMMUNICATION FAILED");
                    break;
                case Status::Code::BAD_OUT_OF_MEMORY:
                    validationResult.SetRSC(jvs::rsc_e::BAD_COMMUNICATION_CAPACITY_EXCEEDED);
                    validationResult.SetDescription("FAILED TO FETCH FROM API SERVER: OUT OF MEMORY");
                    break;
                default:
                    validationResult.SetRSC(jvs::rsc_e::BAD_COMMUNICATION);
                    validationResult.SetDescription("FAILED TO FETCH FROM API SERVER");
                    break;
                }
                
                callback(validationResult);
                s_IsJarvisTaskRunning = false;
                vTaskDelete(NULL);
            }

            LOG_INFO(logger, "[TASK: implEthernetTask] Stack Remaind: %u Bytes", uxTaskGetStackHighWaterMark(NULL));
            LOG_INFO(logger, "Config Start: %u Bytes", ESP.getFreeHeap());
            
            Preferences nvs;
            nvs.begin("jarvis");
            nvs.putBool("jarvisFlag",false);
            nvs.end();

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
                    validationResult.SetRSC(jvs::rsc_e::BAD_COMMUNICATION);
                    validationResult.SetDescription("PAYLOAD INSUFFICIENT OR INCOMPLETE");
                    break;
                case Status::Code::BAD_NO_DATA:
                    validationResult.SetRSC(jvs::rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE);
                    validationResult.SetDescription("PAYLOAD EMPTY");
                    break;
                case Status::Code::BAD_DATA_ENCODING_INVALID:
                    validationResult.SetRSC(jvs::rsc_e::BAD_DECODING_ERROR);
                    validationResult.SetDescription("PAYLOAD INVALID ENCODING");
                    break;
                case Status::Code::BAD_OUT_OF_MEMORY:
                    validationResult.SetRSC(jvs::rsc_e::BAD_OUT_OF_MEMORY);
                    validationResult.SetDescription("PAYLOAD OUT OF MEMORY");
                    break;
                case Status::Code::BAD_ENCODING_LIMITS_EXCEEDED:
                    validationResult.SetRSC(jvs::rsc_e::BAD_DECODING_CAPACITY_EXCEEDED);
                    validationResult.SetDescription("PAYLOAD EXCEEDED NESTING LIMIT");
                    break;
                case Status::Code::BAD_UNEXPECTED_ERROR:
                    validationResult.SetRSC(jvs::rsc_e::BAD_UNEXPECTED_ERROR);
                    validationResult.SetDescription("UNDEFINED CONDITION");
                    break;
                default:
                    validationResult.SetRSC(jvs::rsc_e::BAD_UNEXPECTED_ERROR);
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
            JARVIS* jarvis = new(std::nothrow) JARVIS();
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
        s_JarvisApiPayload.shrink_to_fit();
    }


    /**
     * @todo 상태 코드를 반환하도록 코드를 수정해야 합니다.
     */
    void ApplyJarvisTask()
    {
        for (auto& pair : *jarvis)
        {
            const jvs::cfg_key_e key = pair.first;
            if (key == jvs::cfg_key_e::NODE)
            {
                applyNodeCIN(pair.second);
                s_HasNode = true;

                for (auto it = pair.second.begin(); it != pair.second.end(); ++it)
                {
                    delete *it;
                }
                break;
            }
        }

        for (auto& pair : *jarvis)
        {
            const jvs::cfg_key_e key = pair.first;

            switch (key)
            {
            case jvs::cfg_key_e::ALARM:
                applyAlarmCIN(pair.second);
                for (auto it = pair.second.begin(); it != pair.second.end(); ++it)
                {
                    delete *it;
                }
                break;
            case jvs::cfg_key_e::OPERATION_TIME:
                applyOperationTimeCIN(pair.second);
                for (auto it = pair.second.begin(); it != pair.second.end(); ++it)
                {
                    delete *it;
                }
                break;
            case jvs::cfg_key_e::RS485:
                applyRS485CIN(pair.second);
                /**
                 * @todo 현재 MODLINK-L에서 해당 설정값을 제거하면 RTU 설정시 문제가 발생해서 주석처리 해주었음
                 * 
                 */
                // for (auto it = pair.second.begin(); it != pair.second.end(); ++it)
                // {
                //     delete *it;
                // }
                break;
            case jvs::cfg_key_e::PRODUCTION_INFO:
                applyProductionInfoCIN(pair.second);
                for (auto it = pair.second.begin(); it != pair.second.end(); ++it)
                {
                    delete *it;
                }
                break;
            case jvs::cfg_key_e::MODBUS_RTU:
                {
                    for (auto cin : *jarvis)
                    {
                        if (cin.first == jvs::cfg_key_e::RS485)
                        {
                            jvs::config::Rs485* rs485CIN = static_cast<jvs::config::Rs485*>(cin.second[0]);
                            applyModbusRtuCIN(pair.second, rs485CIN);
                            for (auto it = pair.second.begin(); it != pair.second.end(); ++it)
                            {
                                delete *it;
                            }
                            break;
                        }
                    }
                }
                break;
            case jvs::cfg_key_e::MODBUS_TCP:
                applyModbusTcpCIN(pair.second);
                for (auto it = pair.second.begin(); it != pair.second.end(); ++it)
                {
                    delete *it;
                }
                break;
            case jvs::cfg_key_e::NODE:
            case jvs::cfg_key_e::LTE_CatM1:
            case jvs::cfg_key_e::OPERATION:
            case jvs::cfg_key_e::RS232:
            case jvs::cfg_key_e::WIFI4:
            case jvs::cfg_key_e::ETHERNET:
                break;
            default:
                ASSERT(false, "UNIMPLEMENTED CONFIGURATION SERVICES");
                break;
            }
        }

        for (auto& pair : *jarvis)
        {
            pair.second.clear();
        }
    }

    void applyAlarmCIN(std::vector<jvs::config::Base*>& vectorAlarmCIN)
    {
        AlarmMonitor& alarmMonitor = AlarmMonitor::GetInstance();
        for (auto cin : vectorAlarmCIN)
        {
            alarmMonitor.Add(static_cast<jvs::config::Alarm*>(cin));
        }
        alarmMonitor.StartTask();
    }
    
    void applyNodeCIN(std::vector<jvs::config::Base*>& vectorNodeCIN)
    {
        im::NodeStore* nodeStore = im::NodeStore::CreateInstanceOrNULL();
        for (auto& baseCIN : vectorNodeCIN)
        {
            jvs::config::Node* nodeCIN = static_cast<jvs::config::Node*>(baseCIN);
            nodeStore->Create(nodeCIN);
        }
    }
    
    void applyOperationTimeCIN(std::vector<jvs::config::Base*>& vectorOperationTimeCIN)
    {
        OperationTime& operationTime = OperationTime::GetInstance();
        for (auto cin : vectorOperationTimeCIN)
        {
            operationTime.Config(static_cast<jvs::config::OperationTime*>(cin));
        }
        operationTime.StartTask();
    }
    
    void applyProductionInfoCIN(std::vector<jvs::config::Base*>& vectorProductionInfoCIN)
    {
        ProductionInfo& productionInfo = ProductionInfo::GetInstance();
        for (auto cin : vectorProductionInfoCIN)
        {
            productionInfo.Config(static_cast<jvs::config::Production*>(cin));
        }
        productionInfo.StartTask();
    }

    void applyRS485CIN(std::vector<jvs::config::Base*>& vectorRS485CIN)
    {
    #if defined(MODLINK_L) || defined(MODLINK_ML10)
        ASSERT((vectorRS485CIN.size() == 1), "THERE MUST BE ONLY ONE RS-485 CIN FOR MODLINK-L AND MODLINK-ML10");
        jvs::config::Rs485* cin = Convert.ToRS485CIN(vectorRS485CIN[0]);
        if (cin->GetPortIndex().second == jvs::prt_e::PORT_2)
        {
            RS485 = new(std::nothrow) RS485Class(Serial2, 17, -1, -1);
            if (RS485 == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR RS485 INTERFACE");
            }
        }

    #elif defined(MODLINK_T2) || defined(MODLINK_B)
        for (auto& Rs485CIN : vectorRS485CIN)
        {
            size_t count = 0;
            while (count < 5)
            {
                Status ret = spear.SetJarvisLinkConfig(Rs485CIN, jvs::cfg_key_e::RS485);
                if (ret == Status(Status::Code::GOOD))
                {
                    break;
                count++;
                }
                delay(100);
            }            
        }

    #endif
    }

    void applyModbusRtuCIN(std::vector<jvs::config::Base*>& vectorModbusRTUCIN, jvs::config::Rs485* rs485CIN)
    {
        if (s_HasNode == false)
        {
            LOG_WARNING(logger, "NO NODE");
            return;
        }
        
        if (jvs::config::operation.GetPlanExpired().second == true)
        {
            LOG_WARNING(logger, "EXPIRED SERVICE PLAN: MODBUS TASK IS NOT GOING TO START");
            return;
        }

    #if defined(MODLINK_L) || defined(MODLINK_ML10)
        ModbusRtuVector.clear();
        mVectorModbusRTU.clear();
        
        for (auto& modbusRTUCIN : vectorModbusRTUCIN)
        {
            ModbusRTU* modbusRTU = new ModbusRTU();
            modbusRTU->SetPort(rs485CIN);
            jvs::config::ModbusRTU* cin = static_cast<jvs::config::ModbusRTU*>(modbusRTUCIN);
            modbusRTU->mPort = cin->GetPort().second;
            Status ret = modbusRTU->Config(cin);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO CONFIGURE MODBUS RTU");
                return;
            }
            mVectorModbusRTU.emplace_back(*cin);
            ModbusRtuVector.emplace_back(*modbusRTU);
        }

        LOG_INFO(logger, "Configured Modbus RTU protocol, mVectorModbusRTU size : %d",mVectorModbusRTU.size());

        SetPollingInterval(jvs::config::operation.GetIntervalPolling().second);
        StartModbusRtuTask();
        StartTaskCyclicalsMSG(jvs::config::operation.GetIntervalServer().second);
    #else
        ModbusRtuVector.clear();
        mVectorModbusRTU.clear();
        
        std::set<jvs::prt_e> link;
        for (auto& modbusRTUCIN : vectorModbusRTUCIN)
        {
            ModbusRTU* modbusRTU = new ModbusRTU();
            jvs::config::ModbusRTU* cin = static_cast<jvs::config::ModbusRTU*>(modbusRTUCIN);
            modbusRTU->mPort = cin->GetPort().second;
            link.insert(modbusRTU->mPort);
            Status ret = modbusRTU->Config(cin);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO CONFIGURE MODBUS RTU");
                return;
            }
            mVectorModbusRTU.emplace_back(*cin);
            ModbusRtuVector.emplace_back(*modbusRTU);
        }

        LOG_INFO(logger, "Configured Modbus RTU protocol, mVectorModbusRTU size : %d",mVectorModbusRTU.size());
        
        size_t count = 0;
        while (count < 5)
        {
            Status ret = spear.SetJarvisProtocolConfig(link);
            if (ret == Status(Status::Code::GOOD))
            {
                break;
            }
            count++;
            delay(100);
        }

        StartModbusRtuTask();
        StartTaskCyclicalsMSG(jvs::config::operation.GetIntervalServer().second);
    #endif
        
    }

    void applyModbusTcpCIN(std::vector<jvs::config::Base*>& vectorModbusTCPCIN)
    {
    #if defined(MODLINK_L) || defined(MODLINK_ML10)
        ASSERT((true), "ETHERNET MUST BE CONFIGURE FOR MODLINK-B AND MODLINK-T2");
    #endif

        if (s_HasNode == false)
        {
            LOG_WARNING(logger, "NO NODE");
            return;
        }

        if (jvs::config::operation.GetPlanExpired().second == true)
        {
            LOG_WARNING(logger, "EXPIRED SERVICE PLAN: MODBUS TASK IS NOT GOING TO START");
            return;
        }
        ModbusTcpVector.clear();
        mVectorModbusTCP.clear();
        
        for (auto& modbusTCPCIN : vectorModbusTCPCIN)
        {
            ModbusTCP* modbusTCP = new ModbusTCP();
            jvs::config::ModbusTCP* cin = static_cast<jvs::config::ModbusTCP*>(modbusTCPCIN);
            Status ret = modbusTCP->Config(cin);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO CONFIGURE MODBUS TCP");
                return;
            }
            mVectorModbusTCP.emplace_back(*cin);
            ModbusTcpVector.emplace_back(*modbusTCP);
        }

        SetPollingInterval(jvs::config::operation.GetIntervalPolling().second);
        StartModbusTcpTask();
        StartTaskCyclicalsMSG(jvs::config::operation.GetIntervalServer().second);
    }
}