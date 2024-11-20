/**
 * @file Helper.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 정보 모델에서 공통적으로 사용할 수 있는 함수를 정의합니다.
 * 
 * @date 2024-09-26
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */



/*
#include <string.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Helper.h"



namespace muffin { namespace im {

    Status ConvertString(const char* src, string_t* sink)
    {
        ASSERT((sink != nullptr), "SINK STRING CANNOT BE A NULL POINTER");
        ASSERT((src != nullptr), "SOURCE STRING CANNOT BE A NULL POINTER");
        ASSERT((strlen(src) != 0), "NO MEANING IN CONVERTING AN EMPTY STRING");

        sink->Length = strlen(src);
        sink->Data = (uint8_t*)(malloc(sink->Length * sizeof(uint8_t)));
        
        if (unlikely(sink->Data == nullptr))
        {
            LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR STRING");
            sink->Length = 0;
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }

        memcpy(sink->Data, src, sink->Length);
        return Status(Status::Code::GOOD);;
    }

    Status ConvertString(const std::string& src, string_t* sink)
    {
        return ConvertString(src.c_str(), sink);
    }

    std::string ConvertString(const string_t& string)
    {
        return std::string(string.Data, string.Data + string.Length);
    }

    const char* ConvertClassTypeEnum(const class_type_e type)
    {
        switch (type)
        {
        case class_type_e::UNSPECIFIED:
            return "UNSPECIFIED";
        case class_type_e::OBJECT:
            return "OBJECT";
        case class_type_e::VARIABLE:
            return "VARIABLE";
        case class_type_e::METHOD:
            return "METHOD";
        case class_type_e::OBJECT_TYPE:
            return "OBJECT TYPE";
        case class_type_e::VARIABLE_TYPE:
            return "VARIABLE TYPE";
        case class_type_e::REFERENCE_TYPE:
            return "REFERENCE TYPE";
        case class_type_e::DATA_TYPE:
            return "DATA TYPE";
        case class_type_e::VIEW:
            return "VIEW";
        default:
            ASSERT(false, "UNDEFINED NODE CLASS TYPE");
            return nullptr;
        }
    }
}}
*/