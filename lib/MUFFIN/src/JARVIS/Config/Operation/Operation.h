/**
 * @file Operation.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Operation 설정 정보를 관리하는 클래스를 선언합니다.
 * 
 * @date 2025-01-17
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include "Common/Status.h"
#include "Common/DataStructure/bitset.h"
#include "JARVIS/Include/Base.h"
#include "JARVIS/Include/TypeDefinitions.h"



namespace muffin { namespace jarvis { namespace config {

    class Operation : public Base
    {
    public:
        Operation() : Base(cfg_key_e::OPERATION) {}
        virtual ~Operation() override {}
    public:
        Operation& operator=(const Operation& obj);
        bool operator==(const Operation& obj) const;
        bool operator!=(const Operation& obj) const;
    public:
        void SetPlanExpired(const bool planExpired);
        void SetFactoryReset(const bool factoryReset);
        void SetServerNIC(const snic_e snic);
        void SetIntervalServer(const uint16_t interval);
        void SetIntervalPolling(const uint16_t interval);
    public:
        std::pair<Status, bool> GetPlanExpired() const;
        std::pair<Status, bool> GetFactoryReset() const;
        std::pair<Status, snic_e> GetServerNIC() const;
        std::pair<Status, uint16_t> GetIntervalServer() const;
        std::pair<Status, uint16_t> GetIntervalPolling() const;
    private:
        typedef enum class SetFlagEnum : uint8_t
        {
            SERVICE_PLAN   = 0,
            FACTORY_RESET  = 1,
            SERVICE_NIC    = 2,
            PUB_INTERVAL   = 3,
            DAQ_INTERVAL   = 4,
            TOP            = 5
        } set_flag_e;
        bitset<static_cast<uint8_t>(set_flag_e::TOP)> mSetFlags;
    private:
        bool mPlanExpired;
        bool mFactoryReset;
        snic_e mServerNIC;
        uint16_t mIntervalServer;
        uint16_t mIntervalPolling;
    };


    extern Operation opeartion;
}}}