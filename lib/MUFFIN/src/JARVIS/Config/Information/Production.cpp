/**
 * @file Production.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 생산 정보 설정 형식을 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-10-07
 * @version 1.0.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Assert.hpp"
#include "Common/Logger/Logger.h"
#include "Production.h"



namespace muffin { namespace jvs { namespace config {

    Production::Production()
        : Base(cfg_key_e::PRODUCTION_INFO)
    {
    }

    Production::~Production()
    {
    }

    Production& Production::operator=(const Production& obj)
    {
        if (this != &obj)
        {
            mNodeIdTotal   =  obj.mNodeIdTotal;       
            mNodeIdGood    =  obj.mNodeIdGood;
            mNodeIdNG      =  obj.mNodeIdNG;
        }
        
        return *this;
    }

    bool Production::operator==(const Production& obj) const
    {
       return (
            mNodeIdTotal   ==  obj.mNodeIdTotal  &&       
            mNodeIdGood    ==  obj.mNodeIdGood   &&
            mNodeIdNG      ==  obj.mNodeIdNG   
        );
    }

    bool Production::operator!=(const Production& obj) const
    {
        return !(*this == obj);
    }

    void Production::SetNodeIdTotal(const std::string& nodeID)
    {
        ASSERT((nodeID.size() == 4), "NODE ID MUST BE A STRING WITH LEGNTH OF 4");

        mNodeIdTotal = nodeID;
        mIsTotalSet = true;
    }

    void Production::SetNodeIdGood(const std::string& nodeID)
    {
        ASSERT((nodeID.size() == 4), "NODE ID MUST BE A STRING WITH LEGNTH OF 4");

        mNodeIdGood = nodeID;
        mIsGoodSet = true;
    }

    void Production::SetNodeIdNG(const std::string& nodeID)
    {
        ASSERT((nodeID.size() == 4), "NODE ID MUST BE A STRING WITH LEGNTH OF 4");

        mNodeIdNG = nodeID;
        mIsNgSet = true;
    }

    std::pair<Status, std::string> Production::GetNodeIdTotal() const
    {
        if (mIsTotalSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mNodeIdTotal);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mNodeIdTotal);
        }
    }

    std::pair<Status, std::string> Production::GetNodeIdGood() const
    {
        if (mIsGoodSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mNodeIdGood);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mNodeIdGood);
        }
    }

    std::pair<Status, std::string> Production::GetNodeIdNG() const
    {
        if (mIsNgSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mNodeIdNG);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mNodeIdNG);
        }
    }
}}}