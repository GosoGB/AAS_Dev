/**
 * @file NodeValidator.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Node 설정 정보가 유효한지 검사하는 클래스를 정의합니다.
 * 
 * @date 2024-10-14
 * @version 0.0.1
 * 
 * @copyright Copyright Edgecross Inc. (c) 2024
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "Jarvis/Config/Information/Node.h"
#include "NodeValidator.h"



namespace muffin { namespace jarvis {

    NodeValidator::NodeValidator()
        : mAddressType(Status(Status::Code::UNCERTAIN), adtp_e::NUMERIC)
        , mAddress(Status(Status::Code::UNCERTAIN), addr_u())
        , mModbusArea(Status(Status::Code::UNCERTAIN), mb_area_e::COILS)
        , mBitIndex(Status(Status::Code::UNCERTAIN), 0)
        , mAddressQuantity(Status(Status::Code::UNCERTAIN), 0)
        , mNumericScale(Status(Status::Code::UNCERTAIN), scl_e::NEGATIVE_1)
        , mNumericOffset(Status(Status::Code::UNCERTAIN), 0.0f)
        , mMappingRules(Status(Status::Code::UNCERTAIN), std::map<uint16_t, std::string>())
        , mDataUnitOrders(Status(Status::Code::UNCERTAIN), std::vector<DataUnitOrder>())
        , mDataTypes(Status(Status::Code::UNCERTAIN), std::vector<muffin::jarvis::dt_e>())
        , mFormatString(Status(Status::Code::UNCERTAIN), std::string())
        , mPatternUID(std::regex(R"(^(?:(P|A|E)\d{3}|(?:DI|DO|MD)\d{2})$)"))
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Constructed at address: %p", this);
    #endif
    }
    
    NodeValidator::~NodeValidator()
    {
    #if defined(DEBUG)
        LOG_VERBOSE(logger, "Destroyed at address: %p", this);
    #endif
    }

    Status NodeValidator::Inspect(const cfg_key_e key, const JsonArray arrayCIN, cin_vector* outVector)
    {
        ASSERT((outVector != nullptr), "OUTPUT PARAMETER <outVector> CANNOT BE A NULL POINTER");
        ASSERT((arrayCIN.isNull() == false), "OUTPUT PARAMETER <arrayCIN> CANNOT BE NULL");

        for (JsonObject json : arrayCIN)
        {
            Status ret = validateMandatoryKeys(json);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "MANDATORY KEYS CANNOT BE MISSING");
                return ret;
            }

            ret = validateMandatoryValues(json);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "MANDATORY KEY'S VALUE CANNOT BE NULL");
                return ret;
            }

            mNodeID = json["id"].as<std::string>();
            if (mNodeID.length() != 4)
            {
                LOG_ERROR(logger, "INVALID NODE ID: %s", mNodeID.c_str());
                return Status(Status::Code::BAD_NODE_ID_INVALID);
            }

            mAddressType = convertToAdressType(json["adtp"].as<uint8_t>());
            if (mAddressType.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "UNDEFINED OR UNSUPPORTED ADDRESS TYPE: %s", mAddressType.first.c_str());
                return mAddressType.first;
            }
            
            mAddress = convertToAddress(json["addr"].as<JsonVariant>());
            if (mAddress.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID ADDRESS: %s", mAddress.first.c_str());
                return mAddress.first;
            }
            
            mDataTypes = processDataTypes(json["dt"].as<JsonArray>());
            if (mDataTypes.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID DATA TYPES: %s", mDataTypes.first.c_str());
                return mDataTypes.first;
            }

            mUID = json["uid"].as<std::string>();
            if (std::regex_match(mUID, mPatternUID) == false)
            {
                LOG_ERROR(logger, "INVALID UID: %s", mUID.c_str());
                return Status(Status::Code::BAD_DATA_ENCODING_INVALID);
            }

            mDisplayName = json["name"].as<std::string>();
            mDisplayUnit = json["unit"].as<std::string>();
            mIsEventType = json["event"].as<bool>();
            /*Node 설정 형식에서 필수로 입력해야 하는 속성은 모두 유효합니다.*/

            mModbusArea = convertToModbusArea(json["area"].as<JsonVariant>());
            if (mModbusArea.first.ToCode() != Status::Code::GOOD && 
                mModbusArea.first.ToCode() != Status::Code::GOOD_NO_DATA)
            {
                LOG_ERROR(logger, "FAILED TO CONVERT TO MODBUS AREA: %s", mModbusArea.first.c_str());
                return mModbusArea.first;
            }

            mBitIndex = convertToBitIndex(json["bit"].as<JsonVariant>());
            if (mBitIndex.first.ToCode() != Status::Code::GOOD && 
                mBitIndex.first.ToCode() != Status::Code::GOOD_NO_DATA)
            {
                LOG_ERROR(logger, "FAILED TO CONVERT TO BIT INDEX: %s", mBitIndex.first.c_str());
                return mBitIndex.first;
            }

            mAddressQuantity = convertToAddressQuantity(json["qty"].as<JsonVariant>());
            if (mAddressQuantity.first.ToCode() != Status::Code::GOOD && 
                mAddressQuantity.first.ToCode() != Status::Code::GOOD_NO_DATA)
            {
                LOG_ERROR(logger, "FAILED TO CONVERT TO """""": %s", mAddressQuantity.first.c_str());
                return mAddressQuantity.first;
            }

            mNumericScale = convertToNumericScale(json["scl"].as<JsonVariant>());
            if (mNumericScale.first.ToCode() != Status::Code::GOOD && 
                mNumericScale.first.ToCode() != Status::Code::GOOD_NO_DATA)
            {
                LOG_ERROR(logger, "FAILED TO CONVERT TO """""": %s", mNumericScale.first.c_str());
                return mNumericScale.first;
            }

            mNumericOffset = convertToNumericOffset(json["ofst"].as<JsonVariant>());
            if (mNumericOffset.first.ToCode() != Status::Code::GOOD && 
                mNumericOffset.first.ToCode() != Status::Code::GOOD_NO_DATA)
            {
                LOG_ERROR(logger, "FAILED TO CONVERT TO """""": %s", mNumericOffset.first.c_str());
                return mNumericOffset.first;
            }

            mMappingRules = convertToMappingRules(json["map"].as<JsonVariant>());
            if (mMappingRules.first.ToCode() != Status::Code::GOOD && 
                mMappingRules.first.ToCode() != Status::Code::GOOD_NO_DATA)
            {
                LOG_ERROR(logger, "FAILED TO CONVERT TO """""": %s", mMappingRules.first.c_str());
                return mMappingRules.first;
            }

            mDataUnitOrders = processDataUnitOrders(json["ord"].as<JsonVariant>());
            if (mDataUnitOrders.first.ToCode() != Status::Code::GOOD &&
                mDataUnitOrders.first.ToCode() != Status::Code::GOOD_NO_DATA)
            {
                LOG_ERROR(logger, "FAILED TO CONVERT TO """""": %s", mDataUnitOrders.first.c_str());
                return mDataUnitOrders.first;
            }

            mFormatString = convertToFormatString(json["fmt"].as<JsonVariant>());
            if (mFormatString.first.ToCode() != Status::Code::GOOD &&
                mFormatString.first.ToCode() != Status::Code::GOOD_NO_DATA)
            {
                LOG_ERROR(logger, "FAILED TO CONVERT TO """""": %s", mFormatString.first.c_str());
                return mFormatString.first;
            }

            ret = validateModbusArea();
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID MODBUS AREA CONFIG: %s", ret.c_str());
                return ret;
            }
            
            ret = validateBitIndex();
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID BIT INDEX CONFIG: %s", ret.c_str());
                return ret;
            }

            ret = validateAddressQuantity();
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID ADDRESS QUANTITY CONFIG: %s", ret.c_str());
                return ret;
            }
            
            ret = validateNumericScale();
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID NUMERIC SCALE CONFIG: %s", ret.c_str());
                return ret;
            }

            ret = validateNumericOffset();
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID NUMERIC OFFSET CONFIG: %s", ret.c_str());
                return ret;
            }

            ret = validateMappingRules();
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID MAPPING RULES CONFIG: %s", ret.c_str());
                return ret;
            }

            ret = validateDataUnitOrders();
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID DATA UNIT ORDER CONFIG: %s", ret.c_str());
                return ret;
            }

            ret = validateDataTypes();
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID DATA TYPES CONFIG: %s", ret.c_str());
                return ret;
            }

            ret = validateFormatString();
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "INVALID FORMAT STRING CONFIG: %s", ret.c_str());
                return ret;
            }

            config::Node* node = new(std::nothrow) config::Node();
            if (node == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR CIN: NODE");
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            }

            node->SetNodeID(mNodeID);
            node->SetAddressType(mAddressType.second);
            node->SetAddrress(mAddress.second);
            node->SetDataTypes(std::move(mDataTypes.second));
            node->SetDeprecableUID(mUID);
            node->SetDeprecableDisplayName(mDisplayName);
            node->SetDeprecableDisplayUnit(mDisplayUnit);
            node->SetAttributeEvent(mIsEventType);

            if (mModbusArea.first == Status::Code::GOOD)
            {
                node->SetModbusArea(mModbusArea.second);
            }
            
            if (mBitIndex.first == Status::Code::GOOD)
            {
                node->SetBitIndex(mBitIndex.second);
            }

            if (mAddressQuantity.first == Status::Code::GOOD)
            {
                node->SetNumericAddressQuantity(mAddressQuantity.second);
            }

            if (mNumericScale.first == Status::Code::GOOD)
            {
                node->SetNumericScale(mNumericScale.second);
            }
            
            if (mNumericOffset.first == Status::Code::GOOD)
            {
                node->SetNumericOffset(mNumericOffset.second);
            }

            if (mMappingRules.first == Status::Code::GOOD)
            {
                node->SetMappingRules(std::move(mMappingRules.second));
            }

            if (mDataUnitOrders.first == Status::Code::GOOD)
            {
                node->SetDataUnitOrders(std::move(mDataUnitOrders.second));
            }
            
            if (mFormatString.first == Status::Code::GOOD)
            {
                node->SetFormatString(mFormatString.second);
            }

            try
            {
                outVector->emplace_back(std::move(node));
            }
            catch(const std::bad_alloc& e)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR NODE CIN: %s", e.what());
                return Status(Status::Code::BAD_OUT_OF_MEMORY);
            }
            catch(const std::exception& e)
            {
                LOG_ERROR(logger, "FAILED TO EMPLACE NODE CIN: %s", e.what());
                return Status(Status::Code::BAD_UNEXPECTED_ERROR);
            }
        }
    }

    /**
     * @return Status
     *     @li Status::Code::GOOD 모든 설정 키(key)가 있습니다.
     *     @li Status::Code::BAD_ENCODING_ERROR 설정 키(key) 중 하나 이상이 없습니다.
     */
    Status NodeValidator::validateMandatoryKeys(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json.containsKey("id");
        isValid &= json.containsKey("adtp");
        isValid &= json.containsKey("addr");
        isValid &= json.containsKey("area");
        isValid &= json.containsKey("bit");
        isValid &= json.containsKey("qty");
        isValid &= json.containsKey("scl");
        isValid &= json.containsKey("ofst");
        isValid &= json.containsKey("map");
        isValid &= json.containsKey("ord");
        isValid &= json.containsKey("dt");
        isValid &= json.containsKey("fmt");
        isValid &= json.containsKey("uid");
        isValid &= json.containsKey("name");
        isValid &= json.containsKey("unit");
        isValid &= json.containsKey("event");

        if (isValid == true)
        {
            return Status(Status::Code::GOOD);
        }
        else
        {
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }
    }

    /**
     * @return Status
     *     @li Status::Code::GOOD 필수 설정 키(key)에 값이 존재하며 형식이 올바릅니다.
     *     @li Status::Code::BAD_ENCODING_ERROR 필수 설정 키(key) 중 하나 이상의 값의 형식이 올바르지 않습니다.
     */
    Status NodeValidator::validateMandatoryValues(const JsonObject json)
    {
        bool isValid = true;
        isValid &= json["id"].isNull()     == false;
        isValid &= json["adtp"].isNull()   == false;
        isValid &= json["addr"].isNull()   == false;
        isValid &= json["dt"].isNull()     == false;
        isValid &= json["uid"].isNull()    == false;
        isValid &= json["name"].isNull()   == false;
        isValid &= json["unit"].isNull()   == false;
        isValid &= json["event"].isNull()  == false;
        isValid &= json["id"].is<std::string>();
        isValid &= json["adtp"].is<uint8_t>();
        isValid &= json["addr"].is<JsonVariant>();
        isValid &= json["dt"].is<JsonArray>();
        isValid &= json["uid"].is<std::string>();
        isValid &= json["name"].is<std::string>();
        isValid &= json["unit"].is<std::string>();
        isValid &= json["event"].is<bool>();

        if (isValid == true)
        {
            return Status(Status::Code::GOOD);
        }
        else
        {
            return Status(Status::Code::BAD_ENCODING_ERROR);
        }
    }

    /**
     * @return Status
     *     @li Status::Code::GOOD Modbus 메모리 영역 설정이 없거나, 설정 정보가 유효합니다.
     *     @li Status::Code::BAD_CONFIGURATION_ERROR 다른 Node 속성을 고려했을 때 선결조건 또는 관계가 유효하지 않습니다.
     *     @li Status::Code::BAD_SERVICE_UNSUPPORTED 현재 펌웨어 버전에서는 Modbus 프로토콜의 확장 주소 체계를 지원하지 않습니다.
     */
    Status NodeValidator::validateModbusArea()
    {
        if (mModbusArea.first.ToCode() == Status::Code::GOOD_NO_DATA)
        {
            LOG_VERBOSE(logger, "Modbus memory area is not enabled");
            return Status(Status::Code::GOOD);
        }
        
        if (mAddressType.second != adtp_e::NUMERIC)
        {
            LOG_ERROR(logger, "ADDRESS TYPE MUST BE NUMERIC TO ENABLE MODBUS PROTOCOL");
            return Status(Status::Code::BAD_CONFIGURATION_ERROR);
        }
        
        if (mAddress.second.Numeric > 0x270F)
        {
            LOG_ERROR(logger, "EXTENDED REGISTER ADDRESS FOR MODBUS PROTOCOL IS NOT SUPPORTED");
            return Status(Status::Code::BAD_SERVICE_UNSUPPORTED);
        }

        if (mModbusArea.second == mb_area_e::COILS || mModbusArea.second == mb_area_e::DISCRETE_INPUT)
        {
            if (mBitIndex.first == Status::Code::GOOD)
            {
                LOG_ERROR(logger, "MODBUS AREA CANNOT BE CONFIGURED AS \"Coils\" OR \"Discrete Inputs\" WITH BIT INDEX");
                return Status(Status::Code::BAD_CONFIGURATION_ERROR);
            }
            
            if (mAddressQuantity.first == Status::Code::GOOD)
            {
                LOG_ERROR(logger, "MODBUS AREA CANNOT BE CONFIGURED AS \"Coils\" OR \"Discrete Inputs\" WITH ADDRESS QUANTITY");
                return Status(Status::Code::BAD_CONFIGURATION_ERROR);
            }
            
            if (mNumericScale.first == Status::Code::GOOD)
            {
                LOG_ERROR(logger, "MODBUS AREA CANNOT BE CONFIGURED AS \"Coils\" OR \"Discrete Inputs\" WITH NUMERIC SCALE");
                return Status(Status::Code::BAD_CONFIGURATION_ERROR);
            }
            
            if (mNumericOffset.first == Status::Code::GOOD)
            {
                LOG_ERROR(logger, "MODBUS AREA CANNOT BE CONFIGURED AS \"Coils\" OR \"Discrete Inputs\" WITH NUMERIC OFFSET");
                return Status(Status::Code::BAD_CONFIGURATION_ERROR);
            }
            
            if (mDataUnitOrders.first == Status::Code::GOOD)
            {
                LOG_ERROR(logger, "MODBUS AREA CANNOT BE CONFIGURED AS \"Coils\" OR \"Discrete Inputs\" WITH DATA UNIT ORDERS");
                return Status(Status::Code::BAD_CONFIGURATION_ERROR);
            }
            
            if (mDataTypes.second.size() != 1)
            {
                LOG_ERROR(logger, "MODBUS AREA CANNOT BE CONFIGURED AS \"Coils\" OR \"Discrete Inputs\" IF MORE THAN ONE DATA TYPE IS PROVIDED");
                return Status(Status::Code::BAD_CONFIGURATION_ERROR);
            }
            
            if (mDataTypes.second.front() != dt_e::BOOLEAN)
            {
                LOG_ERROR(logger, "MODBUS AREA CANNOT BE CONFIGURED AS \"Coils\" OR \"Discrete Inputs\" IF DATA TYPE IS NOT BOOLEAN");
                return Status(Status::Code::BAD_CONFIGURATION_ERROR);
            }
            
            if (mFormatString.first == Status::Code::GOOD)
            {
                LOG_ERROR(logger, "MODBUS AREA CANNOT BE CONFIGURED AS \"Coils\" OR \"Discrete Inputs\" WITH FORMAT STRING");
                return Status(Status::Code::BAD_CONFIGURATION_ERROR);
            }

            LOG_VERBOSE(logger, "Modbus area can be configured as \"Coils\" or \"Discrete Inputs\"");
            return Status(Status::Code::GOOD);
        }
        else
        {
            if (mAddressQuantity.first == Status::Code::GOOD_NO_DATA)
            {
                LOG_ERROR(logger, "MODBUS AREA CANNOT BE CONFIGURED AS \"Input Registers\" OR \"Holding Registers\" WITHOUT ADDRESS QUANTITY");
                return Status(Status::Code::BAD_CONFIGURATION_ERROR);
            }
            
            LOG_VERBOSE(logger, "Modbus area can be configured as \"Input Registers\" or \"Holding Registers\"");
            return Status(Status::Code::GOOD);
        }
    }

    /**
     * @return Status
     *     @li Status::Code::GOOD 비트 인덱스 설정이 없거나, 설정 정보가 유효합니다.
     *     @li Status::Code::BAD_CONFIGURATION_ERROR 다른 Node 속성을 고려했을 때 선결조건 또는 관계가 유효하지 않습니다.
     */
    Status NodeValidator::validateBitIndex()
    {
        if (mBitIndex.first.ToCode() == Status::Code::GOOD_NO_DATA)
        {
            LOG_VERBOSE(logger, "Bit index is not enabled");
            return Status(Status::Code::GOOD);
        }

        if (mAddressQuantity.first == Status::Code::GOOD)
        {
            LOG_ERROR(logger, "BIT INDEX CANNOT BE CONFIGURED WITH ADDRESS QUANTITY");
            return Status(Status::Code::BAD_CONFIGURATION_ERROR);
        }
        
        if (mNumericScale.first == Status::Code::GOOD)
        {
            LOG_ERROR(logger, "BIT INDEX CANNOT BE CONFIGURED WITH NUMERIC SCALE");
            return Status(Status::Code::BAD_CONFIGURATION_ERROR);
        }
        
        if (mNumericOffset.first == Status::Code::GOOD)
        {
            LOG_ERROR(logger, "BIT INDEX CANNOT BE CONFIGURED WITH NUMERIC OFFSET");
            return Status(Status::Code::BAD_CONFIGURATION_ERROR);
        }

        if (mFormatString.first == Status::Code::GOOD)
        {
            LOG_ERROR(logger, "BIT INDEX CANNOT BE CONFIGURED WITH FORMAT STRING");
            return Status(Status::Code::BAD_CONFIGURATION_ERROR);
        }

        if (mDataTypes.second.size() != 1)
        {
            LOG_ERROR(logger, "BIT INDEX CANNOT BE CONFIGURED WITH MORE THAN ONE DATA TYPE");
            return Status(Status::Code::BAD_CONFIGURATION_ERROR);
        }

        if (mDataUnitOrders.first == Status::Code::GOOD)
        {
            if (mDataUnitOrders.second.size() != 1)
            {
                LOG_ERROR(logger, "BIT INDEX CANNOT BE CONFIGURED WITH MORE THAN ONE DATA UNIT ORDER");
                return Status(Status::Code::BAD_CONFIGURATION_ERROR);
            }
        }

        if (mModbusArea.first == Status::Code::GOOD)
        {
            if (mModbusArea.second == mb_area_e::COILS || mModbusArea.second == mb_area_e::DISCRETE_INPUT)
            {
                LOG_ERROR(logger, "BIT INDEX CANNOT BE CONFIGURED WITH MODBUS AREA \"Coils\" OR \"Discrete Inputs\"");
                return Status(Status::Code::BAD_CONFIGURATION_ERROR);
            }
        }

        switch (mDataTypes.second.front())
        {
            case dt_e::INT8:
            case dt_e::UINT8:
                if (mBitIndex.second > 7)
                {
                    LOG_ERROR(logger, "BIT INDEX OUT OF RANGE FOR 8-BIT INTEGER: %u", mBitIndex.second);
                    return Status(Status::Code::BAD_CONFIGURATION_ERROR);
                }
                break;
            case dt_e::INT16:
            case dt_e::UINT16:
                if (mBitIndex.second > 15)
                {
                    LOG_ERROR(logger, "BIT INDEX OUT OF RANGE FOR 16-BIT INTEGER: %u", mBitIndex.second);
                    return Status(Status::Code::BAD_CONFIGURATION_ERROR);
                }
                break;
            case dt_e::INT32:
            case dt_e::UINT32:
                if (mBitIndex.second > 31)
                {
                    LOG_ERROR(logger, "BIT INDEX OUT OF RANGE FOR 32-BIT INTEGER: %u", mBitIndex.second);
                    return Status(Status::Code::BAD_CONFIGURATION_ERROR);
                }
                break;
            case dt_e::INT64:
            case dt_e::UINT64:
                if (mBitIndex.second > 63)
                {
                    LOG_ERROR(logger, "BIT INDEX OUT OF RANGE FOR 64-BIT INTEGER: %u", mBitIndex.second);
                    return Status(Status::Code::BAD_CONFIGURATION_ERROR);
                }
                break;
            default:
                LOG_ERROR(logger, "INVALID DATA TYPE FOR BIT INDEX: %u", 
                    static_cast<uint8_t>(mDataTypes.second.front()));
                return Status(Status::Code::BAD_CONFIGURATION_ERROR);
        }

        LOG_VERBOSE(logger, "Bit index [%u] can be configured with data type: %u",
            mBitIndex.second, static_cast<uint8_t>(mDataTypes.second.front()));
        return Status(Status::Code::GOOD);
    }

    /**
     * @return Status
     *     @li Status::Code::GOOD Numeric Address Quantity 설정 정보가 유효합니다.
     *     @li Status::Code::BAD_CONFIGURATION_ERROR 다른 Node 속성을 고려했을 때 선결조건 또는 관계가 유효하지 않습니다.
     */
    Status NodeValidator::validateAddressQuantity()
    {
        if (mAddressType.second != adtp_e::NUMERIC)
        {
            if (mAddressQuantity.first == Status::Code::GOOD)
            {
                LOG_ERROR(logger, "NUMERIC ADDRESS QUANTITY CANNOT BE CONFIGURED IF ADDRESS TYPE IS NOT NUMERIC");
                return Status(Status::Code::BAD_CONFIGURATION_ERROR);
            }
            else
            {
                LOG_VERBOSE(logger, "Numeric address quantity is not enabled");
                return Status(Status::Code::GOOD);
            }
        }
        
        if (mAddressType.second == adtp_e::NUMERIC)
        {
            if (mAddressQuantity.first == Status::Code::GOOD_NO_DATA)
            {
                LOG_ERROR(logger, "NUMERIC ADDRESS QUANTITY MUST BE CONFIGURED IF ADDRESS TYPE IS NUMERIC");
                return Status(Status::Code::BAD_CONFIGURATION_ERROR);
            }
        }

        if (mBitIndex.first == Status::Code::GOOD)
        {
            LOG_ERROR(logger, "NUMERIC ADDRESS QUANTITY CANNOT BE CONFIGURED IF BIT INDEX IS ENABLED");
            return Status(Status::Code::BAD_CONFIGURATION_ERROR);
        }

        if (mModbusArea.first == Status::Code::GOOD)
        {
            if (mModbusArea.second == mb_area_e::COILS || mModbusArea.second == mb_area_e::DISCRETE_INPUT)
            {
                LOG_ERROR(logger, "NUMERIC ADDRESS QUANTITY CANNOT BE CONFIGURED WITH MODBUS AREA \"Coils\" OR \"Discrete Inputs\"");
                return Status(Status::Code::BAD_CONFIGURATION_ERROR);
            }
        }

        LOG_VERBOSE(logger, "Configured numeric address quantity: %u", mAddressQuantity.second);
        return Status(Status::Code::GOOD);
    }

    /**
     * @return Status
     *     @li Status::Code::GOOD Numeric Scale 설정이 없거나, 설정 정보가 유효합니다.
     *     @li Status::Code::BAD_CONFIGURATION_ERROR 다른 Node 속성을 고려했을 때 선결조건 또는 관계가 유효하지 않습니다.
     */
    Status NodeValidator::validateNumericScale()
    {
        if (mNumericScale.first.ToCode() == Status::Code::GOOD_NO_DATA)
        {
            LOG_VERBOSE(logger, "Numeric scale is not enabled");
            return Status(Status::Code::GOOD);
        }
        
        if (mDataTypes.second.size() != 1)
        {
            LOG_ERROR(logger, "NUMERIC SCALE CANNOT BE CONFIGURED WITH MORE THAN ONE DATA TYPES");
            return Status(Status::Code::BAD_CONFIGURATION_ERROR);
        }

        if (mDataUnitOrders.first == Status::Code::GOOD)
        {
            if (mDataUnitOrders.second.size() != 1)
            {
                LOG_ERROR(logger, "NUMERIC SCALE CANNOT BE CONFIGURED WITH MORE THAN ONE DATA UNIT ORDERS");
                return Status(Status::Code::BAD_CONFIGURATION_ERROR);
            }
        }
        
        switch (mDataTypes.second.front())
        {
            case dt_e::STRING:
            case dt_e::BOOLEAN:
                LOG_ERROR(logger, "NUMERIC SCALE CANNOT BE CONFIGURED WITH \"STRING\" OR \"BOOLEAN\" DATA TYPE");
                return Status(Status::Code::BAD_CONFIGURATION_ERROR);
            default:
                break;
        }
        
        if (mBitIndex.first == Status::Code::GOOD)
        {
            LOG_ERROR(logger, "NUMERIC SCALE CANNOT BE CONFIGURED IF BIT INDEX IS ENABLED");
            return Status(Status::Code::BAD_CONFIGURATION_ERROR);
        }

        if (mModbusArea.first == Status::Code::GOOD)
        {
            if (mModbusArea.second == mb_area_e::COILS || mModbusArea.second == mb_area_e::DISCRETE_INPUT)
            {
                LOG_ERROR(logger, "NUMERIC SCALE CANNOT BE CONFIGURED WITH MODBUS AREA \"Coils\" OR \"Discrete Inputs\"");
                return Status(Status::Code::BAD_CONFIGURATION_ERROR);
            }
        }

        LOG_VERBOSE(logger, "Configured numeric scale: %d", static_cast<int8_t>(mNumericScale.second));
        return Status(Status::Code::GOOD);
    }

    /**
     * @return Status
     *     @li Status::Code::GOOD Numeric Offset 설정이 없거나, 설정 정보가 유효합니다.
     *     @li Status::Code::BAD_CONFIGURATION_ERROR 다른 Node 속성을 고려했을 때 선결조건 또는 관계가 유효하지 않습니다.
     */
    Status NodeValidator::validateNumericOffset()
    {
        if (mNumericOffset.first.ToCode() == Status::Code::GOOD_NO_DATA)
        {
            LOG_VERBOSE(logger, "Numeric offset is not enabled");
            return Status(Status::Code::GOOD);
        }

        if (mDataTypes.second.size() != 1)
        {
            LOG_ERROR(logger, "NUMERIC OFFSET CANNOT BE CONFIGURED WITH MORE THAN ONE DATA TYPES");
            return Status(Status::Code::BAD_CONFIGURATION_ERROR);
        }

        if (mDataUnitOrders.first == Status::Code::GOOD)
        {
            if (mDataUnitOrders.second.size() != 1)
            {
                LOG_ERROR(logger, "NUMERIC OFFSET CANNOT BE CONFIGURED WITH MORE THAN ONE DATA UNIT ORDERS");
                return Status(Status::Code::BAD_CONFIGURATION_ERROR);
            }
        }
        
        switch (mDataTypes.second.front())
        {
            case dt_e::STRING:
            case dt_e::BOOLEAN:
                LOG_ERROR(logger, "NUMERIC OFFSET CANNOT BE CONFIGURED WITH \"STRING\" OR \"BOOLEAN\" DATA TYPE");
                return Status(Status::Code::BAD_CONFIGURATION_ERROR);
            default:
                break;
        }
        
        if (mBitIndex.first == Status::Code::GOOD)
        {
            LOG_ERROR(logger, "NUMERIC OFFSET CANNOT BE CONFIGURED IF BIT INDEX IS ENABLED");
            return Status(Status::Code::BAD_CONFIGURATION_ERROR);
        }

        if (mModbusArea.first == Status::Code::GOOD)
        {
            if (mModbusArea.second == mb_area_e::COILS || mModbusArea.second == mb_area_e::DISCRETE_INPUT)
            {
                LOG_ERROR(logger, "NUMERIC OFFSET CANNOT BE CONFIGURED WITH MODBUS AREA \"Coils\" OR \"Discrete Inputs\"");
                return Status(Status::Code::BAD_CONFIGURATION_ERROR);
            }
        }

        LOG_VERBOSE(logger, "Configured numeric offset: %d", );
        return Status(Status::Code::GOOD);
    }

    /**
     * @return Status
     *     @li Status::Code::GOOD Mapping Rules 설정이 없거나, 설정 정보가 유효합니다.
     *     @li Status::Code::BAD_CONFIGURATION_ERROR 다른 Node 속성을 고려했을 때 선결조건 또는 관계가 유효하지 않습니다.
     */
    Status NodeValidator::validateMappingRules()
    {
        if (mMappingRules.first.ToCode() == Status::Code::GOOD_NO_DATA)
        {
            LOG_VERBOSE(logger, "Mapping rules are not enabled");
            return Status(Status::Code::GOOD);
        }

        if (mDataTypes.second.size() != 1)
        {
            LOG_ERROR(logger, "MAPPING RULES CANNOT BE CONFIGURED WITH MORE THAN ONE DATA TYPES");
            return Status(Status::Code::BAD_CONFIGURATION_ERROR);
        }

        if (mDataUnitOrders.first == Status::Code::GOOD)
        {
            if (mDataUnitOrders.second.size() != 1)
            {
                LOG_ERROR(logger, "MAPPING RULES CANNOT BE CONFIGURED WITH MORE THAN ONE DATA UNIT ORDERS");
                return Status(Status::Code::BAD_CONFIGURATION_ERROR);
            }
        }
        
        switch (mDataTypes.second.front())
        {
            case dt_e::STRING:
            case dt_e::FLOAT32:
            case dt_e::FLOAT64:
                LOG_ERROR(logger, "MAPPING RULES CANNOT BE CONFIGURED WITH \"STRING\", \"FP32\" OR \"FP64\" DATA TYPE");
                return Status(Status::Code::BAD_CONFIGURATION_ERROR);
            default:
                break;
        }

        LOG_VERBOSE(logger, "Configured mapping rules");
        return Status(Status::Code::GOOD);
    }

    /**
     * @return Status
     *     @li Status::Code::GOOD Data Unit Orders 설정이 없거나, 설정 정보가 유효합니다.
     *     @li Status::Code::BAD_CONFIGURATION_ERROR 다른 Node 속성을 고려했을 때 선결조건 또는 관계가 유효하지 않습니다.
     */
    Status NodeValidator::validateDataUnitOrders()
    {
        if (mDataUnitOrders.first.ToCode() == Status::Code::GOOD_NO_DATA)
        {
            LOG_VERBOSE(logger, "Data unit orders are not enabled");
            return Status(Status::Code::GOOD);
        }
        
        if (mBitIndex.first == Status::Code::GOOD)
        {
            LOG_ERROR(logger, "DATA UNIT ORDERS CANNOT BE CONFIGURED IF BIT INDEX IS ENABLED");
            return Status(Status::Code::BAD_CONFIGURATION_ERROR);
        }

        if (mModbusArea.first == Status::Code::GOOD)
        {
            if (mModbusArea.second == mb_area_e::COILS || mModbusArea.second == mb_area_e::DISCRETE_INPUT)
            {
                LOG_ERROR(logger, "DATA UNIT ORDERS CANNOT BE CONFIGURED WITH MODBUS AREA \"Coils\" OR \"Discrete Inputs\"");
                return Status(Status::Code::BAD_CONFIGURATION_ERROR);
            }
        }
        
        if (mDataUnitOrders.second.size() != mDataTypes.second.size())
        {
            LOG_ERROR(logger, "DATA UNIT ORDERS MUST HAVE EQUAL LENGTH OF ELEMENTS WITH DATA TYPES");
            return Status(Status::Code::BAD_CONFIGURATION_ERROR);
        }

        for (const auto& dataUnitOrder : mDataUnitOrders.second)
        {
            std::set<uint8_t> setIndex;

            for (const auto& orderType : dataUnitOrder)
            {
                const auto result = setIndex.emplace(orderType.Index);
                if (result.second == false)
                {
                    LOG_ERROR(logger, "DATA UNIT ORDER INDICES CANNOT BE DUPLICATED");
                    return Status(Status::Code::BAD_CONFIGURATION_ERROR);
                }
            }
        }

        LOG_VERBOSE(logger, "Configured data unit orders");
        return Status(Status::Code::GOOD);
    }
    
    /**
     * @return Status
     *     @li Status::Code::GOOD Data Types 설정이 없거나, 설정 정보가 유효합니다.
     *     @li Status::Code::BAD_CONFIGURATION_ERROR 다른 Node 속성을 고려했을 때 선결조건 또는 관계가 유효하지 않습니다.
     */
    Status NodeValidator::validateDataTypes()
    {
        if (mDataTypes.first.ToCode() == Status::Code::GOOD_NO_DATA)
        {
            LOG_VERBOSE(logger, "Data types are not enabled");
            return Status(Status::Code::GOOD);
        }
        
        if (mBitIndex.first == Status::Code::GOOD)
        {
            LOG_ERROR(logger, "DATA TYPES CANNOT BE CONFIGURED IF BIT INDEX IS ENABLED");
            return Status(Status::Code::BAD_CONFIGURATION_ERROR);
        }
        
        if (mDataUnitOrders.first == Status::Code::GOOD)
        {
            if (mDataTypes.second.size() != mDataUnitOrders.second.size())
            {
                LOG_ERROR(logger, "DATA TYPES MUST HAVE EQUAL LENGTH OF ELEMENTS WITH DATA UNIT ORDERS");
                return Status(Status::Code::BAD_CONFIGURATION_ERROR);
            }
            
            for (uint8_t i = 0; i < mDataTypes.second.size(); ++i)
            {
                const DataUnitOrder& dataUnitOrder = mDataUnitOrders.second.at(i);
                size_t sumOrderSize = 0;

                for (const auto& order : dataUnitOrder)
                {
                    sumOrderSize += static_cast<uint8_t>(order.DataUnit);
                }
                
                size_t dataTypeSize = 0;
                switch (mDataTypes.second.at(i))
                {
                    case dt_e::BOOLEAN:
                        LOG_ERROR(logger, "DATA TYPE \"BOOLEAN\" CANNOT BE CONFIGURED WITH DATA UNIT ORDER");
                        return Status(Status::Code::BAD_CONFIGURATION_ERROR);
                    case dt_e::INT8:
                    case dt_e::UINT8:
                    case dt_e::STRING:
                        dataTypeSize = 8;
                        break;
                    case dt_e::INT16:
                    case dt_e::UINT16:
                        dataTypeSize = 16;
                        break;
                    case dt_e::INT32:
                    case dt_e::UINT32:
                    case dt_e::FLOAT32:
                        dataTypeSize = 32;
                        break;
                    case dt_e::INT64:
                    case dt_e::UINT64:
                    case dt_e::FLOAT64:
                        dataTypeSize = 64;
                        break;
                    default:
                        return Status(Status::Code::BAD_UNEXPECTED_ERROR);
                }
            
                if (dataTypeSize != sumOrderSize)
                {
                    LOG_ERROR(logger, "DATA TYPE SIZE DOES NOT MATCH WITH THE SUM OF DATA UNIT ORDERS");
                    return Status(Status::Code::BAD_CONFIGURATION_ERROR);
                }
            }
        }

        if (mModbusArea.first == Status::Code::GOOD)
        {
            if (mModbusArea.second == mb_area_e::COILS || mModbusArea.second == mb_area_e::DISCRETE_INPUT)
            {
                if (mDataTypes.second.size() != 1)
                {
                    LOG_ERROR(logger, "DATA TYPE MUST HAVE ONE ELEMENT WITH MODBUS AREA \"Coils\" OR \"Discrete Inputs\"");
                    return Status(Status::Code::BAD_CONFIGURATION_ERROR);
                }
                else if (mDataTypes.second.front() != dt_e::BOOLEAN)
                {
                    LOG_ERROR(logger, "DATA TYPE MUST BE \"BOOLEAN\" WITH MODBUS AREA \"Coils\" OR \"Discrete Inputs\"");
                    return Status(Status::Code::BAD_CONFIGURATION_ERROR);
                }
            }
            else
            {
                constexpr uint8_t REGISTER_SIZE = 16;
                const size_t totalRegisterSize = REGISTER_SIZE * mAddressQuantity.second;
                size_t sumDataTypeSize = 0;

                for (const auto& dataType : mDataTypes.second)
                {
                    switch (dataType)
                    {
                        case dt_e::INT8:
                        case dt_e::UINT8:
                        case dt_e::STRING:
                            sumDataTypeSize += 8;
                            break;
                        case dt_e::INT16:
                        case dt_e::UINT16:
                            sumDataTypeSize += 16;
                            break;
                        case dt_e::INT32:
                        case dt_e::UINT32:
                        case dt_e::FLOAT32:
                            sumDataTypeSize += 32;
                            break;
                        case dt_e::INT64:
                        case dt_e::UINT64:
                        case dt_e::FLOAT64:
                            sumDataTypeSize += 64;
                            break;
                        default:
                            LOG_ERROR(logger, "DATA TYPE \"BOOLEAN\" CANNOT BE CONFIGURED WITH MODBUS AREA USING REGISTERS");
                            return Status(Status::Code::BAD_CONFIGURATION_ERROR);
                    }
                }

                if (totalRegisterSize != sumDataTypeSize)
                {
                    LOG_ERROR(logger, "TOTAL SIZE OF MODBUS REGISTERS DOES NOT MATCH WITH THE SUM OF EACH DATA TYPES");
                    return Status(Status::Code::BAD_CONFIGURATION_ERROR);
                }
            }
        }

        LOG_VERBOSE(logger, "Configured data types");
        return Status(Status::Code::GOOD);
    }

    /**
     * @return Status
     *     @li Status::Code::GOOD Format String 설정이 없거나, 설정 정보가 유효합니다.
     *     @li Status::Code::BAD_CONFIGURATION_ERROR 다른 Node 속성을 고려했을 때 선결조건 또는 관계가 유효하지 않습니다.
     */
    Status NodeValidator::validateFormatString()
    {
        if (mFormatString.first.ToCode() == Status::Code::GOOD_NO_DATA)
        {
            LOG_VERBOSE(logger, "Format string is not enabled");
            return Status(Status::Code::GOOD);
        }
        
        if (mBitIndex.first == Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FORMAT STRING CANNOT BE CONFIGURED IF BIT INDEX IS ENABLED");
            return Status(Status::Code::BAD_CONFIGURATION_ERROR);
        }

        if (mModbusArea.first == Status::Code::GOOD)
        {
            if (mModbusArea.second == mb_area_e::COILS || mModbusArea.second == mb_area_e::DISCRETE_INPUT)
            {
                LOG_ERROR(logger, "FORMAT STRING CANNOT BE CONFIGURED WITH MODBUS AREA \"Coils\" OR \"Discrete Inputs\"");
                return Status(Status::Code::BAD_CONFIGURATION_ERROR);
            }
        }

        if (mVectorFormatSpecifier.size() != mDataTypes.second.size())
        {
            LOG_ERROR(logger, "THE NUMBER OF FORMAT SPECIFIERS MUST BE EQUAL TO THE NUMBER OF DATA TYPES");
            return Status(Status::Code::BAD_CONFIGURATION_ERROR);
        }

        for (uint8_t i = 0; i < mVectorFormatSpecifier.size(); ++i)
        {
            fmt_spec_e specifier = mVectorFormatSpecifier[i];
            dt_e dataType = mDataTypes.second[i];

            switch (specifier)
            {
            case fmt_spec_e::INTEGER_32:
                if (dataType != dt_e::INT8  && dataType != dt_e::INT16  && dataType != dt_e::INT32)
                {
                    LOG_ERROR(logger, "INVALID DATA TYPE FOR FORMAT SPECIFIER \"32-BIT INTEGER\"");
                    return Status(Status::Code::BAD_CONFIGURATION_ERROR);
                }
                break;
            case fmt_spec_e::INTEGER_64:
                if (dataType != dt_e::INT8  && dataType != dt_e::INT16  && dataType != dt_e::INT32 && dataType != dt_e::INT64)
                {
                    LOG_ERROR(logger, "INVALID DATA TYPE FOR FORMAT SPECIFIER \"64-BIT INTEGER\"");
                    return Status(Status::Code::BAD_CONFIGURATION_ERROR);
                }
                break;
            case fmt_spec_e::UNSIGNED_INTEGER_32:
                if (dataType != dt_e::UINT8 && dataType != dt_e::UINT16 && dataType != dt_e::UINT32)
                {
                    LOG_ERROR(logger, "INVALID DATA TYPE FOR FORMAT SPECIFIER \"UNSIGNED 32-BIT INTEGER\"");
                    return Status(Status::Code::BAD_CONFIGURATION_ERROR);
                }
                break;
            case fmt_spec_e::UNSIGNED_INTEGER_64:
                if (dataType != dt_e::UINT8 && dataType != dt_e::UINT16 && dataType != dt_e::UINT32 && dataType != dt_e::UINT64)
                {
                    LOG_ERROR(logger, "INVALID DATA TYPE FOR FORMAT SPECIFIER \"UNSIGNED 64-BIT INTEGER\"");
                    return Status(Status::Code::BAD_CONFIGURATION_ERROR);
                }
                break;
            case fmt_spec_e::FLOATING_POINT_64:
                if (dataType != dt_e::FLOAT32 && dataType != dt_e::FLOAT64)
                {
                    LOG_ERROR(logger, "INVALID DATA TYPE FOR FORMAT SPECIFIER \"64-BIT FLOATING POINT\"");
                    return Status(Status::Code::BAD_CONFIGURATION_ERROR);
                }
                break;
            case fmt_spec_e::CHARACTER:
            case fmt_spec_e::STRING:
                if (dataType != dt_e::STRING)
                {
                    LOG_ERROR(logger, "INVALID DATA TYPE FOR FORMAT SPECIFIER \"STRING\"");
                    return Status(Status::Code::BAD_CONFIGURATION_ERROR);
                }
                break;
            case fmt_spec_e::HEX_LOWERCASE:
            case fmt_spec_e::HEX_UPPERCASE:
                if (static_cast<uint8_t>(dt_e::UINT64) < static_cast<uint8_t>(dataType) && 
                    static_cast<uint8_t>(dataType) < static_cast<uint8_t>(dt_e::BOOLEAN))
                if (dataType == dt_e::BOOLEAN || dataType == dt_e::FLOAT32 || dataType == dt_e::FLOAT64 || dataType == dt_e::STRING)
                {
                    LOG_ERROR(logger, "INVALID DATA TYPE FOR FORMAT SPECIFIER \"HEXA CODE\"");
                    return Status(Status::Code::BAD_CONFIGURATION_ERROR);
                }
                break;
            default:
                LOG_ERROR(logger, "INVALID DATA TYPE FOR FORMAT SPECIFIER: %u", static_cast<uint8_t>(specifier));
                return Status(Status::Code::BAD_CONFIGURATION_ERROR);
            }
        }
    }

    std::pair<Status, std::vector<dt_e>> NodeValidator::processDataTypes(JsonArray dataTypes)
    {
        std::vector<dt_e> vectorDataTypes;
        const size_t length = dataTypes.size();
        
        if (length == 0)
        {
            LOG_ERROR(logger, "THE ARRAY OF DATA TYPES CANNOT BE EMPTY");
            return std::make_pair(Status(Status::Code::BAD_NO_DATA_AVAILABLE), vectorDataTypes);
        }
        else if (length > UINT8_MAX)
        {
            LOG_ERROR(logger, "THE ARRAY OF DATA TYPES HAS TOO MANY ELEMENTS: %u", length);
            return std::make_pair(Status(Status::Code::BAD_ENCODING_ERROR), vectorDataTypes);
        }
        
        try
        {
            vectorDataTypes.reserve(length);
        }
        catch (const std::bad_alloc& e)
        {
            LOG_ERROR(logger, "%s", e.what());
            return std::make_pair(Status(Status::Code::BAD_OUT_OF_MEMORY), vectorDataTypes);
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(logger, "FAILED TO RESERVE FOR DATA TYPES: %s", e.what());
            return std::make_pair(Status(Status::Code::BAD_UNEXPECTED_ERROR), vectorDataTypes);
        }

        for (const auto dataType : dataTypes)
        {
            if (dataType.is<uint8_t>() == false)
            {
                LOG_ERROR(logger, "DATA TYPE MUST BE AN 8-BIT UNSIGNED INTEGER");

                vectorDataTypes.clear();
                return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), vectorDataTypes);
            }
            
            const auto retDT = convertToDataType(dataType.as<uint8_t>());
            if (retDT.first.ToCode() != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO CONVERT TO DATA TYPE: %s", retDT.first.c_str());

                vectorDataTypes.clear();
                return std::make_pair(retDT.first, vectorDataTypes);
            }
            
            try
            {
                vectorDataTypes.emplace_back(retDT.second);
            }
            catch(const std::exception& e)
            {
                LOG_ERROR(logger, "FAILED TO EMPLACE DATA TYPE: %s", e.what());

                vectorDataTypes.clear();
                return std::make_pair(Status(Status::Code::BAD_UNEXPECTED_ERROR), vectorDataTypes);
            }
        }

        return std::make_pair(Status(Status::Code::GOOD), vectorDataTypes);
    }
    
    /**
     * @return std::pair<Status, std::vector<DataUnitOrder>> 
     *     @li Status::Code::GOOD 설정 값이 올바릅니다.
     *     @li Status::Code::GOOD_NO_DATA 설정 값이 NULL 입니다.
     *     @li Status::Code::BAD_ENCODING_ERROR 설정 형식이 올바르지 않습니다.
     *     @li Status::Code::BAD_DATA_ENCODING_INVALID 설정 값이 올바르지 않습니다.
     *     @li Status::Code::BAD_OUT_OF_RANGE 키(key) 값이 int32_t 타입으로 변환 가능한 범위를 벗어났습니다.
     *     @li Status::Code::BAD_OUT_OF_MEMORY 설정 값을 저장에 할당 가능한 메모리가 부족합니다.
     *     @li Status::Code::BAD_UNEXPECTED_ERROR 알 수 없는 예외가 발생했습니다.
     */
    std::pair<Status, std::vector<DataUnitOrder>> NodeValidator::processDataUnitOrders(JsonVariant dataUnitOrders)
    {
        std::vector<DataUnitOrder> vectorDataUnitOrder;

        if (dataUnitOrders.isNull() == true)
        {
            LOG_VERBOSE(logger, "Data unit order values were not provied");
            return std::make_pair(Status(Status::Code::GOOD_NO_DATA), vectorDataUnitOrder);
        }

        if (dataUnitOrders.is<JsonArray>() == false)
        {
            LOG_ERROR(logger, "DATA UNIT ORDERS MUST BE JSON ARRAY");
            return std::make_pair(Status(Status::Code::BAD_ENCODING_ERROR), vectorDataUnitOrder);
        }

        JsonArray arrayDataUnitOrders = dataUnitOrders.as<JsonArray>();
        const size_t length = arrayDataUnitOrders.size();

        if (length == 0)
        {
            LOG_ERROR(logger, "DATA UNIT ORDERS CANNOT BE AN EMPTY ARRAY");
            return std::make_pair(Status(Status::Code::BAD_ENCODING_ERROR), vectorDataUnitOrder);
        }
        else if (length > UINT8_MAX)
        {
            LOG_ERROR(logger, "THE ARRAY OF DATA UNIT ORDERS HAS TOO MANY ELEMENTS: %u", length);
            return std::make_pair(Status(Status::Code::BAD_ENCODING_ERROR), vectorDataUnitOrder);
        }
        
        try
        {
            vectorDataUnitOrder.reserve(length);
        }
        catch (const std::bad_alloc& e)
        {
            LOG_ERROR(logger, "%s", e.what());
            return std::make_pair(Status(Status::Code::BAD_OUT_OF_MEMORY), vectorDataUnitOrder);
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(logger, "FAILED TO RESERVE FOR DATA UNIT ORDERS: %s", e.what());
            return std::make_pair(Status(Status::Code::BAD_UNEXPECTED_ERROR), vectorDataUnitOrder);
        }

        for (auto subarrayDataUnitOrders : arrayDataUnitOrders)
        {
            if (subarrayDataUnitOrders.is<JsonArray>() == false)
            {
                LOG_ERROR(logger, "ELEMENT ARRAY MUST A JSON ARRAY");

                vectorDataUnitOrder.clear();
                return std::make_pair(Status(Status::Code::BAD_ENCODING_ERROR), vectorDataUnitOrder);
            }
            
            JsonArray elementsArray = subarrayDataUnitOrders.as<JsonArray>();
            const size_t elementsLength = elementsArray.size();

            if (elementsLength == 0)
            {
                LOG_ERROR(logger, "SUB-ARRAY OF DATA UNIT ORDERS CANNOT BE EMPTY");

                vectorDataUnitOrder.clear();
                return std::make_pair(Status(Status::Code::BAD_ENCODING_ERROR), vectorDataUnitOrder);
            }
            else if (elementsLength > UINT8_MAX)
            {
                LOG_ERROR(logger, "THE SUB-ARRAY OF DATA UNIT ORDERS HAS TOO MANY ELEMENTS: %u", elementsLength);

                vectorDataUnitOrder.clear();
                return std::make_pair(Status(Status::Code::BAD_ENCODING_ERROR), vectorDataUnitOrder);
            }
            
            DataUnitOrder dataUnitOrder(elementsLength);
            for (auto element : elementsArray)
            {
                if (element.is<std::string>() == false)
                {
                    LOG_ERROR(logger, "ELEMENT OF DATA UNIT ORDERS MUST BE A STRING");

                    vectorDataUnitOrder.clear();
                    return std::make_pair(Status(Status::Code::BAD_ENCODING_ERROR), vectorDataUnitOrder);
                }

                const auto retConvert = convertToDataUnitOrderType(element.as<std::string>());
                if (retConvert.first.ToCode() != Status::Code::GOOD)
                {
                    LOG_ERROR(logger, "INVALID ELEMENT FOR DATA UNIT ORDER: %s", element.as<std::string>().c_str());

                    vectorDataUnitOrder.clear();
                    return std::make_pair(retConvert.first, vectorDataUnitOrder);
                }
                
                Status ret = dataUnitOrder.EmplaceBack(retConvert.second);
                if (ret != Status::Code::GOOD)
                {
                    LOG_ERROR(logger, "FAILED TO EMPLACE DATA UNIT ORDER ELEMENT: %s", ret.c_str());

                    vectorDataUnitOrder.clear();
                    return std::make_pair(ret, vectorDataUnitOrder);
                }
            }

            try
            {
                vectorDataUnitOrder.emplace_back(dataUnitOrder);
            }
            catch(const std::exception& e)
            {
                LOG_ERROR(logger, "FAILED TO EMPLACE DATA UNIT ORDER: %s", e.what());
                return std::make_pair(Status(Status::Code::BAD_UNEXPECTED_ERROR), vectorDataUnitOrder);
            }
        }
    }
    
    std::pair<Status, adtp_e> NodeValidator::convertToAdressType(const uint8_t type)
    {
        switch (type)
        {
        case 0:
            return std::make_pair(Status(Status::Code::GOOD), adtp_e::NUMERIC);
        case 1:
        case 2:
        case 3:
            return std::make_pair(Status(Status::Code::BAD_SERVICE_UNSUPPORTED), adtp_e::NUMERIC);
        default:
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), adtp_e::NUMERIC);
        }
    }

    std::pair<Status, addr_u> NodeValidator::convertToAddress(JsonVariant address)
    {
        ASSERT((mAddressType.first == Status::Code::GOOD), "ADDRESS TYPE MUST BE CONFIGURED IN ADVANCE");

        addr_u addressUnion;
        addressUnion.Numeric = 0;

        switch (mAddressType.second)
        {
        case adtp_e::NUMERIC:
            if (address.is<uint32_t>() == false)
            {
                LOG_ERROR(logger, "INVALID NUMERIC ADDRESS: VALUE MUST BE 32-BIT UNSIGNED INTEGER");
                return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), addressUnion);
            }
            else
            {
                addressUnion.Numeric = address.as<uint32_t>();
                return std::make_pair(Status(Status::Code::GOOD), addressUnion);
            }
        case adtp_e::STRING:
        case adtp_e::BYTE_STRING:
        case adtp_e::GUID:
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_UNSUPPORTED), addressUnion);
        default:
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), addressUnion);
        }
    }

    /**
     * @return std::pair<Status, mb_area_e> 
     *     @li Status::Code::GOOD 설정 값이 올바릅니다.
     *     @li Status::Code::GOOD_NO_DATA 설정 값이 NULL 입니다.
     *     @li Status::Code::BAD_DATA_ENCODING_INVALID 설정 값이 올바르지 않습니다.
     */
    std::pair<Status, mb_area_e> NodeValidator::convertToModbusArea(JsonVariant modbusArea)
    {
        if (modbusArea.isNull() == true)
        {
            LOG_VERBOSE(logger, "Modbus area value was not provied");
            return std::make_pair(Status(Status::Code::GOOD_NO_DATA), mb_area_e::COILS);
        }

        if (modbusArea.is<uint8_t>() == false)
        {
            LOG_ERROR(logger, "MODBUS AREA MUST BE A 8-BIT UNSIGNED INTEGER");
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), mb_area_e::COILS);
        }
        
        switch (modbusArea.as<uint8_t>())
        {
        case 1:
            return std::make_pair(Status(Status::Code::GOOD), mb_area_e::COILS);
        case 2:
            return std::make_pair(Status(Status::Code::GOOD), mb_area_e::DISCRETE_INPUT);
        case 3:
            return std::make_pair(Status(Status::Code::GOOD), mb_area_e::INPUT_REGISTER);
        case 4:
            return std::make_pair(Status(Status::Code::GOOD), mb_area_e::HOLDING_REGISTER);
        default:
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), mb_area_e::COILS);
        }
    }

    /**
     * @return std::pair<Status, uint8_t> 
     *     @li Status::Code::GOOD 설정 값이 올바릅니다.
     *     @li Status::Code::GOOD_NO_DATA 설정 값이 NULL 입니다.
     *     @li Status::Code::BAD_DATA_ENCODING_INVALID 설정 값이 올바르지 않습니다.
     */
    std::pair<Status, uint8_t> NodeValidator::convertToBitIndex(JsonVariant bitIndex)
    {
        if (bitIndex.isNull() == true)
        {
            LOG_VERBOSE(logger, "Bit index value was not provied");
            return std::make_pair(Status(Status::Code::GOOD_NO_DATA), UINT8_MAX);
        }

        if (bitIndex.is<uint8_t>() == false)
        {
            LOG_ERROR(logger, "BIT INDEX MUST BE A 8-BIT UNSIGNED INTEGER");
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), UINT8_MAX);
        }
        
        return std::make_pair(Status(Status::Code::GOOD), bitIndex.as<uint8_t>());
    }

    /**
     * @return std::pair<Status, uint8_t> 
     *     @li Status::Code::GOOD 설정 값이 올바릅니다.
     *     @li Status::Code::GOOD_NO_DATA 설정 값이 NULL 입니다.
     *     @li Status::Code::BAD_DATA_ENCODING_INVALID 설정 값이 올바르지 않습니다.
     */
    std::pair<Status, uint8_t> NodeValidator::convertToAddressQuantity(JsonVariant addressQuantity)
    {
        if (addressQuantity.isNull() == true)
        {
            LOG_VERBOSE(logger, "Address quantity value was not provied");
            return std::make_pair(Status(Status::Code::GOOD_NO_DATA), UINT8_MAX);
        }

        if (addressQuantity.is<uint8_t>() == false)
        {
            LOG_ERROR(logger, "ADDRESS QUANTITY MUST BE A 8-BIT UNSIGNED INTEGER");
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), UINT8_MAX);
        }
        
        if (addressQuantity.as<uint8_t>() == 0)
        {
            LOG_ERROR(logger, "ADDRESS QUANTITY MUST BE GREATER THAN 0");
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), UINT8_MAX);
        }

        return std::make_pair(Status(Status::Code::GOOD), addressQuantity.as<uint8_t>());
    }

    /**
     * @return std::pair<Status, scl_e> 
     *     @li Status::Code::GOOD 설정 값이 올바릅니다.
     *     @li Status::Code::GOOD_NO_DATA 설정 값이 NULL 입니다.
     *     @li Status::Code::BAD_DATA_ENCODING_INVALID 설정 값이 올바르지 않습니다.
     */
    std::pair<Status, scl_e> NodeValidator::convertToNumericScale(JsonVariant numericScale)
    {
        if (numericScale.isNull() == true)
        {
            LOG_VERBOSE(logger, "Numeric scale value was not provied");
            return std::make_pair(Status(Status::Code::GOOD_NO_DATA), scl_e::NEGATIVE_1);
        }

        if (numericScale.is<int8_t>() == false)
        {
            LOG_ERROR(logger, "NUMERIC SCALE MUST BE A 8-BIT SIGNED INTEGER");
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), scl_e::NEGATIVE_1);
        }
        
        switch (numericScale.as<int8_t>())
        {
        case -3:
            return std::make_pair(Status(Status::Code::GOOD), scl_e::NEGATIVE_3);
        case -2:
            return std::make_pair(Status(Status::Code::GOOD), scl_e::NEGATIVE_2);
        case -1:
            return std::make_pair(Status(Status::Code::GOOD), scl_e::NEGATIVE_1);
        case 1:
            return std::make_pair(Status(Status::Code::GOOD), scl_e::POSITIVE_1);
        case 2:
            return std::make_pair(Status(Status::Code::GOOD), scl_e::POSITIVE_2);
        case 3:
            return std::make_pair(Status(Status::Code::GOOD), scl_e::POSITIVE_3);
        default:
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), scl_e::NEGATIVE_3);
        }
    }

    /**
     * @return std::pair<Status, float> 
     *     @li Status::Code::GOOD 설정 값이 올바릅니다.
     *     @li Status::Code::GOOD_NO_DATA 설정 값이 NULL 입니다.
     *     @li Status::Code::BAD_DATA_ENCODING_INVALID 설정 값이 올바르지 않습니다.
     */
    std::pair<Status, float> NodeValidator::convertToNumericOffset(JsonVariant numericOffset)
    {
        if (numericOffset.isNull() == true)
        {
            LOG_VERBOSE(logger, "Numeric offset value was not provied");
            return std::make_pair(Status(Status::Code::GOOD_NO_DATA), std::numeric_limits<float>::max());
        }

        if (numericOffset.is<float>() == false)
        {
            LOG_ERROR(logger, "NUMERIC OFFSET MUST BE A 32-BIT FLOATING POINT");
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), std::numeric_limits<float>::max());
        }
        
        return std::make_pair(Status(Status::Code::GOOD), numericOffset.as<float>());
    }

    /**
     * @return std::pair<Status, float> 
     *     @li Status::Code::GOOD 설정 값이 올바릅니다.
     *     @li Status::Code::GOOD_NO_DATA 설정 값이 NULL 입니다.
     *     @li Status::Code::BAD_ENCODING_ERROR 설정 형식이 올바르지 않습니다.
     *     @li Status::Code::BAD_DATA_ENCODING_INVALID 설정 값이 올바르지 않습니다.
     *     @li Status::Code::BAD_OUT_OF_RANGE 키(key) 값이 int32_t 타입으로 변환 가능한 범위를 벗어났습니다.
     *     @li Status::Code::BAD_OUT_OF_MEMORY 설정 값을 저장에 할당 가능한 메모리가 부족합니다.
     *     @li Status::Code::BAD_UNEXPECTED_ERROR 알 수 없는 예외가 발생했습니다.
     */
    std::pair<Status, std::map<uint16_t, std::string>> NodeValidator::convertToMappingRules(JsonObject mappingRules)
    {
        std::map<uint16_t, std::string> mapMappingRules;

        if (mappingRules.isNull() == true)
        {
            LOG_VERBOSE(logger, "Mapping rules was not provied");
            return std::make_pair(Status(Status::Code::GOOD_NO_DATA), mapMappingRules);
        }

        if (mappingRules.size() == 0)
        {
            LOG_ERROR(logger, "MAPPING RULES CANNOT BE AN EMPTY JSON OBJECT");
            return std::make_pair(Status(Status::Code::BAD_ENCODING_ERROR), mapMappingRules);
        }

        for (auto rule : mappingRules)
        {
            uint16_t numericKey = 0;
            
            try
            {
                const int key = std::stoi(rule.key().c_str());
                if (key > UINT16_MAX)
                {
                    LOG_ERROR(logger, "KEY MUST BE A 16-BIT UNSIGNED INTEGER");

                    mapMappingRules.clear();
                    return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), mapMappingRules);
                }
                numericKey = static_cast<uint16_t>(key);
            }
            catch (const std::invalid_argument& e)
            {
                LOG_ERROR(logger, "INVALID KEY ARGUMENT: %u", numericKey);

                mapMappingRules.clear();
                return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), mapMappingRules);
            }
            catch (const std::out_of_range& e)
            {
                LOG_ERROR(logger, "KEY OUT OF RANGE: %u", numericKey);

                mapMappingRules.clear();
                return std::make_pair(Status(Status::Code::BAD_OUT_OF_RANGE), mapMappingRules);
            }

            try
            {
                const auto ret = mapMappingRules.emplace(numericKey, rule.value().as<std::string>());
                if (ret.second == false)
                {
                    LOG_ERROR(logger, "KEYS CANNOT BE DUPLICATED: %u", numericKey);

                    mapMappingRules.clear();
                    return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), mapMappingRules);
                }
            }
            catch (const std::bad_alloc& e)
            {
                LOG_ERROR(logger, "%s", e.what());
                return std::make_pair(Status(Status::Code::BAD_OUT_OF_MEMORY), mapMappingRules);
            }
            catch(const std::exception& e)
            {
                LOG_ERROR(logger, "FAILED TO EMPLACE MAPPING RULE: %s", e.what());
                return std::make_pair(Status(Status::Code::BAD_UNEXPECTED_ERROR), mapMappingRules);
            }
        }

        return std::make_pair(Status(Status::Code::GOOD), mapMappingRules);
    }

    /**
     * @return std::pair<Status, float> 
     *     @li Status::Code::GOOD 설정 값이 올바릅니다.
     *     @li Status::Code::BAD_DATA_ENCODING_INVALID 설정 값이 올바르지 않습니다.
     *     @li Status::Code::BAD_OUT_OF_RANGE 키(key) 값이 int32_t 타입으로 변환 가능한 범위를 벗어났습니다.
     */
    std::pair<Status, ord_t> NodeValidator::convertToDataUnitOrderType(const std::string& value)
    {
        std::string stringIndex;
        ord_t dataUnitOrder;

        const char dataUnit = value.at(0);
        if (dataUnit == 'W')
        {
            dataUnitOrder.DataUnit = data_unit_e::WORD;
            stringIndex = value.substr(1);
        }
        else if (dataUnit == 'B')
        {
            dataUnitOrder.DataUnit = data_unit_e::BYTE;

            const char dataUnit = value.at(1);
            if (dataUnit == 'H')
            {
                dataUnitOrder.ByteOrder = byte_order_e::HIGHER;
            }
            else if (dataUnit == 'L')
            {
                dataUnitOrder.ByteOrder = byte_order_e::LOWER;
            }
            else
            {
                LOG_ERROR(logger, "BYTE ORDER MUST BE FOLLOWED WHEN THE DATA UNIT IS BYTE");
                return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), dataUnitOrder);
            }

            stringIndex = value.substr(2);
        }
        else
        {
            LOG_ERROR(logger, "DATA UNIT ORDER MUST START WITH 'W' OR 'B'");
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), dataUnitOrder);
        }
        
        for (char ch : stringIndex)
        {
            if (std::isdigit(ch) == false)
            {
                LOG_ERROR(logger, "INDEX MUST CONSIST OF DIGITS");
                return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), dataUnitOrder);
            }
        }

        int numericIndex = 0;
        try
        {
            numericIndex = std::stoi(stringIndex);
            if (numericIndex > UINT8_MAX)
            {
                LOG_ERROR(logger, "INDEX MUST BE AN 8-BIT UNSIGNED INTEGER");
                return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), dataUnitOrder);
            }
        }
        catch (const std::invalid_argument& e)
        {
            LOG_ERROR(logger, "INVALID INDEX ARGUMENT: %s", stringIndex.c_str());
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), dataUnitOrder);
        }
        catch (const std::out_of_range& e)
        {
            LOG_ERROR(logger, "INDEX VALUE OUT OF RANGE: %s", stringIndex.c_str());
            return std::make_pair(Status(Status::Code::BAD_OUT_OF_RANGE), dataUnitOrder);
        }

        dataUnitOrder.Index = numericIndex;
        return std::make_pair(Status(Status::Code::GOOD), dataUnitOrder);
    }

    std::pair<Status, dt_e> NodeValidator::convertToDataType(const uint8_t dataType)
    {
        switch (dataType)
        {
        case  0:
            return std::make_pair(Status(Status::Code::GOOD), dt_e::BOOLEAN);
        case  1:
            return std::make_pair(Status(Status::Code::GOOD), dt_e::INT8);
        case  2:
            return std::make_pair(Status(Status::Code::GOOD), dt_e::UINT8);
        case  3:
            return std::make_pair(Status(Status::Code::GOOD), dt_e::INT16);
        case  4:
            return std::make_pair(Status(Status::Code::GOOD), dt_e::UINT16);
        case  5:
            return std::make_pair(Status(Status::Code::GOOD), dt_e::INT32);
        case  6:
            return std::make_pair(Status(Status::Code::GOOD), dt_e::UINT32);
        case  7:
            return std::make_pair(Status(Status::Code::GOOD), dt_e::INT64);
        case  8:
            return std::make_pair(Status(Status::Code::GOOD), dt_e::UINT64);
        case  9:
            return std::make_pair(Status(Status::Code::GOOD), dt_e::FLOAT32);
        case 10:
            return std::make_pair(Status(Status::Code::GOOD), dt_e::FLOAT64);
        case 11:
            return std::make_pair(Status(Status::Code::GOOD), dt_e::STRING);
        default:
            return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), dt_e::BOOLEAN);
        }
    }

    std::pair<Status, std::string> NodeValidator::convertToFormatString(const JsonVariant formatString)
    {
        if (formatString.isNull() == true)
        {
            LOG_VERBOSE(logger, "Format string value is not provied");
            return std::make_pair(Status(Status::Code::GOOD_NO_DATA), std::string());
        }

        if (formatString.is<const char*>() == false)
        {
            LOG_ERROR(logger, "FORMAT STRING MUST BE A STRING");
            return std::make_pair(Status(Status::Code::BAD_ENCODING_ERROR), std::string());
        }
        
        const char* format = formatString.as<const char*>();
        if (strlen(format) == 0)
        {
            LOG_ERROR(logger, "FORMAT STRING CANNOT BE AN EMPTY STRING");
            return std::make_pair(Status(Status::Code::BAD_ENCODING_ERROR), std::string());
        }
        
        while (*format)
        {
            if (*format == '%')
            {
                ++format;

                while (isdigit(*format) || *format == '.')
                {
                    ++format;
                }

                bool isLong     = false;
                bool isLongLong = false;
                if (*format == 'l')
                {
                    ++format;
                    isLong = true;

                    if (*format == 'l')
                    {
                        isLongLong = true;
                        ++format;
                    }
                }

                fmt_spec_e formatSpecifier;
                if (*format == 'd')
                {
                    if (isLongLong == true)
                    {
                        formatSpecifier = fmt_spec_e::INTEGER_64;
                    }
                    else
                    {
                        formatSpecifier = fmt_spec_e::INTEGER_32;
                    }
                }
                else if (*format == 'u')
                {
                    if (isLongLong == true)
                    {
                        formatSpecifier = fmt_spec_e::UNSIGNED_INTEGER_64;
                    }
                    else
                    {
                        formatSpecifier = fmt_spec_e::UNSIGNED_INTEGER_32;
                    }
                }
                else if (*format == 'f')
                {
                    if (isLongLong == true)
                    {
                        goto INVALID_SPECIFIER;
                    }
                    else
                    {
                        formatSpecifier = fmt_spec_e::FLOATING_POINT_64;
                    }
                }
                else if (*format == 'c')
                {
                    if (isLong == true || isLongLong == true)
                    {
                        goto INVALID_SPECIFIER;
                    }
                    else
                    {
                        formatSpecifier = fmt_spec_e::CHARACTER;
                    }
                }
                else if (*format == 's')
                {
                    if (isLong == true || isLongLong == true)
                    {
                        goto INVALID_SPECIFIER;
                    }
                    else
                    {
                        formatSpecifier = fmt_spec_e::STRING;
                    }
                }
                else if (*format == 'x')
                {
                    if (isLong == true || isLongLong == true)
                    {
                        goto INVALID_SPECIFIER;
                    }
                    else
                    {
                        formatSpecifier = fmt_spec_e::HEX_LOWERCASE;
                    }
                }
                else if (*format == 'X')
                {
                    if (isLong == true || isLongLong == true)
                    {
                        goto INVALID_SPECIFIER;
                    }
                    else
                    {
                        formatSpecifier = fmt_spec_e::HEX_UPPERCASE;
                    }
                }
                else
                {
                    goto INVALID_SPECIFIER;
                }

                try
                {
                    mVectorFormatSpecifier.emplace_back(formatSpecifier);
                }
                catch (const std::bad_alloc& e)
                {
                    LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY: %s", e.what());
                    return std::make_pair(Status(Status::Code::BAD_OUT_OF_MEMORY), std::string());
                }
                catch(const std::exception& e)
                {
                    LOG_ERROR(logger, "FAILED TO EMPLACE FORMAT SPECIFIER: %s", e.what());
                    return std::make_pair(Status(Status::Code::BAD_UNEXPECTED_ERROR), std::string());
                }
                
                ++format;
            }
            else
            {
                ++format;
            }
        }

        if (mVectorFormatSpecifier.size() == 0)
        {
            LOG_ERROR(logger, "NO FORMAT SPECIFIER IS IN THE FORMAT STRING: %s", format);
            return std::make_pair(Status(Status::Code::BAD_ENCODING_ERROR), std::string());
        }
        else
        {
            return std::make_pair(Status(Status::Code::GOOD), formatString.as<std::string>());
        }
        

    INVALID_SPECIFIER:
        LOG_ERROR(logger, "INVALID FORMAT STRING: %s", format);

        mVectorFormatSpecifier.clear();
        return std::make_pair(Status(Status::Code::BAD_DATA_ENCODING_INVALID), std::string());
    }
}}