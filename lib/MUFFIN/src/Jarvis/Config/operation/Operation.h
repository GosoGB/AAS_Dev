/**
 * @file Operation.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Operation 인터페이스 설정 정보를 관리하는 클래스를 선언합니다.
 * 
 * @date 2024-09-02
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once


#include "Common/Status.h"
#include "Jarvis/Include/Base.h"



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
        Status SetExpired(const bool exp);
        Status SetOTA(const bool ota);
        Status SetIntervalServer(const uint32_t& intvsrv);
        Status SetIntervalPolling(const uint16_t& intvpoll);
        Status SetServerNIC(const std::string& snic);

    public:
        bool GetExpired() const;
        bool GetOTA() const;
        const uint32_t& GetIntervalServer() const;
        const uint16_t& GetIntervalPolling() const;
        const std::string& GetServerNIC() const;

    private:
        bool mExpired = false;
        bool mOTA = false;
        std::string mServerNIC;
        uint32_t mIntervalServer;
        uint16_t mIntervalPolling;

    };
}}}