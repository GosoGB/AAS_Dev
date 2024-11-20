/**
 * @file Helper.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Core 모듈 전반에 걸쳐 공통으로 사용할 수 있는 함수를 선언합니다.
 * 
 * @date 2024-10-15
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <esp_system.h>
#include <string>
#include <utility>
#include <vector>

#include "Common/Assert.h"
#include "Common/Status.h"
#include "Protocol/MQTT/Include/Message.h"



namespace muffin {

    void printResetReason(const esp_reset_reason_t reason);
    
    template <typename T>
    Status EmplaceBack(T&& value, std::vector<T>* vector)
    {
        ASSERT((vector != nullptr), "OUTPUT PARAMETER \"vector\" CAN NOT BE A NULL POINTER");

        try
        {
            vector->emplace_back(std::forward<T>(value));
            return Status(Status::Code::GOOD);
        }
        catch(const std::bad_alloc& e)
        {
            LOG_ERROR(logger, "FAILED TO EMPLACE BACK: %s", e.what());
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(logger, "FAILED TO EMPLACE BACK: %s", e.what());
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        }
    }
}