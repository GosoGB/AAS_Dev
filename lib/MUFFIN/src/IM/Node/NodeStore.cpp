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
            // uint32_t prev = ESP.getFreeHeap();
            LOG_DEBUG(logger, "Remained Heap: %u Bytes", ESP.getFreeHeap());
            void* block = memoryPool.Allocate(28);
            Node* node = new(block) Node(cin);

            mMapNode.emplace(node->GetNodeID(), node);
            
            LOG_DEBUG(logger, "size of Node Memory: %u Bytes", sizeof(Node));
            // LOG_DEBUG(logger, "Node Memory: %u Bytes", prev - ESP.getFreeHeap());
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

#if defined(MT11)
    psram::map<uint16_t, psram::vector<im::Node*>> NodeStore::GetIntervalCustomNode(psram::map<uint16_t, psram::vector<std::string>> nodeIdMap, uint16_t defaultInterval)
    {
        psram::map<uint16_t, psram::vector<im::Node*>> IntervalCustomNodeMap;
        psram::vector<Node*> cyclicalNodeVector = GetCyclicalNode();

        for (const auto& _pair : nodeIdMap)
        {
            const uint16_t interval = _pair.first;
            const psram::vector<std::string>& nodeIdVector = _pair.second;

            psram::vector<im::Node*> nodes;
            nodes.reserve(nodeIdVector.size());

            for (const auto& nodeId : nodeIdVector)
            {
                auto pair = mMapNode.find(nodeId);
                im::Node* nodeReference = pair->second;
                nodes.emplace_back(nodeReference);
                cyclicalNodeVector.erase(std::remove(cyclicalNodeVector.begin(), cyclicalNodeVector.end(), nodeReference),cyclicalNodeVector.end());    
            }

            IntervalCustomNodeMap.emplace(interval, std::move(nodes));
        }

        // defaultInterval 키가 이미 존재하면 cyclicalNodeVector를 해당 벡터에 추가
        auto it = IntervalCustomNodeMap.find(defaultInterval);
        if (it != IntervalCustomNodeMap.end()) 
        {
            it->second.insert(it->second.end(), cyclicalNodeVector.begin(), cyclicalNodeVector.end());
        } 
        else if (!cyclicalNodeVector.empty()) 
        {
            IntervalCustomNodeMap.emplace(defaultInterval, std::move(cyclicalNodeVector));
        }
        
        return IntervalCustomNodeMap;

    }

    psram::vector<Node*> NodeStore::GetCyclicalNode()
    {
        psram::vector<Node*> cyclicalNodeVector;

        for (auto& node : mMapNode)
        {
            if (node.second->HasAttributeEvent() == false)
            {
                cyclicalNodeVector.emplace_back(node.second);
            }
        }
        
        return cyclicalNodeVector;
    }

    psram::vector<Node*> NodeStore::GetEventNode()
    {
        psram::vector<Node*> EventNodeVector;

        for (auto& node : mMapNode)
        {
            if (node.second->HasAttributeEvent() == true)
            {
                EventNodeVector.emplace_back(node.second);
            }
        }
        
        return EventNodeVector;
    }

#else
    std::map<uint16_t, std::vector<im::Node*>> NodeStore::GetIntervalCustomNode(std::map<uint16_t, std::vector<std::string>> nodeIdMap, uint16_t defaultInterval)
    {
        std::map<uint16_t, std::vector<im::Node*>> IntervalCustomNodeMap;
        std::vector<Node*> cyclicalNodeVector = GetCyclicalNode();

        for (const auto& _pair : nodeIdMap)
        {
            const uint16_t interval = _pair.first;
            const std::vector<std::string>& nodeIdVector = _pair.second;

            std::vector<im::Node*> nodes;
            nodes.reserve(nodeIdVector.size());

            for (const auto& nodeId : nodeIdVector)
            {
                auto it = mMapNode.find(nodeId);
                if (it != mMapNode.end() && it->second != nullptr) 
                {
                    im::Node* currentNode = it->second;
                    nodes.emplace_back(currentNode);
                    cyclicalNodeVector.erase(std::remove(cyclicalNodeVector.begin(), cyclicalNodeVector.end(), currentNode),cyclicalNodeVector.end());
                } 
                else 
                {
                    LOG_ERROR(logger, "NODE NOT FOUND OR NULLPTR FOR ID: [%s]", nodeId.c_str());
                }
            }

            IntervalCustomNodeMap.emplace(interval, std::move(nodes));
        }

        // defaultInterval 키가 이미 존재하면 cyclicalNodeVector를 해당 벡터에 추가
        auto it = IntervalCustomNodeMap.find(defaultInterval);
        if (it != IntervalCustomNodeMap.end()) 
        {
            it->second.insert(it->second.end(), cyclicalNodeVector.begin(), cyclicalNodeVector.end());
        } 
        else if (!cyclicalNodeVector.empty()) 
        {
            IntervalCustomNodeMap.emplace(defaultInterval, std::move(cyclicalNodeVector));
        }
        
        return IntervalCustomNodeMap;

    }
    std::vector<Node*> NodeStore::GetCyclicalNode()
    {
        std::vector<Node*> cyclicalNodeVector;

        for (auto& node : mMapNode)
        {
            if (node.second->HasAttributeEvent() == false)
            {
                cyclicalNodeVector.emplace_back(node.second);
            }
        }
        
        return cyclicalNodeVector;
    }

    std::vector<Node*> NodeStore::GetEventNode()
    {
        std::vector<Node*> EventNodeVector;

        for (auto& node : mMapNode)
        {
            if (node.second->HasAttributeEvent() == true)
            {
                EventNodeVector.emplace_back(node.second);
            }
        }
        
        return EventNodeVector;
    }
#endif
    

    size_t NodeStore::GetArrayNodeCount()
    {
        size_t ArrayNodeCount = 0;

        for (auto& node : mMapNode)
        {
            if (node.second->IsArrayNode() == true)
            {
                ArrayNodeCount++;
            }
        }

        return ArrayNodeCount;
    }

    NodeStore* NodeStore::mInstance = nullptr;
}}