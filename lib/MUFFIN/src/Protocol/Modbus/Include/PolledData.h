/**
 * @file PolledData.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 단일 Modbus 슬레이브로부터 수집한 데이터를 표현하는 클래스를 선언합니다.
 * 
 * @date 2024-10-20
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <map>
#include <vector>

#include "Common/Status.h"
#include "Jarvis/Include/TypeDefinitions.h"
#include "TypeDefinitions.h"



namespace muffin { namespace modbus {

    class PolledData
    {
    public:
        PolledData();
        virtual ~PolledData();
    public:
        Status UpdateCoil(const uint16_t address, const int8_t value);
        datum_t RetrieveCoil(const uint16_t address) const;
    private:
        std::map<jarvis::mb_area_e, std::vector<datum_t>> mMapDatumByArea;
    };
}}