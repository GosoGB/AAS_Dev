/**
 * @file ValidateConfigService.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief MFM 설정 정보의 유효성을 검증하는 서비스를 정의합니다.
 * 
 * @date 2025-01-28
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Common/Convert/ConvertClass.h"
#include "Common/Time/TimeUtils.h"
#include "IM/Custom/Constants.h"
#include "JARVIS/JARVIS.h"
#include "ServiceSets/JarvisServiceSet/ValidateConfigService.h"
#include "Storage/ESP32FS/ESP32FS.h"



namespace muffin {

    Status ValidateConfigService(jarvis_struct_t* output)
    {
        ASSERT((esp32FS.DoesExist(JARVIS_PATH_FETCHED) == Status::Code::GOOD), "FETCHED JARVIS CONFIG MUST EXIST");
        ASSERT((jarvis != nullptr), "JARVIS INSTANCE MUST BE CREATED");
        ASSERT((output != nullptr), "OUTPUT PARAMETER CANNOT BE NULL");

        File file = esp32FS.Open(JARVIS_PATH_FETCHED, "r", false);
        if (file == false)
        {
            return Status(Status::Code::BAD_DEVICE_FAILURE);
        }
        
        JSON json;
        JsonDocument doc;
        output->SourceTimestamp = GetTimestampInMillis();

        Status ret = json.Deserialize(file, &doc);
        file.close();

        if (ret != Status::Code::GOOD)
        {
            switch (ret.ToCode())
            {
            case Status::Code::BAD_END_OF_STREAM:
                output->ResponseCode = Convert.ToUInt16(jvs::rsc_e::BAD_COMMUNICATION);
                output->Description = "PAYLOAD INSUFFICIENT OR INCOMPLETE";
                break;

            case Status::Code::BAD_NO_DATA:
                output->ResponseCode = Convert.ToUInt16(jvs::rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE);
                output->Description = "PAYLOAD EMPTY";
                break;

            case Status::Code::BAD_DATA_ENCODING_INVALID:
                output->ResponseCode = Convert.ToUInt16(jvs::rsc_e::BAD_DECODING_ERROR);
                output->Description = "PAYLOAD INVALID ENCODING";
                break;

            case Status::Code::BAD_OUT_OF_MEMORY:
                output->ResponseCode = Convert.ToUInt16(jvs::rsc_e::BAD_OUT_OF_MEMORY);
                output->Description = "PAYLOAD OUT OF MEMORY";
                break;

            case Status::Code::BAD_ENCODING_LIMITS_EXCEEDED:
                output->ResponseCode = Convert.ToUInt16(jvs::rsc_e::BAD_DECODING_CAPACITY_EXCEEDED);
                output->Description = "PAYLOAD EXCEEDED NESTING LIMIT";
                break;

            case Status::Code::BAD_UNEXPECTED_ERROR:
                output->ResponseCode = Convert.ToUInt16(jvs::rsc_e::BAD_UNEXPECTED_ERROR);
                output->Description = "UNDEFINED CONDITION";
                break;

            default:
                output->ResponseCode = Convert.ToUInt16(jvs::rsc_e::BAD_UNEXPECTED_ERROR);
                output->Description = "UNDEFINED CONDITION";
                break;
            }
            
            LOG_ERROR(logger, "FAILED TO DECODE JARVIS CONFIG: %s", ret.c_str());
            ret = Status::Code::BAD_ENCODING_ERROR;
            return ret;
        }
        
        jarvis->Clear();
        jvs::ValidationResult result = jarvis->Validate(doc);
        doc.clear();

        output->ResponseCode  = Convert.ToUInt16(result.GetRSC());
        output->Description   = result.GetDescription();
        if (result.GetRSC() >= jvs::rsc_e::BAD)
        {
            ret = Status::Code::BAD_CONFIGURATION_ERROR;

            for (auto& key : result.RetrieveKeyWithNG())
            {
                output->Config.emplace_back(Convert.ToString(key));
            }
        }
        return ret;
    }
}