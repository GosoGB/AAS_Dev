/**
 * @file Optime.h
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


    class Optime : public Base
    {
    public:
        Optime();
        virtual ~Optime() override;
    public:
        Optime& operator=(const Optime& obj);
        bool operator==(const Optime& obj) const;
        bool operator!=(const Optime& obj) const;
    
    public:
        Status SetNodeID(const std::string& nodeID);
        Status SetOptimeType(const uint8_t& type);
        Status SetCriterion(const uint32_t& criterion);
        Status SetOperator(const std::string& operater);
    
    public:
        const std::string& GetNodeID() const;
        const uint8_t& GetOptimeType() const;
        const uint32_t& GetCriterion() const;
        const std::string& GetOperator() const;

    private:
        std::string mNodeID;
        uint8_t mOptimeType;
        uint32_t mCriterion;
        std::string mOperater;
    };
}}}
