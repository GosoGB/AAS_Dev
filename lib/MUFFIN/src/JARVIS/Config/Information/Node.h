/**
 * @file Node.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Node 설정 형식을 표현하는 클래스를 선언합니다.
 * 
 * @date 2024-10-08
 * @version 1.0.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#pragma once

#include <map>
#include <vector>

#include "Common/Status.h"
#include "JARVIS/Include/Base.h"
#include "JARVIS/Include/DataUnitOrder.h"



namespace muffin { namespace jarvis { namespace config {

    class Node : public Base
    {
    public:
        Node();
        virtual ~Node() override;
    public:
        Node& operator=(const Node& obj);
        bool operator==(const Node& obj) const;
        bool operator!=(const Node& obj) const;
    public:
        void SetNodeID(const std::string& nodeID);
        void SetAddressType(const adtp_e type);
        void SetAddrress(const addr_u address);
        void SetModbusArea(const mb_area_e area);
        void SetBitIndex(const uint8_t index);
        void SetNumericAddressQuantity(const uint8_t quantity);
        void SetNumericScale(const scl_e scale);
        void SetNumericOffset(const float offset);
        void SetMappingRules(const std::map<std::uint16_t, std::string>&& mappingRules) noexcept;
        void SetDataUnitOrders(const std::vector<DataUnitOrder>&& orders) noexcept;
        void SetDataTypes(const std::vector<dt_e>&& dt) noexcept;
        void SetFormatString(const std::string& format);
        void SetDeprecableUID(const std::string& uid);
        void SetDeprecableDisplayName(const std::string& displayName);
        void SetDeprecableDisplayUnit(const std::string& displayUnit);
        void SetAttributeEvent(const bool hasEvent);
    public:
        std::pair<Status, std::string> GetNodeID() const;
        std::pair<Status, adtp_e> GetAddressType() const;
        std::pair<Status, addr_u> GetAddrress() const;
        std::pair<Status, mb_area_e> GetModbusArea() const;
        std::pair<Status, uint8_t> GetBitIndex() const;
        std::pair<Status, uint8_t> GetNumericAddressQuantity() const;
        std::pair<Status, scl_e> GetNumericScale() const;
        std::pair<Status, float> GetNumericOffset() const;
        std::pair<Status, std::map<std::uint16_t, std::string>> GetMappingRules() const;
        std::pair<Status, std::vector<DataUnitOrder>> GetDataUnitOrders() const;
        std::pair<Status, std::vector<dt_e>> GetDataTypes() const;
        std::pair<Status, std::string> GetFormatString() const;
        std::pair<Status, std::string> GetDeprecableUID() const;
        std::pair<Status, std::string> GetDeprecableDisplayName() const;
        std::pair<Status, std::string> GetDeprecableDisplayUnit() const;
        std::pair<Status, bool> GetAttributeEvent() const;
    private:
        bool mIsNodeIdSet                   = false;
        bool mIsAddressTypeSet              = false;
        bool mIsAddressSet                  = false;
        bool mIsModbusAreaSet               = false;
        bool mIsBitIndexSet                 = false;
        bool mIsAddressQuantitySet          = false;
        bool mIsNumericScaleSet             = false;
        bool mIsNumericOffsetSet            = false;
        bool mIsMappingRulesSet             = false;
        bool mIsDataUnitOrdersSet           = false;
        bool mIsDataTypesSet                = false;
        bool mIsFormatStringSet             = false;
        bool mIsDeprecableUidSet            = false;
        bool mIsDeprecableDisplayNameSet    = false;
        bool mIsDeprecableDisplayUnitSet    = false;
        bool mIsAttributeEventSet           = false;
    private:
        std::string mNodeID;
        adtp_e mAddressType;
        addr_u mAddress;
        mb_area_e mModbusArea;
        uint8_t mBitIndex;
        uint8_t mAddressQuantity;
        scl_e mNumericScale;
        float mNumericOffset;
        std::map<std::uint16_t, std::string> mMapMappingRules;
        std::vector<DataUnitOrder> mVectorDataUnitOrders;
        std::vector<dt_e> mVectorDataTypes;
        std::string mFormatString;
        std::string mDeprecableUID;
        std::string mDeprecableDisplayName;
        std::string mDeprecableDisplayUnit;
        bool mHasAttributeEvent;
    };
}}}
