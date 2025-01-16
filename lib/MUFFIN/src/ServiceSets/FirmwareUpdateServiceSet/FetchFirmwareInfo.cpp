              /**
 * @file FetchFirmwareInfo.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief API 서버로부터 펌웨어 정보를 가져오는 서비스를 정의합니다.
 * 
 * @date 2025-01-16
 * @version 1.2.2
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
#include "OTA/Include/TypeDefinitions.h"



namespace muffin {

    ota::url_t ApiURL =
    {
    #if defined(DEBUG)
        .Host = "112.171.127.186",
        .Port = 8123,
        .Scheme = http_scheme_e::HTTP
    #else
        .Host = "api.fota.edgecross.ai",
        .Port = 443,
        .Scheme = http_scheme_e::HTTPS
    #endif
    };

    Status FetchFirmwareInfo(http::IHTTP* interface, std::string* output)
    {
        http::RequestHeader header(
            rest_method_e::GET,
            ApiURL.Scheme,
            ApiURL.Host,
            ApiURL.Port,
            "/firmware/file/version/release",
        #if defined(MODLINK_L)
            strcpy("MODLINK-L/", FW_VERSION_ESP32.GetSemanticVersion())
        #elif defined(MODLINK_T2)
            strcpy("MODLINK-T2/", FW_VERSION_ESP32.GetSemanticVersion())
        #endif
        );

        http::RequestParameter parameters;
        parameters.Add("mac", macAddress.GetEthernet());
        
        INetwork* nic = interface->RetrieveNIC();
        std::pair<Status, size_t> mutex = nic->TakeMutex();
        if (mutex.first != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO TAKE MUTEX");
            return mutex.first;
        }
        
        Status ret = interface->GET(mutex.second, header, parameters);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO FETCH: %s", ret.c_str());
            nic->ReleaseMutex();
            return ret;
        }

        ret = interface->Retrieve(mutex.second, output);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO RETRIEVE PAYLOAD FROM MODEM: %s", ret.c_str());
            nic->ReleaseMutex();
            return ret;
        }

        LOG_INFO(logger, "Firmware Version Info\n%s", output->c_str());
        nic->ReleaseMutex();
        return ret;
    }
}