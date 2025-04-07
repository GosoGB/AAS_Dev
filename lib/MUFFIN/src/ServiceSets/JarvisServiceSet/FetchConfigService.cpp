/**
 * @file FetchConfigService.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MFM 서버로부터 최신 설정 정보를 가져오는 함수를 정의합니다.
 * 
 * @date 2025-01-24
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include "Common/Assert.h"
#include "Common/Convert/ConvertClass.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "DataFormat/JSON/JSON.h"
#include "IM/Custom/Constants.h"
#include "IM/Custom/FirmwareVersion/FirmwareVersion.h"
#include "IM/Custom/MacAddress/MacAddress.h"
#include "JARVIS/Config/Operation/Operation.h"
#include "Network/CatM1/CatM1.h"
#include "Network/Ethernet/Ethernet.h"
#include "Protocol/HTTP/IHTTP.h"
#include "ServiceSets/JarvisServiceSet/FetchConfigService.h"
#include "ServiceSets/NetworkServiceSet/RetrieveServiceNicService.h"
#include "ServiceSets/MqttServiceSet/MqttTaskService.h"
#include "Storage/ESP32FS/ESP32FS.h"



namespace muffin {

    http::RequestHeader createRequestHeader()
    {
        char buffer[32] = {'\0'};
    #if defined(MODLINK_L)
        snprintf(buffer, sizeof(buffer), "MODLINK-L/%s", FW_VERSION_ESP32.GetSemanticVersion());
    #elif defined(MODLINK_T2)
        snprintf(buffer, sizeof(buffer), "MODLINK-T2/%s", FW_VERSION_ESP32.GetSemanticVersion());
    #endif

        http::RequestHeader header(
            rest_method_e::GET, 
            http_scheme_e::HTTPS, 
        #if defined(DEBUG)
            "api.mfm.edgecross.dev", 
        #else
            "api.mfm.edgecross.ai",
        #endif
            443, 
            "/api/mfm/device/write", 
            buffer);

        return header;
    }

    http::RequestParameter createRequestParameter()
    {
        http::RequestParameter parameters;
        parameters.Add("mac", macAddress.GetEthernet());
        return parameters;
    }

    Status FetchConfigService()
    {
        jarvis_struct_t response;
        response.SourceTimestamp = GetTimestampInMillis();
        
        INetwork* snic = RetrieveServiceNicService();
        if (snic->IsConnected() == false)
        {
            response.ResponseCode = Convert.ToUInt16(jvs::rsc_e::BAD_COMMUNICATION);
            response.Description = "FAILED TO FETCH DUE TO SERVICE NETWORK HAD BEEN DISCONNECTED";
            PublishResponseJARVIS(response);
            return Status(Status::Code::BAD_NOT_CONNECTED);
        }
        
        std::pair<Status, size_t> mutex = snic->TakeMutex();
        if (mutex.first != Status::Code::GOOD)
        {
            response.ResponseCode = Convert.ToUInt16(jvs::rsc_e::BAD_TEMPORARY_UNAVAILABLE);
            response.Description = "FAILED TO FETCH: SERVICE NETWORK BUSY";
            PublishResponseJARVIS(response);
            return mutex.first;
        }

        // http::RequestHeader header = createRequestHeader();
        // const http::RequestParameter parameters = createRequestParameter();
        // Status ret = httpClient->GET(mutex.second, header, parameters);
        // if (ret != Status::Code::GOOD)
        // {
        //     response.ResponseCode = Convert.ToUInt16(jvs::rsc_e::BAD_COMMUNICATION);
        //     response.Description = "FAILED TO FETCH: ";
        //     response.Description += ret.c_str();
        //     snic->ReleaseMutex();
        //     PublishResponseJARVIS(response);
        //     return ret;
        // }
        
        // std::string payload;
        
        // ret = httpClient->Retrieve(mutex.second, &payload);
        // if (ret != Status::Code::GOOD)
        // {
        //     response.ResponseCode = Convert.ToUInt16(jvs::rsc_e::BAD_INTERNAL_ERROR);
        //     response.Description = "FAILED TO FETCH: RETRIEVE ERROR";
        //     snic->ReleaseMutex();
        //     PublishResponseJARVIS(response);
        //     return ret;
        // }
        std::string payload = R"({"ver":"v4","cnt":{"melsec":[{"ip":"10.11.12.116","prt":3000,"ps":0,"df":0,"nodes":["n001","n002"]}],"rs232":[],"rs485":[],"wifi":[],"eth":[{"dhcp":true,"ip":null,"snm":null,"gtw":null,"dns1":null,"dns2":null}],"catm1":[{"md":"LM5","ctry":"KR"}],"mbrtu":[],"mbtcp":[],"op":[{"snic":"lte","exp":false,"intvPoll":1,"intvSrv":60,"rst":false}],"node":[{"id":"n001","adtp":0,"addr":3003,"area":9,"bit":null,"qty":null,"scl":null,"ofst":null,"dt":[0],"ord":null,"fmt":null,"uid":"P001","event":true},{"id":"n002","adtp":0,"addr":1000,"area":14,"bit":null,"qty":1,"scl":-2,"ofst":null,"dt":[3],"ord":null,"fmt":null,"uid":"P002","event":true}],"alarm":[],"optime":[],"prod":[]}})";
          
        LOG_INFO(logger,"payload : %s",payload.c_str());

        File file = esp32FS.Open(JARVIS_PATH_FETCHED, "w", true);
        if (file == false)
        {
            response.ResponseCode = Convert.ToUInt16(jvs::rsc_e::BAD_HARDWARE_FAILURE);
            response.Description = "FAILED TO FETCH: MEMORY I/O ERROR";
            snic->ReleaseMutex();
            PublishResponseJARVIS(response);
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
        
        const size_t writtenBytes = file.write(reinterpret_cast<const uint8_t*>(payload.data()),
                                               payload.length());
        file.flush();
        file.close();

        if (writtenBytes != payload.length())
        {
            response.ResponseCode = Convert.ToUInt16(jvs::rsc_e::BAD_HARDWARE_FAILURE);
            response.Description = "FAILED TO FETCH: DATA LOST DUE TO MEMORY I/O ERROR";
            snic->ReleaseMutex();
            PublishResponseJARVIS(response);
            return Status(Status::Code::BAD_DATA_LOST);
        }
        snic->ReleaseMutex();

        // ret = Status::Code::GOOD;
        return Status(Status::Code::GOOD);
    }
}