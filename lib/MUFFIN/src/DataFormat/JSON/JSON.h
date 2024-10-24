/**
 * @file JSON.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief JSON 데이터 포맷 인코딩 및 디코딩을 수행하는 클래스를 선언합니다.
 * 
 * @date 2024-09-27
 * @version 0.0.1
 * 
 * @todo 인코딩 함수를 추가로 구현해야 합니다.
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <ArduinoJson.h>
#include <Protocol/MQTT/Include/TypeDefinitions.h>

#include "Common/Status.h"


namespace muffin {

    class JSON
    {
    public:
        JSON();
        virtual ~JSON();
    public:
        // std::pair<Status, std::string> Serialize();
        Status Deserialize(const std::string& payload, JsonDocument* json);
    };
}