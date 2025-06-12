/**
 * @file Core.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief MUFFIN 프레임워크를 초기화 기능을 제공하는 클래스를 정의합니다.
 * 
 * @date 2025-01-20
 * @version 1.3.1
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
#include "Task/PubTask.h"
#include "Protocol/Modbus/ModbusMutex.h"
#include "Protocol/Modbus/ModbusTCP.h"
#include "Protocol/Modbus/ModbusRTU.h"
#include "Protocol/Melsec/MelsecClient.h"
#include "Protocol/Modbus/Include/ArduinoModbus/src/ModbusRTUClient.h"
#include "JARVIS/Config/Protocol/ModbusRTU.h"
#include "JARVIS/Config/Protocol/EthernetIP.h"
#include "JARVIS/Config/Operation/Operation.h"
#include "IM/Node/Include/TypeDefinitions.h"
#include "IM/AC/Alarm/DeprecableAlarm.h"
#include "Protocol/HTTP/CatHTTP/CatHTTP.h"
#include "Protocol/HTTP/Include/TypeDefinitions.h"
#include "Protocol/HTTP/IHTTP.h"
#include "Protocol/HTTP/LwipHTTP/LwipHTTP.h"

#include "ServiceSets/FirmwareUpdateServiceSet/FirmwareUpdateService.h"
#include "ServiceSets/FirmwareUpdateServiceSet/SendMessageService.h"
#include "ServiceSets/HttpServiceSet/StartHttpClientService.h"
#include "ServiceSets/JarvisServiceSet/ApplyOperationService.h"
#include "ServiceSets/JarvisServiceSet/FetchConfigService.h"
#include "ServiceSets/JarvisServiceSet/ValidateConfigService.h"
#include "ServiceSets/MqttServiceSet/StartMqttClientService.h"
#include "ServiceSets/MqttServiceSet/MqttTaskService.h"

#include "CLI/CLI.h"
#include "Network/CatM1/CatM1.h"
#include "Network/Ethernet/W5500/W5500.h"
#include "Network/Ethernet/LAN8720/LAN8720.h"
#include "Network/Ethernet/EthernetFactory.h"
#include "ServiceSets/JarvisServiceSet/FetchConfigService.h"




namespace muffin {

    std::vector<muffin::jvs::config::ModbusRTU> mConfigVectorMbRTU;
    std::vector<muffin::jvs::config::ModbusTCP> mConfigVectorMbTCP;
    std::vector<muffin::jvs::config::Melsec> mConfigVectorMelsec;
    std::vector<muffin::jvs::config::EthernetIP> mConfigVectorEthernetIP;
    
    uint32_t s_PollingIntervalInMillis = 1000;
    uint16_t s_PublishIntervalInSeconds = 60;

    void listDir(const char* dirname, const uint8_t levels)
    {
        File root = esp32FS.Open(dirname);
        if (!root)
        {
            Serial.println("FAILED TO OPEN ROOT DIRECTORY");
            return;
        }

        if (!root.isDirectory())
        {
            return;
        }
    
        File file = root.openNextFile();
        while (file)
        {
            if (file.isDirectory())
            {
                Serial.print("  DIR : ");
                Serial.println(file.path());
                if (levels)
                {
                    listDir(file.path(), levels - 1);
                }
            }
            else
            {
                Serial.print("    FILE: ");
                Serial.print(file.name());
                Serial.print("\tSIZE: ");
                Serial.println(file.size());
            }
            
            file = root.openNextFile();
        }
    }

    /**
     * @todo Ver.1.3 미만 펌웨어가 없다면 본 함수는 삭제할 예정임
     */
    void replaceDeprecatedPaths()
    {
        listDir("/", 2);
        LOG_DEBUG(logger, "Remained Flash Memory: %u Bytes", esp32FS.GetTotalBytes() - esp32FS.GetUsedBytes());

        if (esp32FS.DoesExist(DEPRECATED_INIT_FILE_PATH) == Status::Code::GOOD)
        {
            esp32FS.Rename(DEPRECATED_INIT_FILE_PATH, INIT_FILE_PATH);
            esp32FS.RemoveDirectory("/init");
        }

        bool hasJarvisPath = false;
        if (esp32FS.DoesExist(DEPRECATED_JARVIS_PATH) == Status::Code::GOOD)
        {
            esp32FS.Rename(DEPRECATED_JARVIS_PATH, JARVIS_PATH);
            hasJarvisPath = true;
        }
        
        if (esp32FS.DoesExist(DEPRECATED_JARVIS_PATH_FETCHED) == Status::Code::GOOD)
        {
            esp32FS.Rename(DEPRECATED_JARVIS_PATH_FETCHED, JARVIS_PATH_FETCHED);
            hasJarvisPath = true;
        }

        if (hasJarvisPath == true)
        {
            esp32FS.RemoveDirectory("/jarvis");
        }
        
        bool hasUpdatePath = false;
        if (esp32FS.DoesExist(DEPRECATED_OTA_REQUEST_PATH) == Status::Code::GOOD)
        {
            esp32FS.Rename(DEPRECATED_OTA_REQUEST_PATH, OTA_REQUEST_PATH);
            hasUpdatePath = true;
        }
        
        if (esp32FS.DoesExist(DEPRECATED_OTA_CHUNK_PATH_ESP32) == Status::Code::GOOD)
        {
            esp32FS.Rename(DEPRECATED_OTA_CHUNK_PATH_ESP32, OTA_CHUNK_PATH_ESP32);
            hasUpdatePath = true;
        }
        
        if (esp32FS.DoesExist(DEPRECATED_OTA_CHUNK_PATH_MEGA) == Status::Code::GOOD)
        {
            esp32FS.Rename(DEPRECATED_OTA_CHUNK_PATH_MEGA, OTA_CHUNK_PATH_MEGA);
            hasUpdatePath = true;
        }

        if (hasUpdatePath == true)
        {
            esp32FS.RemoveDirectory("/ota");
        }
        
        if (esp32FS.DoesExist(DEPRECATED_LWIP_HTTP_PATH) == Status::Code::GOOD)
        {
            esp32FS.Rename(DEPRECATED_LWIP_HTTP_PATH, LWIP_HTTP_PATH);
            esp32FS.RemoveDirectory("/http");
        }
        
        if (esp32FS.DoesExist(DEPRECATED_SPEAR_LINK1_PATH) == Status::Code::GOOD)
        {
            esp32FS.Remove(DEPRECATED_SPEAR_LINK1_PATH);
            esp32FS.RemoveDirectory("/spear/link1");
        }
        
        if (esp32FS.DoesExist(DEPRECATED_SPEAR_LINK2_PATH) == Status::Code::GOOD)
        {
            esp32FS.Remove(DEPRECATED_SPEAR_LINK2_PATH);
            esp32FS.RemoveDirectory("/spear/link2");
        }
        
        if (esp32FS.DoesExist(DEPRECATED_SPEAR_PRTCL_PATH) == Status::Code::GOOD)
        {
            esp32FS.Remove(DEPRECATED_SPEAR_PRTCL_PATH);
            esp32FS.RemoveDirectory("/spear/protocol");
            esp32FS.RemoveDirectory("/spear");
        }

        listDir("/", 2);
        LOG_DEBUG(logger, "Remained Flash Memory: %u Bytes", esp32FS.GetTotalBytes() - esp32FS.GetUsedBytes());
    }

    void Core::Init()
    {
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        settimeofday(&tv, NULL);
        
        logger.Init();
    #if defined(MT10) || defined(MB10) || defined(MT11)
        CommandLineInterface commandLineInterface;
        if (commandLineInterface.Init() == Status(Status::Code::GOOD))
        {
            init_cfg_t initConfig;
            Status ret = readInitConfig(&initConfig);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FATAL ERROR: FAILED TO LOAD INIT CONFIG FILE");
                std::abort();
            }
            initConfig.ReconfigCode = static_cast<uint8_t>(reconfiguration_code_e::CLI_USER_CONFIG_CHANGE);
            ret = writeInitConfig(initConfig);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO UPDATE JARVIS FLAG: %s", ret.c_str());
                std::abort();
            }
            
            delay(2000);
    #if !defined(MT11)
            spear.Reset();
    #endif
            esp_restart();
        }
    #endif 
    #if defined(MT11)
        ethernet = new(std::nothrow) W5500(w5500::if_e::EMBEDDED);
        ethernet->Init();
        char mac[13] = {'\0'};
        ethernet->GetMacAddress(mac);
        macAddress.SetMacAddress(mac);
    #endif    
        LOG_INFO(logger, "MAC Address: %s", macAddress.GetEthernet());
        LOG_INFO(logger, "Semantic Version: %s,  Version Code: %u", 
            FW_VERSION_ESP32.GetSemanticVersion(),
            FW_VERSION_ESP32.GetVersionCode());

    #if defined(MT10) || defined(MB10)
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

        Status ret = esp32FS.Begin(false);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FATAL ERROR: FAILED TO MOUNT ESP32 FILE SYSTEM");
            std::abort();
        }
        LOG_INFO(logger, "File system: %s", ret.c_str());
        replaceDeprecatedPaths();

        init_cfg_t initConfig;
        ret = readInitConfig(&initConfig);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FATAL ERROR: FAILED TO LOAD INIT CONFIG FILE");
            std::abort();
        }

        deviceStatus.SetReconfigurationCode(static_cast<reconfiguration_code_e>(initConfig.ReconfigCode));

        if (isResetByPanic() == true)
        {
            LOG_INFO(logger,"PanicResetCount : %d ",initConfig.PanicResetCount);
            if (++initConfig.PanicResetCount == MAX_RETRY_COUNT)
            {
                ret = esp32FS.Remove(JARVIS_PATH);
                deviceStatus.SetReconfigurationCode(reconfiguration_code_e::FIRMWARE_DEFAULT_RECOVERY);
                initConfig.ReconfigCode = static_cast<uint8_t>(reconfiguration_code_e::FIRMWARE_DEFAULT_RECOVERY);
            // #if !defined(DEBUG)
            // ---------------------------------------------------------------------
            // |  @todo #1  JARVIS 설정 초기화 사유를 <INIT_FILE_PATH>에 저장해야 함  |
            // |  @todo #2  mfm/status 토픽에 설정값에 변화가 있었다고 알려줘야 함     |
            // ---------------------------------------------------------------------
            // #endif
                if (ret == Status::Code::BAD_DEVICE_FAILURE)
                {
                    LOG_ERROR(logger, "FATAL ERROR: FAILED TO REMOVE JARVIS CONFIG");
                    std::abort();
                }
            }

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
        else
        {
            if(initConfig.PanicResetCount != 0)
            {
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
#if defined(MT10) || defined(MB10) || defined(MT11)
        if (esp32FS.DoesExist(SERVICE_URL_PATH) == Status::Code::BAD_NOT_FOUND)
        {
            LOG_INFO(logger, "No Service URL Config found");
            
            ret = createDefaultServiceUrl();
            if (ret == Status::Code::BAD_DEVICE_FAILURE)
            {
                LOG_ERROR(logger, "FAILED TO CREATE DEAULT SERVICE URL CONFIG");
                std::abort();
            }

            LOG_INFO(logger, "Created default Service URL config");
        }
        
        ret = loadServiceUrlConfig();
        if (ret == Status::Code::BAD_DATA_LOST)
        {
            LOG_ERROR(logger, "FAILED TO LOAD SERVICE URL CONFIG: %s", ret.c_str());
            std::abort();
        }
#endif
        ret = loadJarvisConfig();
        if (ret == Status::Code::BAD_DATA_LOST)
        {
            LOG_ERROR(logger, "FAILED TO LOAD JARVIS CONFIG: %s", ret.c_str());
            std::abort();
        }
        else if (ret == Status::Code::UNCERTAIN)
        {
            LOG_WARNING(logger, "JARVIS CONFIG MIGHT NOT WORK");

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

            if (initConfig.HasPendingUpdate == true)
            {
                break;
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

            ret = StartMqttTaskService(initConfig, &muffin::Core::writeInitConfig);
            if (ret != Status::Code::GOOD)
            {
                goto RETRY;
            }

        RETRY:
            vTaskDelay(SECOND_IN_MILLIS / portTICK_PERIOD_MS);
            
        } while (ret != Status::Code::GOOD);


        if (initConfig.HasPendingJARVIS == true)
        {
            LOG_DEBUG(logger, "Remained Heap: %u Bytes, before unloading", ESP.getFreeHeap());
            unloadJarvisConfig();
            LOG_DEBUG(logger, "Remained Heap: %u Bytes, after unloading", ESP.getFreeHeap());

            initConfig.HasPendingJARVIS = false;
            initConfig.ReconfigCode = static_cast<uint8_t>(reconfiguration_code_e::JARVIS_USER_CONFIG_CHANGE);
            ret = writeInitConfig(initConfig);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO UPDATE JARVIS FLAG: %s", ret.c_str());
                std::abort();
            }

            ret = FetchConfigService();
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO FETCH JARVIS: %s", ret.c_str());
                esp32FS.Remove(JARVIS_PATH_FETCHED);
                std::abort();
            }

            jarvis_struct_t response;
            ret = ValidateConfigService(&response);
            if (ret == Status::Code::GOOD)
            {
                LOG_INFO(logger, "Fetched JARVIS config is valid");
                esp32FS.Remove(JARVIS_PATH);
                esp32FS.Rename(JARVIS_PATH_FETCHED, JARVIS_PATH);
            }
            else
            {
                LOG_INFO(logger, "Fetched JARVIS config is invalid");
                esp32FS.Remove(JARVIS_PATH_FETCHED);
            }

            PublishResponseJARVIS(response);
            /**
             * @todo 발행 실패 시 무한루프에 빠지게 되는 로직을 수정할 것
             *       타임아웃을 걸어두면 좋을 거 같음
             */
            while (mqtt::cdo.Count() != 0)
            {
                delay(1);
            }
            
        #if defined(MT10) || defined(MB10)
            spear.Reset();
        #endif 
            esp_restart();
        }
        
        if (initConfig.HasPendingUpdate == true)
        {
            LOG_DEBUG(logger, "Remained Heap: %u Bytes, before unloading", ESP.getFreeHeap());
            unloadJarvisConfig();
            LOG_DEBUG(logger, "Remained Heap: %u Bytes, after unloading", ESP.getFreeHeap());

            initConfig.HasPendingUpdate = false;
            ret = writeInitConfig(initConfig);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO UPDATE JARVIS FLAG: %s", ret.c_str());
                std::abort();
            }

            ret = FirmwareUpdateService();
            if (ret == Status::Code::GOOD)
            {
                LOG_INFO(logger, "Updated successfully");
            }
            else
            {
                LOG_ERROR(logger, "FAILED TO UPDATE: %s", ret.c_str());
            }
            
        #if defined(MT10) || defined(MB10)
            spear.Reset();
        #endif 
            esp_restart();
        }
        
        esp32FS.Remove(LWIP_HTTP_PATH);
        
        ApplyJarvisTask();
        PublishFirmwareStatusMessageService();
        PublishStatusEventMessageService(&initConfig);
    }

    Status Core::readInitConfig(init_cfg_t* output)
    {
        ASSERT((output != nullptr), "OUTPUT PARAMETER CANNOT BE NULL");

        Status ret = esp32FS.DoesExist(INIT_FILE_PATH);
        LOG_INFO(logger, "Init config file: %s", ret.c_str());

        if (ret == Status::Code::BAD_NOT_FOUND)
        {
            output->PanicResetCount   = 0;
            output->HasPendingJARVIS  = 0;
            output->HasPendingUpdate  = 0;
            output->ReconfigCode      = 0;

            ret = writeInitConfig(*output);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO WRITE DEFAULT INIT CONFIG");
                esp32FS.Remove(INIT_FILE_PATH);
                return ret;
            }

            LOG_INFO(logger, "Created DEFAULT init config file");
            return Status(Status::Code::GOOD);
        }
        
        File file = esp32FS.Open(INIT_FILE_PATH, "r", "false");
        if (file == false)
        {
            LOG_ERROR(logger, "FATAL ERROR: FAILED TO OPEN INIT CONFIG FILE");
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
        
        const size_t size = file.size() + 1;
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

        if (output->HasPendingJARVIS == false)
        {
            if (esp32FS.DoesExist(JARVIS_PATH_FETCHED) == Status::Code::GOOD)
            {
                esp32FS.Remove(JARVIS_PATH_FETCHED);
            }
        }
        
        if (output->HasPendingUpdate == false)
        {
            if (esp32FS.DoesExist(OTA_CHUNK_PATH_MEGA) == Status::Code::GOOD)
            {
                esp32FS.Remove(OTA_CHUNK_PATH_MEGA);
            }

            if (esp32FS.DoesExist(OTA_CHUNK_PATH_ESP32) == Status::Code::GOOD)
            {
                esp32FS.Remove(OTA_CHUNK_PATH_ESP32);
            }

            if (esp32FS.DoesExist(OTA_REQUEST_PATH) == Status::Code::GOOD)
            {
                esp32FS.Remove(OTA_REQUEST_PATH);
            }
        }

        return ret;
    }

    Status Core::writeInitConfig(const init_cfg_t& config)
    {
        const uint8_t size = 20;
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

        file.write(reinterpret_cast<uint8_t*>(buffer), sizeof(buffer));
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
        file.close();

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
        doc["ver"] = "v5";

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
        cnt["mc"].to<JsonArray>();
        cnt["eip"].to<JsonArray>();

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

        const size_t size = measureJson(doc) + 1;
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
#if defined(MT10) || defined(MB10) || defined(MT11)
    Status Core::createDefaultServiceUrl()
    {
        File file = esp32FS.Open(SERVICE_URL_PATH, "w", true);
        if (file == false)
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }

        JsonDocument doc;
        doc["ver"] = "v5";
        JsonObject mqtt = doc["mqtt"].to<JsonObject>();

        mqtt["host"] = "mmm.broker.edgecross.ai";
        mqtt["port"] = 8883;
        mqtt["scheme"] = 2;
        mqtt["id"]   = "edgeaiot";
        mqtt["pw"]   = "!edge1@1159";
        mqtt["checkCert"] = true;


        
        JsonObject mfm = doc["mfm"].to<JsonObject>();
        mfm["host"] = "api.mfm.edgecross.ai";
        mfm["port"] = 443;
        mfm["scheme"] = 2;
        mfm["checkCert"] = true;

        doc["ntp"] = "time.google.com";

        const size_t size = measureJson(doc) + 1;
        char buffer[size] = {'\0'};
        serializeJson(doc, buffer, size);
        doc.clear();

        file.write(reinterpret_cast<uint8_t*>(buffer), size);
        file.flush();
        file.close();

        char readback[size] = {'\0'};
        file = esp32FS.Open(SERVICE_URL_PATH);
        file.readBytes(readback, size);
        file.close();

        if (strcmp(buffer, readback) != 0)
        {
            esp32FS.Remove(SERVICE_URL_PATH);
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }

        return Status(Status::Code::GOOD);
    }

    Status Core::loadServiceUrlConfig()
    {
        ASSERT((esp32FS.DoesExist(SERVICE_URL_PATH) == Status::Code::GOOD), "JARVIS CONFIG MUST EXIST");
        File file = esp32FS.Open(SERVICE_URL_PATH, "r", false);
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
            esp32FS.Remove(SERVICE_URL_PATH);
            return ret;
        }

        if (doc.containsKey("mqtt"))
        {
            JsonObject mqtt = doc["mqtt"].as<JsonObject>();

            bool isValid = true;
            isValid &= mqtt.containsKey("port");
            isValid &= mqtt.containsKey("host");
            isValid &= mqtt.containsKey("scheme");
            isValid &= mqtt.containsKey("id");
            isValid &= mqtt.containsKey("pw");
            isValid &= mqtt.containsKey("checkCert");
            
            if (isValid != true)
            {
                LOG_ERROR(logger,"[MQTT BROKER URL] MANDATORY KEY CANNOT BE MISSING");
                return Status(Status::Code::BAD_INVALID_ARGUMENT);
            }

            isValid &= mqtt["port"].isNull() == false;
            isValid &= mqtt["host"].isNull() == false;      
            isValid &= mqtt["scheme"].isNull()  == false;
            isValid &= mqtt["id"].isNull()  == false;
            isValid &= mqtt["pw"].isNull()  == false;
            isValid &= mqtt["checkCert"].isNull()  == false;
            isValid &= mqtt["port"].is<uint16_t>();
            isValid &= mqtt["host"].is<std::string>();
            isValid &= mqtt["scheme"].is<uint8_t>();
            isValid &= mqtt["id"].is<std::string>();
            isValid &= mqtt["pw"].is<std::string>();
            isValid &= mqtt["checkCert"].is<bool>();
            
            if (isValid != true)
            {
                LOG_ERROR(logger,"[MQTT BROKER URL] MANDATORY KEY'S VALUE CANNOT BE NULL");
                return Status(Status::Code::BAD_INVALID_ARGUMENT);
            }


            LOG_DEBUG(logger,"mqtt broker host: %s",mqtt["host"].as<std::string>().c_str());
            LOG_DEBUG(logger,"mqtt broker port: %u",mqtt["port"].as<uint16_t>());
            LOG_DEBUG(logger,"mqtt broker user name: %s",mqtt["id"].as<std::string>().c_str());
            LOG_DEBUG(logger,"mqtt broker password: %s",mqtt["pw"].as<std::string>().c_str());
            LOG_DEBUG(logger,"mqtt broker scheme: %s",mqtt["scheme"].as<uint8_t>() == 1 ? "MQTT" : "MQTTS");
            LOG_DEBUG(logger,"mqtt broker validate certificate: %s",mqtt["checkCert"].as<bool>() == true ? "Enabled" : "Disabled");


            brokerInfo.SetHost(mqtt["host"].as<std::string>());
            brokerInfo.SetPort(mqtt["port"].as<uint16_t>());
            brokerInfo.SetUsername(mqtt["id"].as<std::string>());
            brokerInfo.SetPassword(mqtt["pw"].as<std::string>());
            brokerInfo.EnableSSL(mqtt["scheme"].as<uint8_t>() == 1 ? false : true);
            brokerInfo.EnableValidateCert(mqtt["checkCert"].as<bool>());
        }

        if (doc.containsKey("ntp"))
        {
            bool isValid = true;
            isValid &= doc["ntp"].isNull() == false;
            isValid &= doc["ntp"].is<std::string>();

            if (isValid != true)
            {
                LOG_ERROR(logger,"[NTP SERVER URL] MANDATORY KEY'S VALUE CANNOT BE NULL");
                ret = Status::Code::BAD_INVALID_ARGUMENT;
            }

            ntpServer = doc["ntp"].as<std::string>();
            LOG_DEBUG(logger, "ntpServer : %s",ntpServer.c_str());
        }

        if (doc.containsKey("mfm"))
        {
            JsonObject mfm = doc["mfm"].as<JsonObject>();

            bool isValid = true;
            isValid &= mfm.containsKey("port");
            isValid &= mfm.containsKey("host");
            isValid &= mfm.containsKey("scheme");
            isValid &= mfm.containsKey("checkCert");

            if (isValid != true)
            {
                LOG_ERROR(logger,"[MFM SERVER] MANDATORY KEY CANNOT BE MISSING");
                ret = Status::Code::BAD_INVALID_ARGUMENT;
            }
            
            isValid &= mfm["port"].isNull() == false;
            isValid &= mfm["port"].is<uint16_t>();
            isValid &= mfm["host"].isNull() == false;
            isValid &= mfm["host"].is<std::string>();
            isValid &= mfm["scheme"].isNull() == false;
            isValid &= mfm["scheme"].is<uint16_t>();
            isValid &= mfm["checkCert"].isNull() == false;
            isValid &= mfm["checkCert"].is<bool>();

            if (isValid != true)
            {
                LOG_ERROR(logger,"[MFM SERVER] MANDATORY KEY'S VALUE CANNOT BE NULL");
                ret = Status::Code::BAD_INVALID_ARGUMENT;
            }

            mfmPort = mfm["port"].as<uint16_t>();
            mfmHost = mfm["host"].as<std::string>();
            mfmValidateCert = mfm["checkCert"].as<bool>();

            mfmScheme = static_cast<http_scheme_e>(mfm["scheme"].as<uint16_t>());
            LOG_DEBUG(logger, "mfmHost : %s, mfmPost : %u, mfmScheme : %s, mfmValidateCert : %s",
                mfmHost.c_str(), mfmPort, mfmScheme == http_scheme_e::HTTP ? "HTTP" : "HTTPS", 
                mfmValidateCert == true ? "Enabled" : "Disabled");
        }
        
        return Status(Status::Code::GOOD);
    }   
#endif

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
        
        jarvis = new(std::nothrow) JARVIS();
        if (jarvis == nullptr)
        {
            ret = Status::Code::BAD_OUT_OF_MEMORY;
            return ret;
        }

        jvs::ValidationResult result = jarvis->Validate(doc);
        doc.clear();
        if (result.GetRSC() >= jvs::rsc_e::BAD)
        {
            ret = Status::Code::BAD_DATA_LOST;
        }
        if (result.GetRSC() >= jvs::rsc_e::UNCERTAIN)
        {
            ret = Status::Code::UNCERTAIN;
        }

        return ret;
    }

    void Core::unloadJarvisConfig()
    {
        for (auto it = jarvis->begin(); it != jarvis->end(); ++it)
        {
            if (it->second.size() == 0)
            {
                continue;
            }

            if (it->first == jvs::cfg_key_e::NODE)
            {
                continue;
            }

            for (auto _it = it->second.begin(); _it != std::prev(it->second.end()); ++_it)
            {
                auto item = _it.operator*();
                delete item;
            }

            it->second.clear();
        }

        jarvis->Clear();
    }

    void Core::PublishStatusEventMessageService(init_cfg_t* output)
    {
        const std::string payload =  deviceStatus.ToStringEvent();
        mqtt::Message message(mqtt::topic_e::JARVIS_STATUS, payload);
        mqtt::cdo.Store(message);

        if (output->ReconfigCode != static_cast<uint8_t>(reconfiguration_code_e::NONE))
        {
            output->HasPendingJARVIS = false;
            output->HasPendingUpdate = false;
            output->ReconfigCode = static_cast<uint8_t>(reconfiguration_code_e::NONE);
            deviceStatus.SetReconfigurationCode(reconfiguration_code_e::NONE);
            Status ret = writeInitConfig(*output);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO UPDATE JARVIS FLAG: %s", ret.c_str());
                std::abort();
            }
        }
    }

    Core core;
}