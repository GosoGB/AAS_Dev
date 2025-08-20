/**
 * @file NodeTable.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief Ethernet/IP 프로토콜 전용의 NodeTable을 정의합니다. 
 * 
 * @date 2025-07-01
 * @version 1.5.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2025
 */

#if defined(MT11)


#include <string.h>

#include "Common/Assert.hpp"
#include "Common/Logger/Logger.h"
#include "NodeTable.h"


namespace muffin { namespace ethernetIP {

    NodeTable::NodeTable()
    {
    }
    
    NodeTable::~NodeTable()
    {
    }

    Status NodeTable::Update(NodeRef& node)
    {
        try
        {
            mVectorNodeReference.emplace_back(&node);
            return Status(Status::Code::GOOD);
        }
        catch(const std::bad_alloc& e)
        {
            LOG_ERROR(logger, "%s: %s", e.what(), node.GetNodeID());
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(logger, "%s: %s", e.what(), node.GetNodeID());
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        }

        
    }

    Status NodeTable::Remove(NodeRef& node)
    {
        try
        {
            auto it = std::find(mVectorNodeReference.begin(), mVectorNodeReference.end(), &node);
            if (it != mVectorNodeReference.end())
            {
                mVectorNodeReference.erase(it);
                return Status(Status::Code::GOOD);
            }
            else
            {
                LOG_WARNING(logger, "NODE REFERENCE NOT FOUND IN NODE TABLE: %s", node.GetNodeID());
                return Status(Status::Code::BAD_NOT_FOUND);
            }
        }
        catch (const std::bad_alloc& e)
        {
            LOG_ERROR(logger, "%s: %s", e.what(), node.GetNodeID());
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }
        catch (const std::exception& e)
        {
            LOG_ERROR(logger, "%s: %s", e.what(), node.GetNodeID());
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        }
    }

    void NodeTable::Clear()
    {
        mVectorNodeReference.clear();
    }

    std::pair<Status, psram::vector<im::Node*>> NodeTable::RetrieveEntireNode() const
    {
        return std::make_pair(Status(Status::Code::GOOD), mVectorNodeReference); 
    }

}}



#endif