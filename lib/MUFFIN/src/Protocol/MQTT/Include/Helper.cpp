/**
 * @file Helper.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MQTT 프로토콜 전반에 걸쳐 공통으로 사용할 수 있는 함수를 선언합니다.
 * 
 * @date 2024-09-12
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include <errno.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
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