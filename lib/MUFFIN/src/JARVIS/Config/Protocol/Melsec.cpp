/**
 * @file Melsec.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Modbus TCP 프로토콜 설정 형식을 표현하는 클래스를 정의합니다.
 * 
 * @date 2025-03-31
 * @version 1.4.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Melsec.h"



namespace muffin { namespace jvs { namespace config {

    Melsec::Melsec()
        : Base(cfg_key_e::MELSEC)
    {
    }

    Melsec::~Melsec()
    {
    #if defined(DEBUG)
        //LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    Melsec& Melsec::operator=(const Melsec& obj)
    {
        if (this != &obj)
        {
            mIPv4              = obj.mIPv4;
            mPlcSeies          = obj.mPlcSeies;
            mPort              = obj.mPort;
            mNodes             = obj.mNodes;
            mDataFormat        = obj.mDataFormat;
            mEthernetInterface = obj.mEthernetInterface;
            mScanRate          = obj.mScanRate;
        }

        return *this;
    }

    bool Melsec::operator==(const Melsec& obj) const
    {
    return (
        mPlcSeies              == obj.mPlcSeies   &&
            mIPv4              == obj.mIPv4       &&
            mNodes             == obj.mNodes      &&
            mPort              == obj.mPort       &&
            mDataFormat        == obj.mDataFormat &&
            mEthernetInterface == obj.mEthernetInterface &&
            mScanRate          == obj.mScanRate
        );
    }

    bool Melsec::operator!=(const Melsec& obj) const
    {
        return !(*this == obj);
    }

    void Melsec::SetIPv4(const IPAddress& ipv4)
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

    void Melsec::SetPort(const uint16_t prt)
    {
        ASSERT((0 != prt), "INVALID PORT NUMBER");

        mPort = prt;
        mIsPortSet = true;
    }

    void Melsec::SetPlcSeries(const ps_e plcSeies)
    {
        mPlcSeies = plcSeies;
        mIsPlcSeiesSet = true;
    }

    void Melsec::SetDataFormat(const df_e dataFormat)
    {
        mDataFormat = dataFormat;
        mIsDataFormatSet = true;
    }

    void Melsec::SetNodes(std::vector<std::string>&& nodes) noexcept
    {
        ASSERT((nodes.size() != 0), "NODE REFERENCES CANNOT BE NULL");

        mNodes = std::move(nodes);
        mIsNodesSet = true;
    }

    void Melsec::SetEthernetInterface(const if_e eth)
    {
        mEthernetInterface = eth;
        mIsEthernetInterfaceSet = true;
    }

    void Melsec::SetScanRate(const uint16_t sr)
    {
        mScanRate = sr;
        mIsScanRateSet = true;
    }

    std::pair<Status, if_e> Melsec::GetEthernetInterface() const
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

    std::pair<Status, ps_e> Melsec::GetPlcSeies() const
    {
        if (mIsPlcSeiesSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mPlcSeies);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mPlcSeies);
        }
    }

    std::pair<Status, df_e> Melsec::GetDataFormat() const
    {
        if (mIsDataFormatSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mDataFormat);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mDataFormat);
        }
    }

    std::pair<Status, IPAddress> Melsec::GetIPv4() const
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

    std::pair<Status, uint16_t> Melsec::GetPort() const
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

    std::pair<Status, std::vector<std::string>> Melsec::GetNodes() const
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

    std::pair<Status, uint16_t> Melsec::GetScanRate() const
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