/**
 * @file Core.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief MUFFIN 프레임워크 내부의 핵심 기능을 제공하는 클래스를 정의합니다.
 * 
 * @date 2024-10-30
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include <esp_system.h>

#include "Jarvis/Jarvis.h"
#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "Common/Convert/ConvertClass.h"
#include "Core.h"
#include "Core/Task/ModbusTask.h"
#include "Device/DeviceStatus.h"
#include "DataFormat/JSON/JSON.h"
#include "IM/AC/Alarm/DeprecableAlarm.h"
#include "IM/EA/DeprecableOperationTime.h"
#include "IM/EA/DeprecableProductionInfo.h"
#include "IM/FirmwareVersion/FirmwareVersion.h"
#include "IM/Node/NodeStore.h"
#include "Initializer/Initializer.h"
#include "Protocol/MQTT/CatMQTT/CatMQTT.h"
#include "Protocol/MQTT/CDO.h"
#include "Protocol/SPEAR/SPEAR.h"
#include "Storage/ESP32FS/ESP32FS.h"
#include "Task/MqttTask.h"
#include "Task/JarvisTask.h"
#include "Task/UpdateTask.h"
#include "Task/CyclicalPubTask.h"
#include "Protocol/Modbus/ModbusMutex.h"
#include "Protocol/Modbus/ModbusTCP.h"
#include "Protocol/Modbus/ModbusRTU.h"
#include "Protocol/Modbus/Include/ArduinoModbus/src/ModbusRTUClient.h"
#include "Jarvis/Config/Protocol/ModbusRTU.h"
#include "IM/Node/Include/TypeDefinitions.h"
#include "IM/AC/Alarm/DeprecableAlarm.h"



namespace muffin {

    std::vector<muffin::jarvis::config::ModbusRTU> mVectorModbusRTU;
    std::vector<muffin::jarvis::config::ModbusTCP> mVectorModbusTCP;
    muffin::jarvis::config::Ethernet mEthernet;
    

    Core* Core::CreateInstance() noexcept
    {
        if (mInstance == nullptr)
        {
            logger.Init();
            LOG_INFO(logger, "[ESP32] Semantic Version: %s,  Version Code: %u",
                FW_VERSION_ESP32.GetSemanticVersion(),
                FW_VERSION_ESP32.GetVersionCode());

        #if defined(MODLINK_T2)
            if (spear.Init() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "NO SIGN-ON REQUEST FROM ATmega2560. WILL RESTART ESP32.");
                spear.Reset();
                esp_restart();
            }

            if (spear.VersionEnquiryService() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO VERSION SERVICE FROM THE MEGA2560");
                spear.Reset();
                esp_restart();
            }

            LOG_INFO(logger, "[MEGA2560] Semantic Version: %s,  Version Code: %u",
                FW_VERSION_MEGA2560.GetSemanticVersion(),
                FW_VERSION_MEGA2560.GetVersionCode());
        #endif
        
            mInstance = new(std::nothrow) Core();
            if (mInstance == nullptr)
            {
                LOG_ERROR(logger, "FATAL ERROR OCCURED: FAILED TO ALLOCATE MEMORY FOR MUFFIN CORE");
                esp_restart();
            }
        }

        return mInstance;
    }

    Core& Core::GetInstance() noexcept
    {
        ASSERT((mInstance != nullptr), "NO INSTANCE CREATED: CALL FUNCTION \"CreateInstance\" IN ADVANCE");
        return *mInstance;
    }

    void Core::Init()
    {
        /**
         * @todo Reset 사유에 따라 자동으로 초기화 하는 기능의 개발이 필요합니다.
         * @details JARVIS 설정으로 인해 런타임에 크래시 같은 문제가 있을 수 있습니다.
         *          이러한 경우에는 계속해서 반복적으로 MODLINK가 리셋되는 현상이 발생할
         *          수 있습니다. 따라서 reset 사유를 확인하여 JARVIS 설정을 초기화 하는
         *          기능이 필요합니다. 단, 다른 부서와의 협의가 선행되어야 합니다.
         */
        DeviceStatus* deviceStatus = DeviceStatus::CreateInstanceOrNULL();
        deviceStatus->SetResetReason(esp_reset_reason());

        Initializer initializer;
        initializer.StartOrCrash();


        /**
         * @todo 설정이 정상적이지 않을 때 어떻게 처리할 것인지 결정해야 합니다.
         * @details 현재는 설정에 실패했을 때, 최대 5번까지 다시 시도하게 되어 있습니다.
         *          설정에 성공했다면 루프를 빠져나가게 됩니다.
         */
        for (uint8_t i = 0; i < MAX_RETRY_COUNT; ++i)
        {
            Status ret = initializer.Configure();
            if (ret == Status::Code::GOOD)
            {
                LOG_INFO(logger, "MUFFIN is configured and ready to go!");
                break;
            }
            
            if ((i + 1) == MAX_RETRY_COUNT)
            {
                LOG_ERROR(logger, "FAILED TO CONFIGURE MUFFIN: %s", ret.c_str());
                esp_restart();
            }
            else
            {
                LOG_WARNING(logger, "[TRIAL: #%u] CONFIGURATION WAS UNSUCCESSFUL: %s", i, ret.c_str());
            }
            vTaskDelay((5 * SECOND_IN_MILLIS) / portTICK_PERIOD_MS);
        }
    #if !defined(CATFS)
        StartTaskMQTT();
        StartUpdateTask();
    #endif
    }

    void Core::RouteMqttMessage(const mqtt::Message& message)
    {
        const mqtt::topic_e topic = message.GetTopicCode();
        const std::string payload = message.GetPayload();

        switch (topic)
        {
        /**
         * @todo 현재 CIA 개체에 저장된 메시지를 Peek() 하는 게 아니라 Retrieve()로
         *       가져오기 때문에 startJarvisTask()가 실패하는 경우 해당 메시지는
         *       소실되게 됩니다. 따라서 요청이 사라지지 않게 보완하는 작업이 필요합니다.
         */
        case mqtt::topic_e::JARVIS_REQUEST:
            startJarvisTask(payload);
            return;
        case mqtt::topic_e::REMOTE_CONTROL_REQUEST:
            startRemoteControll(payload);
            break;
        case mqtt::topic_e::FOTA_UPDATE:
            startOTA(payload);
            break;
        default:
            ASSERT(false, "UNDEFINED ERROR: MAY BE NEWLY DEFINED TOPIC OR AN UNEXPECTED ERROR");
            break;
        }
        
        // mqtt::CatMQTT& catMqtt = mqtt::CatMQTT::GetInstance();
    }

    void Core::startJarvisTask(const std::string& payload)
    {
        jarvis_task_params* pvParameters = new(std::nothrow) jarvis_task_params();
        if (pvParameters == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR TASK PARAMETERS");
            return;
        }
        
        pvParameters->Callback = onJarvisValidationResult;
        pvParameters->RequestPayload = payload;

        /**
         * @todo 스택 오버플로우를 방지하기 위해서 JARVIS 설정 정보 크기에 따라서 태스크에 할당하는 스택 메모리의 크기를 조정해야 합니다.
         * @todo 실행 도중에 태스크를 시작하면 필요한만큼의 스택을 할당할 수 없습니다. 그 외에도 태스크를 실행시키는 데 실패할 수 있습니다.
         *       이러한 경우에는 서버로 오류 메시지를 보낼 수 있도록 코드를 추가로 개발해야 합니다.
         */
        BaseType_t taskCreationResult = xTaskCreatePinnedToCore(
            ProcessJarvisRequestTask,   // Function to be run inside of the task
            "ProcessJarvisRequestTask", // The identifier of this task for men
            10 * KILLOBYTE,             // Stack memory size to allocate
            pvParameters,	            // Task parameters to be passed to the function
            0,				            // Task Priority for scheduling
            NULL,                       // The identifier of this task for machines
            0				            // Index of MCU core where the function to run
        );

        /**
         * @todo 태스크 생성에 실패했음을 호출자에게 반환해야 합니다.
         * @todo 호출자는 반환된 값을 보고 적절한 처리를 해야 합니다.
         */
        jarvis::rsc_e rsc;
        std::string description;
        switch (taskCreationResult)
        {
        case pdPASS:
            LOG_INFO(logger, "The JARVIS task has been started");
            // return Status(Status::Code::GOOD);
            return;

        case pdFAIL:
            // return Status(Status::Code::BAD_UNEXPECTED_ERROR);
            rsc = jarvis::rsc_e::BAD;
            description = "FAILED TO START WITHOUT SPECIFIC REASON";
            LOG_ERROR(logger, description.c_str());
            break;

        case errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY:
            // return Status(Status::Code::BAD_OUT_OF_MEMORY);
            rsc = jarvis::rsc_e::BAD_OUT_OF_MEMORY;
            description = "FAILED TO ALLOCATE ENOUGH MEMORY FOR THE TASK";
            LOG_ERROR(logger, description.c_str());
            break;

        default:
            // return Status(Status::Code::BAD_UNEXPECTED_ERROR);
            rsc = jarvis::rsc_e::BAD_UNEXPECTED_ERROR;
            description = "UNKNOWN ERROR: " + Convert.ToString(taskCreationResult);
            LOG_ERROR(logger, description.c_str());
            break;
        }

        jarvis_struct_t messageConfig;
        messageConfig.ResponseCode     = Convert.ToUInt16(rsc);
        messageConfig.Description      = description;
        messageConfig.SourceTimestamp  = GetTimestampInMillis();

        JSON json;
        std::string payload = json.Serialize(messageConfig);

        mqtt::Message message(mqtt::topic_e::JARVIS_RESPONSE, payload);
        mqtt::CDO& cdo = mqtt::CDO::GetInstance();

        Status ret = cdo.Store(message);
        if (ret != Status::Code::GOOD)
        {
            /**
             * @todo Store 실패시 flash 메모리에 저장하는 것과 같은 방법을 적용하여 실패에 강건하도록 코드를 작성해야 합니다.
             */
            LOG_ERROR(logger, "FAIL TO STORE JARVIS RESPONSE MESSAGE INTO CDO: %s", ret.c_str());
            return;
        }
    }

    void Core::startRemoteControll(const std::string& payload)
    {
        JSON json;
        remote_controll_struct_t messageconfig;
        std::string serializedPayload;
        JsonDocument doc;
        Status retJSON = json.Deserialize(payload, &doc);
        std::string Description;

    
        if (retJSON != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO DESERIALIZE JSON: %s", retJSON.c_str());

            switch (retJSON.ToCode())
            {
            case Status::Code::BAD_END_OF_STREAM:
                Description = "PAYLOAD INSUFFICIENT OR INCOMPLETE";
                break;
            case Status::Code::BAD_NO_DATA:
                Description ="PAYLOAD EMPTY";
                break;
            case Status::Code::BAD_DATA_ENCODING_INVALID:
                Description = "PAYLOAD INVALID ENCODING";
                break;
            case Status::Code::BAD_OUT_OF_MEMORY:
                Description = "PAYLOAD OUT OF MEMORY";
                break;
            case Status::Code::BAD_ENCODING_LIMITS_EXCEEDED:
                Description = "PAYLOAD EXCEEDED NESTING LIMIT";
                break;
            case Status::Code::BAD_UNEXPECTED_ERROR:
                Description = "UNDEFINED CONDITION";
                break;
            default:
                Description = "UNDEFINED CONDITION";
                break;
            }
            
            messageconfig.SourceTimestamp = GetTimestampInMillis(); 
            messageconfig.ResponseCode    = "900 :" + Description;

            serializedPayload = json.Serialize(messageconfig);
            mqtt::Message message(mqtt::topic_e::REMOTE_CONTROL_RESPONSE, serializedPayload);
            mqtt::CDO& cdo = mqtt::CDO::GetInstance();
            Status ret = cdo.Store(message);
            if (ret != Status::Code::GOOD)
            {
                /**
                 * @todo Store 실패시 falsh 메모리에 저장하는 방법
                 * 
                 */
                LOG_ERROR(logger, "FAIL TO SAVE MESSAGE IN CDO STORE");
            }
            return;
        }
        
        /**
         * @todo JSON Message에 대해 Validator 구현해야함
         */
        JsonArray request = doc["req"].as<JsonArray>();
        messageconfig.RequestData = request;
        messageconfig.ID = doc["id"].as<std::string>();
        std::vector<std::pair<std::string, std::string>> remoteData;
        for (JsonObject obj : request)
        {
            std::string uid = obj["uid"].as<std::string>(); 
            std::string value = obj["val"].as<std::string>(); 
            
            remoteData.emplace_back(uid, value); 
        }


        AlarmMonitor& alarmMonitor = AlarmMonitor::GetInstance();
        std::pair<bool,std::vector<std::string>> retUCL;
        std::pair<bool,std::vector<std::string>> retLCL;
        retUCL = alarmMonitor.GetUclUid();
        retLCL = alarmMonitor.GetLclUid();

        if (retUCL.first == true)
        {
            for (auto& uclUid : retUCL.second )
            {
                if (uclUid == remoteData.at(0).first)
                {
                   bool result = alarmMonitor.ConvertUCL(uclUid,remoteData.at(0).second);
                   if (result)
                   {
                        messageconfig.SourceTimestamp   = GetTimestampInMillis();
                        messageconfig.ResponseCode      = "200";                
                        
                   }
                   else
                   {
                        messageconfig.SourceTimestamp   = GetTimestampInMillis();
                        messageconfig.ResponseCode      = "900";
                   }

                    serializedPayload = json.Serialize(messageconfig);
                    mqtt::Message message(mqtt::topic_e::REMOTE_CONTROL_RESPONSE, serializedPayload);
                    mqtt::CDO& cdo = mqtt::CDO::GetInstance();
                    Status ret = cdo.Store(message);
                    if (ret != Status::Code::GOOD)
                    {
                        /**
                         * @todo Store 실패시 falsh 메모리에 저장하는 방법
                         * 
                         */
                        LOG_ERROR(logger, "FAIL TO SAVE MESSAGE IN CDO STORE");
                    }
                    return ;
                   
                }
                
            }
            
        }
        
        if (retLCL.first == true)
        {
            for (auto& lclUid : retLCL.second )
            {
                if (lclUid == remoteData.at(0).first)
                {
                   bool result = alarmMonitor.ConvertLCL(lclUid,remoteData.at(0).second);
                   if (result)
                   {
                        messageconfig.SourceTimestamp   = GetTimestampInMillis();
                        messageconfig.ResponseCode      = "200";
                   }
                   else
                   {
                        messageconfig.SourceTimestamp   = GetTimestampInMillis();
                        messageconfig.ResponseCode      = "900";
                   }

                    serializedPayload = json.Serialize(messageconfig);
                    mqtt::Message message(mqtt::topic_e::REMOTE_CONTROL_RESPONSE, serializedPayload);
                    mqtt::CDO& cdo = mqtt::CDO::GetInstance();
                    Status ret = cdo.Store(message);
                    if (ret != Status::Code::GOOD)
                    {
                        /**
                         * @todo Store 실패시 falsh 메모리에 저장하는 방법
                         * 
                         */
                        LOG_ERROR(logger, "FAIL TO SAVE MESSAGE IN CDO STORE");
                    }
                    return ;
                }
                
            }
            
        }

         /**
         * @todo 스카우터,프로직스 기준으로 제어명령은 한개밖에 들어오지 않아 고정으로 설정해두었음 remoteData.at(0), 추후 변경시 수정해야함
         */


        im::NodeStore& nodeStore = im::NodeStore::GetInstance();
        
        std::pair<Status, im::Node*> ret = nodeStore.GetNodeReferenceUID(remoteData.at(0).first);
        std::pair<Status, uint16_t> retConvertModbus = std::make_pair(Status(Status::Code::UNCERTAIN),0);
        
        if (ret.first != Status::Code::GOOD)
        {
            messageconfig.SourceTimestamp   = GetTimestampInMillis();
            messageconfig.ResponseCode  = "900 : UNDEFINED UID : " + remoteData.at(0).first;

            serializedPayload = json.Serialize(messageconfig);
            mqtt::Message message(mqtt::topic_e::REMOTE_CONTROL_RESPONSE, serializedPayload);
            mqtt::CDO& cdo = mqtt::CDO::GetInstance();
            Status ret = cdo.Store(message);
            if (ret != Status::Code::GOOD)
            {
                /**
                 * @todo Store 실패시 falsh 메모리에 저장하는 방법
                 * 
                 */
                LOG_ERROR(logger, "FAIL TO SAVE MESSAGE IN CDO STORE");
            }
            return ;
        }
    
        
        
        retConvertModbus = ret.second->VariableNode.ConvertModbusData(remoteData.at(0).second);
        if (retConvertModbus.first != Status::Code::GOOD)
        {
            messageconfig.SourceTimestamp = GetTimestampInMillis();
            messageconfig.ResponseCode    = "900 : " + retConvertModbus.first.ToString();
            serializedPayload = json.Serialize(messageconfig);
            mqtt::Message message(mqtt::topic_e::REMOTE_CONTROL_RESPONSE, serializedPayload);
            mqtt::CDO& cdo = mqtt::CDO::GetInstance();
            Status ret = cdo.Store(message);
            if (ret != Status::Code::GOOD)
            {
                /**
                 * @todo Store 실패시 falsh 메모리에 저장하는 방법
                 * 
                 */
                LOG_ERROR(logger, "FAIL TO SAVE MESSAGE IN CDO STORE");
            }
            return;
        }
        else
        {
            uint8_t writeResult = 0;
            if (mVectorModbusTCP.size() != 0)
            {
                for (auto& TCP : mVectorModbusTCP)
                {
                    std::pair<muffin::Status, std::vector<std::string>> retVector = TCP.GetNodes();
                    if (retVector.first == Status(Status::Code::GOOD))
                    {
                        for (auto& nodeId : retVector.second )
                        {
                            if (nodeId == ret.second->GetNodeID())
                            {
                                std::pair<muffin::Status, uint8_t> retSlaveID =  TCP.GetSlaveID();
                                if (retSlaveID.first != Status(Status::Code::GOOD))
                                {
                                    retSlaveID.second = 0;
                                }
                                jarvis::mb_area_e modbusArea = ret.second->VariableNode.GetModbusArea();
                                jarvis::addr_u modbusAddress = ret.second->VariableNode.GetAddress();
                                std::pair<bool, uint8_t> retBit = ret.second->VariableNode.GetBitindex();
                    
                                if (retBit.first == true)
                                {
                                    ModbusTCP& modbusTCP = ModbusTCP::GetInstance();
                                    modbus::datum_t registerData =  modbusTCP.GetAddressValue(retSlaveID.second, modbusAddress.Numeric, modbusArea);
                                    LOG_INFO(logger, "RAW DATA : %u ", registerData.Value);
                                    retConvertModbus.second = bitWrite(registerData.Value, retBit.second, retConvertModbus.second);
                                    LOG_INFO(logger, "RAW Data after bit index conversion : %u ", retConvertModbus.second);
                                }
                                
                                writeResult = 0;
                                if (xSemaphoreTake(xSemaphoreModbusTCP, 1000)  != pdTRUE)
                                {
                                    LOG_WARNING(logger, "[MODBUS TCP] THE WRITE MODULE IS BUSY. TRY LATER.");
                                    goto ERROR_RESPONSE;
                                }

                                switch (modbusArea)
                                {
                                case jarvis::mb_area_e::COILS:
                                    writeResult = modbusTCPClient.coilWrite(retSlaveID.second, modbusAddress.Numeric,retConvertModbus.second);
                                    break;
                                case jarvis::mb_area_e::HOLDING_REGISTER:
                                    writeResult = modbusTCPClient.holdingRegisterWrite(retSlaveID.second,modbusAddress.Numeric,retConvertModbus.second);
                                    break;
                                default:
                                    LOG_ERROR(logger,"THIS AREA IS NOT SUPPORTED, AREA : %d ", modbusArea);
                                    break;
                                }

                                xSemaphoreGive(xSemaphoreModbusTCP);
                                break;
                            }
                            
                        }
                        
                    }
                }
            }
            if (mVectorModbusRTU.size() != 0)
            {
                for (auto& RTU : mVectorModbusRTU)
                {
                    std::pair<muffin::Status, std::vector<std::string>> retVector = RTU.GetNodes();
                    if (retVector.first == Status(Status::Code::GOOD))
                    {
                        for (auto& nodeId : retVector.second )
                        {
                            if (nodeId == ret.second->GetNodeID())
                            {
                                std::pair<muffin::Status, uint8_t> retSlaveID =  RTU.GetSlaveID();
                                if (retSlaveID.first != Status(Status::Code::GOOD))
                                {
                                    retSlaveID.second = 0;
                                }

                                jarvis::mb_area_e modbusArea = ret.second->VariableNode.GetModbusArea();
                                jarvis::addr_u modbusAddress = ret.second->VariableNode.GetAddress();
                                std::pair<bool, uint8_t> retBit = ret.second->VariableNode.GetBitindex();
                    
                                if (retBit.first == true)
                                {
                                    ModbusRTU& modbusRTU = ModbusRTU::GetInstance();
                                    modbus::datum_t registerData =  modbusRTU.GetAddressValue(retSlaveID.second, modbusAddress.Numeric, modbusArea);
                                    LOG_INFO(logger, "RAW DATA : %u ", registerData.Value);
                                    retConvertModbus.second = bitWrite(registerData.Value, retBit.second, retConvertModbus.second);
                                    LOG_INFO(logger, "RAW Data after bit index conversion : %u ", retConvertModbus.second);
                                }
                                
                                writeResult = 0;
                                if (xSemaphoreTake(xSemaphoreModbusRTU, 1000)  != pdTRUE)
                                {
                                    LOG_WARNING(logger, "[MODBUS RTU] THE WRITE MODULE IS BUSY. TRY LATER.");
                                    goto ERROR_RESPONSE;
                                }

                                switch (modbusArea)
                                {
                                case jarvis::mb_area_e::COILS:
                                    writeResult = ModbusRTUClient.coilWrite(retSlaveID.second, modbusAddress.Numeric,retConvertModbus.second);
                                    break;
                                case jarvis::mb_area_e::HOLDING_REGISTER:
                                    writeResult = ModbusRTUClient.holdingRegisterWrite(retSlaveID.second,modbusAddress.Numeric,retConvertModbus.second);
                                    break;
                                default:
                                    LOG_ERROR(logger,"THIS AREA IS NOT SUPPORTED, AREA : %d ", modbusArea);
                                    break;
                                }

                                xSemaphoreGive(xSemaphoreModbusRTU);
                                break;
                            }
                        }
                    }
                }
                
                
            }
            
ERROR_RESPONSE:
            if (writeResult == 1)
            {
                messageconfig.ResponseCode = "200";
            }
            else
            {
                messageconfig.ResponseCode = "900 : UNEXPECTED ERROR";
            }
            messageconfig.SourceTimestamp  = GetTimestampInMillis();
            serializedPayload = json.Serialize(messageconfig);

            mqtt::Message message(mqtt::topic_e::REMOTE_CONTROL_RESPONSE, serializedPayload);

            mqtt::CDO& cdo = mqtt::CDO::GetInstance();
            Status ret = cdo.Store(message);
            if (ret != Status::Code::GOOD)
            {
                /**
                 * @todo Store 실패시 falsh 메모리에 저장하는 방법
                 * 
                 */
                LOG_ERROR(logger, "FAIL TO SAVE MESSAGE IN CDO STORE");
            }
            return;
        }
    }

    void Core::onJarvisValidationResult(jarvis::ValidationResult result)
    {
        if (result.GetRSC() >= jarvis::rsc_e::BAD)
        {
            jarvis_struct_t messageConfig;
            messageConfig.ResponseCode     = Convert.ToUInt16(result.GetRSC());
            messageConfig.Description      = result.GetDescription();
            messageConfig.SourceTimestamp  = GetTimestampInMillis();

            std::vector<jarvis::cfg_key_e> vectorKeyWithNG = result.RetrieveKeyWithNG();
            for (auto& key : vectorKeyWithNG)
            {
                messageConfig.Config.emplace_back(Convert.ToString(key));
            }

            JSON json;
            std::string payload = json.Serialize(messageConfig);

            mqtt::Message message(mqtt::topic_e::JARVIS_RESPONSE, payload);
            mqtt::CDO& cdo = mqtt::CDO::GetInstance();

            Status ret = cdo.Store(message);
            if (ret != Status::Code::GOOD)
            {
                /**
                 * @todo Store 실패시 flash 메모리에 저장하는 것과 같은 방법을 적용하여 실패에 강건하도록 코드를 작성해야 합니다.
                 */
                LOG_ERROR(logger, "FAIL TO STORE JARVIS RESPONSE MESSAGE INTO CDO: %s", ret.c_str());
                return;
            }

            return;
        }


        /**
         * @todo 모든 태스크를 종료해야 합니다.
         */
        {
            StopCyclicalsMSGTask();
            StopModbusRtuTask();
            StopModbusTcpTask();

            AlarmMonitor& alarmMonitor = AlarmMonitor::GetInstance();
            alarmMonitor.StopTask();
            alarmMonitor.Clear();
            
            ProductionInfo& productionInfo = ProductionInfo::GetInstance();
            productionInfo.StopTask();
            productionInfo.Clear();
            
            OperationTime& operationTime = OperationTime::GetInstance();
            operationTime.StopTask();
            operationTime.Clear();

            ModbusRTU* modbusRTU = ModbusRTU::CreateInstanceOrNULL();
            modbusRTU->Clear();

            im::NodeStore* nodeStore = im::NodeStore::CreateInstanceOrNULL();
            nodeStore->Clear();
        }

        
        /**
         * @todo 설정 값 적용한 다음에 문제가 없으면 결과 값을 전송하도록 순서를 변경해야 합니다.
         */
        JSON json;
        jarvis_struct_t messageConfig;

        messageConfig.ResponseCode     = Convert.ToUInt16(result.GetRSC());
        messageConfig.Description      = result.GetDescription();
        messageConfig.SourceTimestamp  = GetTimestampInMillis();

        std::vector<jarvis::cfg_key_e> vectorKeyWithNG = result.RetrieveKeyWithNG();

        for (auto& key : vectorKeyWithNG)
        {
           messageConfig.Config.emplace_back(Convert.ToString(key));
        }

        std::string payload = json.Serialize(messageConfig);
        mqtt::Message message(mqtt::topic_e::JARVIS_RESPONSE, payload);
        mqtt::CDO& cdo = mqtt::CDO::GetInstance();
        Status ret = cdo.Store(message);
        if (ret != Status::Code::GOOD)
        {
             /**
             * @todo Store 실패시 falsh 메모리에 저장하는 방법
             * 
             */
            LOG_ERROR(logger, "FAIL TO SAVE MESSAGE IN CDO STORE");
        }


        LOG_INFO(logger, "JARVIS configuration is valid. Proceed to save and apply");
        std::string jarvisPayload;
        RetrieveJarvisRequestPayload(&jarvisPayload);


        ESP32FS& esp32FS = ESP32FS::GetInstance();
        /** 
         * @todo string이 아니라 enum class와 ConvertClass를 사용하도록 코드를 수정해야 합니다.
         */
        File file = esp32FS.Open("/jarvis/config.json", "w", true);

        /**
         * @todo 성능 향상을 위해 속도가 더 빠른 buffered IO 방식으로 코드를 수정해야 합니다.
         */
        for (size_t i = 0; i < jarvisPayload.length(); ++i)
        {
            file.write(jarvisPayload[i]);
        }

        /**
         * @todo 저장 성공 유무를 확인할 수 있는 기능을 추가해야 합니다.
         */
        file.close();

        /**
         * @todo 설정 정보가 올바르게 설정되었는지 확인하는 기능을 추가해야 합니다.
         */
        ApplyJarvisTask();
    }

    void Core::startOTA(const std::string& payload)
    {
        StartManualFirmwareUpdate(payload);
    }

    Core* Core::mInstance = nullptr;
    jarvis::ValidationResult Core::mJarvisValidationResult;
}