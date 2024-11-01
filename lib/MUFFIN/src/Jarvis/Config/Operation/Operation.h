/**
 * @file Operation.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Operation 설정 정보를 관리하는 클래스를 선언합니다.
 * 
 * @date 2024-10-08
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include "Common/Status.h"
#include "Jarvis/Include/Base.h"
#include "Jarvis/Include/TypeDefinitions.h"



namespace muffin { namespace jarvis { namespace config {

    class Operation : public Base
    {
    public:
        Operation();
        virtual ~Operation() override;
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
        bool mIsPlanExpiredSet        = false;
        bool mIsFactoryResetSet       = false;
        bool mIsServerNicSet          = false;
        bool mIsIntervalServerSet     = false;
        bool mIsIntervalPollingSet    = false;
    private:
        bool mPlanExpired;
        bool mFactoryReset;
        snic_e mServerNIC;
        uint16_t mIntervalServer;
        uint16_t mIntervalPolling;
    };
}}}