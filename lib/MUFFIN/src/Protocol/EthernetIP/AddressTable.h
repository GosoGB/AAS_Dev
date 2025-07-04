/**
 * @file AddressTable.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief Ethernet/IP 프로토콜 전용의 AddressTable을 정의합니다. 
 * 
 * @date 2025-07-01
 * @version 1.5.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2025
 */

#if defined(MT11)


#pragma once

#include <map>
#include <set>

#include "Common/Status.h"
#include "JARVIS/Include/TypeDefinitions.h"



namespace muffin { namespace ethernetIP {

    class AddressTable
    {
    public:
        AddressTable();
        virtual ~AddressTable();
    
  
    
    };



}}


#endif