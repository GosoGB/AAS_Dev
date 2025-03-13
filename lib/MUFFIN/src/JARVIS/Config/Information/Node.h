/**
 * @file Node.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Node 설정 형식을 표현하는 클래스를 선언합니다.
 * 
 * @date 2025-03-13
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include <map>
#include <vector>

#include "Common/Status.h"
#include "JARVIS/Include/Base.h"
#include "JARVIS/Include/DataUnitOrder.h"



namespace muffin { namespace jvs { namespace config {

    class Node : public Base
    {
    public:
        Node();
        virtual ~Node() override {}
    public:
        Node& operator=(const Node& obj);
        bool operator==(const Node& obj) const;
        bool operator!=(const Node& obj) const;
    public:
        void SetNodeID(const char* nodeID);
        void SetAddressType(const adtp_e type);
        void SetAddrress(const addr_u address);
        void SetModbusArea(const mb_area_e area);
        void SetBitIndex(const uint8_t index);
        void SetNumericAddressQuantity(const uint8_t quantity);
        void SetNumericScale(const scl_e scale);
        void SetNumericOffset(const float offset);
        void SetDataUnitOrders(const std::vector<DataUnitOrder>&& orders) noexcept;
        void SetDataTypes(const std::vector<dt_e>&& dt) noexcept;
        void SetFormatString(const std::string& format);
        void SetDeprecableUID(const char* uid);
        void SetAttributeEvent(const bool hasEvent);
    public:
        std::pair<Status, const char*> GetNodeID() const;
        std::pair<Status, adtp_e> GetAddressType() const;
        std::pair<Status, addr_u> GetAddrress() const;
        std::pair<Status, mb_area_e> GetModbusArea() const;
        std::pair<Status, uint8_t> GetBitIndex() const;
        std::pair<Status, uint8_t> GetNumericAddressQuantity() const;
        std::pair<Status, scl_e> GetNumericScale() const;
        std::pair<Status, float> GetNumericOffset() const;
        std::pair<Status, std::vector<DataUnitOrder>> GetDataUnitOrders() const;
        std::pair<Status, std::vector<dt_e>> GetDataTypes() const;
        std::pair<Status, std::string> GetFormatString() const;
        std::pair<Status, const char*> GetDeprecableUID() const;
        std::pair<Status, bool> GetAttributeEvent() const;
    private:
        /**
         * @todo bitset 구조로 변경 시 80개 노드 기준 1KB 가량 절감 가능
         */
        bool mIsNodeIdSet                   = false;
        bool mIsAddressTypeSet              = false;
        bool mIsAddressSet                  = false;
        bool mIsModbusAreaSet               = false;
        bool mIsBitIndexSet                 = false;
        bool mIsAddressQuantitySet          = false;
        bool mIsNumericScaleSet             = false;
        bool mIsNumericOffsetSet            = false;
        bool mIsDataUnitOrdersSet           = false;
        bool mIsDataTypesSet                = false;
        bool mIsFormatStringSet             = false;
        bool mIsDeprecableUidSet            = false;
        bool mIsAttributeEventSet           = false;
    private:
        char mNodeID[5];
        adtp_e mAddressType;
        addr_u mAddress;
        mb_area_e mModbusArea;
        uint8_t mBitIndex;
        uint8_t mAddressQuantity;
        scl_e mNumericScale;
        float mNumericOffset;
        std::vector<DataUnitOrder> mVectorDataUnitOrders;
        std::vector<dt_e> mVectorDataTypes;
        std::string mFormatString;
        char mDeprecableUID[5];
        bool mHasAttributeEvent;
    };
}}}
