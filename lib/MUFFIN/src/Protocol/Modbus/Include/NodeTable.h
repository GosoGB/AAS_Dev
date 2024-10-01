/**
 * @file NodeTable.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 단일 Modbus 마스터와 연결된 Node에 대한 참조 정보를 표현하는 클래스를 선언합니다.
 * 
 * @date 2024-10-01
 * @version 0.0.1
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
    public:
        Status Update(const uint8_t slaveID, im::Node& node);
        const std::vector<im::Node*>& RetrieveBySlaveID(const uint8_t slaveID) const;
    private:
        std::map<uint8_t, std::vector<im::Node*>> mReferenceBySlaveMap;
    };
}}