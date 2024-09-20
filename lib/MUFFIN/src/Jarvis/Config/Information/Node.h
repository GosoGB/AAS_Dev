/**
 * @file Node.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-09-20
 * 
 * @copyright Copyright (c) 2024
 * 
 */




#pragma once

#include <map>
#include <vector>

#include "Common/Status.h"
#include "Jarvis/Include/Base.h"



namespace muffin { namespace jarvis { namespace config {


    class Node : public Base
    {
    public:
        Node(const std::string& key);
        virtual ~Node() override;
    public:
        Node& operator=(const Node& obj);
        bool operator==(const Node& obj) const;
        bool operator!=(const Node& obj) const;
    
    public:
        Status SetNodeID(const std::string& nodeid);
        Status SetAddressType(const uint8_t& adtp);
        // Status SetAddrress(const std::string& addr);
        Status SetAddrress(const uint16_t& addr);
        Status SetModbusArea(const uint8_t& area);
        Status SetBitIndex(const uint8_t& bit);
        Status SetModbusRegisterQuantity(const uint8_t& qty);
        Status SetDataScale(const int8_t& scl);
        Status SetDataOffset(const float& ofst);
        Status SetMappingRules(const std::map<std::uint16_t, std::string>& map);
        Status SetDataOrder(const std::vector<std::string>& ord);
        Status SetDataTypes(const std::vector<uint8_t>& dt);
        Status SetFormatString(const std::string& fmt);
        Status SetDataName(const std::string& name);
        Status SetDataUnit(const std::string& unit);
        Status SetUID(const std::string& uid);
        Status SetPID(const std::string& pid);
    
    public:
        const std::string& GetNodeID() const;
        const uint8_t& GetAddressType() const;
        // const std::string& GetStrAddrress() const;
        const uint16_t& GetUint16Addrress() const;
        const uint8_t& GetModbusArea() const;
        const uint8_t& GetBitIndex() const;
        const uint8_t& GetModbusRegisterQuantity() const;

        const int8_t& GetDataScale() const;
        const float& GetDataOffset() const;
        const std::vector<std::string>& GetDataOrder() const;
        const std::vector<uint8_t>& GetDataTypes() const;
        const std::string& GetFormatString() const;
        const std::map<std::uint16_t, std::string>& GetMappingRules() const;

        const std::string& GetDataName() const;
        const std::string& GetDataUnit() const;
        const std::string& GetUID() const;
        const std::string& GetPID() const;

    public:
        Status HasModbusArea(bool* hasArea) const;
        Status HasBitIndex(bool* hasBit) const;
        Status HasModbusRegisterQuantity(bool* hasQty) const;
        Status HasDataScale(bool* hasScl) const;
        Status HasDataOffset(bool* hasOfst) const;
        Status HasDataOrder(bool* hasOrd) const;
        Status HasDataTypes(bool* hasDt) const;
        Status HasFormatString(bool* hasFmt) const;
        Status HasMappingRules(bool* hasMap) const;
        Status HasDataName(bool* hasName) const;
        Status HasDataUnit(bool* hasUnit) const;

    private:
        bool mIsAddrStr    = false;
        bool mIsNodeIdSet  = false;
        bool mIsAddrSet    = false;
        bool mIsAreaSet    = false;
        bool mIsBitSet     = false;
        bool mIsQtySet     = false;
        bool mIsSclSet     = false;
        bool mIsOfstSet    = false;
        bool mIsOrdSet     = false;
        bool mIsDtSet      = false;
        bool mIsFmtSet     = false;
        bool mIsMapSet     = false;
        bool mIsNameSet    = false;
        bool mIsUnitSet    = false;
        
    private:
        std::map<std::uint16_t, std::string> mMap;
        std::vector<std::string> mOrd;
        std::vector<uint8_t> mDt;
        std::string mStrAddr;
        uint16_t mUintAddr;
        uint8_t mAddrType  = 0;
        uint8_t mBit;
        uint8_t mArea      = 0;
        uint8_t mQty       = 0;
        int8_t mScl        = 0;
        float mOffset      = 0;
        std::string mNodeId;
        std::string mFmt;
        std::string mName;
        std::string mUnit;
        std::string mUID;
        std::string mPID;
    };
}}}
