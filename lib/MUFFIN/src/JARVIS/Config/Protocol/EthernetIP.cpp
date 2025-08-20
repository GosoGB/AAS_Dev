/**
 * @file EthernetIP.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Modbus TCP 프로토콜 설정 형식을 표현하는 클래스를 정의합니다.
 * 
 * @date 2025-07-01
 * @version 1.5.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Assert.hpp"
#include "Common/Logger/Logger.h"
#include "EthernetIP.h"



namespace muffin { namespace jvs { namespace config {

    EthernetIP::EthernetIP()
        : Base(cfg_key_e::ETHERNET_IP)
    {
    }

    EthernetIP::~EthernetIP()
    {
    #if defined(DEBUG)
        //LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    EthernetIP& EthernetIP::operator=(const EthernetIP& obj)
    {
        if (this != &obj)
        {
            mIPv4              = obj.mIPv4;
            mPort              = obj.mPort;
            mNodes             = obj.mNodes;
            mEthernetInterface = obj.mEthernetInterface;
            mScanRate          = obj.mScanRate;
        }

        return *this;
    }

    bool EthernetIP::operator==(const EthernetIP& obj) const
    {
    return (
            mIPv4              == obj.mIPv4       &&
            mNodes             == obj.mNodes      &&
            mPort              == obj.mPort       &&
            mEthernetInterface == obj.mEthernetInterface &&
            mScanRate          == obj.mScanRate
        );
    }

    bool EthernetIP::operator!=(const EthernetIP& obj) const
    {
        return !(*this == obj);
    }

    void EthernetIP::SetIPv4(const IPAddress& ipv4)
    {
        ASSERT(
            (
                ipv4 != IPAddress(0, 0, 0, 0)        ||
                ipv4 != IPAddress(127, 0, 0, 1)      ||
                ipv4 != IPAddress(192, 0, 2, 0)      ||
                ipv4 != IPAddress(203, 0, 113, 0)    ||
                ipv4 != IPAddress(255, 255, 255, 255)
            ),
            "INVALID IPv4 ADDRESS"
        );

        mIPv4 = ipv4;
        mIsIPv4Set = true;
    }

    void EthernetIP::SetPort(const uint16_t prt)
    {
        ASSERT((0 != prt), "INVALID PORT NUMBER");

        mPort = prt;
        mIsPortSet = true;
    }

    void EthernetIP::SetNodes(std::vector<std::string>&& nodes) noexcept
    {
        ASSERT((nodes.size() != 0), "NODE REFERENCES CANNOT BE NULL");

        mNodes = std::move(nodes);
        mIsNodesSet = true;
    }

    void EthernetIP::SetEthernetInterface(const if_e eth)
    {
        mEthernetInterface = eth;
        mIsEthernetInterfaceSet = true;
    }

    void EthernetIP::SetScanRate(const uint16_t sr)
    {
        mScanRate = sr;
        mIsScanRateSet = true;
    }

    std::pair<Status, if_e> EthernetIP::GetEthernetInterface() const
    {
        if (mIsEthernetInterfaceSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mEthernetInterface);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mEthernetInterface);
        }
    }

    std::pair<Status, IPAddress> EthernetIP::GetIPv4() const
    {
        if (mIsIPv4Set)
        {
            return std::make_pair(Status(Status::Code::GOOD), mIPv4);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mIPv4);
        }
    }

    std::pair<Status, uint16_t> EthernetIP::GetPort() const
    {
        if (mIsPortSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mPort);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mPort);
        }
    }

    std::pair<Status, std::vector<std::string>> EthernetIP::GetNodes() const
    {
        if (mIsNodesSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mNodes);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mNodes);
        }
    }

    std::pair<Status, uint16_t> EthernetIP::GetScanRate() const
    {
        if (mIsScanRateSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mScanRate);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mScanRate);
        }
    }
}}}