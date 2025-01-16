/**
 * @file ModbusRTU.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Modbus RTU 프로토콜 설정 형식을 표현하는 클래스를 선언합니다.
 * 
 * @date 2024-10-14
 * @version 1.0.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <vector>

#include "Common/Status.h"
#include "JARVIS/Include/Base.h"
#include "JARVIS/Include/TypeDefinitions.h"



namespace muffin { namespace jarvis { namespace config {

    class ModbusRTU : public Base
    {
    public:
        ModbusRTU();
        virtual ~ModbusRTU() override;
    public:
        ModbusRTU& operator=(const ModbusRTU& obj);
        bool operator==(const ModbusRTU& obj) const;
        bool operator!=(const ModbusRTU& obj) const;
    public:
        void SetPort(const prt_e prt);
        void SetSlaveID(const uint8_t sid);
        void SetNodes(std::vector<std::string>&& nodes) noexcept;
    public:
        std::pair<Status, prt_e> GetPort() const;
        std::pair<Status, uint8_t> GetSlaveID() const;
        std::pair<Status, std::vector<std::string>> GetNodes() const;
    private:
        bool mIsNodesSet   = false;
        bool mIsPortSet    = false;
        bool mIsSlaveIdSet = false;
    private:
        std::vector<std::string> mNodes;
        prt_e mPort;
        uint8_t mSlaveID;
    };
}}}