/**
 * @file Convert.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 다양한 데이터 타입 간의 변환을 지원하는 유틸리티 클래스를 선언합니다.
 * 
 * @date 2024-10-17
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <string>
#include <sys/_stdint.h>



namespace muffin {

    class ConvertClass
    {
    public:
        static int8_t ToInt8(const std::string& input);
        static int16_t ToInt16(const std::string& input);
        static int32_t ToInt32(const std::string& input);
        static int64_t ToInt64(const std::string& input);
    public:
        static uint8_t ToUInt8(const std::string& input);
        static uint16_t ToUInt16(const std::string& input);
        static uint32_t ToUInt32(const std::string& input);
        static uint64_t ToUInt64(const std::string& input);
    public:
        static float ToFloat(const std::string& input);
        static double ToDouble(const std::string& input);
    public:
        template <typename T>
        static std::string ToString(const T input)
        {
            static_assert(std::is_integral<T>::value, "Only integral types are supported");
            return std::to_string(input);
        }
    };


    extern ConvertClass Convert;
}