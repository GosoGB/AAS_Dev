              /**
 * @file FetchFirmwareInfo.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief API 서버로부터 펌웨어 정보를 가져오는 서비스를 정의합니다.
 * 
 * @date 2025-01-16
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "FetchFirmwareInfo.h"
#include "IM/Custom/FirmwareVersion/FirmwareVersion.h"
#include "IM/Custom/MacAddress/MacAddress.h"
#include "Protocol/HTTP/Include/RequestHeader.h"
#include "Protocol/HTTP/Include/RequestParameter.h"
#include "Protocol/HTTP/Include/TypeDefinitions.h"
#include "Protocol/HTTP/IHTTP.h"
#include "ServiceSets/NetworkServiceSet/RetrieveServiceNicService.h"



namespace muffin {

    Status FetchFirmwareInfo(const ota::fw_info_t& info, std::string* output)
    {
        INetwork* snic = RetrieveServiceNicService();
        std::pair<Status, size_t> mutex = snic->TakeMutex();
        if (mutex.first != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO TAKE MUTEX");
            return mutex.first;
        }
        
        #if defined(MODLINK_L)
            char userAgent[32] = "MODLINK-L/";
        #elif defined(MT10)
            char userAgent[32] = "MT10/";
        #elif defined(MT11)
            char userAgent[32] = "MT11/";
        #endif

        http::RequestHeader header(
            rest_method_e::GET,
            info.Head.API.Scheme,
            info.Head.API.Host,
            info.Head.API.Port,
            "/firmware/file/v1.1/version/manual",
            strcat(userAgent, FW_VERSION_ESP32.GetSemanticVersion())
        );

        http::RequestParameter parameters;
        parameters.Add("mac", macAddress.GetEthernet());
        parameters.Add("otaId", std::to_string(info.Head.ID));

        Status ret = httpClient->GET(mutex.second, header, parameters);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO FETCH: %s", ret.c_str());
            snic->ReleaseMutex();
            return ret;
        }

        ret = httpClient->Retrieve(mutex.second, output);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE PAYLOAD FROM MODEM: %s", ret.c_str());
            snic->ReleaseMutex();
            return ret;
        }

        LOG_INFO(logger, "Firmware Version Info\n%s", output->c_str());
        snic->ReleaseMutex();
        return ret;
    }
}