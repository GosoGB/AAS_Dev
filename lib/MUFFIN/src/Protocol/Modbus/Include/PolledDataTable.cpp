/**
 * @file PolledDataTable.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 다중 Modbus 슬레이브로부터 수집한 데이터를 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-10-02
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "PolledDataTable.h"



namespace muffin { namespace modbus {

    PolledDataTable::PolledDataTable()
    {
    }
    
    PolledDataTable::~PolledDataTable()
    {
    }

    Status PolledDataTable::UpdateCoil(const uint8_t slaveID, const uint16_t address, const bool value)
    {
        if (mMapPolledDataBySlave.find(slaveID) == mMapPolledDataBySlave.end())
        {
            mMapPolledDataBySlave.emplace(std::make_pair(slaveID, PolledData()));
        }
        
        auto it = mMapPolledDataBySlave.find(slaveID);
        return it->second.UpdateCoil(address, value);
    }

    bool PolledDataTable::RetrieveCoil(const uint8_t slaveID, const uint16_t address) const
    {
        ASSERT((mMapPolledDataBySlave.find(slaveID) != mMapPolledDataBySlave.end()), "SLAVE ID NOT FOUND");

        auto it = mMapPolledDataBySlave.find(slaveID);
        return it->second.RetrieveCoil(address);
    }
}}