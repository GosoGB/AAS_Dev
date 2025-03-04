/**
 * @file Node.cpp
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Node 설정 형식을 표현하는 클래스를 정의합니다.
 * 
 * @date 2024-10-14
 * @version 1.0.0
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Node.h"



namespace muffin { namespace jarvis { namespace config {

    Node::Node()
        : Base(cfg_key_e::NODE)
    {
    }

    Node::~Node()
    {
    }

    Node& Node::operator=(const Node& obj)
    {
        if (this != &obj)
        {
            mNodeID                 = obj.mNodeID;
            mAddressType            = obj.mAddressType;
            mAddress                = obj.mAddress;
            mModbusArea             = obj.mModbusArea;
            mBitIndex               = obj.mBitIndex;
            mAddressQuantity        = obj.mAddressQuantity;
            mNumericScale           = obj.mNumericScale;
            mNumericOffset          = obj.mNumericOffset;
            mMapMappingRules        = obj.mMapMappingRules;
            mVectorDataUnitOrders   = obj.mVectorDataUnitOrders;
            mVectorDataTypes        = obj.mVectorDataTypes;
            mFormatString           = obj.mFormatString;
            mDeprecableUID          = obj.mDeprecableUID;
            mDeprecableDisplayName  = obj.mDeprecableDisplayName;
            mDeprecableDisplayUnit  = obj.mDeprecableDisplayUnit;
            mHasAttributeEvent      = obj.mHasAttributeEvent;
        }
        
        return *this;
    }

    bool Node::operator==(const Node& obj) const
    {
       return (
            mNodeID                 == obj.mNodeID                  &&
            mAddressType            == obj.mAddressType             &&
            mAddress.Numeric        == obj.mAddress.Numeric         &&
            mModbusArea             == obj.mModbusArea              &&
            mBitIndex               == obj.mBitIndex                &&
            mAddressQuantity        == obj.mAddressQuantity         &&
            mNumericScale           == obj.mNumericScale            &&
            mNumericOffset          == obj.mNumericOffset           &&
            mMapMappingRules        == obj.mMapMappingRules         &&
            std::equal(mVectorDataUnitOrders.begin(), mVectorDataUnitOrders.end(), obj.mVectorDataUnitOrders.begin()) &&
            mVectorDataTypes        == obj.mVectorDataTypes         &&
            mFormatString           == obj.mFormatString            &&
            mDeprecableUID          == obj.mDeprecableUID           &&
            mDeprecableDisplayName  == obj.mDeprecableDisplayName   &&
            mDeprecableDisplayUnit  == obj.mDeprecableDisplayUnit   &&
            mHasAttributeEvent      == obj.mHasAttributeEvent
        );
    }

    bool Node::operator!=(const Node& obj) const
    {
        return !(*this == obj);
    }

    void Node::SetNodeID(const std::string& nodeID)
    {
        ASSERT((nodeID.size() == 4), "NODE ID MUST BE A STRING WITH LEGNTH OF 4");

        mNodeID = nodeID;
        mIsNodeIdSet = true;
    }

    void Node::SetAddressType(const adtp_e type)
    {
        ASSERT(
            (
                [&]()
                {
                    if (mIsModbusAreaSet == true && type != adtp_e::NUMERIC)
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
        mIsAddressTypeSet = true;
    }

    void Node::SetAddrress(const addr_u address)
    {
        ASSERT((mIsAddressTypeSet == true), "ADDRESS TYPE MUST BE SET BEFOREHAND");

        mAddress = address;
        mIsAddressSet = true;
    }

    void Node::SetModbusArea(const mb_area_e area)
    {
        ASSERT((mIsAddressTypeSet == true), "ADDRESS TYPE MUST BE SET BEFOREHAND");
        ASSERT((mAddressType == adtp_e::NUMERIC), "ADDRESS TYPE MUST BE SET TO NUMERIC");

        mModbusArea = area;
        mIsModbusAreaSet = true;
    }

    void Node::SetBitIndex(const uint8_t index)
    {
        // 데이터 타입 유효성 검사
        ASSERT((mIsDataTypesSet == true), "DATA TYPE MUST BE SET BEFOREHAND");
        ASSERT((mVectorDataTypes.size() == 1), "BIT INDEX CAN ONLY BE APPLIED WHEN THERE IS ONLY ONE DATA TYPE");
        ASSERT(
            (
                [&]()
                {
                    if (mIsDataUnitOrdersSet == true)
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
        ASSERT((mIsAddressQuantitySet  == false), "BIT INDEX CAN ONLY BE APPLIED WHEN ADDRESS QUANTITY IS DISABLED");
        ASSERT((mIsNumericScaleSet     == false), "BIT INDEX CAN ONLY BE APPLIED WHEN NUMERIC SCALING IS DISABLED");
        ASSERT((mIsNumericOffsetSet    == false), "BIT INDEX CAN ONLY BE APPLIED WHEN NUMERIC OFFSET IS DISABLED");
        ASSERT((mIsFormatStringSet     == false), "BIT INDEX CAN ONLY BE APPLIED WHEN FORMAT STRING IS DISABLED");

        // Modbus 메모리 영역 유효성 검사
        ASSERT(
            (
                [&]()
                {
                    if (mIsModbusAreaSet == true)
                    {// mb_area_e 기본 값은 COILS이므로 설정됐는지 여부에 대한 검사가 선결조건임
                        if (mModbusArea == mb_area_e::COILS || mModbusArea == mb_area_e::DISCRETE_INPUT)
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
        mIsBitIndexSet = true;
    }

    void Node::SetNumericAddressQuantity(const uint8_t quantity)
    {
        ASSERT((mIsAddressTypeSet == true), "ADDRESS TYPE MUST BE SET BEFOREHAND");
        ASSERT((mAddressType == adtp_e::NUMERIC), "ADDRESS TYPE MUST BE NUMERIC");
        ASSERT((mIsBitIndexSet == false), "NUMERIC ADDRESS QUANTITY CANNOT BE SET WHEN BIT INDEX IS ENABLED");
        ASSERT(
            (
                [&]()
                {
                    if (mIsModbusAreaSet == true)
                    {// mb_area_e 기본 값은 COILS이므로 설정됐는지 여부에 대한 검사가 선결조건임
                        if (mModbusArea == mb_area_e::COILS || mModbusArea == mb_area_e::DISCRETE_INPUT)
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
        mIsAddressQuantitySet = true;
    }

    void Node::SetNumericScale(const scl_e scale)
    {
        ASSERT((mIsDataTypesSet == true), "DATA TYPE MUST BE SET BEFOREHAND");
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
        ASSERT((mIsBitIndexSet == false), "NUMERIC SCALE CANNOT BE SET WHEN BIT INDEX IS ENABLED");

        mNumericScale = scale;
        mIsNumericScaleSet = true;
    }

    void Node::SetNumericOffset(const float offset)
    {
        ASSERT((mIsDataTypesSet == true), "DATA TYPE MUST BE SET BEFOREHAND");
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
        ASSERT((mIsBitIndexSet == false), "NUMERIC OFFSET CANNOT BE SET WHEN BIT INDEX IS ENABLED");

        mNumericOffset = offset;
        mIsNumericOffsetSet = true;
    }

    void Node::SetMappingRules(const std::map<std::uint16_t, std::string>&& mappingRules) noexcept
    {
        ASSERT((mIsDataTypesSet == true), "DATA TYPE MUST BE SET BEFOREHAND");
        ASSERT((mVectorDataTypes.size() == 1), "MAPPING RULES CAN ONLY BE APPLIED WHEN THERE IS ONLY ONE DATA TYPE");
        ASSERT(
            (
                [&]()
                {
                    switch (mVectorDataTypes.front())
                    {
                    case dt_e::STRING:
                    case dt_e::FLOAT32:
                    case dt_e::FLOAT64:
                        return false;
                    default:
                        return true;
                    }
                }()
            ), "MAPPING RULES CANNOT BE APPLIED TO DATA WHICH IS STRING, FP32 OR FP64 TYPE"
        );
        ASSERT((mappingRules.size() > 0), "INVALID MAPPING RULES: NO RULE AT ALL");

        mMapMappingRules = std::move(mappingRules);
        mIsMappingRulesSet = true;
    }

    void Node::SetDataUnitOrders(const std::vector<DataUnitOrder>&& orders) noexcept
    {
        ASSERT((mIsDataTypesSet == true), "DATA TYPE MUST BE SET BEFOREHAND");
        ASSERT((mVectorDataTypes.size() > 0), "DATA UNIT ORDERS CANNOT BE APPLIED WHEN THERE IS NO DATA TYPE");
        ASSERT(
            (
                [&]()
                {
                    if (mIsModbusAreaSet == true)
                    {// mb_area_e 기본 값은 COILS이므로 설정됐는지 여부에 대한 검사가 선결조건임
                        if (mModbusArea == mb_area_e::COILS || mModbusArea == mb_area_e::DISCRETE_INPUT)
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
        mIsDataUnitOrdersSet = true;
    }

    void Node::SetDataTypes(const std::vector<dt_e>&& dt) noexcept
    {
        ASSERT(
            (
                [&]()
                {
                    if (mIsModbusAreaSet == true)
                    {// mb_area_e 기본 값은 COILS이므로 설정됐는지 여부에 대한 검사가 선결조건임
                        if (mModbusArea == mb_area_e::COILS || mModbusArea == mb_area_e::DISCRETE_INPUT)
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
        ASSERT((mIsBitIndexSet == false), "DATA TYPES CANNOT BE SET WHEN BIT INDEX IS ENABLED");

        mVectorDataTypes = std::move(dt);
        mIsDataTypesSet = true;
    }

    void Node::SetFormatString(const std::string& format)
    {
        // 속성 간 의존성 검사
        ASSERT((mIsBitIndexSet == false), "FORMAT STRING CANNOT BE SET WHEN BIT INDEX IS ENABLED");
        ASSERT((mIsDataTypesSet == true), "DATA TYPE MUST BE SET BEFOREHAND");
        ASSERT((mVectorDataTypes.size() > 0), "DATA TYPE CANNOT BE EMPTY ARRAY");
        
        // 데이터 타입 유효성 검사
        ASSERT(
            (
                [&]()
                {
                    if (mIsModbusAreaSet == true)
                    {// mb_area_e 기본 값은 COILS이므로 설정됐는지 여부에 대한 검사가 선결조건임
                        if (mModbusArea == mb_area_e::COILS || mModbusArea == mb_area_e::DISCRETE_INPUT)
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
        mIsFormatStringSet = true;
    }

    void Node::SetDeprecableUID(const std::string& uid)
    {
        ASSERT((uid.size() == 4), "UID MUST BE A STRING WITH LEGNTH OF 4");
        ASSERT(
            (
                uid.substr(0, 1) == "P"  || 
                uid.substr(0, 1) == "A"  || 
                uid.substr(0, 1) == "E"  ||
                uid.substr(0, 2) == "DI" || 
                uid.substr(0, 2) == "DO" ||
                uid.substr(0, 2) == "MD"
            ), "UID MUST START WITH ONE OF PREFIXES, \"P\", \"A\", \"E\", \"DI\", \"DO\", \"MD\""
        );

        mDeprecableUID = uid;
        mIsDeprecableUidSet = true;
    }

    void Node::SetDeprecableDisplayName(const std::string& displayName)
    {
        mDeprecableDisplayName = displayName;
        mIsDeprecableDisplayNameSet = true;
    }

    void Node::SetDeprecableDisplayUnit(const std::string& displayUnit)
    {
        mDeprecableDisplayUnit = displayUnit;
        mIsDeprecableDisplayUnitSet = true;
    }

    void Node::SetAttributeEvent(const bool hasEvent)
    {
        mHasAttributeEvent = hasEvent;
        mIsAttributeEventSet = true;
    }

    std::pair<Status, std::string> Node::GetNodeID() const
    {
        if (mIsNodeIdSet)
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
        if (mIsAddressTypeSet)
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
        if (mIsAddressSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mAddress);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mAddress);
        }
    }

    std::pair<Status, mb_area_e> Node::GetModbusArea() const
    {
        if (mIsModbusAreaSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mModbusArea);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mModbusArea);
        }
    }

    std::pair<Status, uint8_t> Node::GetBitIndex() const
    {
        if (mIsBitIndexSet)
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
        if (mIsAddressQuantitySet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mAddressQuantity);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mAddressQuantity);
        }
    }

    std::pair<Status, scl_e> Node::GetNumericScale() const
    {
        if (mIsNumericScaleSet)
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
        if (mIsNumericOffsetSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mNumericOffset);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mNumericOffset);
        }
    }

    std::pair<Status, std::map<std::uint16_t, std::string>> Node::GetMappingRules() const
    {
        if (mIsMappingRulesSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mMapMappingRules);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mMapMappingRules);
        }
    }

    std::pair<Status, std::vector<DataUnitOrder>> Node::GetDataUnitOrders() const
    {
        if (mIsDataUnitOrdersSet)
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
        if (mIsDataTypesSet)
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
        if (mIsFormatStringSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mFormatString);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mFormatString);
        }
    }

    std::pair<Status, std::string> Node::GetDeprecableUID() const
    {
        if (mIsDeprecableUidSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mDeprecableUID);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mDeprecableUID);
        }
    }

    std::pair<Status, std::string> Node::GetDeprecableDisplayName() const
    {
        if (mIsDeprecableDisplayNameSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mDeprecableDisplayName);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mDeprecableDisplayName);
        }
    }

    std::pair<Status, std::string> Node::GetDeprecableDisplayUnit() const
    {
        if (mIsDeprecableDisplayUnitSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mDeprecableDisplayUnit);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mDeprecableDisplayUnit);
        }
    }

    std::pair<Status, bool> Node::GetAttributeEvent() const
    {
        if (mIsAttributeEventSet)
        {
            return std::make_pair(Status(Status::Code::GOOD), mHasAttributeEvent);
        }
        else
        {
            return std::make_pair(Status(Status::Code::BAD), mHasAttributeEvent);
        }
    }
}}}