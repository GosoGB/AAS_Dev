/**
 * @file NodeTable.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 단일 Modbus 마스터와 연결된 Node에 대한 참조 정보를 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-10-01
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "NodeTable.h"



namespace muffin { namespace modbus {
    
    NodeTable::NodeTable()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    NodeTable::~NodeTable()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    Status NodeTable::Update(const uint8_t slaveID, im::Node& node)
    {
        try
        {
            mReferenceBySlaveMap[slaveID].emplace_back(&node);
        }
        catch(const std::bad_alloc& e)
        {
            LOG_ERROR(logger, "%s: %s", e.what(), node.GetNodeID().c_str());
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(logger, "%s: %s", e.what(), node.GetNodeID().c_str());
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        }
        LOG_VERBOSE(muffin::logger, "Added node: %s", node.GetNodeID().c_str());
    }

    const std::vector<im::Node*>& NodeTable::RetrieveBySlaveID(const uint8_t slaveID) const
    {
        if (mReferenceBySlaveMap.find(slaveID) == mReferenceBySlaveMap.end())
        {
            std::vector<im::Node*> emptyVector;
            return emptyVector;
        }
        
        return mReferenceBySlaveMap.at(slaveID);
    }
}}