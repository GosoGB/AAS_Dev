/**
 * @file NodeTable.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 단일 Modbus 마스터와 연결된 Node에 대한 참조 정보를 표현하는 클래스를 선언합니다.
 * 
 * @date 2024-10-03
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <map>
#include <vector>

#include "Common/Status.h"
#include "IM/Node/Node.h"



namespace muffin { namespace modbus {

    class NodeTable
    {
    public:
        NodeTable();
        virtual ~NodeTable();
    private:
        using NodeRef = im::Node;
    public:
        Status Update(const uint8_t slaveID, NodeRef& node);
        Status Remove(const uint8_t slaveID, NodeRef& node);
        void Clear();
    public:
        std::pair<Status, std::vector<im::Node*>> RetrieveEntireNode() const;
        std::pair<Status, std::set<uint8_t>> RetrieveEntireSlaveID() const;
        std::pair<Status, std::vector<im::Node*>> RetrieveNodeBySlaveID(const uint8_t slaveID) const;
    private:
    #if defined(DEBUG)
        void printCell(const uint8_t cellWidth, const char* value, uint8_t* castedBuffer) const;
        void printCell(const uint8_t cellWidth, const uint16_t value, uint8_t* castedBuffer) const;
        void printReferenceTable() const;
    #else
        /**
         * @todo release 빌드 시에는 csv 형태로 로그를 만들어서 서버로 전송해야 함
         */
        void printAddressTable() const;
    #endif
    
    private:
        /**
         * @brief 
         */
        std::map<uint8_t, std::vector<NodeRef*>> mMapNodeReferenceBySlave;
    };
}}