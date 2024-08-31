/**
 * @file ScopedTimer.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 콜 스택을 수명 범위로 갖는 타이머를 정의합니다.
 * 
 * @date 2024-09-01
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Logger/Logger.h"
#include "ScopedTimer.h"



namespace muffin {

    ScopeTimer::ScopeTimer(const std::string& file, const std::string& function)
        : mFileName(file)
        , mFunctionName(function)
        , mStartMillis(GetTimestampInMillis())
    {
    }
    
    ScopeTimer::~ScopeTimer()
    {
        LOG_DEBUG(logger, "[%s][%s] Processing Time: %llu ms", 
            mFileName.c_str(),
            mFunctionName.c_str(),
            GetTimestampInMillis() - mStartMillis
        );
    }
}