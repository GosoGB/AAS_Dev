/**
 * @file PolledData.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 단일 Modbus 슬레이브로부터 수집한 데이터를 표현하는 클래스를 선언합니다.
 * 
 * @date 2024-10-02
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <map>
#include <vector>

#include "TypeDefinitions.h"



namespace muffin { namespace modbus {

    class PolledData
    {
    public:
        PolledData();
        virtual ~PolledData();
    public:
        Status UpdateCoil(const uint16_t address, const bool value);
    public:
        bool RetrieveCoil(const uint16_t address) const;
    private:
        std::map<area_e, std::vector<datum_t>> mMapPolledDataByArea;
    };
}}