/**
 * @file FetchConfigService.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MFM 서버로부터 최신 설정 정보를 가져오는 함수를 정의합니다.
 * 
 * @date 2025-01-24
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "IM/Custom/Constants.h"
#include "IM/Custom/FirmwareVersion/FirmwareVersion.h"
#include "IM/Custom/MacAddress/MacAddress.h"
#include "JARVIS/Config/Operation/Operation.h"
#include "Network/CatM1/CatM1.h"
#include "Network/Ethernet/Ethernet.h"
#include "Protocol/HTTP/IHTTP.h"
#include "ServiceSets/JarvisServiceSet/FetchConfigService.h"
#include "ServiceSets/NetworkServiceSet/RetrieveServiceNicService.h"
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
        INetwork* snic = RetrieveServiceNicService();
        if (snic->IsConnected() == false)
        {
            LOG_ERROR(logger, "FAILED TO FETCH DUE TO SNIC DISCONNECTION");
            return Status(Status::Code::BAD_NOT_CONNECTED);
        }
        
        std::pair<Status, size_t> mutex = snic->TakeMutex();
        if (mutex.first != Status::Code::GOOD)
        {
            return mutex.first;
        }

        http::RequestHeader header = createRequestHeader();
        const http::RequestParameter parameters = createRequestParameter();
        Status ret = httpClient->GET(mutex.second, header, parameters);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO REQUEST FOR FETCHING CONFIG");
            return ret;
        }
        
        std::string response;
        ret = httpClient->Retrieve(mutex.second, &response);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE THE RESPONSE");
            return ret;
        }

        LOG_INFO(logger, "Fetched JARVIS config from the server");
        snic->ReleaseMutex();

        File file = esp32FS.Open(FS_JARVIS_PATH_TMP, "w", true);
        if (file == false)
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
        
        const size_t writtenBytes = file.write(reinterpret_cast<const uint8_t*>(response.data()),
                                               response.length());

        if (writtenBytes != response.length())
        {
            return Status(Status::Code::BAD_DATA_LOST);
        }

        return ret;
    }
}
