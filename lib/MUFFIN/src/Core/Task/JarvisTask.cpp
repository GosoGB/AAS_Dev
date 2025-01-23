/**
 * @file JarvisTask.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 수신한 JARIVS 설정 정보를 검증하여 유효하다면 적용하는 태스크를 정의합니다.
 * 
 * @date 2024-10-18
 * @version 1.0.0
 * 
 * @copyright Copyright (c) EdgecrBoss Inc. 2024
 */





#include "Preferences.h"
#include "Common/Assert.h"
#include "Common/Status.h"
#include "Common/Time/TimeUtils.h"
#include "Common/Convert/ConvertClass.h"
#include "Core/Core.h"
#include "Core/Task/NetworkTask.h"
#include "Core/Task/ModbusTask.h"
#include "Core/Task/CyclicalPubTask.h"
#include "DataFormat/JSON/JSON.h"
#include "IM/Node/NodeStore.h"
#include "IM/Custom/MacAddress/MacAddress.h"
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
    static bool s_HasFactoryResetCommand   = false;
    static bool s_HasPlanExpirationCommand = false;
    static uint16_t s_PollingInterval =  1;
    static uint16_t s_PublishInterval = 60;
    static jvs::snic_e s_ServerNIC = jvs::snic_e::LTE_CatM1;

    #if defined(DEBUG)
        mqtt::BrokerInfo brokerInfo(
            macAddress.GetEthernet(),
            "mqtt.vitcon.iotops.opsnow.com",
            8883,
            40,
            mqtt::socket_e::SOCKET_0,
            "vitcon",
            "tkfkdgo5!@#$"
        );
    #else
        mqtt::BrokerInfo brokerInfo(
            macAddress.GetEthernet(),
            "mqtt.vitcon.iotops.opsnow.com",
            8883,
            40,
            mqtt::socket_e::SOCKET_0,
            "vitcon",
            "tkfkdgo5!@#$"
        );
    #endif


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


#ifdef DEBUG
    //LOG_DEBUG(logger, "[TASK: JARVIS][PARAMS RECEIVED] Stack Remaind: %u Bytes", uxTaskGetStackHighWaterMark(NULL));
#endif

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

#ifdef DEBUG
    //LOG_DEBUG(logger, "[TASK: JARVIS][SINGLE TASK CHECK] Stack Remaind: %u Bytes", uxTaskGetStackHighWaterMark(NULL));
#endif

      
        {/* API 서버로부터 JARVIS 설정 정보를 가져오는 데 성공한 경우에만 태스크를 이어가도록 설계되어 있습니다.*/

            Status ret = Status(Status::Code::UNCERTAIN);
            switch (s_ServerNIC)
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
            if (key == jvs::cfg_key_e::LTE_CatM1)
            {
                applyLteCatM1CIN(pair.second);
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
            if (key == jvs::cfg_key_e::ETHERNET)
            {
                applyEthernetCIN(pair.second);
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
            if (key == jvs::cfg_key_e::OPERATION)
            {
                applyOperationCIN(pair.second);
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
            case jvs::cfg_key_e::NODE:
            case jvs::cfg_key_e::LTE_CatM1:
            case jvs::cfg_key_e::OPERATION:
            case jvs::cfg_key_e::RS232:
            case jvs::cfg_key_e::WIFI4:
            case jvs::cfg_key_e::ETHERNET:
                break;
            case jvs::cfg_key_e::MODBUS_TCP:
                applyModbusTcpCIN(pair.second);
                for (auto it = pair.second.begin(); it != pair.second.end(); ++it)
                {
                    delete *it;
                }
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

    void applyOperationCIN(std::vector<jvs::config::Base*>& vectorOperationCIN)
    {
    #if defined(MODLINK_L) || defined(MODLINK_ML10)
        ASSERT((vectorOperationCIN.size() == 1), "THERE MUST BE ONLY ONE OPERATION CIN: %u", vectorOperationCIN.size());
    #endif

        //LOG_DEBUG(logger, "Start applying Operation CIN");
        jvs::config::Operation* cin = static_cast<jvs::config::Operation*>(vectorOperationCIN[0]);

        s_HasFactoryResetCommand   = cin->GetFactoryReset().second;
        LOG_INFO(logger,"s_HasFactoryResetCommand : %s", s_HasFactoryResetCommand == true ? "true":"false" );
        s_HasPlanExpirationCommand = cin->GetPlanExpired().second;
        s_PollingInterval = cin->GetIntervalPolling().second;
        s_PublishInterval = cin->GetIntervalServer().second;
        s_ServerNIC = cin->GetServerNIC().second;

        if (s_HasFactoryResetCommand == true)
        {
            esp32FS.Format();
            esp_restart();
        }
    }

    uint16_t RetrievePublishInterval()
    {
        return s_PublishInterval;
    }

    jvs::snic_e RetrieveServerNIC()
    {
        return s_ServerNIC;
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
    
    void applyLteCatM1CIN(std::vector<jvs::config::Base*>& vectorLteCatM1CIN)
    {
        ASSERT((vectorLteCatM1CIN.size() == 1), "THERE MUST BE ONLY ONE LTE Cat.M1 CIN");
        LOG_INFO(logger, "Start to apply LTE Cat.M1 configuration");

        jvs::config::CatM1* cin = Convert.ToCatM1CIN(vectorLteCatM1CIN[0]);
        while (InitCatM1(cin) != Status::Code::GOOD)
        {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        InitCatHTTP();
        ConnectToBroker();
        StartCatM1Task();
    }

    void applyModbusRtuCIN(std::vector<jvs::config::Base*>& vectorModbusRTUCIN, jvs::config::Rs485* rs485CIN)
    {
        if (s_HasNode == false)
        {
            LOG_WARNING(logger, "NO NODE");
            return;
        }
        
        if (s_HasPlanExpirationCommand == true)
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

        SetPollingInterval(s_PollingInterval);
        StartModbusRtuTask();
        StartTaskCyclicalsMSG(s_PublishInterval);
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
        StartTaskCyclicalsMSG(s_PublishInterval);
    #endif
        
    }

    void applyEthernetCIN(std::vector<jvs::config::Base*>& vectorEthernetCIN)
    {
    #if defined(MODLINK_L) || defined(MODLINK_ML10)
        ASSERT((true), "ETHERNET MUST BE CONFIGURE FOR MODLINK-B AND MODLINK-T2");
    #endif
        ASSERT((vectorEthernetCIN.size() == 1), "THERE MUST BE ONLY ONE ETHERNET CIN FOR MODLINK-B AND MODLINK-T2");

        jvs::config::Ethernet* cin = Convert.ToEthernetCIN(vectorEthernetCIN[0]);

        ethernet = new Ethernet();
        Status ret = ethernet->Init();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO INITIALIZE ETHERNET");
            return;
        }
        LOG_INFO(logger,"Initialized ethernet interface");
        
        ret = ethernet->Config(cin);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CONFIGURE ETHERNET CIN");
            return;
        }
        LOG_INFO(logger,"Configured ethernet CIN");
        
        ret = ethernet->Connect();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CONNECT ETHERNET");
            return;
        }
        LOG_INFO(logger,"Ethernet has been connected");
        
        while (ethernet->IsConnected() == false)
        {
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        
        /**
         * @todo 서비스 네트워크에 따라서 실행되게 코드 수정 필요함
         */
        ethernet->SyncWithNTP();
        mqtt::Message lwt = mqtt::GenerateWillMessage(false);
        mqttClient = new mqtt::LwipMQTT(brokerInfo, lwt);
        httpClient = new http::LwipHTTP();
        ConnectToBrokerEthernet();
        StartEthernetTask();
        return;
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

        if (s_HasPlanExpirationCommand == true)
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

        SetPollingInterval(s_PollingInterval);
        StartModbusTcpTask();
        StartTaskCyclicalsMSG(s_PublishInterval);
    }

    Status strategyCatHttp()
    {
        CatM1& catM1 = CatM1::GetInstance();
        const auto mutexHandle = catM1.TakeMutex();
      
        /* API 서버로부터 JARVIS 설정 정보를 가져오는 데 성공한 경우에만 태스크를 이어가도록 설계되어 있습니다.*/
            JSON json;
            JsonDocument doc;

            http::CatHTTP& catHttp = http::CatHTTP::GetInstance();
        #if defined(DEBUG)
            http::RequestHeader header(rest_method_e::GET, http_scheme_e::HTTPS, "api.mfm.edgecross.dev", 443, "/api/mfm/device/write", "MODLINK-L/0.0.1");
        #else
            http::RequestHeader header(rest_method_e::GET, http_scheme_e::HTTPS, "api.mfm.edgecross.ai", 443, "/api/mfm/device/write", "MODLINK-L/0.0.1");
        #endif
            http::RequestParameter parameters;
            parameters.Add("mac", macAddress.GetEthernet());

        #ifdef DEBUG
            //LOG_DEBUG(logger, "[TASK: JARVIS][REQUEST HTTP] Stack Remaind: %u Bytes", uxTaskGetStackHighWaterMark(NULL));
        #endif
            catHttp.SetSinkToCatFS(true, "http_response_file");
            Status ret = catHttp.GET(mutexHandle.second, header, parameters);
            catHttp.SetSinkToCatFS(false, "");
            if (ret != Status::Code::GOOD)
            {
                return ret;
            }
 
            catM1.ReleaseMutex();
            CatFS* catFS = CatFS::CreateInstanceOrNULL(catM1);
            ret = catFS->Begin();
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger," FAIL TO BEGIN CATFS");
                return ret;
            }
            s_JarvisApiPayload.clear();
            ret = catFS->DownloadFile("http_response_file", &s_JarvisApiPayload);
            LOG_INFO(logger, "RECEIVED JARVIS: %s", s_JarvisApiPayload.c_str());

            return ret;
    }

    Status strategyLwipHttp()
    {
        http::lwipHTTP = new(std::nothrow) http::LwipHTTP();
        if (http::lwipHTTP == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR LWIP HTTP CLIENT");
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }
        
        INetwork* nic = http::lwipHTTP->RetrieveNIC();
        std::pair<Status, size_t> mutex = nic->TakeMutex();
        if (mutex.first != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO TAKE MUTEX");
            return Status(Status::Code::BAD_TIMEOUT);
        }
    
    #if defined(DEBUG)
        http::RequestHeader header(
            rest_method_e::GET,
            http_scheme_e::HTTPS,
            "api.mfm.edgecross.dev",
            443,
            "/api/mfm/device/write",
            "MODLINK-L/0.0.1"
        );
    #else
        http::RequestHeader header(rest_method_e::GET, http_scheme_e::HTTPS, "api.mfm.edgecross.ai", 443, "/api/mfm/device/write", "MODLINK-L/0.0.1");
    #endif
        http::RequestParameter parameters;
        parameters.Add("mac", macAddress.GetEthernet());

        http::lwipHTTP->GET(mutex.second, header, parameters);
        s_JarvisApiPayload.clear();

        Status ret = http::lwipHTTP->Retrieve(mutex.second, &s_JarvisApiPayload);
        nic->ReleaseMutex();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE: %s", ret.c_str());
            return ret;
        }
        
        LOG_INFO(logger, "RECEIVED JARVIS: %s", s_JarvisApiPayload.c_str());
        LOG_INFO(logger, "AFTER [TASK: implEthernetTask] Stack Remaind: %u Bytes", uxTaskGetStackHighWaterMark(NULL));
        LOG_INFO(logger, "AFTER Config Start: %u Bytes", ESP.getFreeHeap());
        return Status(Status::Code::GOOD);
    }

}