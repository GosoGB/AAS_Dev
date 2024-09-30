/**
 * @file ModbusRTU.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Modbus RTU 프로토콜 클래스를 선언합니다.
 * 
 * @date 2024-09-28
 * @version 0.0.1
 * 
 * 
 * @todo merge 할 수 없는 주소들은 set에 추가되지 않는 버그가 있습니다.
 *       Modbus 주소만 따로 관리하는 클래스를 추가하는 게 좋아보입니다.
 *       기본적으로 메모리 영역별로 주소 값을 관리하는 것이 좋아보입니다.
 *       또한 향후 여러 슬레이브를 사용할 때를 대비하는 게 좋겠습니다.
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <vector>

#include "Common/Status.h"
#include "Include/TypeDefinitions.h"
#include "Include/AddressTable.h"
#include "IM/Node/Node.h"



namespace muffin {

    class ModbusRTU
    {
    public:
        ModbusRTU(/*포트 번호, 슬레이브 번호를 받아야 합니다.*/);
        virtual ~ModbusRTU();
    public:
        Status AddNodeReference(im::Node* node);
        Status RemoveReferece(const std::string& nodeID);
    private:
        std::vector<im::Node*> mNodeReferences;
    };
}