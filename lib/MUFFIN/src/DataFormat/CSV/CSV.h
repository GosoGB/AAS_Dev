/**
 * @file CSV.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief CSV 데이터 포맷 인코딩 및 디코딩을 수행하는 클래스를 선언합니다.
 * 
 * @date 2025-02-05
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 * 
 * 
 * @todo 구현한 다음 LTE Cat.M1 모뎀과 관련된 모든 클래스에 적용해야 합니다.
 * @details LTE Cat.M1 모뎀과 MODLINK가 주고 받는 데이터가 주로 CSV 형식과 비슷합니다.
 *          지금은 delimiter를 하나하나 위치를 찾아가며 작업하기 때문에 코드 가독성이
 *          굉장히 떨어집니다. CSV 클래스는 이를 상당히 개선할 수 있을 것이라 기대합니다.
 */




#pragma once

#include <string>
#include <sys/_stdint.h>
#include <vector>

#include "Common/Status.h"
#include "IM/Custom/TypeDefinitions.h"



namespace muffin {

    class CSV
    {
    public:
        Status Decode(const char* input, init_cfg_t* output);
        Status Decode(const char* input, ota_chunk_info_t* output);
    public:
        Status Encode(const init_cfg_t& input, const size_t length, char output[]);
        Status Encode(const ota_chunk_info_t& input, const size_t length, char output[]);
    private:
        Status parse(const char* input, const uint8_t columnCount, const uint8_t columnWidth, char** output);
    };
}