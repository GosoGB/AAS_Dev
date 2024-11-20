/**
 * @file ScopedTimer.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 콜 스택을 수명 범위로 갖는 타이머를 선언합니다.
 * 
 * @date 2024-09-01
 * @version 1.0.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <string>

#include "TimeUtils.h"



namespace muffin {

    class ScopedTimer
    {
    public:
        ScopedTimer(const std::string& file, const std::string& function);
        virtual ~ScopedTimer();
    private:
        const std::string mFileName;
        const std::string mFunctionName;
        const uint64_t mStartMillis;
    };
}