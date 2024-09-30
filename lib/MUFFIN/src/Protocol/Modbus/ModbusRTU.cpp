/**
 * @file ModbusRTU.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Modbus RTU 프로토콜 클래스를 정의합니다.
 * 
 * @date 2024-09-27
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include <string.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "ModbusRTU.h"



namespace muffin {

    ModbusRTU::ModbusRTU()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    ModbusRTU::~ModbusRTU()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }
    
    Status ModbusRTU::AddNodeReference(const uint8_t slaveID, im::Node& node)
    {
        try
        {
            mNodeReferences.emplace_back(&node);
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

        im::NumericAddressRange range(node.VariableNode.GetAddress(), node.VariableNode.GetQuantity());
        mAddressTable.UpdateAddressTable(slaveID, node.VariableNode.GetModbusArea(), range);
        return Status(Status::Code::GOOD);
    }

    Status ModbusRTU::RemoveReferece(const std::string& nodeID)
    {
        for (auto it = mNodeReferences.begin(); it != mNodeReferences.end(); ++it)
        {
            if ((*it)->GetNodeID() == nodeID)
            {
                mNodeReferences.erase(it);
                LOG_VERBOSE(muffin::logger, "Removed node: %s", nodeID.c_str());
                return Status(Status::Code::GOOD);
            }
        }
        
        LOG_ERROR(muffin::logger, "FAILED TO REMOVE REFERENCE TO NODE \"%s\" SINCE IT'S NOT FOUND");
        return Status(Status::Code::BAD_NOT_FOUND);
    }
}