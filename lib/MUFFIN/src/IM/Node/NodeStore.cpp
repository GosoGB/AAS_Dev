/**
 * @file NodeStore.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Node 개체를 관리하는 백엔드 클래스를 정의합니다.
 * 
 * @date 2024-10-20
 * @version 0.0.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "NodeStore.h"



namespace muffin { namespace im {

    NodeStore::NodeStore()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }

    NodeStore::~NodeStore()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    Status NodeStore::Create(const jarvis::config::Node* cin)
    {
        try
        {
            Node node(cin);
            mMapNode.emplace(node.GetNodeID(), node);
            return Status(Status::Code::GOOD);
        }
        catch(const std::bad_alloc& e)
        {
            LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR NEW NODE: %s", e.what());
            return Status(Status::Code::BAD_OUT_OF_MEMORY);
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(logger, "FAILED TO EMPLACE WITH UNKNOWN EXCEPTION: %s", e.what());
            return Status(Status::Code::BAD_UNEXPECTED_ERROR);
        }
    }

    Status NodeStore::Remove(const std::string& nodeID)
    {
        ASSERT(false, "IMPLEMENTATION ERROR: FUNCTION IS NOT IMPLEMENTED");
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }

    Status NodeStore::Clear()
    {
        ASSERT(false, "IMPLEMENTATION ERROR: FUNCTION IS NOT IMPLEMENTED");
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }

    Status NodeStore::Get(const std::string& nodeID)
    {
        ASSERT(false, "IMPLEMENTATION ERROR: FUNCTION IS NOT IMPLEMENTED");
        return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
    }
}}