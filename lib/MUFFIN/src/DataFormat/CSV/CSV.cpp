/**
 * @file CSV.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief CSV 데이터 포맷 인코딩 및 디코딩을 수행하는 클래스를 정의합니다.
 * 
 * @date 2024-10-17
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "CSV.h"




namespace muffin {
    
    CSV::CSV()
    {
    }
    
    CSV::~CSV()
    {
    }

    Status CSV::Deserialize(const std::string& input, std::vector<std::string>* output)
    {
        ASSERT(false, "NOT IMPLEMENTED SERVICE");
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }

    Status CSV::Deserialize(const std::vector<uint8_t>& input, std::vector<std::string>* output)
    {
        ASSERT(false, "NOT IMPLEMENTED SERVICE");
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }
}