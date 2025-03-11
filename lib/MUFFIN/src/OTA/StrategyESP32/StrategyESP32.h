/**
 * @file StrategyESP32.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief ESP32 전용 펌웨어 업데이트 클래스를 선언합니다.
 * 
 * @date 2025-01-15
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include "Common/Status.h"
#include "OTA/Include/TypeDefinitions.h"



namespace muffin { namespace ota {

    class StrategyESP32
    {
    public:
        StrategyESP32() {}
        virtual ~StrategyESP32() {}
    public:
        Status Init(const fw_info_t& info);
        Status Write(const size_t size, uint8_t byteArray[]);
        Status TearDown();
    private:
        Status processErrorCode(const uint8_t errorCode) const;
    };
}}