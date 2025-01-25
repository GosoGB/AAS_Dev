/**
 * @file Core.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief MUFFIN 프레임워크를 초기화 기능을 제공하는 클래스를 정의합니다.
 * 
 * @date 2025-01-20
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include <esp_system.h>
#include <Preferences.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "Common/Convert/ConvertClass.h"
#include "Core.h"
#include "Core/Task/ModbusTask.h"

#include "DataFormat/CSV/CSV.h"

#include "IM/Custom/Device/DeviceStatus.h"
#include "JARVIS/JARVIS.h"
#include "IM/AC/Alarm/DeprecableAlarm.h"
#include "IM/Custom/Constants.h"
#include "IM/Custom/FirmwareVersion/FirmwareVersion.h"
#include "IM/Custom/MacAddress/MacAddress.h"
#include "IM/EA/DeprecableOperationTime.h"
#include "IM/EA/DeprecableProductionInfo.h"
#include "IM/Node/NodeStore.h"

#include "Protocol/MQTT/CatMQTT/CatMQTT.h"
#include "Protocol/MQTT/CDO.h"
#include "Protocol/SPEAR/SPEAR.h"
#include "Storage/ESP32FS/ESP32FS.h"
#include "Task/JarvisTask.h"
#include "Task/CyclicalPubTask.h"
#include "Protocol/Modbus/ModbusMutex.h"
#include "Protocol/Modbus/ModbusTCP.h"
#include "Protocol/Modbus/ModbusRTU.h"
#include "Protocol/Modbus/Include/ArduinoModbus/src/ModbusRTUClient.h"
#include "JARVIS/Config/Protocol/ModbusRTU.h"
#include "IM/Node/Include/TypeDefinitions.h"
#include "IM/AC/Alarm/DeprecableAlarm.h"
#include "Protocol/HTTP/CatHTTP/CatHTTP.h"
#include "Protocol/HTTP/Include/TypeDefinitions.h"
#include "Protocol/HTTP/IHTTP.h"
#include "Protocol/HTTP/LwipHTTP/LwipHTTP.h"

#include "ServiceSets/FirmwareUpdateServiceSet/SendMessageService.h"
#include "ServiceSets/HttpServiceSet/StartHttpClientService.h"
#include "ServiceSets/JarvisServiceSet/ApplyOperationService.h"
#include "ServiceSets/MqttServiceSet/StartMqttClientService.h"



namespace muffin {

    std::vector<muffin::jvs::config::ModbusRTU> mVectorModbusRTU;
    std::vector<muffin::jvs::config::ModbusTCP> mVectorModbusTCP;


    void Core::Init()
    {
        logger.Init();
        LOG_INFO(logger, "MAC Address: %s", macAddress.GetEthernet());
        LOG_INFO(logger, "Semantic Version: %s,  Version Code: %u", 
            FW_VERSION_ESP32.GetSemanticVersion(),
            FW_VERSION_ESP32.GetVersionCode());

        Status ret = esp32FS.Begin(false);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FATAL ERROR: FAILED TO MOUNT ESP32 FILE SYSTEM");
            std::abort();
        }
        LOG_INFO(logger, "File system: %s", ret.c_str());

        init_cfg_t initConfig;
        ret = readInitConfig(&initConfig);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FATAL ERROR: FAILED TO LOAD INIT CONFIG FILE");
            std::abort();
        }

        if (isResetByPanic() == true)
        {
            if (++initConfig.PanicResetCount == MAX_RETRY_COUNT)
            {
                ret = esp32FS.Remove(JARVIS_PATH);
            #if !defined(DEBUG)
            ---------------------------------------------------------------------
            |  @todo #1  JARVIS 설정 초기화 사유를 <INIT_FILE_PATH>에 저장해야 함  |
            |  @todo #2  mfm/status 토픽에 설정값에 변화가 있었다고 알려줘야 함     |
            ---------------------------------------------------------------------
            #endif
                if (ret == Status::Code::BAD_DEVICE_FAILURE)
                {
                    LOG_ERROR(logger, "FATAL ERROR: FAILED TO REMOVE JARVIS CONFIG");
                    std::abort();
                }
            }
            
            initConfig.PanicResetCount = 0;
            ret = writeInitConfig(initConfig);
            if (ret == Status::Code::BAD_ENCODING_ERROR)
            {
                LOG_ERROR(logger, "FATAL ERROR: FAILED TO ENCODE INIT CONFIG");
                std::abort();
            }
            else if (ret == Status::Code::BAD_DEVICE_FAILURE)
            {
                LOG_ERROR(logger, "FATAL ERROR: FAILED TO WRITE INIT CONFIG");
                std::abort();
            }

            LOG_INFO(logger, "JARVIS config has been reset due to panic reset");
        }

        if (esp32FS.DoesExist(JARVIS_PATH) == Status::Code::BAD_NOT_FOUND)
        {
            LOG_INFO(logger, "No JARVIS config found");
            
            ret = createDefaultJARVIS();
            if (ret == Status::Code::BAD_DEVICE_FAILURE)
            {
                LOG_ERROR(logger, "FAILED TO CREATE DEAULT JARVIS CONFIG");
                std::abort();
            }

            LOG_INFO(logger, "Created default JARVIS config");
        }
        
        ret = loadJarvisConfig();
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO LOAD JARVIS CONFIG: %s", ret.c_str());
            std::abort();
        }
        
        do
        {
            ret = ApplyOperationService();
            if (ret != Status::Code::GOOD)
            {
                goto RETRY;
            }

            ret = InitHttpService();
            if (ret != Status::Code::GOOD)
            {
                goto RETRY;
            }

            ret = InitMqttClientService();
            if (ret != Status::Code::GOOD)
            {
                goto RETRY;
            }

            ret = ConnectMqttClientService();
            if (ret != Status::Code::GOOD)
            {
                goto RETRY;
            }

        RETRY:
            vTaskDelay(SECOND_IN_MILLIS / portTICK_PERIOD_MS);
            
        } while (ret != Status::Code::GOOD);


        if (initConfig.HasPendingJARVIS == true)
        {
            ;
        }
        
        if (initConfig.HasPendingUpdate == true)
        {
            // start firmware update
            // reset device
        }
    /*
        if (mHasJarvisCommand == false && mHasFotaCommand == true)
        {
            Preferences nvs;
            nvs.begin("fota");
            const std::string payload = nvs.getString("fotaPayload").c_str();
            LOG_WARNING(logger, "payload : %s",payload.c_str());
            nvs.putBool("fotaFlag",false);
            nvs.end();

            StartOTA(payload);
        }
    */
        
        // call ApplyJarvisTask() function; a.k.a. start jarvis task




        PublishFirmwareStatusMessageService();




#if !defined(DEBUG)
    #if defined(MODLINK_T2)
    {
        uint8_t trialCount = 0;    
        while (spear.Init() != Status::Code::GOOD)
        {
            if (trialCount == MAX_RETRY_COUNT)
            {
                LOG_ERROR(logger, "FAILED TO GET SIGN-ON REQUEST FROM ATmega2560");
                goto SPEAR_FAILED;
            }
            spear.Reset();
            ++trialCount;
        }
        
        if (spear.VersionEnquiryService() != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO QUERY VERSION OF THE ATmega2560");
            goto SPEAR_FAILED;
        }
        goto SPEAR_SUCCEDED;
        
    SPEAR_FAILED:
        LOG_ERROR(logger, "FAILED TO INITIALIZE SPEAR");
    SPEAR_SUCCEDED:
        LOG_INFO(logger, "[MEGA2560] Semantic Version: %s,  Version Code: %u",
            FW_VERSION_MEGA2560.GetSemanticVersion(),
            FW_VERSION_MEGA2560.GetVersionCode());
    }
    #endif
#endif
    }

    Status Core::readInitConfig(init_cfg_t* output)
    {
        ASSERT((output != nullptr), "OUTPUT PARAMETER CANNOT BE NULL");

        Status ret = esp32FS.DoesExist(INIT_FILE_PATH);
        LOG_INFO(logger, "Init config file: %s", ret.c_str());

        if (ret == Status::Code::BAD_NOT_FOUND)
        {
            File file = esp32FS.Open(INIT_FILE_PATH, "w", true);
            if (file == false)
            {
                LOG_ERROR(logger, "FATAL ERROR: FAILED TO OPEN INIT CONFIG FILE");
                return Status(Status::Code::BAD_DEVICE_FAILURE);
            }
            
            const init_cfg_t DEFAULT_INIT_CONFIG = {
                .PanicResetCount  = 0,
                .HasPendingJARVIS = 0,
                .HasPendingUpdate = 0
            };

            char buffer[16] = {'\0'};
            sprintf(buffer, "%u,%u,%u", DEFAULT_INIT_CONFIG.PanicResetCount, 
                                        DEFAULT_INIT_CONFIG.HasPendingJARVIS, 
                                        DEFAULT_INIT_CONFIG.HasPendingUpdate);
            file.write(reinterpret_cast<uint8_t*>(buffer), sizeof(buffer));
            file.close();

            file = esp32FS.Open(INIT_FILE_PATH, "r", false);
            if (file == false)
            {
                LOG_ERROR(logger, "FATAL ERROR: FAILED TO OPEN INIT CONFIG FILE");
                return Status(Status::Code::BAD_DEVICE_FAILURE);
            }

            char readback[16] = {'\0'};
            file.readBytes(readback, sizeof(readback));
            file.close();

            if (strcmp(buffer, readback) != 0)
            {
                LOG_ERROR(logger, "FATAL ERROR: FAILED TO CREATE INIT CONFIG FILE");
                if (esp32FS.Format() != Status::Code::GOOD)
                {
                    LOG_ERROR(logger, "FATAL ERROR: FAILED TO FORMAT FILE SYSTEM");
                }                
                return Status(Status::Code::BAD_DEVICE_FAILURE);
            }
            
            LOG_INFO(logger, "Created init config file");
            return Status(Status::Code::GOOD);
        }
        
        File file = esp32FS.Open(INIT_FILE_PATH, "r", "false");
        if (file == false)
        {
            LOG_ERROR(logger, "FATAL ERROR: FAILED TO OPEN INIT CONFIG FILE");
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
        
        const size_t size = file.size();
        char buffer[size] = {'\0'};
        file.readBytes(buffer, size);

        CSV csv;
        ret = csv.Decode(buffer, output);
        if (ret == Status::Code::BAD_NO_DATA)
        {
            LOG_ERROR(logger, "FATAL ERROR: EMPTY INIT CONFIG");
            return ret;
        }
        else if (ret == Status::Code::BAD_DATA_LOST)
        {
            LOG_ERROR(logger, "FATAL ERROR: CORRUPTED INIT CONFIG");
            return ret;
        }
        else if (ret == Status::Code::UNCERTAIN_DATA_SUBNORMAL)
        {
            LOG_WARNING(logger, "INIT CONFIG: BUFFER OVERFLOW");
        }
        
        return ret;
    }

    Status Core::writeInitConfig(const init_cfg_t& config)
    {
        const uint8_t size = 16;
        char buffer[size] = {'\0'};

        CSV csv;
        Status ret = csv.Encode(config, size, buffer);
        if (ret != Status::Code::GOOD)
        {
            return ret;
        }
        
        File file = esp32FS.Open(INIT_FILE_PATH, "w", true);
        if (file == false)
        {
            ret = Status::Code::BAD_DEVICE_FAILURE;
            return ret;
        }

        file.write(reinterpret_cast<uint8_t*>(buffer), strlen(buffer));
        file.flush();
        file.close();
        
        char readback[size] = {'\0'};
        file = esp32FS.Open(INIT_FILE_PATH, "r", false);
        if (file == false)
        {
            ret = Status::Code::BAD_DEVICE_FAILURE;
            return ret;
        }

        file.readBytes(readback, size);
        if (strcmp(buffer, readback) != 0)
        {
            ret = Status::Code::BAD_DEVICE_FAILURE;
            return ret;
        }
        
        ret = Status::Code::GOOD;
        return ret;
    }

    bool Core::isResetByPanic()
    {
        const esp_reset_reason_t resetReason = esp_reset_reason();
        deviceStatus.SetResetReason(resetReason);
        return resetReason == ESP_RST_PANIC;
    }

    Status Core::createDefaultJARVIS()
    {
        File file = esp32FS.Open(JARVIS_PATH, "w", true);
        if (file == false)
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }

        JsonDocument doc;
        doc["ver"] = "v1";

        JsonObject cnt = doc["cnt"].to<JsonObject>();
        cnt["rs232"].to<JsonArray>();
        cnt["rs485"].to<JsonArray>();
        cnt["wifi"].to<JsonArray>();
        cnt["eth"].to<JsonArray>();
        cnt["mbrtu"].to<JsonArray>();
        cnt["mbtcp"].to<JsonArray>();
        cnt["node"].to<JsonArray>();
        cnt["alarm"].to<JsonArray>();
        cnt["optime"].to<JsonArray>();
        cnt["prod"].to<JsonArray>();

        JsonArray catm1 = cnt["catm1"].to<JsonArray>();
        JsonObject _catm1 = catm1.add<JsonObject>();
        _catm1["md"]    = "LM5";
        _catm1["ctry"]  = "KR";

        JsonArray op     = cnt["op"].to<JsonArray>();
        JsonObject _op   = op.add<JsonObject>();
        _op["snic"]      = "lte";
        _op["exp"]       = true;
        _op["intvPoll"]  =  1;
        _op["intvSrv"]   = 60;
        _op["rst"]       = false;

        const size_t size = measureJson(doc);
        char buffer[size] = {'\0'};
        serializeJson(doc, buffer, size);
        doc.clear();

        file.write(reinterpret_cast<uint8_t*>(buffer), size);
        file.flush();
        file.close();

        char readback[size] = {'\0'};
        file = esp32FS.Open(JARVIS_PATH);
        file.readBytes(readback, size);
        file.close();

        if (strcmp(buffer, readback) != 0)
        {
            esp32FS.Remove(JARVIS_PATH);
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }

        return Status(Status::Code::GOOD);
    }

    Status Core::loadJarvisConfig()
    {
        ASSERT((esp32FS.DoesExist(JARVIS_PATH) == Status::Code::GOOD), "JARVIS CONFIG MUST EXIST");

        File file = esp32FS.Open(JARVIS_PATH, "r", false);
        if (file == false)
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
        
        JSON json;
        JsonDocument doc;
        Status ret = json.Deserialize(file, &doc);
        file.close();

        if (ret != Status::Code::GOOD)
        {
            esp32FS.Remove(JARVIS_PATH);
            return ret;
        }

    #if defined(DEBUG)
        doc["cnt"].remove("catm1");
        doc["cnt"]["catm1"].to<JsonArray>();
        JsonObject eth = doc["cnt"]["eth"].add<JsonObject>();
        eth["dhcp"]  = true;
        eth["ip"]    = NULL;
        eth["snm"]   = NULL;
        eth["gtw"]   = NULL;
        eth["dns1"]  = NULL;
        eth["dns2"]  = NULL;
        JsonObject op = doc["cnt"]["op"][0].as<JsonObject>();
        op["snic"] = "eth";
    #else
    -------------------------------------------------------------------
    |  @todo #1  이더넷 테스트를 위해 DEBUG 플래그 안의 코드를 만들었음   |
    |  @todo #2  릴리즈fmf 할 때는 반드시 지우고 펌웨어를 출고시켜야 함   |
    -------------------------------------------------------------------
    #endif
        
        jarvis = new(std::nothrow) JARVIS();
        if (jarvis == nullptr)
        {
            ret = Status::Code::BAD_OUT_OF_MEMORY;
            return ret;
        }
        
        jvs::ValidationResult result = jarvis->Validate(doc);
        doc.clear();

        if (result.GetRSC() == jvs::rsc_e::GOOD)
        {
            ret = Status::Code::BAD_DATA_LOST;
        }
        
        return ret;
    }








    void Core::saveFotaFlag(const std::string& payload)
    {
        Preferences nvs;
        nvs.begin("fota");
        nvs.putBool("fotaFlag",true);
        nvs.putString("fotaPayload",payload.c_str());
        nvs.end();
    }

    void Core::StartJarvisTask()
    {
        jarvis_task_params* pvParameters = new(std::nothrow) jarvis_task_params();
        if (pvParameters == nullptr)
        {
            LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR TASK PARAMETERS");
            return;
        }
        
        pvParameters->Callback = onJarvisValidationResult;

        /**
         * @todo 스택 오버플로우를 방지하기 위해서 JARVIS 설정 정보 크기에 따라서 태스크에 할당하는 스택 메모리의 크기를 조정해야 합니다.
         * @todo 실행 도중에 태스크를 시작하면 필요한만큼의 스택을 할당할 수 없습니다. 그 외에도 태스크를 실행시키는 데 실패할 수 있습니다.
         *       이러한 경우에는 서버로 오류 메시지를 보낼 수 있도록 코드를 추가로 개발해야 합니다.
         */
        BaseType_t taskCreationResult = xTaskCreatePinnedToCore(
            ProcessJarvisRequestTask,   // Function to be run inside of the task
            "ProcessJarvisRequestTask", // The identifier of this task for men
            10 * KILLOBYTE,              // Stack memory size to allocate
            pvParameters,	            // Task parameters to be passed to the function
            0,				            // Task Priority for scheduling
            NULL,                       // The identifier of this task for machines
            0				            // Index of MCU core where the function to run
        );

        /**
         * @todo 태스크 생성에 실패했음을 호출자에게 반환해야 합니다.
         * @todo 호출자는 반환된 값을 보고 적절한 처리를 해야 합니다.
         */
        jvs::rsc_e rsc;
        std::string description;
        switch (taskCreationResult)
        {
        case pdPASS:
            LOG_INFO(logger, "The JARVIS task has been started");
            // return Status(Status::Code::GOOD);
            return;

        case pdFAIL:
            // return Status(Status::Code::BAD_UNEXPECTED_ERROR);
            rsc = jvs::rsc_e::BAD;
            description = "FAILED TO START WITHOUT SPECIFIC REASON";
            LOG_ERROR(logger, description.c_str());
            break;

        case errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY:
            // return Status(Status::Code::BAD_OUT_OF_MEMORY);
            rsc = jvs::rsc_e::BAD_OUT_OF_MEMORY;
            description = "FAILED TO ALLOCATE ENOUGH MEMORY FOR THE TASK";
            LOG_ERROR(logger, description.c_str());
            break;

        default:
            // return Status(Status::Code::BAD_UNEXPECTED_ERROR);
            rsc = jvs::rsc_e::BAD_UNEXPECTED_ERROR;
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
        Status ret = mqtt::cdo.Store(message);
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
            Status ret = mqtt::cdo.Store(message);
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
                    Status ret = mqtt::cdo.Store(message);
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
                    Status ret = mqtt::cdo.Store(message);
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
            Status ret = mqtt::cdo.Store(message);
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
            Status ret = mqtt::cdo.Store(message);
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
    #if defined(MODLINK_T2) || defined(MODLINK_B)
            if (mVectorModbusTCP.size() != 0)
            {
                for (auto& TCP : mVectorModbusTCP)
                {
                    for (auto& modbusTCP : ModbusTcpVector)
                    {
                        if (modbusTCP.GetServerIP() != TCP.GetIPv4().second || modbusTCP.GetServerPort() != TCP.GetPort().second)
                        {
                            continue;
                        }
                        
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
                                    jvs::mb_area_e modbusArea = ret.second->VariableNode.GetModbusArea();
                                    jvs::addr_u modbusAddress = ret.second->VariableNode.GetAddress();
                                    std::pair<bool, uint8_t> retBit = ret.second->VariableNode.GetBitindex();
                        
                                    if (retBit.first == true)
                                    {
                                        modbus::datum_t registerData =  modbusTCP.GetAddressValue(retSlaveID.second, modbusAddress.Numeric, modbusArea);
                                        LOG_DEBUG(logger, "RAW DATA : %u ", registerData.Value);
                                        retConvertModbus.second = bitWrite(registerData.Value, retBit.second, retConvertModbus.second);
                                        LOG_DEBUG(logger, "RAW Data after bit index conversion : %u ", retConvertModbus.second);
                                    }
                                    
                                    writeResult = 0;
                                    if (xSemaphoreTake(xSemaphoreModbusTCP, 1000)  != pdTRUE)
                                    {
                                        LOG_WARNING(logger, "[MODBUS TCP] THE WRITE MODULE IS BUSY. TRY LATER.");
                                        goto ERROR_RESPONSE;
                                    }
                                    modbusTCPClient.end();

                                    modbusTCPClient.begin(modbusTCP.GetServerIP(), modbusTCP.GetServerPort());
                                    LOG_DEBUG(logger, "[MODBUS TCP] 원격제어 : %u",retConvertModbus.second);
                                    switch (modbusArea)
                                    {
                                    case jvs::mb_area_e::COILS:
                                        writeResult = modbusTCPClient.coilWrite(retSlaveID.second, modbusAddress.Numeric,retConvertModbus.second);
                                        break;
                                    case jvs::mb_area_e::HOLDING_REGISTER:
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
            }
    #endif

            if (mVectorModbusRTU.size() != 0)
            {
                for (auto& RTU : mVectorModbusRTU)
                {
                    for (auto& modbusRTU : ModbusRtuVector)
                    {
                        if (RTU.GetPort().second != modbusRTU.mPort)
                        {
                            continue;
                        }

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

                                    jvs::mb_area_e modbusArea = ret.second->VariableNode.GetModbusArea();
                                    jvs::addr_u modbusAddress = ret.second->VariableNode.GetAddress();
                                    std::pair<bool, uint8_t> retBit = ret.second->VariableNode.GetBitindex();
                        
                                    if (retBit.first == true)
                                    {
                                        modbus::datum_t registerData =  modbusRTU.GetAddressValue(retSlaveID.second, modbusAddress.Numeric, modbusArea);
                                        LOG_DEBUG(logger, "RAW DATA : %u ", registerData.Value);
                                        retConvertModbus.second = bitWrite(registerData.Value, retBit.second, retConvertModbus.second);
                                        LOG_DEBUG(logger, "RAW Data after bit index conversion : %u ", retConvertModbus.second);
                                    }
                                    
                                    writeResult = 0;
                                #if defined(MODLINK_L) || defined(MODLINK_ML10)
                                    if (xSemaphoreTake(xSemaphoreModbusRTU, 1000)  != pdTRUE)
                                    {
                                        LOG_WARNING(logger, "[MODBUS RTU] THE WRITE MODULE IS BUSY. TRY LATER.");
                                        goto ERROR_RESPONSE;
                                    }

                                    switch (modbusArea)
                                    {
                                    case jvs::mb_area_e::COILS:
                                        writeResult = ModbusRTUClient.coilWrite(retSlaveID.second, modbusAddress.Numeric,retConvertModbus.second);
                                        break;
                                    case jvs::mb_area_e::HOLDING_REGISTER:
                                        writeResult = ModbusRTUClient.holdingRegisterWrite(retSlaveID.second,modbusAddress.Numeric,retConvertModbus.second);
                                        break;
                                    default:
                                        LOG_ERROR(logger,"THIS AREA IS NOT SUPPORTED, AREA : %d ", modbusArea);
                                        break;
                                    }

                                    LOG_INFO(logger,"제어 결과 : %s",writeResult == 1 ? "성공" : "실패");
                                    xSemaphoreGive(xSemaphoreModbusRTU);
                                #else
                                    spear_remote_control_msg_t msg;
                                    msg.Link = modbusRTU.mPort;
                                    msg.SlaveID = retSlaveID.second;
                                    msg.Area = modbusArea;
                                    msg.Address = modbusAddress.Numeric;
                                    msg.Value = retConvertModbus.second;
                                    
                                    Status result = spear.ExecuteService(msg);
                                    if (result == Status(Status::Code::GOOD))
                                    {
                                        writeResult = 1;
                                    }
                                #endif
                                    break;
                                }
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
            Status ret = mqtt::cdo.Store(message);
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

    void Core::onJarvisValidationResult(jvs::ValidationResult& result)
    {
        if (result.GetRSC() >= jvs::rsc_e::BAD)
        {
            jarvis_struct_t messageConfig;
            messageConfig.ResponseCode     = Convert.ToUInt16(result.GetRSC());
            messageConfig.Description      = result.GetDescription();
            messageConfig.SourceTimestamp  = GetTimestampInMillis();

            std::vector<jvs::cfg_key_e> vectorKeyWithNG = result.RetrieveKeyWithNG();
            for (auto& key : vectorKeyWithNG)
            {
                messageConfig.Config.emplace_back(Convert.ToString(key));
            }

            JSON json;
            std::string payload = json.Serialize(messageConfig);

            mqtt::Message message(mqtt::topic_e::JARVIS_RESPONSE, payload);
            Status ret = mqtt::cdo.Store(message);
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
         * @todo 설정 값 적용한 다음에 문제가 없으면 결과 값을 전송하도록 순서를 변경해야 합니다.
         */
        JSON json;
        jarvis_struct_t messageConfig;

        messageConfig.ResponseCode     = Convert.ToUInt16(result.GetRSC());
        messageConfig.Description      = result.GetDescription();
        messageConfig.SourceTimestamp  = GetTimestampInMillis();

        std::vector<jvs::cfg_key_e> vectorKeyWithNG = result.RetrieveKeyWithNG();

        for (auto& key : vectorKeyWithNG)
        {
           messageConfig.Config.emplace_back(Convert.ToString(key));
        }

        std::string payload = json.Serialize(messageConfig);
        mqtt::Message message(mqtt::topic_e::JARVIS_RESPONSE, payload);
        Status ret = mqtt::cdo.Store(message);
        if (ret != Status::Code::GOOD)
        {
             /**
             * @todo Store 실패시 falsh 메모리에 저장하는 방법
             * 
             */
            LOG_ERROR(logger, "FAIL TO SAVE MESSAGE IN CDO STORE");
        }

        {
            LOG_INFO(logger, "JARVIS configuration is valid. Proceed to save and apply");
            std::string jarvisPayload;
            RetrieveJarvisRequestPayload(&jarvisPayload);

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

            // 혹시라도 MFM RESPONSE를 서버로 못보내고 리셋이 될 수도 있나?
            while (mqtt::cdo.Count() > 0)
            {
                delay(1);
            }
        #if defined(MODLINK_T2) || defined(MODLINK_B)
            spear.Reset();
        #endif 
            ESP.restart();
        }
    }

    void Core::StartOTA(const std::string& payload)
    {
        // StartManualFirmwareUpdate(payload);
    }


    Core core;
}