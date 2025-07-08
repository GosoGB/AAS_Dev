/**
 * @file Node.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Node 설정 형식을 표현하는 클래스를 정의합니다.
 * 
 * @date 2025-03-13
 * @version 1.3.1
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include <string.h>

#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Node.h"



namespace muffin { namespace jvs { namespace config {

    Node::Node()
        : Base(cfg_key_e::NODE)
    {
        memset(mNodeID, '\0', sizeof(mNodeID));
    }

    Node& Node::operator=(const Node& obj)
    {
        if (this != &obj)
        {
            strncpy(mNodeID, obj.mNodeID, sizeof(mNodeID));

            mAddressType            = obj.mAddressType;
            mAddress                = obj.mAddress;
            mNodeArea               = obj.mNodeArea;
            mBitIndex               = obj.mBitIndex;
            mAddressQuantity        = obj.mAddressQuantity;
            mNumericScale           = obj.mNumericScale;
            mNumericOffset          = obj.mNumericOffset;
            mVectorDataUnitOrders   = obj.mVectorDataUnitOrders;
            mVectorDataTypes        = obj.mVectorDataTypes;
            mFormatString           = obj.mFormatString;
            mTopic                  = obj.mTopic;
            mHasAttributeEvent      = obj.mHasAttributeEvent;
            mArrayIndex             = obj.mArrayIndex;
        }
        
        return *this;
    }

    bool Node::operator==(const Node& obj) const
    {
       return (
            mNodeID                 == obj.mNodeID                  &&
            mAddressType            == obj.mAddressType             &&
            mAddress.Numeric        == obj.mAddress.Numeric         &&
            mNodeArea               == obj.mNodeArea                &&
            mBitIndex               == obj.mBitIndex                &&
            mAddressQuantity        == obj.mAddressQuantity         &&
            mNumericScale           == obj.mNumericScale            &&
            mNumericOffset          == obj.mNumericOffset           &&
            std::equal(mVectorDataUnitOrders.begin(), mVectorDataUnitOrders.end(), obj.mVectorDataUnitOrders.begin()) &&
            mVectorDataTypes        == obj.mVectorDataTypes         &&
            mFormatString           == obj.mFormatString            &&
            mTopic                  == obj.mTopic                   &&
            mHasAttributeEvent      == obj.mHasAttributeEvent       &&
            mArrayIndex             == obj.mArrayIndex
        );
    }

    bool Node::operator!=(const Node& obj) const
    {
        return !(*this == obj);
    }

    void Node::SetNodeID(const char* nodeID)
    {
        ASSERT((strlen(nodeID) == 4), "NODE ID MUST BE A STRING WITH LEGNTH OF 4");

        strncpy(mNodeID, nodeID, sizeof(mNodeID));
        mSetFlags.set(static_cast<uint8_t>(set_flag_e::NODE_ID));
    }

    void Node::SetAddressType(const adtp_e type)
    {
        ASSERT(
            (
                [&]()
                {
                    if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::NODE_AREA)) == true && type != adtp_e::NUMERIC)
                    {
                        return false;
                    }
                    else
                    {
                        return true;
                    }
                }()
            ), "ADDRESS TYPE MUST BE NUMERIC WHEN MODBUS PROTOCOL IS USED"
        );

        mAddressType = type;
        mSetFlags.set(static_cast<uint8_t>(set_flag_e::ADDRESS_TYPE));
    }

    void Node::SetAddrress(const addr_u address)
    {
        ASSERT((mSetFlags.test(static_cast<uint8_t>(set_flag_e::ADDRESS_TYPE)) == true), "ADDRESS TYPE MUST BE SET BEFOREHAND");

        mAddress = address;
        mSetFlags.set(static_cast<uint8_t>(set_flag_e::ADDRESS));
    }

    void Node::SetNodeArea(const node_area_e area)
    {
        ASSERT((mSetFlags.test(static_cast<uint8_t>(set_flag_e::ADDRESS_TYPE)) == true), "ADDRESS TYPE MUST BE SET BEFOREHAND");
        ASSERT((mAddressType == adtp_e::NUMERIC), "ADDRESS TYPE MUST BE SET TO NUMERIC");

        mNodeArea = area;
        mSetFlags.set(static_cast<uint8_t>(set_flag_e::NODE_AREA));
    }

    void Node::SetBitIndex(const uint8_t index)
    {
        // 데이터 타입 유효성 검사
        ASSERT((mSetFlags.test(static_cast<uint8_t>(set_flag_e::DATA_TYPE)) == true), "DATA TYPE MUST BE SET BEFOREHAND");
        ASSERT((mVectorDataTypes.size() == 1), "BIT INDEX CAN ONLY BE APPLIED WHEN THERE IS ONLY ONE DATA TYPE");
        ASSERT(
            (
                [&]()
                {
                    if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::DATA_UNIT_ORDERS)) == true)
                    {
                        if (mVectorDataUnitOrders.size() != 1)
                        {
                            return false;
                        }
                    }
                    return true;
                }()
            ), "BIT INDEX CAN ONLY BE APPLIED WHEN THERE IS ONLY ONE ALIGNMENT ORDER"
        );

        // 함께 설정 불가능한 Node 속성에 대한 유효성 검사
        ASSERT((mSetFlags.test(static_cast<uint8_t>(set_flag_e::ADDRESS_QUANTITY)) == false), "BIT INDEX CAN ONLY BE APPLIED WHEN ADDRESS QUANTITY IS DISABLED");
        ASSERT((mSetFlags.test(static_cast<uint8_t>(set_flag_e::NUMERIC_SCALE))    == false), "BIT INDEX CAN ONLY BE APPLIED WHEN NUMERIC SCALING IS DISABLED");
        ASSERT((mSetFlags.test(static_cast<uint8_t>(set_flag_e::NUMERIC_OFFSET))   == false), "BIT INDEX CAN ONLY BE APPLIED WHEN NUMERIC OFFSET IS DISABLED");
        ASSERT((mSetFlags.test(static_cast<uint8_t>(set_flag_e::FORMAT_STRING))    == false), "BIT INDEX CAN ONLY BE APPLIED WHEN FORMAT STRING IS DISABLED");

        // Modbus 메모리 영역 유효성 검사
        ASSERT(
            (
                [&]()
                {
                    if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::NODE_AREA)) == true)
                    {// node_area_e 기본 값은 COILS이므로 설정됐는지 여부에 대한 검사가 선결조건임
                        if (mNodeArea == node_area_e::COILS || mNodeArea == node_area_e::DISCRETE_INPUT)
                        {
                            return false;
                        }
                    }
                    return true;
                }()
            ), "BIT INDEX CANNOT BE SET WHEN MODBUS MEMORY AREA IS ALREADY SET TO BIT"
        );

        // 비트 인덱스의 범위 유효성 검사
        ASSERT(
            (
                [&]()
                {
                    switch (mVectorDataTypes.front())
                    {
                    case dt_e::INT8:
                    case dt_e::UINT8:
                        if (index > 7)
                        {
                            return false;
                        }
                        else
                        {
                            return true;
                        }
                    case dt_e::INT16:
                    case dt_e::UINT16:
                        if (index > 15)
                        {
                            return false;
                        }
                        else
                        {
                            return true;
                        }
                    case dt_e::INT32:
                    case dt_e::UINT32:
                        if (index > 31)
                        {
                            return false;
                        }
                        else
                        {
                            return true;
                        }
                    case dt_e::INT64:
                    case dt_e::UINT64:
                        if (index > 63)
                        {
                            return false;
                        }
                        else
                        {
                            return true;
                        }
                    default:
                        return false;
                    }
                }()
            ), "BIT INDEX OUT OF RANGE OR INVALID DATA TYPE"
        );
        
        mBitIndex = index;
        mSetFlags.set(static_cast<uint8_t>(set_flag_e::BIT_INDEX));
    }

    void Node::SetNumericAddressQuantity(const uint8_t quantity)
    {
        ASSERT((mSetFlags.test(static_cast<uint8_t>(set_flag_e::ADDRESS_TYPE)) == true), "ADDRESS TYPE MUST BE SET BEFOREHAND");
        ASSERT((mAddressType == adtp_e::NUMERIC), "ADDRESS TYPE MUST BE NUMERIC");
        ASSERT((mSetFlags.test(static_cast<uint8_t>(set_flag_e::BIT_INDEX)) == false), "NUMERIC ADDRESS QUANTITY CANNOT BE SET WHEN BIT INDEX IS ENABLED");
        ASSERT(
            (
                [&]()
                {
                    if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::NODE_AREA)) == true)
                    {// node_area_e 기본 값은 COILS이므로 설정됐는지 여부에 대한 검사가 선결조건임
                        if (mNodeArea == node_area_e::COILS || mNodeArea == node_area_e::DISCRETE_INPUT)
                        {
                            return false;
                        }
                    }
                    return true;
                }()
            ), "NUMERIC ADDRESS QUANTITY CAN ONLY BE SET WHEN MODBUS MEMORY AREA IS SET TO REGISTERS"
        );
        ASSERT((quantity != 0), "NUMERIC ADDRESS QUANTITY CANNOT BE SET TO 0");

        mAddressQuantity = quantity;
        mSetFlags.set(static_cast<uint8_t>(set_flag_e::ADDRESS_QUANTITY));
    }

    void Node::SetNumericScale(const scl_e scale)
    {
        ASSERT((mSetFlags.test(static_cast<uint8_t>(set_flag_e::DATA_TYPE)) == true), "DATA TYPE MUST BE SET BEFOREHAND");
        ASSERT((mVectorDataTypes.size() == 1), "NUMERIC SCALE CAN ONLY BE APPLIED WHEN THERE IS ONLY ONE DATA TYPE");
        ASSERT(
            (
                [&]()
                {
                    switch (mVectorDataTypes.front())
                    {
                    case dt_e::STRING:
                    case dt_e::BOOLEAN:
                        return false;
                    default:
                        return true;
                    }
                }()
            ), "NUMERIC SCALE CANNOT BE APPLIED TO DATA WHICH IS STRING OR BOOLEAN TYPE"
        );
        ASSERT((mSetFlags.test(static_cast<uint8_t>(set_flag_e::BIT_INDEX)) == false), "NUMERIC SCALE CANNOT BE SET WHEN BIT INDEX IS ENABLED");

        mNumericScale = scale;
        mSetFlags.set(static_cast<uint8_t>(set_flag_e::NUMERIC_SCALE));
    }

    void Node::SetNumericOffset(const float offset)
    {
        ASSERT((mSetFlags.test(static_cast<uint8_t>(set_flag_e::DATA_TYPE)) == true), "DATA TYPE MUST BE SET BEFOREHAND");
        ASSERT((mVectorDataTypes.size() == 1), "NUMERIC OFFSET CAN ONLY BE APPLIED WHEN THERE IS ONLY ONE DATA TYPE");
        ASSERT(
            (
                [&]()
                {
                    switch (mVectorDataTypes.front())
                    {
                    case dt_e::STRING:
                    case dt_e::BOOLEAN:
                        return false;
                    default:
                        return true;
                    }
                }()
            ), "NUMERIC OFFSET CANNOT BE APPLIED TO DATA WHICH IS STRING OR BOOLEAN TYPE"
        );
        ASSERT((mSetFlags.test(static_cast<uint8_t>(set_flag_e::BIT_INDEX)) == false), "NUMERIC OFFSET CANNOT BE SET WHEN BIT INDEX IS ENABLED");

        mNumericOffset = offset;
        mSetFlags.set(static_cast<uint8_t>(set_flag_e::NUMERIC_OFFSET));
    }

    void Node::SetDataUnitOrders(const std::vector<DataUnitOrder>&& orders) noexcept
    {
        ASSERT((mSetFlags.test(static_cast<uint8_t>(set_flag_e::DATA_TYPE)) == true), "DATA TYPE MUST BE SET BEFOREHAND");
        ASSERT((mVectorDataTypes.size() > 0), "DATA UNIT ORDERS CANNOT BE APPLIED WHEN THERE IS NO DATA TYPE");
        ASSERT(
            (
                [&]()
                {
                    if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::NODE_AREA)) == true)
                    {// node_area_e 기본 값은 COILS이므로 설정됐는지 여부에 대한 검사가 선결조건임
                        if (mNodeArea == node_area_e::COILS || mNodeArea == node_area_e::DISCRETE_INPUT)
                        {
                            return false;
                        }
                    }
                    return true;
                }()
            ), "DATA UNIT ORDERS CAN ONLY BE SET WHEN MODBUS MEMORY AREA IS SET TO REGISTERS"
        );
        ASSERT((orders.size() == mVectorDataTypes.size()), "DATA UNIT ORDERS AND DATA TYPES MUST BE EQUAL IN LENGTH");

        mVectorDataUnitOrders = std::move(orders);
        mSetFlags.set(static_cast<uint8_t>(set_flag_e::DATA_UNIT_ORDERS));
    }

    void Node::SetDataTypes(const std::vector<dt_e>&& dt) noexcept
    {
        ASSERT(
            (
                [&]()
                {
                    if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::NODE_AREA)) == true)
                    {// node_area_e 기본 값은 COILS이므로 설정됐는지 여부에 대한 검사가 선결조건임
                        if (mNodeArea == node_area_e::COILS || mNodeArea == node_area_e::DISCRETE_INPUT)
                        {
                            if (dt.size() != 1)
                            {
                                return false;
                            }
                            else if (dt.front() != dt_e::BOOLEAN)
                            {
                                return false;
                            }
                        }
                    }
                    return true;
                }()
            ), "DATA TYPES CAN ONLY BE SET WHEN MODBUS MEMORY AREA IS SET TO REGISTERS"
        );
        ASSERT((mSetFlags.test(static_cast<uint8_t>(set_flag_e::BIT_INDEX)) == false), "DATA TYPES CANNOT BE SET WHEN BIT INDEX IS ENABLED");

        mVectorDataTypes = std::move(dt);
        mSetFlags.set(static_cast<uint8_t>(set_flag_e::DATA_TYPE));
    }

    void Node::SetFormatString(const std::string& format)
    {
        // 속성 간 의존성 검사
        ASSERT((mSetFlags.test(static_cast<uint8_t>(set_flag_e::BIT_INDEX)) == false), "FORMAT STRING CANNOT BE SET WHEN BIT INDEX IS ENABLED");
        ASSERT((mSetFlags.test(static_cast<uint8_t>(set_flag_e::DATA_TYPE)) == true), "DATA TYPE MUST BE SET BEFOREHAND");
        ASSERT((mVectorDataTypes.size() > 0), "DATA TYPE CANNOT BE EMPTY ARRAY");
        
        // 데이터 타입 유효성 검사
        ASSERT(
            (
                [&]()
                {
                    if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::NODE_AREA)) == true)
                    {// node_area_e 기본 값은 COILS이므로 설정됐는지 여부에 대한 검사가 선결조건임
                        if (mNodeArea == node_area_e::COILS || mNodeArea == node_area_e::DISCRETE_INPUT)
                        {
                            return false;
                        }
                    }
                    return true;
                }()
            ), "FORMAT STRING CAN ONLY BE SET WHEN MODBUS MEMORY AREA IS SET TO REGISTERS"
        );
        ASSERT(
            (
                [&]()
                {
                    for (const auto& dataType : mVectorDataTypes)
                    {
                        switch (dataType)
                        {
                        case dt_e::BOOLEAN:
                            return false;
                        default:
                            return true;
                        }
                    }

                    return true;
                }()
            ), "FORMAT STRING CANNOT BE APPLIED TO DATA WHICH IS BOOLEAN TYPE"
        );

        // format 문자열 유효성 검사
        ASSERT(
            (
                [&]()
                {
                    const char* formatString = format.c_str();
                    uint8_t index = 0;

                    while (*formatString)
                    {
                        if (*formatString == '%')
                        {
                            ++formatString;

                            while (isdigit(*formatString) || *formatString == '.' || *formatString == 'l')
                            {// Check for width and precision modifiers
                                ++formatString;
                            }

                              // Check for valid format specifiers
                            if (*formatString == 'd')
                            {
                                dt_e dt = mVectorDataTypes[index];
                                ++index;

                                if (dt != dt_e::INT8 && dt != dt_e::INT16 && dt != dt_e::INT32)
                                {
                                    return false;
                                }
                            }
                            else if (*formatString == 'u')
                            {
                                dt_e dt = mVectorDataTypes[index];
                                ++index;

                                if (dt != dt_e::UINT8 && dt != dt_e::UINT16 && dt != dt_e::UINT32)
                                {
                                    return false;
                                }
                            }
                            else if (*formatString == 'f')
                            {
                                dt_e dt = mVectorDataTypes[index];
                                ++index;

                                if (dt != dt_e::FLOAT32)
                                {
                                    return false;
                                }
                            }
                            else if (*formatString == 's' || *formatString == 'c')
                            {
                                dt_e dt = mVectorDataTypes[index];
                                ++index;

                                if (dt != dt_e::STRING)
                                {
                                    return false;
                                }
                            }
                            else if (*formatString == 'x' || *formatString == 'X')
                            {
                                dt_e dt = mVectorDataTypes[index];
                                ++index;

                                if (dt != dt_e::INT8 && dt != dt_e::INT16 && dt != dt_e::INT32 &&
                                    dt != dt_e::UINT8 && dt != dt_e::UINT16 && dt != dt_e::UINT32)
                                {
                                    return false;
                                }
                            }
                            else
                            {
                                return false;
                            }
                            
                            ++formatString;
                        }
                        else
                        {
                            ++formatString;
                        }
                    }
                    return true;
                }()
            ), "INVALID FORMAT STRING: %s", format.c_str()
        );
        ASSERT((format.size() != 0), "FORMAT STRING CANNOT BE AN EMPTY STRING");

        mFormatString = format;
        mSetFlags.set(static_cast<uint8_t>(set_flag_e::FORMAT_STRING));
    }

    void Node::SetTopic(const mqtt::topic_e topic)
    {
        mTopic = topic;
        mSetFlags.set(static_cast<uint8_t>(set_flag_e::TOPIC));
    }

    void Node::SetArrayIndex(const std::vector<std::array<uint16_t, 2>> arrayindex)
    {
        mArrayIndex = arrayindex;
        mSetFlags.set(static_cast<uint8_t>(set_flag_e::ARRAY_INDEX));
    }

    void Node::SetAttributeEvent(const bool hasEvent)
    {
        mHasAttributeEvent = hasEvent;
        mSetFlags.set(static_cast<uint8_t>(set_flag_e::ATTRIBUTE_EVENT));
    }

    std::pair<Status, const char*> Node::GetNodeID() const
    {
        if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::NODE_ID)) == true)
        {
            return std::make_pair(Status(Status::Code::GOOD), mNodeID);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mNodeID);
        }
    }

    std::pair<Status, adtp_e> Node::GetAddressType() const
    {
        if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::ADDRESS_TYPE)) == true)
        {
            return std::make_pair(Status(Status::Code::GOOD), mAddressType);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mAddressType);
        }
    }

    std::pair<Status, addr_u> Node::GetAddrress() const
    {
        if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::ADDRESS)) == true)
        {
            return std::make_pair(Status(Status::Code::GOOD), mAddress);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mAddress);
        }
    }

    std::pair<Status, node_area_e> Node::GetNodeArea() const
    {
        if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::NODE_AREA)) == true)
        {
            return std::make_pair(Status(Status::Code::GOOD), mNodeArea);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mNodeArea);
        }
    }

    std::pair<Status, uint8_t> Node::GetBitIndex() const
    {
        if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::BIT_INDEX)) == true)
        {
            return std::make_pair(Status(Status::Code::GOOD), mBitIndex);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mBitIndex);
        }
    }

    std::pair<Status, uint8_t> Node::GetNumericAddressQuantity() const
    {
        if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::ADDRESS_QUANTITY)) == true)
        {
            return std::make_pair(Status(Status::Code::GOOD), mAddressQuantity);
        }
        else if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::BIT_INDEX)) == true)
        {
            return std::make_pair(Status(Status::Code::GOOD_NO_DATA), 1);
        }
        else
        {
            switch (GetNodeArea().second)
            {
            case node_area_e::COILS:
            case node_area_e::DISCRETE_INPUT:
            case node_area_e::SM:
            case node_area_e::X:
            case node_area_e::Y:
            case node_area_e::M:
            case node_area_e::L:
            case node_area_e::F:
            case node_area_e::V:
            case node_area_e::B:
            case node_area_e::TS:
            case node_area_e::TC:
            case node_area_e::LTS:
            case node_area_e::LTC:
            case node_area_e::STS:
            case node_area_e::STC:
            case node_area_e::LSTS:
            case node_area_e::LSTC:
            case node_area_e::CS:
            case node_area_e::CC:
            case node_area_e::LCS:
            case node_area_e::LCC:
            case node_area_e::SB:
            case node_area_e::S:
            case node_area_e::DX:
            case node_area_e::DY:
                return std::make_pair(Status(Status::Code::GOOD_NO_DATA), 1);
            default:
                break;
            }
        }
        return std::make_pair(Status(Status::Code::BAD), mAddressQuantity);
    }

    std::pair<Status, scl_e> Node::GetNumericScale() const
    {
        if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::NUMERIC_SCALE)) == true)
        {
            return std::make_pair(Status(Status::Code::GOOD), mNumericScale);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mNumericScale);
        }
    }

    std::pair<Status, float> Node::GetNumericOffset() const
    {
        if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::NUMERIC_OFFSET)) == true)
        {
            return std::make_pair(Status(Status::Code::GOOD), mNumericOffset);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mNumericOffset);
        }
    }

    std::pair<Status, std::vector<DataUnitOrder>> Node::GetDataUnitOrders() const
    {
        if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::DATA_UNIT_ORDERS)) == true)
        {
            return std::make_pair(Status(Status::Code::GOOD), mVectorDataUnitOrders);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mVectorDataUnitOrders);
        }
    }

    std::pair<Status, std::vector<dt_e>> Node::GetDataTypes() const
    {
        if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::DATA_TYPE)) == true)
        {
            return std::make_pair(Status(Status::Code::GOOD), mVectorDataTypes);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mVectorDataTypes);
        }
    }

    std::pair<Status, std::string> Node::GetFormatString() const
    {
        if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::FORMAT_STRING)) == true)
        {
            return std::make_pair(Status(Status::Code::GOOD), mFormatString);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mFormatString);
        }
    }

    std::pair<Status, bool> Node::GetAttributeEvent() const
    {
        if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::ATTRIBUTE_EVENT)) == true)
        {
            return std::make_pair(Status(Status::Code::GOOD), mHasAttributeEvent);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mHasAttributeEvent);
        }
    }

    std::pair<Status, mqtt::topic_e> Node::GetTopic() const
    {
        if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::TOPIC)) == true)
        {
            return std::make_pair(Status(Status::Code::GOOD), mTopic);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mTopic);
        }
    }

    std::pair<Status, std::vector<std::array<uint16_t, 2>>> Node::GetArrayIndex() const
    {
        if (mSetFlags.test(static_cast<uint8_t>(set_flag_e::ARRAY_INDEX)) == true)
        {
            return std::make_pair(Status(Status::Code::GOOD), mArrayIndex);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mArrayIndex);
        }
    }
}}}