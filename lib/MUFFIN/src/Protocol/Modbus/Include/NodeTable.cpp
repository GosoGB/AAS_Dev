/**
 * @file NodeTable.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 단일 Modbus 마스터와 연결된 Node에 대한 참조 정보를 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-10-03
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#include <string.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "NodeTable.h"



namespace muffin { namespace modbus {
    
    NodeTable::NodeTable()
    {
    }
    
    NodeTable::~NodeTable()
    {
    }

    Status NodeTable::Update(const uint8_t slaveID, NodeRef& node)
    {
        Status ret(Status::Code::UNCERTAIN);

        auto it = mMapNodeReferenceBySlave.find(slaveID);
        if (it == mMapNodeReferenceBySlave.end())
        {
            try
            {
                auto result = mMapNodeReferenceBySlave.emplace(slaveID, std::vector<NodeRef*>());
                it = result.first;
                ASSERT((result.second == true), "FAILED TO EMPLACE NEW PAIR SINCE IT ALREADY EXISTS WHICH DOESN'T MAKE ANY SENSE");
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
        }
        
        try
        {
            it->second.emplace_back(&node);
            //LOG_VERBOSE(muffin::logger, "Emplaced Node reference: %s", node.GetNodeID().c_str());
            // printReferenceTable();
            return Status(Status::Code::GOOD);
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
    }

    Status NodeTable::Remove(const uint8_t slaveID, NodeRef& node)
    {
        auto itSlaveID = mMapNodeReferenceBySlave.find(slaveID);
        if (itSlaveID == mMapNodeReferenceBySlave.end())
        {
            LOG_WARNING(logger, "SLAVE ID OF NODE REFERENCE NOT FOUND: %u, %s", slaveID, node.GetNodeID().c_str());
            return Status(Status::Code::GOOD);
        }

        auto& references = itSlaveID->second;
        for (auto it = references.begin(); it != references.end(); ++it)
        {
            if ((*it)->GetNodeID() == node.GetNodeID())
            {
                references.erase(it);
                //LOG_VERBOSE(muffin::logger, "Removed node: %s", node.GetNodeID().c_str());
                // printReferenceTable();
                return Status(Status::Code::GOOD);
            }
        }

        return Status(Status::Code::BAD_NOT_FOUND);
    }

    void NodeTable::Clear()
    {
        mMapNodeReferenceBySlave.clear();
    }

    std::pair<Status, std::set<uint8_t>> NodeTable::RetrieveEntireSlaveID() const
    {
        std::exception exception;
        Status::Code retCode;

        try
        {
            std::set<uint8_t> slaveIDs;
            for (const auto& pair : mMapNodeReferenceBySlave)
            {
                slaveIDs.emplace(pair.first);
            }
            return std::make_pair(Status(Status::Code::GOOD), slaveIDs);
        }
        catch(const std::bad_alloc& e)
        {
            exception = e;
            retCode = Status::Code::BAD_OUT_OF_MEMORY;
        }
        catch(const std::exception& e)
        {
            exception = e;
            retCode = Status::Code::BAD_UNEXPECTED_ERROR;
        }
        
        LOG_ERROR(logger, "%s", exception.what());
        return std::make_pair(Status(retCode), std::set<uint8_t>());
    }

    std::pair<Status, std::vector<im::Node*>> NodeTable::RetrieveNodeBySlaveID(const uint8_t slaveID) const
    {
        if (mMapNodeReferenceBySlave.find(slaveID) != mMapNodeReferenceBySlave.end())
        {
            return std::make_pair(Status(Status::Code::GOOD), mMapNodeReferenceBySlave.at(slaveID));
        }
        else
        {
            LOG_WARNING(logger, "ADDRESS WITH SLAVE ID NOT FOUND: %u", slaveID);
            return std::make_pair(Status(Status::Code::BAD_NOT_FOUND), std::vector<im::Node*>());
        }
    }

#if defined(DEBUG)
    void NodeTable::printCell(const uint8_t cellWidth, const char* value, uint8_t* castedBuffer) const
    {
        char* buffer = reinterpret_cast<char*>(castedBuffer);

        char cell[cellWidth];
        memset(cell, '\0', cellWidth * sizeof(char));

        snprintf(cell, cellWidth - 1, "| %-*s", cellWidth - 2, value);
        strcat(buffer, cell);
    }

    void NodeTable::printCell(const uint8_t cellWidth, const uint16_t value, uint8_t* castedBuffer) const
    {
        char* buffer = reinterpret_cast<char*>(castedBuffer);

        char cell[cellWidth];
        memset(cell, '\0', cellWidth * sizeof(char));

        snprintf(cell, cellWidth - 1, "| %*u", cellWidth - 4, value);
        strcat(buffer, cell);
    }

    void NodeTable::printReferenceTable() const
    {
        const char* dashLine = "-------------------\n";
        const uint16_t bufferSize = 1024;
        const uint8_t cellWidth = 11;
        
        char buffer[bufferSize];
        uint8_t* castedBuffer = reinterpret_cast<uint8_t*>(buffer);
        memset(buffer, '\0', bufferSize * sizeof(char));
        strcat(buffer, dashLine);
        strcat(buffer, "| Slave  | Node   |\n");
        strcat(buffer, dashLine);

        for (const auto& references : mMapNodeReferenceBySlave)
        {
            const uint8_t slaveID = references.first;
            const auto& nodes = references.second;

            for (const auto& node : nodes)
            {
                printCell(cellWidth, slaveID, castedBuffer);
                printCell(cellWidth, node->GetNodeID().c_str(), castedBuffer);
                strcat(buffer, "|\n");
            }
        }

        strcat(buffer, dashLine);
        LOG_INFO(logger, "Modbus Node Reference Table\n%s\n", buffer);
    }
#else
#endif
}}