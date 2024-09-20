/**
 * @file Production.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 생산 정보 설정 형식을 표현하는 클래스를 선언합니다.
 * 
 * @date 2024-10-07
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include "Common/Status.h"
#include "Jarvis/Include/Base.h"



namespace muffin { namespace jarvis { namespace config {

    class Production : public Base
    {
    public:
        explicit Production(const cfg_key_e category);
        virtual ~Production() override;
    public:
        Production& operator=(const Production& obj);
        bool operator==(const Production& obj) const;
        bool operator!=(const Production& obj) const;
    public:
        void SetNodeIdTotal(const std::string& nodeID);
        void SetNodeIdGood(const std::string& nodeID);
        void SetNodeIdNG(const std::string& nodeID);
    public:
        std::pair<Status, std::string> GetNodeIdTotal() const;
        std::pair<Status, std::string> GetNodeIdGood() const;
        std::pair<Status, std::string> GetNodeIdNG() const;
    private:
        bool mIsTotalSet  = false;
        bool mIsGoodSet   = false;
        bool mIsNgSet     = false;
    private:
        std::string mNodeIdTotal;
        std::string mNodeIdGood;
        std::string mNodeIdNG;
    };
}}}