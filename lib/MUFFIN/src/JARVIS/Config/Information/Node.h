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

#include "Common/DataStructure/bitset.h"
#include "Common/Status.h"
#include "JARVIS/Include/Base.h"
#include "JARVIS/Include/DataUnitOrder.h"
#include "Protocol/MQTT/Include/TypeDefinitions.h"



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
        void SetNodeArea(const node_area_e area);
        void SetBitIndex(const uint8_t index);
        void SetNumericAddressQuantity(const uint8_t quantity);
        void SetNumericScale(const scl_e scale);
        void SetNumericOffset(const float offset);
        void SetDataUnitOrders(const std::vector<DataUnitOrder>&& orders) noexcept;
        void SetDataTypes(const std::vector<dt_e>&& dt) noexcept;
        void SetFormatString(const std::string& format);
        void SetAttributeEvent(const bool hasEvent);
        void SetTopic(const mqtt::topic_e topic);
    
    public:
        std::pair<Status, mqtt::topic_e> GetTopic() const;
        std::pair<Status, const char*> GetNodeID() const;
        std::pair<Status, adtp_e> GetAddressType() const;
        std::pair<Status, addr_u> GetAddrress() const;
        std::pair<Status, node_area_e> GetNodeArea() const;
        std::pair<Status, uint8_t> GetBitIndex() const;
        std::pair<Status, uint8_t> GetNumericAddressQuantity() const;
        std::pair<Status, scl_e> GetNumericScale() const;
        std::pair<Status, float> GetNumericOffset() const;
        std::pair<Status, std::vector<DataUnitOrder>> GetDataUnitOrders() const;
        std::pair<Status, std::vector<dt_e>> GetDataTypes() const;
        std::pair<Status, std::string> GetFormatString() const;
        std::pair<Status, bool> GetAttributeEvent() const;
    private:
        typedef enum class SetFlagEnum : uint8_t
        {
            NODE_ID             =  0,
            ADDRESS_TYPE        =  1,
            ADDRESS             =  2,
            NODE_AREA           =  3,
            BIT_INDEX           =  4,
            ADDRESS_QUANTITY    =  5,
            NUMERIC_SCALE       =  6,
            NUMERIC_OFFSET      =  7,
            DATA_UNIT_ORDERS    =  8,
            DATA_TYPE           =  9,
            FORMAT_STRING       = 10,
            TOPIC               = 11,
            ATTRIBUTE_EVENT     = 12,
            TOP                 = 13
        } set_flag_e;
        bitset<static_cast<uint8_t>(set_flag_e::TOP)> mSetFlags;
    private:
        char mNodeID[5];
        adtp_e mAddressType;
        addr_u mAddress;
        node_area_e mNodeArea;
        uint8_t mBitIndex;
        uint8_t mAddressQuantity;
        scl_e mNumericScale;
        float mNumericOffset;
        std::vector<DataUnitOrder> mVectorDataUnitOrders;
        std::vector<dt_e> mVectorDataTypes;
        std::string mFormatString;
        mqtt::topic_e mTopic;
        bool mHasAttributeEvent;
    };
}}}
