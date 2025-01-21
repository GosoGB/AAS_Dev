/**
 * @file NodeStore.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Node 개체를 관리하는 백엔드 클래스를 정의합니다.
 * 
 * @date 2024-10-20
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Core/MemoryPool/MemoryPool.h"
#include "NodeStore.h"



namespace muffin { namespace im {

    NodeStore* NodeStore::CreateInstanceOrNULL()
    {
        if (mInstance == nullptr)
        {
            mInstance = new(std::nothrow) NodeStore();
            if (mInstance == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR NODE STORE");
                return mInstance;
            }
        }

        return mInstance;
    }

    NodeStore& NodeStore::GetInstance()
    {
        ASSERT((mInstance != nullptr), "NO INSTANCE EXISTS: CALL FUNCTION \"CreateInstanceOrNULL\" INSTEAD");
        return *mInstance;
    }

    std::map<std::string, Node*>::iterator NodeStore::begin()
    {
        return mMapNode.begin();
    }

    std::map<std::string, Node*>::iterator NodeStore::end()
    {
        return mMapNode.end();
    }

    std::map<std::string, Node*>::const_iterator NodeStore::begin() const
    {
        return mMapNode.cbegin();
    }

    std::map<std::string, Node*>::const_iterator NodeStore::end() const
    {
        return mMapNode.cend();
    }

    NodeStore::NodeStore()
    {
    }

    NodeStore::~NodeStore()
    {
    }

    Status NodeStore::Create(const jvs::config::Node* cin)
    {
        try
        {
            uint32_t prev = ESP.getFreeHeap();
            LOG_DEBUG(logger, "Remained Heap: %u Bytes", ESP.getFreeHeap());
            
            void* block = memoryPool.Allocate(799);
            Node* node = new(block) Node(cin);
            mMapNode.emplace(node->GetNodeID(), node);

            LOG_DEBUG(logger, "Node Memory: %u Bytes", prev - ESP.getFreeHeap());
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

    void NodeStore::Clear()
    {
        mMapNode.clear();
    }

    std::pair<Status, Node*> NodeStore::GetNodeReference(const std::string& nodeID)
    {
        const auto it = mMapNode.find(nodeID);
        if (it == mMapNode.end())
        {
            LOG_WARNING(logger, "NO REFERENCE: NODE WITH GIVEN ID NOT FOUND");
            return std::make_pair(Status(Status::Code::BAD_NOT_FOUND), nullptr);
        }
        else
        {
            return std::make_pair(Status(Status::Code::GOOD), it->second);
        }
    }

    std::pair<Status, Node*> NodeStore::GetNodeReferenceUID(const std::string& UID)
    {
        for (auto& node : mMapNode)
        {
            const std::string& mUID = node.second->GetUID();
            if (mUID == UID)
            {
                return std::make_pair(Status(Status::Code::GOOD), node.second);
            }
        }
        return std::make_pair(Status(Status::Code::BAD_NOT_FOUND), nullptr);
    }

    std::vector<Node*> NodeStore::GetCyclicalNode()
    {
        std::vector<Node*> cyclicalNodeVector;

        for (auto& node : mMapNode)
        {
            if(node.second->VariableNode.GetHasAttributeEvent() == false)
            {
                cyclicalNodeVector.emplace_back(node.second);
            }
        }
        
        return cyclicalNodeVector;
    }


    NodeStore* NodeStore::mInstance = nullptr;
}}