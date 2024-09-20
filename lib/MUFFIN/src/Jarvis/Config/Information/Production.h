/**
 * @file Production.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-09-20
 * 
 * @copyright Copyright (c) 2024
 * 
 */




#pragma once

#include "Common/Status.h"
#include "Jarvis/Include/Base.h"



namespace muffin { namespace jarvis { namespace config {


    class Production : public Base
    {
    public:
        Production();
        virtual ~Production() override;
    public:
        Production& operator=(const Production& obj);
        bool operator==(const Production& obj) const;
        bool operator!=(const Production& obj) const;
    
    public:
        Status SetTotalProductionNodeID(const std::string& nodeID);
        Status SetGoodQualityNodeID(const std::string& nodeID);
        Status SetDefectNodeID(const std::string& nodeID);

    public:
        const std::string& GetTotalProductionNodeID() const;
        const std::string& GetGoodQualityNodeID() const;
        const std::string& GetDefectNodeID() const;

    private:
        std::string mTotalProductionNodeID;
        std::string mGoodQualityNodeID;
        std::string mDefectNodeID;
    };
}}}
