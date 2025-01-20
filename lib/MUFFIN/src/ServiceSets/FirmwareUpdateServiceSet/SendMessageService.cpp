// /**
//  * @file SendMessageService.cpp
//  * @author Lee, Sang-jin (lsj31@edgecross.ai)
//  * @author Kim, Joo-sung (joosung5732@edgecross.ai)
//  * 
//  * @brief 펌웨어 상태를 발행하는 서비스를 정의합니다.
//  * 
//  * @date 2025-01-20
//  * @version 1.2.2
//  * 
//  * @copyright Copyright (c) Edgecross Inc. 2024-2025
//  */




// #include "Common/Assert.h"
// #include "Common/Logger/Logger.h"
// #include "DataFormat/JSON/JSON.h"
// #include "IM/Custom/FirmwareVersion/FirmwareVersion.h"
// #include "IM/Custom/MacAddress/MacAddress.h"
// #include "OTA/Include/TypeDefinitions.h"
// #include "Protocol/MQTT/CDO.h"
// #include "Protocol/HTTP/IHTTP.h"
// #include "SendMessageService.h"



// namespace muffin {

//     Status PublishFirmwareStatusMessageService()
//     {
//         fota_status_t status;
//         status.VersionCodeMcu1  = FW_VERSION_ESP32.GetVersionCode();
//         status.VersionMcu1      = FW_VERSION_ESP32.GetSemanticVersion();
//     #if defined(MODLINK_T2)
//         status.VersionCodeMcu2  = FW_VERSION_MEGA2560.GetVersionCode();
//         status.VersionMcu2      = FW_VERSION_MEGA2560.GetSemanticVersion();
//     #endif

//         /**
//          * @todo ATmega2560에 대한 정보도 전송할 수 있도록 코드 수정이 필요합니다.
//          */
//         constexpr size_t size = 256;
//         char buffer[size] = {'\0'};

//         JSON json;
//         json.Serialize(status, size, buffer);
//         LOG_INFO(logger, "\n Topic: fota/status \n Payload: %s", buffer);

//         mqtt::Message message(mqtt::topic_e::FOTA_STATUS, buffer);
//         mqtt::CDO& cdo = mqtt::CDO::GetInstance();
//         Status ret = cdo.Store(message);
//         if (ret != Status::Code::GOOD)
//         {
//             /**
//              * @todo Store 실패시 flash 메모리에 저장하는 것과 같은 방법을 
//              *       적용하여 실패에 강건하도록 코드를 작성해야 합니다.
//              */
//             LOG_ERROR(logger, "FAIL TO STORE: %s", ret.c_str());
//         }
        
//         return ret;
//     }
    
//     Status PostDownloadResult(const ota::fw_info_t& info, const char* result)
//     {
//         INetwork* nic = http::client->RetrieveNIC();
//         const std::pair<Status, size_t> mutex = nic->TakeMutex();
//         if (mutex.first.ToCode() != Status::Code::GOOD)
//         {
//             LOG_ERROR(logger, "FAILED TO TAKE MUTEX");
//             return mutex.first;
//         }

//         #if defined(MODLINK_L)
//             char userAgent[32] = "MODLINK-L/";
//         #elif defined(MODLINK_T2)
//             char userAgent[32] = "MODLINK-T2/";
//         #endif

//         http::RequestHeader header(
//             rest_method_e::POST,
//             http_scheme_e::HTTPS,
//             info.Head.DownloadURL.Host,
//             info.Head.DownloadURL.Port,
//             "/firmware/file/download",
//             strcat(userAgent, FW_VERSION_ESP32.GetSemanticVersion())
//         );
        
//         http::RequestBody body("application/x-www-form-urlencoded");
//         body.AddProperty("mac", macAddress.GetEthernet());
//         body.AddProperty("otaId", std::to_string(info.Head.ID));
//         ASSERT((info.Head.MCU == ota::mcu_e::MCU1 || info.Head.MCU == ota::mcu_e::MCU2), "INVALID MCU TYPE");
//         if (info.Head.MCU == ota::mcu_e::MCU1)
//         {
//             body.AddProperty("mcu1.vc",        std::to_string(info.Head.VersionCode));
//             body.AddProperty("mcu1.version",   info.Head.SemanticVersion);
//             body.AddProperty("mcu1.fileNo",    std::to_string(info.Chunk.IndexArray[info.Chunk.DownloadIDX]));
//             body.AddProperty("mcu1.filepath",  info.Chunk.PathArray[info.Chunk.DownloadIDX]);
//             body.AddProperty("mcu1.result",    result);
//             body.AddProperty("mcu2.vc",        "");
//             body.AddProperty("mcu2.version",   "");
//             body.AddProperty("mcu2.fileNo",    "");
//             body.AddProperty("mcu2.filepath",  "");
//             body.AddProperty("mcu2.result",    "");
//         }
//         else
//         {
//             body.AddProperty("mcu1.vc",        "");
//             body.AddProperty("mcu1.version",   "");
//             body.AddProperty("mcu1.fileNo",    "");
//             body.AddProperty("mcu1.filepath",  "");
//             body.AddProperty("mcu1.result",    "");
//             body.AddProperty("mcu2.vc",        std::to_string(info.Head.VersionCode));
//             body.AddProperty("mcu2.version",   info.Head.SemanticVersion);
//             body.AddProperty("mcu2.fileNo",    std::to_string(info.Chunk.IndexArray[info.Chunk.DownloadIDX]));
//             body.AddProperty("mcu2.filepath",  info.Chunk.PathArray[info.Chunk.DownloadIDX]);
//             body.AddProperty("mcu2.result",    result);
//         }

//         Status ret = http::client->POST(mutex.second, header, body);
//         nic->ReleaseMutex();

//         if (ret != Status::Code::GOOD)
//         {
//             LOG_ERROR(logger, "FAILED TO POST: %s", ret.c_str());
//         }
//         else
//         {
//             LOG_INFO(logger, "POST succeded");
//         }
//         return ret;
//     }
    
//     Status PosthUpdateResult()
//     {
//         CatM1& catM1 = CatM1::GetInstance();
//         const auto mutexHandle = catM1.TakeMutex();
//         if (mutexHandle.first.ToCode() != Status::Code::GOOD)
//         {
//             LOG_ERROR(logger, "UNAVAILABLE DUE TO TOO MANY OPERATIONS. TRY AGAIN LATER");
//             return false;
//         }

//         DeviceStatus& deviceStatus = DeviceStatus::GetInstance();
//         fw_vsn_t version = deviceStatus.GetFirmwareVersion(mcu);

//         http::CatHTTP& catHttp = http::CatHTTP::GetInstance();
//         http::RequestHeader header(
//             rest_method_e::POST, 
//             http_scheme_e::HTTPS, 
//             DownloadURL.Host, 
//             DownloadURL.Port, 
//             "/firmware/file/download/finish", 
//         #if defined(MODLINK_L)
//             "MODLINK-L/" + version.Semantic
//         #elif defined(MODLINK_T2)
//             "MODLINK-T2/" + version.Semantic
//         #endif
//         );
    

//         http::RequestParameter parameters;
//         parameters.Add("mac", macAddress.GetEthernet());
//         parameters.Add("otaId", std::to_string(info.OtaID));
//         if (mcu == mcu_type_e::MCU_ESP32)
//         {
//             parameters.Add("mcu1.vc", std::to_string(info.mcu1.VersionCode));
//             parameters.Add("mcu1.version", info.mcu1.FirmwareVersion);
//             parameters.Add("mcu1.result", result);
//         }
//         else
//         {
//             parameters.Add("mcu2.vc", std::to_string(info.mcu2.VersionCode));
//             parameters.Add("mcu2.version", info.mcu2.FirmwareVersion);
//             parameters.Add("mcu2.result", result);
//         }

//         Status ret = catHttp.POST(mutexHandle.second, header, parameters);
//         if (ret != Status::Code::GOOD)
//         {
//             LOG_ERROR(logger, "FAILED TO FETCH FOTA FROM SERVER: %s", ret.c_str());
//             catM1.ReleaseMutex();
//             return false;
//         }
        
//         catM1.ReleaseMutex();
//         return true;
//     }
// }