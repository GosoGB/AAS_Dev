/**
 * @file Helper.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MQTT 프로토콜 전반에 걸쳐 공통으로 사용할 수 있는 함수를 선언합니다.
 * 
 * @date 2024-09-12
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include <errno.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Time/TimeUtils.h"
#include "IM/Custom/MacAddress/MacAddress.h"
#include "IM/Custom/FirmwareVersion/FirmwareVersion.h"
#include "Helper.h"



namespace muffin { namespace mqtt {
    
    const char* ConvertVersionToString(const version_e version)
    {
        switch (version)
        {
        case version_e::Ver_3_1_0:
            return "Ver.3.1.0";
        case version_e::Ver_3_1_1:
            return "Ver.3.1.1";
        default:
            ASSERT(false, "UNDEFINED OR UNSUPPORTED VERSION");
            return nullptr;
        }
    }

    Message GenerateWillMessage(const bool isConnected)
    {
        const uint8_t size = 64;
        char buffer[size] = {'\0'};

        snprintf(buffer, size, "%s,%llu,false,%s,%s",
            macAddress.GetEthernet(),
            GetTimestampInMillis(),
            FW_VERSION_ESP32.GetSemanticVersion(),
        #if defined(MODLINK_T2) || defined(MODLINK_B)
            FW_VERSION_MEGA2560.GetSemanticVersion()
        #else
            "N/A"
        #endif
        );

        return mqtt::Message(mqtt::topic_e::LAST_WILL, buffer);
    }

    version_e ConvertUInt32ToVersion(const uint32_t integer)
    {
        if (integer == static_cast<uint32_t>(version_e::Ver_3_1_0))
        {
            return static_cast<version_e>(integer);
        }
        else if (integer == static_cast<uint32_t>(version_e::Ver_3_1_1))
        {
            return static_cast<version_e>(integer);
        }
        else
        {
            LOG_ERROR(logger, "UNDEFINED VERSION NUMBER: %u", integer);
            return version_e::UNDEFINED;
        }
    }
}}