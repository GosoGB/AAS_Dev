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
        : mAddressType(rsc_e::UNCERTAIN, adtp_e::NUMERIC)
        , mAddress(rsc_e::UNCERTAIN, addr_u())
        , mModbusArea(rsc_e::UNCERTAIN, mb_area_e::COILS)
        , mBitIndex(rsc_e::UNCERTAIN, 0)
        , mAddressQuantity(rsc_e::UNCERTAIN, 0)
        , mNumericScale(rsc_e::UNCERTAIN, scl_e::NEGATIVE_1)
        , mNumericOffset(rsc_e::UNCERTAIN, 0.0f)
        , mMappingRules(rsc_e::UNCERTAIN, std::map<uint16_t, std::string>())
        , mDataUnitOrders(rsc_e::UNCERTAIN, std::vector<DataUnitOrder>())
        , mDataTypes(rsc_e::UNCERTAIN, std::vector<muffin::jarvis::dt_e>())
        , mFormatString(rsc_e::UNCERTAIN, std::string())
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

    std::pair<rsc_e, std::string> NodeValidator::Inspect(const cfg_key_e key, const JsonArray arrayCIN, cin_vector* outVector)
    {
        ASSERT((outVector != nullptr), "OUTPUT PARAMETER <outVector> CANNOT BE A NULL POINTER");
        ASSERT((arrayCIN.isNull() == false), "OUTPUT PARAMETER <arrayCIN> CANNOT BE NULL");
        ASSERT((arrayCIN.size() != 0), "INPUT PARAMETER <arrayCIN> CANNOT BE 0 IN LENGTH");
        ASSERT((key == cfg_key_e::NODE), "CONFIG CATEGORY DOES NOT MATCH");

        for (JsonObject json : arrayCIN)
        {
            rsc_e rsc = validateMandatoryKeys(json);
            if (rsc != rsc_e::GOOD)
            {
                return std::make_pair(rsc, "INVALID NODE: MANDATORY KEY CANNOT BE MISSING");
            }

            rsc = validateMandatoryValues(json);
            if (rsc != rsc_e::GOOD)
            {
                return std::make_pair(rsc, "INVALID NODE: MANDATORY KEY'S VALUE CANNOT BE NULL");
            }

            mNodeID = json["id"].as<std::string>();
            if (mNodeID.length() != 4)
            {
                const std::string message = "INVALID NODE ID: " + mNodeID;
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }

            mAddressType = convertToAdressType(json["adtp"].as<uint8_t>());
            if (mAddressType.first != rsc_e::GOOD)
            {
                const std::string message = "UNDEFINED OR UNSUPPORTED ADDRESS TYPE: " + std::to_string(json["adtp"].as<uint8_t>());
                return std::make_pair(mAddressType.first, message);
            }
            
            mAddress = convertToAddress(json["addr"].as<JsonVariant>());
            if (mAddress.first != rsc_e::GOOD)
            {
                const std::string message = "INVALID NODE ADDRESS";
                return std::make_pair(mAddress.first, message);
            }
            
            mDataTypes = processDataTypes(json["dt"].as<JsonArray>());
            if (mDataTypes.first != rsc_e::GOOD)
            {
                const std::string message = "INVALID DATA TYPES";
                return std::make_pair(mDataTypes.first, message);
            }

            mUID = json["uid"].as<std::string>();
            if (std::regex_match(mUID, mPatternUID) == false)
            {
                const std::string message = "INVALID UID : " + mUID;
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }

            mDisplayName = json["name"].as<std::string>();
            mDisplayUnit = json["unit"].as<std::string>();
            mIsEventType = json["event"].as<bool>();
            /*Node 설정 형식에서 필수로 입력해야 하는 속성은 모두 유효합니다.*/

            mModbusArea = convertToModbusArea(json["area"].as<JsonVariant>());
            if (mModbusArea.first != rsc_e::GOOD && 
                mModbusArea.first != rsc_e::GOOD_NO_DATA)
            {
                const std::string message = "FAILED TO CONVERT TO MODBUS AREA";
                return std::make_pair(mModbusArea.first, message);
            }

            mBitIndex = convertToBitIndex(json["bit"].as<JsonVariant>());
            if (mBitIndex.first != rsc_e::GOOD && 
                mBitIndex.first != rsc_e::GOOD_NO_DATA)
            {
                const std::string message = "FAILED TO CONVERT TO BIT INDEX";
                return std::make_pair(mBitIndex.first, message);
            }

            mAddressQuantity = convertToAddressQuantity(json["qty"].as<JsonVariant>());
            if (mAddressQuantity.first != rsc_e::GOOD && 
                mAddressQuantity.first != rsc_e::GOOD_NO_DATA)
            {
                const std::string message = "FAILED TO CONVERT TO ADDRESS QUANTITY";
                return std::make_pair(mAddressQuantity.first, message);
            }

            mNumericScale = convertToNumericScale(json["scl"].as<JsonVariant>());
            if (mNumericScale.first != rsc_e::GOOD && 
                mNumericScale.first != rsc_e::GOOD_NO_DATA)
            {
                const std::string message = "FAILED TO CONVERT TO NUMERIC SCALE";
                return std::make_pair(mNumericScale.first, message);
            }

            mNumericOffset = convertToNumericOffset(json["ofst"].as<JsonVariant>());
            if (mNumericOffset.first != rsc_e::GOOD && 
                mNumericOffset.first != rsc_e::GOOD_NO_DATA)
            {
                const std::string message = "FAILED TO CONVERT TO NUMERIC OFFSET";
                return std::make_pair(mNumericOffset.first, message);
            }

            mMappingRules = convertToMappingRules(json["map"].as<JsonVariant>());
            if (mMappingRules.first != rsc_e::GOOD && 
                mMappingRules.first != rsc_e::GOOD_NO_DATA)
            {
                const std::string message = "FAILED TO CONVERT TO MAPPING RULES";
                return std::make_pair(mMappingRules.first, message);
            }

            mDataUnitOrders = processDataUnitOrders(json["ord"].as<JsonVariant>());
            if (mDataUnitOrders.first != rsc_e::GOOD &&
                mDataUnitOrders.first != rsc_e::GOOD_NO_DATA)
            {
                const std::string message = "FAILED TO CONVERT TO DATA UNIT ORDERS";
                return std::make_pair(mDataUnitOrders.first, message);
            }

            mFormatString = convertToFormatString(json["fmt"].as<JsonVariant>());
            if (mFormatString.first != rsc_e::GOOD &&
                mFormatString.first != rsc_e::GOOD_NO_DATA)
            {
                const std::string message = "FAILED TO CONVERT TO FORMAT STRING";
                return std::make_pair(mFormatString.first, message);
            }

            std::pair<rsc_e, std::string> result = validateModbusArea();
            if (result.first != rsc_e::GOOD)
            {
                LOG_ERROR(logger, "INVALID MODBUS AREA CONFIG");
                return result;
            }
            
            result = validateBitIndex();
            if (result.first != rsc_e::GOOD)
            {
                LOG_ERROR(logger, "INVALID BIT INDEX CONFIG");
                return result;
            }

            result = validateAddressQuantity();
            if (result.first != rsc_e::GOOD &&
                result.first != rsc_e::GOOD_NO_DATA)
            {
                LOG_ERROR(logger, "INVALID ADDRESS QUANTITY CONFIG");
                return result;
            }
            
            result = validateNumericScale();
            if (result.first != rsc_e::GOOD)
            {
                LOG_ERROR(logger, "INVALID NUMERIC SCALE CONFIG");
                return result;
            }

            result = validateNumericOffset();
            if (result.first != rsc_e::GOOD)
            {
                LOG_ERROR(logger, "INVALID NUMERIC OFFSET CONFIG");
                return result;
            }

            result = validateMappingRules();
            if (result.first != rsc_e::GOOD)
            {
                LOG_ERROR(logger, "INVALID MAPPING RULES CONFIG");
                return result;
            }

            result = validateDataUnitOrders();
            if (result.first != rsc_e::GOOD)
            {
                LOG_ERROR(logger, "INVALID DATA UNIT ORDER CONFIG");
                return result;
            }

            result = validateDataTypes();
            if (result.first != rsc_e::GOOD)
            {
                LOG_ERROR(logger, "INVALID DATA TYPES CONFIG");
                return result;
            }

            result = validateFormatString();
            if (result.first != rsc_e::GOOD)
            {
                LOG_ERROR(logger, "INVALID FORMAT STRING CONFIG");
                return result;
            }

            config::Node* node = new(std::nothrow) config::Node();
            if (node == nullptr)
            {
                return std::make_pair(rsc_e::BAD_OUT_OF_MEMORY, "FAILED TO ALLOCATE MEMORY FOR NODE CONFIG");
            }

            node->SetNodeID(mNodeID);
            node->SetAddressType(mAddressType.second);
            node->SetAddrress(mAddress.second);
            node->SetDataTypes(std::move(mDataTypes.second));
            node->SetDeprecableUID(mUID);
            node->SetDeprecableDisplayName(mDisplayName);
            node->SetDeprecableDisplayUnit(mDisplayUnit);
            node->SetAttributeEvent(mIsEventType);

            if (mModbusArea.first == rsc_e::GOOD)
            {
                node->SetModbusArea(mModbusArea.second);
            }
            
            if (mBitIndex.first == rsc_e::GOOD)
            {
                node->SetBitIndex(mBitIndex.second);
            }

            if (mAddressQuantity.first == rsc_e::GOOD)
            {
                node->SetNumericAddressQuantity(mAddressQuantity.second);
            }

            if (mNumericScale.first == rsc_e::GOOD)
            {
                node->SetNumericScale(mNumericScale.second);
            }
            
            if (mNumericOffset.first == rsc_e::GOOD)
            {
                node->SetNumericOffset(mNumericOffset.second);
            }

            if (mMappingRules.first == rsc_e::GOOD)
            {
                node->SetMappingRules(std::move(mMappingRules.second));
            }

            if (mDataUnitOrders.first == rsc_e::GOOD)
            {
                node->SetDataUnitOrders(std::move(mDataUnitOrders.second));
            }
            
            if (mFormatString.first == rsc_e::GOOD)
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

                delete node;
                outVector->clear();
                return std::make_pair(rsc_e::BAD_OUT_OF_MEMORY, "FAILED TO ALLOCATE MEMORY FOR NODE");
            }
            catch(const std::exception& e)
            {
                LOG_ERROR(logger, "FAILED TO EMPLACE NODE CIN: %s", e.what());
                
                delete node;
                outVector->clear();
                return std::make_pair(rsc_e::BAD_UNEXPECTED_ERROR, "FAILED TO EMPLACE NODE CIN");
            }

            mNodeID.clear();
            mAddressType      = std::make_pair(rsc_e::UNCERTAIN, adtp_e::NUMERIC);
            mAddress          = std::make_pair(rsc_e::UNCERTAIN, addr_u());
            mModbusArea       = std::make_pair(rsc_e::UNCERTAIN, mb_area_e::COILS);
            mBitIndex         = std::make_pair(rsc_e::UNCERTAIN, 0);
            mAddressQuantity  = std::make_pair(rsc_e::UNCERTAIN, 0);
            mNumericScale     = std::make_pair(rsc_e::UNCERTAIN, scl_e::NEGATIVE_1);
            mNumericOffset    = std::make_pair(rsc_e::UNCERTAIN, 0.0f);
            mMappingRules     = std::make_pair(rsc_e::UNCERTAIN, std::map<uint16_t, std::string>());
            mDataUnitOrders   = std::make_pair(rsc_e::UNCERTAIN, std::vector<DataUnitOrder>());
            mDataTypes        = std::make_pair(rsc_e::UNCERTAIN, std::vector<muffin::jarvis::dt_e>());
            mFormatString     = std::make_pair(rsc_e::UNCERTAIN, std::string());
            mUID.clear();
            mDisplayName.clear();
            mDisplayUnit.clear();
            mIsEventType = false;
            mVectorFormatSpecifier.clear();
        }

        return std::make_pair(rsc_e::GOOD, "GOOD");    
    }

    /**
     * @return Status
     *     @li rsc_e::GOOD 모든 설정 키(key)가 있습니다.
     *     @li Status::Code::BAD_ENCODING_ERROR 설정 키(key) 중 하나 이상이 없습니다.
     */
    rsc_e NodeValidator::validateMandatoryKeys(const JsonObject json)
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
            return rsc_e::GOOD;
        }
        else
        {
            return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
        }
    }

    /**
     * @return Status
     *     @li rsc_e::GOOD 필수 설정 키(key)에 값이 존재하며 형식이 올바릅니다.
     *     @li Status::Code::BAD_ENCODING_ERROR 필수 설정 키(key) 중 하나 이상의 값의 형식이 올바르지 않습니다.
     */
    rsc_e NodeValidator::validateMandatoryValues(const JsonObject json)
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
            return rsc_e::GOOD;
        }
        else
        {
            return rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
        }
    }

    /**
     * @return Status
     *     @li rsc_e::GOOD Modbus 메모리 영역 설정이 없거나, 설정 정보가 유효합니다.
     *     @li Status::Code::BAD_CONFIGURATION_ERROR 다른 Node 속성을 고려했을 때 선결조건 또는 관계가 유효하지 않습니다.
     *     @li Status::Code::BAD_SERVICE_UNSUPPORTED 현재 펌웨어 버전에서는 Modbus 프로토콜의 확장 주소 체계를 지원하지 않습니다.
     */
    std::pair<rsc_e, std::string> NodeValidator::validateModbusArea()
    {
        if (mModbusArea.first == rsc_e::GOOD_NO_DATA)
        {
            const std::string message = "Modbus memory area is not enabled";
            return std::make_pair(rsc_e::GOOD, message);
        }
        
        if (mAddressType.second != adtp_e::NUMERIC)
        {
            const std::string message = "ADDRESS TYPE MUST BE NUMERIC TO ENABLE MODBUS PROTOCOL";
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }
        
        if (mAddress.second.Numeric > 0x270F)
        {
            const std::string message = "EXTENDED REGISTER ADDRESS FOR MODBUS PROTOCOL IS NOT SUPPORTED";
            return std::make_pair(rsc_e::BAD_UNSUPPORTED_CONFIGURATION, message);
        }

        if (mModbusArea.second == mb_area_e::COILS || mModbusArea.second == mb_area_e::DISCRETE_INPUT)
        {
            if (mBitIndex.first == rsc_e::GOOD)
            {
                const std::string message = "MODBUS AREA CANNOT BE CONFIGURED AS \"Coils\" OR \"Discrete Inputs\" WITH BIT INDEX";
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
            
            if (mAddressQuantity.first == rsc_e::GOOD)
            {
                const std::string message = "MODBUS AREA CANNOT BE CONFIGURED AS \"Coils\" OR \"Discrete Inputs\" WITH ADDRESS QUANTITY";
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
            
            if (mNumericScale.first == rsc_e::GOOD)
            {
                const std::string message = "MODBUS AREA CANNOT BE CONFIGURED AS \"Coils\" OR \"Discrete Inputs\" WITH NUMERIC SCALE";
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
            
            if (mNumericOffset.first == rsc_e::GOOD)
            {
                const std::string message = "MODBUS AREA CANNOT BE CONFIGURED AS \"Coils\" OR \"Discrete Inputs\" WITH NUMERIC OFFSET";
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
            
            if (mDataUnitOrders.first == rsc_e::GOOD)
            {
                const std::string message = "MODBUS AREA CANNOT BE CONFIGURED AS \"Coils\" OR \"Discrete Inputs\" WITH DATA UNIT ORDERS";
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
            
            if (mDataTypes.second.size() != 1)
            {
                const std::string message = "MODBUS AREA CANNOT BE CONFIGURED AS \"Coils\" OR \"Discrete Inputs\" IF MORE THAN ONE DATA TYPE IS PROVIDED";
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
            
            if (mDataTypes.second.front() != dt_e::BOOLEAN)
            {
                const std::string message = "MODBUS AREA CANNOT BE CONFIGURED AS \"Coils\" OR \"Discrete Inputs\" IF DATA TYPE IS NOT BOOLEAN";
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
            
            if (mFormatString.first == rsc_e::GOOD)
            {
                const std::string message = "MODBUS AREA CANNOT BE CONFIGURED AS \"Coils\" OR \"Discrete Inputs\" WITH FORMAT STRING";
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }

            LOG_VERBOSE(logger, "Modbus area can be configured as \"Coils\" or \"Discrete Inputs\"");
            return std::make_pair(rsc_e::GOOD, "GOOD");
        }
        else
        {
            if(mBitIndex.first == rsc_e::GOOD)
            {
                if (mAddressQuantity.first == rsc_e::GOOD)
                {
                    const std::string message = "MODBUS AREA CANNOT BE CONFIGURED AS \"Input Registers\" OR \"Holding Registers\" WITH BOTH BIT INDEX AND ADDRESS QUANTITY";
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
                }
            }
            else if (mAddressQuantity.first == rsc_e::GOOD_NO_DATA)
            {
                const std::string message = "MODBUS AREA CANNOT BE CONFIGURED AS \"Input Registers\" OR \"Holding Registers\" WITHOUT ADDRESS QUANTITY";
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
            
            LOG_VERBOSE(logger, "Modbus area can be configured as \"Input Registers\" or \"Holding Registers\"");
            return std::make_pair(rsc_e::GOOD, "GOOD");
        }
    }

    /**
     * @return Status
     *     @li rsc_e::GOOD 비트 인덱스 설정이 없거나, 설정 정보가 유효합니다.
     *     @li Status::Code::BAD_CONFIGURATION_ERROR 다른 Node 속성을 고려했을 때 선결조건 또는 관계가 유효하지 않습니다.
     */
    std::pair<rsc_e, std::string> NodeValidator::validateBitIndex()
    {
        if (mBitIndex.first == rsc_e::GOOD_NO_DATA)
        {
            const std::string message = "Bit index is not enabled";
            return std::make_pair(rsc_e::GOOD, message);
        }

        if (mAddressQuantity.first == rsc_e::GOOD)
        {
            const std::string message = "BIT INDEX CANNOT BE CONFIGURED WITH ADDRESS QUANTITY";
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }
        
        if (mNumericScale.first == rsc_e::GOOD)
        {
            const std::string message = "BIT INDEX CANNOT BE CONFIGURED WITH NUMERIC SCALE";
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }
        
        if (mNumericOffset.first == rsc_e::GOOD)
        {
            const std::string message = "BIT INDEX CANNOT BE CONFIGURED WITH NUMERIC OFFSET";
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        if (mFormatString.first == rsc_e::GOOD)
        {
            const std::string message = "BIT INDEX CANNOT BE CONFIGURED WITH FORMAT STRING";
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        if (mDataTypes.second.size() != 1)
        {
            const std::string message = "BIT INDEX CANNOT BE CONFIGURED WITH MORE THAN ONE DATA TYPE";
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        if (mDataUnitOrders.first == rsc_e::GOOD)
        {
            if (mDataUnitOrders.second.size() != 1)
            {
                const std::string message = "BIT INDEX CANNOT BE CONFIGURED WITH MORE THAN ONE DATA UNIT ORDER";
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
        }

        if (mModbusArea.first == rsc_e::GOOD)
        {
            if (mModbusArea.second == mb_area_e::COILS || mModbusArea.second == mb_area_e::DISCRETE_INPUT)
            {
                const std::string message = "BIT INDEX CANNOT BE CONFIGURED WITH MODBUS AREA \"Coils\" OR \"Discrete Inputs\"";
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
        }

        switch (mDataTypes.second.front())
        {
            case dt_e::INT8:
            case dt_e::UINT8:
                if (mBitIndex.second > 7)
                {
                    const std::string message = "BIT INDEX OUT OF RANGE FOR 8-BIT INTEGER: " + std::to_string(mBitIndex.second);
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
                }
                break;
            case dt_e::INT16:
            case dt_e::UINT16:
                if (mBitIndex.second > 15)
                {
                    const std::string message = "BIT INDEX OUT OF RANGE FOR 16-BIT INTEGER: " + std::to_string(mBitIndex.second);
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
                }
                break;
            case dt_e::INT32:
            case dt_e::UINT32:
                if (mBitIndex.second > 31)
                {
                    const std::string message = "BIT INDEX OUT OF RANGE FOR 32-BIT INTEGER: " + std::to_string(mBitIndex.second);
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
                }
                break;
            case dt_e::INT64:
            case dt_e::UINT64:
                if (mBitIndex.second > 63)
                {
                    const std::string message = "BIT INDEX OUT OF RANGE FOR 64-BIT INTEGER: " + std::to_string(mBitIndex.second);
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
                }
                break;
            default:
                const std::string message = "INVALID DATA TYPE FOR BIT INDEX: " + std::to_string(static_cast<uint8_t>(mDataTypes.second.front()));
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        LOG_VERBOSE(logger, "Bit index [%u] can be configured with data type: %u",
            mBitIndex.second, static_cast<uint8_t>(mDataTypes.second.front()));

        return std::make_pair(rsc_e::GOOD, "GOOD");
    }

    /**
     * @return Status
     *     @li rsc_e::GOOD Numeric Address Quantity 설정 정보가 유효합니다.
     *     @li Status::Code::BAD_CONFIGURATION_ERROR 다른 Node 속성을 고려했을 때 선결조건 또는 관계가 유효하지 않습니다.
     */
    std::pair<rsc_e, std::string> NodeValidator::validateAddressQuantity()
    {
        if (mAddressType.second != adtp_e::NUMERIC)
        {
            if (mAddressQuantity.first == rsc_e::GOOD)
            {
                const std::string message = "NUMERIC ADDRESS QUANTITY CANNOT BE CONFIGURED IF ADDRESS TYPE IS NOT NUMERIC";
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
            else
            {
                const std::string message = "Numeric address quantity is not enabled";
                return std::make_pair(rsc_e::GOOD, message);
            }
        }
        
        if (mAddressQuantity.first == rsc_e::GOOD_NO_DATA)
        {
            if (mBitIndex.first == rsc_e::GOOD)
            {
                return std::make_pair(rsc_e::GOOD_NO_DATA, "Address quantity is null by bit index config");
            }

            if (mModbusArea.first == rsc_e::GOOD)
            {
                if (mModbusArea.second == mb_area_e::COILS || mModbusArea.second == mb_area_e::DISCRETE_INPUT)
                {
                    return std::make_pair(rsc_e::GOOD_NO_DATA, "Address quantity is null by Modbus area config");
                }
                else
                {   // WORD면 QTY가 있어야대
                    const std::string message = "NUMERIC ADDRESS QUANTITY MUST BE CONFIGURED IF MODBUS AREA \"INPUT REGISTERS\" OR \"HOLDING REGISTERS\"";
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
                }
            }

            // qty , bit , modbus area도 없는 애들은 여기로 빠진다.
            const std::string message = "NUMERIC ADDRESS QUANTITY MUST BE CONFIGURED IF ADDRESS TYPE IS NUMERIC";
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        if (mBitIndex.first == rsc_e::GOOD)
        {
            const std::string message = "NUMERIC ADDRESS QUANTITY CANNOT BE CONFIGURED IF BIT INDEX IS ENABLED";
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        if (mModbusArea.first == rsc_e::GOOD)
        {
            if (mModbusArea.second == mb_area_e::COILS || mModbusArea.second == mb_area_e::DISCRETE_INPUT)
            {
                const std::string message = "NUMERIC ADDRESS QUANTITY CANNOT BE CONFIGURED WITH MODBUS AREA \"Coils\" OR \"Discrete Inputs\"";
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
        }

        LOG_VERBOSE(logger, "Configured numeric address quantity: %u", mAddressQuantity.second);
        return std::make_pair(rsc_e::GOOD, "GOOD");    
    }

    /**
     * @return Status
     *     @li rsc_e::GOOD Numeric Scale 설정이 없거나, 설정 정보가 유효합니다.
     *     @li Status::Code::BAD_CONFIGURATION_ERROR 다른 Node 속성을 고려했을 때 선결조건 또는 관계가 유효하지 않습니다.
     */
    std::pair<rsc_e, std::string> NodeValidator::validateNumericScale()
    {
        if (mNumericScale.first == rsc_e::GOOD_NO_DATA)
        {
            const std::string message = "Numeric scale is not enabled";
            return std::make_pair(rsc_e::GOOD, message);
        }
        
        if (mDataTypes.second.size() != 1)
        {
            const std::string message = "NUMERIC SCALE CANNOT BE CONFIGURED WITH MORE THAN ONE DATA TYPES";
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        if (mDataUnitOrders.first == rsc_e::GOOD)
        {
            if (mDataUnitOrders.second.size() != 1)
            {
                const std::string message = "NUMERIC SCALE CANNOT BE CONFIGURED WITH MORE THAN ONE DATA UNIT ORDERS";
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
        }
        
        switch (mDataTypes.second.front())
        {
            case dt_e::STRING:
            case dt_e::BOOLEAN:
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, "NUMERIC SCALE CANNOT BE CONFIGURED WITH \"STRING\" OR \"BOOLEAN\" DATA TYPE");
            default:
                break;
        }
        
        if (mBitIndex.first == rsc_e::GOOD)
        {
            const std::string message = "NUMERIC SCALE CANNOT BE CONFIGURED IF BIT INDEX IS ENABLED";
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        if (mModbusArea.first == rsc_e::GOOD)
        {
            if (mModbusArea.second == mb_area_e::COILS || mModbusArea.second == mb_area_e::DISCRETE_INPUT)
            {
                const std::string message = "NUMERIC SCALE CANNOT BE CONFIGURED WITH MODBUS AREA \"Coils\" OR \"Discrete Inputs\"";
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
        }

        LOG_VERBOSE(logger, "Configured numeric scale: %d", static_cast<int8_t>(mNumericScale.second));
        return std::make_pair(rsc_e::GOOD, "GOOD");    
    }

    /**
     * @return Status
     *     @li rsc_e::GOOD Numeric Offset 설정이 없거나, 설정 정보가 유효합니다.
     *     @li Status::Code::BAD_CONFIGURATION_ERROR 다른 Node 속성을 고려했을 때 선결조건 또는 관계가 유효하지 않습니다.
     */
    std::pair<rsc_e, std::string> NodeValidator::validateNumericOffset()
    {
        if (mNumericOffset.first == rsc_e::GOOD_NO_DATA)
        {
            const std::string message = "Numeric offset is not enabled";
            return std::make_pair(rsc_e::GOOD, message);
        }

        if (mDataTypes.second.size() != 1)
        {
            const std::string message = "NUMERIC OFFSET CANNOT BE CONFIGURED WITH MORE THAN ONE DATA TYPES";
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        if (mDataUnitOrders.first == rsc_e::GOOD)
        {
            if (mDataUnitOrders.second.size() != 1)
            {
                const std::string message = "NUMERIC OFFSET CANNOT BE CONFIGURED WITH MORE THAN ONE DATA UNIT ORDERS";
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
        }
        
        switch (mDataTypes.second.front())
        {
            case dt_e::STRING:
            case dt_e::BOOLEAN:
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, "NUMERIC OFFSET CANNOT BE CONFIGURED WITH \"STRING\" OR \"BOOLEAN\" DATA TYPE");
            default:
                break;
        }
        
        if (mBitIndex.first == rsc_e::GOOD)
        {
            const std::string message = "NUMERIC OFFSET CANNOT BE CONFIGURED IF BIT INDEX IS ENABLED";
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        if (mModbusArea.first == rsc_e::GOOD)
        {
            if (mModbusArea.second == mb_area_e::COILS || mModbusArea.second == mb_area_e::DISCRETE_INPUT)
            {
                const std::string message = "NUMERIC OFFSET CANNOT BE CONFIGURED WITH MODBUS AREA \"Coils\" OR \"Discrete Inputs\"";
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
        }

        LOG_VERBOSE(logger, "Configured numeric offset: %.2f", mNumericOffset.second);
        return std::make_pair(rsc_e::GOOD, "GOOD");
    }

    /**
     * @return Status
     *     @li rsc_e::GOOD Mapping Rules 설정이 없거나, 설정 정보가 유효합니다.
     *     @li Status::Code::BAD_CONFIGURATION_ERROR 다른 Node 속성을 고려했을 때 선결조건 또는 관계가 유효하지 않습니다.
     */
    std::pair<rsc_e, std::string> NodeValidator::validateMappingRules()
    {
        if (mMappingRules.first == rsc_e::GOOD_NO_DATA)
        {
            const std::string message = "Mapping rules are not enabled";
            return std::make_pair(rsc_e::GOOD, message);
        }

        if (mDataTypes.second.size() != 1)
        {
            const std::string message = "MAPPING RULES CANNOT BE CONFIGURED WITH MORE THAN ONE DATA TYPES";
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        if (mDataUnitOrders.first == rsc_e::GOOD)
        {
            if (mDataUnitOrders.second.size() != 1)
            {
                const std::string message = "MAPPING RULES CANNOT BE CONFIGURED WITH MORE THAN ONE DATA UNIT ORDERS";
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
        }
        
        switch (mDataTypes.second.front())
        {
            case dt_e::STRING:
            case dt_e::FLOAT32:
            case dt_e::FLOAT64:
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, "MAPPING RULES CANNOT BE CONFIGURED WITH \"STRING\", \"FP32\" OR \"FP64\" DATA TYPE");
            default:
                break;
        }

        LOG_VERBOSE(logger, "Configured mapping rules");
        return std::make_pair(rsc_e::GOOD, "GOOD");   
    }

    /**
     * @return Status
     *     @li rsc_e::GOOD Data Unit Orders 설정이 없거나, 설정 정보가 유효합니다.
     *     @li Status::Code::BAD_CONFIGURATION_ERROR 다른 Node 속성을 고려했을 때 선결조건 또는 관계가 유효하지 않습니다.
     */
    std::pair<rsc_e, std::string> NodeValidator::validateDataUnitOrders()
    {
        if (mDataUnitOrders.first == rsc_e::GOOD_NO_DATA)
        {
            const std::string message = "Data unit orders are not enabled";
            return std::make_pair(rsc_e::GOOD, message);
        }
        
        if (mBitIndex.first == rsc_e::GOOD)
        {
            const std::string message = "DATA UNIT ORDERS CANNOT BE CONFIGURED IF BIT INDEX IS ENABLED";
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        if (mModbusArea.first == rsc_e::GOOD)
        {
            if (mModbusArea.second == mb_area_e::COILS || mModbusArea.second == mb_area_e::DISCRETE_INPUT)
            {
                const std::string message = "DATA UNIT ORDERS CANNOT BE CONFIGURED WITH MODBUS AREA \"Coils\" OR \"Discrete Inputs\"";
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
        }
        
        if (mDataUnitOrders.second.size() != mDataTypes.second.size())
        {
            const std::string message = "DATA UNIT ORDERS MUST HAVE EQUAL LENGTH OF ELEMENTS WITH DATA TYPES";
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        for (const auto& dataUnitOrder : mDataUnitOrders.second)
        {
            std::set<uint8_t> setIndex;

            for (const auto& orderType : dataUnitOrder)
            {
                const auto result = setIndex.emplace(orderType.Index);
                if (result.second == false)
                {
                    const std::string message = "DATA UNIT ORDER INDICES CANNOT BE DUPLICATED";
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
                    
                }
            }
        }

        LOG_VERBOSE(logger, "Configured data unit orders");
        return std::make_pair(rsc_e::GOOD, "GOOD");
    }
    
    /**
     * @return Status
     *     @li rsc_e::GOOD Data Types 설정이 없거나, 설정 정보가 유효합니다.
     *     @li Status::Code::BAD_CONFIGURATION_ERROR 다른 Node 속성을 고려했을 때 선결조건 또는 관계가 유효하지 않습니다.
     */
    std::pair<rsc_e, std::string> NodeValidator::validateDataTypes()
    {
        if (mDataTypes.first == rsc_e::GOOD_NO_DATA)
        {
            const std::string message = "Data types are not enabled";
            return std::make_pair(rsc_e::GOOD, message);
        }
        
        if (mDataUnitOrders.first == rsc_e::GOOD)
        {
            if (mDataTypes.second.size() != mDataUnitOrders.second.size())
            {
                const std::string message = "DATA TYPES MUST HAVE EQUAL LENGTH OF ELEMENTS WITH DATA UNIT ORDERS";
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
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
                        return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, "DATA TYPE \"BOOLEAN\" CANNOT BE CONFIGURED WITH DATA UNIT ORDER");
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
                        return std::make_pair(rsc_e::BAD_UNEXPECTED_ERROR, "BAD_UNEXPECTED_ERROR");
                }
            
                if (dataTypeSize != sumOrderSize)
                {
                    const std::string message = "DATA TYPE SIZE DOES NOT MATCH WITH THE SUM OF DATA UNIT ORDERS";
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
                }
            }
        }

        if (mModbusArea.first == rsc_e::GOOD)
        {
            if (mModbusArea.second == mb_area_e::COILS || mModbusArea.second == mb_area_e::DISCRETE_INPUT)
            {
                if (mDataTypes.second.size() != 1)
                {
                    const std::string message = "DATA TYPE MUST HAVE ONE ELEMENT WITH MODBUS AREA \"Coils\" OR \"Discrete Inputs\"";
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
                }
                else if (mDataTypes.second.front() != dt_e::BOOLEAN)
                {
                    const std::string message = "DATA TYPE MUST BE \"BOOLEAN\" WITH MODBUS AREA \"Coils\" OR \"Discrete Inputs\"";
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
                }
            }
            else
            {
                constexpr uint8_t REGISTER_SIZE = 16;
                if(mBitIndex.first == rsc_e::GOOD)
                {
                    constexpr uint8_t ONLY_SINGLE_REGISTER = 1;
                    mAddressQuantity.second = ONLY_SINGLE_REGISTER;
                }
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
                            const std::string message = "DATA TYPE \"BOOLEAN\" CANNOT BE CONFIGURED WITH MODBUS AREA USING REGISTERS";
                            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
                    }
                }

                if (totalRegisterSize != sumDataTypeSize)
                {
                    const std::string message = "TOTAL SIZE OF MODBUS REGISTERS DOES NOT MATCH WITH THE SUM OF EACH DATA TYPES";
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
                }
            }
        }

        LOG_VERBOSE(logger, "Configured data types");
        return std::make_pair(rsc_e::GOOD, "GOOD");
    }

    /**
     * @return Status
     *     @li rsc_e::GOOD Format String 설정이 없거나, 설정 정보가 유효합니다.
     *     @li Status::Code::BAD_CONFIGURATION_ERROR 다른 Node 속성을 고려했을 때 선결조건 또는 관계가 유효하지 않습니다.
     */
    std::pair<rsc_e, std::string> NodeValidator::validateFormatString()
    {
        if (mFormatString.first == rsc_e::GOOD_NO_DATA)
        {
            const std::string message = "Format string is not enabled";
            return std::make_pair(rsc_e::GOOD, message);
        }
        
        if (mBitIndex.first == rsc_e::GOOD)
        {
            const std::string message = "FORMAT STRING CANNOT BE CONFIGURED IF BIT INDEX IS ENABLED";
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        if (mModbusArea.first == rsc_e::GOOD)
        {
            if (mModbusArea.second == mb_area_e::COILS || mModbusArea.second == mb_area_e::DISCRETE_INPUT)
            {
                const std::string message = "FORMAT STRING CANNOT BE CONFIGURED WITH MODBUS AREA \"Coils\" OR \"Discrete Inputs\"";
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
        }

        if (mVectorFormatSpecifier.size() != mDataTypes.second.size())
        {
            const std::string message = "THE NUMBER OF FORMAT SPECIFIERS MUST BE EQUAL TO THE NUMBER OF DATA TYPES";
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
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
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, "INVALID DATA TYPE FOR FORMAT SPECIFIER \"32-BIT INTEGER\"");
                }
                break;
            case fmt_spec_e::INTEGER_64:
                if (dataType != dt_e::INT8  && dataType != dt_e::INT16  && dataType != dt_e::INT32 && dataType != dt_e::INT64)
                {
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, "INVALID DATA TYPE FOR FORMAT SPECIFIER \"64-BIT INTEGER\"");
                }
                break;
            case fmt_spec_e::UNSIGNED_INTEGER_32:
                if (dataType != dt_e::UINT8 && dataType != dt_e::UINT16 && dataType != dt_e::UINT32)
                {
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, "INVALID DATA TYPE FOR FORMAT SPECIFIER \"UNSIGNED 32-BIT INTEGER\"");
                }
                break;
            case fmt_spec_e::UNSIGNED_INTEGER_64:
                if (dataType != dt_e::UINT8 && dataType != dt_e::UINT16 && dataType != dt_e::UINT32 && dataType != dt_e::UINT64)
                {
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, "INVALID DATA TYPE FOR FORMAT SPECIFIER \"UNSIGNED 64-BIT INTEGER\"");
                }
                break;
            case fmt_spec_e::FLOATING_POINT_64:
                if (dataType != dt_e::FLOAT32 && dataType != dt_e::FLOAT64)
                {
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, "INVALID DATA TYPE FOR FORMAT SPECIFIER \"64-BIT FLOATING POINT\"");
                }
                break;
            case fmt_spec_e::CHARACTER:
            case fmt_spec_e::STRING:
                if (dataType != dt_e::STRING)
                {
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, "INVALID DATA TYPE FOR FORMAT SPECIFIER \"STRING\"");
                }
                break;
            case fmt_spec_e::HEX_LOWERCASE:
            case fmt_spec_e::HEX_UPPERCASE:
                if (dataType == dt_e::BOOLEAN || dataType == dt_e::FLOAT32 || dataType == dt_e::FLOAT64 || dataType == dt_e::STRING)
                {
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, "INVALID DATA TYPE FOR FORMAT SPECIFIER \"HEXA CODE\"");
                }
                break;
            default:
                const std::string message = "INVALID DATA TYPE FOR FORMAT SPECIFIER:" + std::to_string(static_cast<uint8_t>(specifier));
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
        }

        return std::make_pair(rsc_e::GOOD, "GOOD");
    }

    std::pair<rsc_e, std::vector<dt_e>> NodeValidator::processDataTypes(JsonArray dataTypes)
    {
        std::vector<dt_e> vectorDataTypes;
        const size_t length = dataTypes.size();
        
        if (length == 0)
        {
            
            LOG_ERROR(logger, "THE ARRAY OF DATA TYPES CANNOT BE EMPTY");
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, vectorDataTypes);
        }
        else if (length > UINT8_MAX)
        {
            LOG_ERROR(logger, "THE ARRAY OF DATA TYPES HAS TOO MANY ELEMENTS: %u", length);
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, vectorDataTypes);
        }
        
        try
        {
            vectorDataTypes.reserve(length);
        }
        catch (const std::bad_alloc& e)
        {
            LOG_ERROR(logger, "%s", e.what());
            return std::make_pair(rsc_e::BAD_OUT_OF_MEMORY, vectorDataTypes);
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(logger, "FAILED TO RESERVE FOR DATA TYPES: %s", e.what());
            return std::make_pair(rsc_e::BAD_UNEXPECTED_ERROR, vectorDataTypes);
        }

        for (const auto dataType : dataTypes)
        {
            if (dataType.is<uint8_t>() == false)
            {
                LOG_ERROR(logger, "DATA TYPE MUST BE AN 8-BIT UNSIGNED INTEGER");

                vectorDataTypes.clear();
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, vectorDataTypes);
            }
            
            const auto retDT = convertToDataType(dataType.as<uint8_t>());
            if (retDT.first != rsc_e::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO CONVERT TO DATA TYPE");

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
                return std::make_pair(rsc_e::BAD_UNEXPECTED_ERROR, vectorDataTypes);
            }
        }

        return std::make_pair(rsc_e::GOOD, vectorDataTypes);
    }
    
    /**
     * @return std::pair<Status, std::vector<DataUnitOrder>> 
     *     @li rsc_e::GOOD 설정 값이 올바릅니다.
     *     @li rsc_e::GOOD_NO_DATA 설정 값이 NULL 입니다.
     *     @li Status::Code::BAD_ENCODING_ERROR 설정 형식이 올바르지 않습니다.
     *     @li Status::Code::BAD_DATA_ENCODING_INVALID 설정 값이 올바르지 않습니다.
     *     @li Status::Code::BAD_OUT_OF_RANGE 키(key) 값이 int32_t 타입으로 변환 가능한 범위를 벗어났습니다.
     *     @li Status::Code::BAD_OUT_OF_MEMORY 설정 값을 저장에 할당 가능한 메모리가 부족합니다.
     *     @li Status::Code::BAD_UNEXPECTED_ERROR 알 수 없는 예외가 발생했습니다.
     */
    std::pair<rsc_e, std::vector<DataUnitOrder>> NodeValidator::processDataUnitOrders(JsonVariant dataUnitOrders)
    {
        std::vector<DataUnitOrder> vectorDataUnitOrder;

        if (dataUnitOrders.isNull() == true)
        {
            LOG_VERBOSE(logger, "Data unit order values were not provied");
            return std::make_pair(rsc_e::GOOD_NO_DATA, vectorDataUnitOrder);
        }

        if (dataUnitOrders.is<JsonArray>() == false)
        {
            LOG_ERROR(logger, "DATA UNIT ORDERS MUST BE JSON ARRAY");
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, vectorDataUnitOrder);
        }

        JsonArray arrayDataUnitOrders = dataUnitOrders.as<JsonArray>();
        const size_t length = arrayDataUnitOrders.size();

        if (length == 0)
        {
            LOG_ERROR(logger, "DATA UNIT ORDERS CANNOT BE AN EMPTY ARRAY");
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, vectorDataUnitOrder);
        }
        else if (length > UINT8_MAX)
        {
            LOG_ERROR(logger, "THE ARRAY OF DATA UNIT ORDERS HAS TOO MANY ELEMENTS: %u", length);
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, vectorDataUnitOrder);
        }
        
        try
        {
            vectorDataUnitOrder.reserve(length);
        }
        catch (const std::bad_alloc& e)
        {
            LOG_ERROR(logger, "%s", e.what());
            return std::make_pair(rsc_e::BAD_OUT_OF_MEMORY, vectorDataUnitOrder);
        }
        catch(const std::exception& e)
        {
            LOG_ERROR(logger, "FAILED TO RESERVE FOR DATA UNIT ORDERS: %s", e.what());
            return std::make_pair(rsc_e::BAD_UNEXPECTED_ERROR, vectorDataUnitOrder);
        }

        for (auto subarrayDataUnitOrders : arrayDataUnitOrders)
        {
            if (subarrayDataUnitOrders.is<JsonArray>() == false)
            {
                LOG_ERROR(logger, "ELEMENT ARRAY MUST A JSON ARRAY");

                vectorDataUnitOrder.clear();
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, vectorDataUnitOrder);
            }
            
            JsonArray elementsArray = subarrayDataUnitOrders.as<JsonArray>();
            const size_t elementsLength = elementsArray.size();

            if (elementsLength == 0)
            {
                LOG_ERROR(logger, "SUB-ARRAY OF DATA UNIT ORDERS CANNOT BE EMPTY");

                vectorDataUnitOrder.clear();
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, vectorDataUnitOrder);
            }
            else if (elementsLength > UINT8_MAX)
            {
                LOG_ERROR(logger, "THE SUB-ARRAY OF DATA UNIT ORDERS HAS TOO MANY ELEMENTS: %u", elementsLength);

                vectorDataUnitOrder.clear();
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, vectorDataUnitOrder);
            }
            
            DataUnitOrder dataUnitOrder(elementsLength);
            for (auto element : elementsArray)
            {
                if (element.is<std::string>() == false)
                {
                    LOG_ERROR(logger, "ELEMENT OF DATA UNIT ORDERS MUST BE A STRING");

                    vectorDataUnitOrder.clear();
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, vectorDataUnitOrder);
                }

                const auto retConvert = convertToDataUnitOrderType(element.as<std::string>());
                if (retConvert.first != rsc_e::GOOD)
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
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, vectorDataUnitOrder);
                }
            }

            try
            {
                vectorDataUnitOrder.emplace_back(dataUnitOrder);
            }
            catch(const std::exception& e)
            {
                LOG_ERROR(logger, "FAILED TO EMPLACE DATA UNIT ORDER: %s", e.what());
                return std::make_pair(rsc_e::BAD_UNEXPECTED_ERROR, vectorDataUnitOrder);
            }
        }

        return std::make_pair(rsc_e::GOOD, vectorDataUnitOrder);
    }
    
    std::pair<rsc_e, adtp_e> NodeValidator::convertToAdressType(const uint8_t type)
    {
        switch (type)
        {
        case 0:
            return std::make_pair(rsc_e::GOOD, adtp_e::NUMERIC);
        case 1:
        case 2:
        case 3:
            return std::make_pair(rsc_e::BAD_UNSUPPORTED_CONFIGURATION, adtp_e::NUMERIC);
        default:
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, adtp_e::NUMERIC);
        }
    }

    std::pair<rsc_e, addr_u> NodeValidator::convertToAddress(JsonVariant address)
    {
        ASSERT((mAddressType.first == rsc_e::GOOD), "ADDRESS TYPE MUST BE CONFIGURED IN ADVANCE");

        addr_u addressUnion;
        addressUnion.Numeric = 0;

        switch (mAddressType.second)
        {
        case adtp_e::NUMERIC:
            if (address.is<uint32_t>() == false)
            {
                LOG_ERROR(logger, "INVALID NUMERIC ADDRESS: VALUE MUST BE 32-BIT UNSIGNED INTEGER");
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, addressUnion);
            }
            else
            {
                addressUnion.Numeric = address.as<uint32_t>();
                return std::make_pair(rsc_e::GOOD, addressUnion);
            }
        case adtp_e::STRING:
        case adtp_e::BYTE_STRING:
        case adtp_e::GUID:
            return std::make_pair(rsc_e::BAD_UNSUPPORTED_CONFIGURATION, addressUnion);
        default:
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, addressUnion);
        }
    }

    /**
     * @return std::pair<Status, mb_area_e> 
     *     @li rsc_e::GOOD 설정 값이 올바릅니다.
     *     @li rsc_e::GOOD_NO_DATA 설정 값이 NULL 입니다.
     *     @li Status::Code::BAD_DATA_ENCODING_INVALID 설정 값이 올바르지 않습니다.
     */
    std::pair<rsc_e, mb_area_e> NodeValidator::convertToModbusArea(JsonVariant modbusArea)
    {
        if (modbusArea.isNull() == true)
        {
            LOG_VERBOSE(logger, "Modbus area value was not provied");
            return std::make_pair(rsc_e::GOOD_NO_DATA, mb_area_e::COILS);
        }

        if (modbusArea.is<uint8_t>() == false)
        {
            LOG_ERROR(logger, "MODBUS AREA MUST BE A 8-BIT UNSIGNED INTEGER");
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, mb_area_e::COILS);
        }
        
        switch (modbusArea.as<uint8_t>())
        {
        case 1:
            return std::make_pair(rsc_e::GOOD, mb_area_e::COILS);
        case 2:
            return std::make_pair(rsc_e::GOOD, mb_area_e::DISCRETE_INPUT);
        case 3:
            return std::make_pair(rsc_e::GOOD, mb_area_e::INPUT_REGISTER);
        case 4:
            return std::make_pair(rsc_e::GOOD, mb_area_e::HOLDING_REGISTER);
        default:
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, mb_area_e::COILS);
        }
    }

    /**
     * @return std::pair<Status, uint8_t> 
     *     @li rsc_e::GOOD 설정 값이 올바릅니다.
     *     @li rsc_e::GOOD_NO_DATA 설정 값이 NULL 입니다.
     *     @li Status::Code::BAD_DATA_ENCODING_INVALID 설정 값이 올바르지 않습니다.
     */
    std::pair<rsc_e, uint8_t> NodeValidator::convertToBitIndex(JsonVariant bitIndex)
    {
        if (bitIndex.isNull() == true)
        {
            LOG_VERBOSE(logger, "Bit index value was not provied");
            return std::make_pair(rsc_e::GOOD_NO_DATA, UINT8_MAX);
        }

        if (bitIndex.is<uint8_t>() == false)
        {
            LOG_ERROR(logger, "BIT INDEX MUST BE A 8-BIT UNSIGNED INTEGER");
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, UINT8_MAX);
        }
        
        return std::make_pair(rsc_e::GOOD, bitIndex.as<uint8_t>());
    }

    /**
     * @return std::pair<Status, uint8_t> 
     *     @li rsc_e::GOOD 설정 값이 올바릅니다.
     *     @li rsc_e::GOOD_NO_DATA 설정 값이 NULL 입니다.
     *     @li Status::Code::BAD_DATA_ENCODING_INVALID 설정 값이 올바르지 않습니다.
     */
    std::pair<rsc_e, uint8_t> NodeValidator::convertToAddressQuantity(JsonVariant addressQuantity)
    {
        if (addressQuantity.isNull() == true)
        {
            LOG_VERBOSE(logger, "Address quantity value was not provied");
            return std::make_pair(rsc_e::GOOD_NO_DATA, UINT8_MAX);
        }

        if (addressQuantity.is<uint8_t>() == false)
        {
            LOG_ERROR(logger, "ADDRESS QUANTITY MUST BE A 8-BIT UNSIGNED INTEGER");
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, UINT8_MAX);
        }
        
        if (addressQuantity.as<uint8_t>() == 0)
        {
            LOG_ERROR(logger, "ADDRESS QUANTITY MUST BE GREATER THAN 0");
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, UINT8_MAX);
        }

        return std::make_pair(rsc_e::GOOD, addressQuantity.as<uint8_t>());
    }

    /**
     * @return std::pair<Status, scl_e> 
     *     @li rsc_e::GOOD 설정 값이 올바릅니다.
     *     @li rsc_e::GOOD_NO_DATA 설정 값이 NULL 입니다.
     *     @li Status::Code::BAD_DATA_ENCODING_INVALID 설정 값이 올바르지 않습니다.
     */
    std::pair<rsc_e, scl_e> NodeValidator::convertToNumericScale(JsonVariant numericScale)
    {
        if (numericScale.isNull() == true)
        {
            LOG_VERBOSE(logger, "Numeric scale value was not provied");
            return std::make_pair(rsc_e::GOOD_NO_DATA, scl_e::NEGATIVE_1);
        }

        if (numericScale.is<int8_t>() == false)
        {
            LOG_ERROR(logger, "NUMERIC SCALE MUST BE A 8-BIT SIGNED INTEGER");
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, scl_e::NEGATIVE_1);
        }
        
        switch (numericScale.as<int8_t>())
        {
        case -3:
            return std::make_pair(rsc_e::GOOD, scl_e::NEGATIVE_3);
        case -2:
            return std::make_pair(rsc_e::GOOD, scl_e::NEGATIVE_2);
        case -1:
            return std::make_pair(rsc_e::GOOD, scl_e::NEGATIVE_1);
        case 1:
            return std::make_pair(rsc_e::GOOD, scl_e::POSITIVE_1);
        case 2:
            return std::make_pair(rsc_e::GOOD, scl_e::POSITIVE_2);
        case 3:
            return std::make_pair(rsc_e::GOOD, scl_e::POSITIVE_3);
        default:
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, scl_e::NEGATIVE_3);
        }
    }

    /**
     * @return std::pair<Status, float> 
     *     @li rsc_e::GOOD 설정 값이 올바릅니다.
     *     @li rsc_e::GOOD_NO_DATA 설정 값이 NULL 입니다.
     *     @li Status::Code::BAD_DATA_ENCODING_INVALID 설정 값이 올바르지 않습니다.
     */
    std::pair<rsc_e, float> NodeValidator::convertToNumericOffset(JsonVariant numericOffset)
    {
        if (numericOffset.isNull() == true)
        {
            LOG_VERBOSE(logger, "Numeric offset value was not provied");
            return std::make_pair(rsc_e::GOOD_NO_DATA, std::numeric_limits<float>::max());
        }

        if (numericOffset.is<float>() == false)
        {
            LOG_ERROR(logger, "NUMERIC OFFSET MUST BE A 32-BIT FLOATING POINT");
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, std::numeric_limits<float>::max());
        }
        
        return std::make_pair(rsc_e::GOOD, numericOffset.as<float>());
    }

    /**
     * @return std::pair<Status, float> 
     *     @li rsc_e::GOOD 설정 값이 올바릅니다.
     *     @li rsc_e::GOOD_NO_DATA 설정 값이 NULL 입니다.
     *     @li Status::Code::BAD_ENCODING_ERROR 설정 형식이 올바르지 않습니다.
     *     @li Status::Code::BAD_DATA_ENCODING_INVALID 설정 값이 올바르지 않습니다.
     *     @li Status::Code::BAD_OUT_OF_RANGE 키(key) 값이 int32_t 타입으로 변환 가능한 범위를 벗어났습니다.
     *     @li Status::Code::BAD_OUT_OF_MEMORY 설정 값을 저장에 할당 가능한 메모리가 부족합니다.
     *     @li Status::Code::BAD_UNEXPECTED_ERROR 알 수 없는 예외가 발생했습니다.
     */
    std::pair<rsc_e, std::map<uint16_t, std::string>> NodeValidator::convertToMappingRules(JsonObject mappingRules)
    {
        std::map<uint16_t, std::string> mapMappingRules;

        if (mappingRules.isNull() == true)
        {
            LOG_VERBOSE(logger, "Mapping rules was not provied");
            return std::make_pair(rsc_e::GOOD_NO_DATA, mapMappingRules);
        }

        if (mappingRules.size() == 0)
        {
            LOG_ERROR(logger, "MAPPING RULES CANNOT BE AN EMPTY JSON OBJECT");
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, mapMappingRules);
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
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, mapMappingRules);
                }
                numericKey = static_cast<uint16_t>(key);
            }
            catch (const std::invalid_argument& e)
            {
                LOG_ERROR(logger, "INVALID KEY ARGUMENT: %u", numericKey);

                mapMappingRules.clear();
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, mapMappingRules);
            }
            catch (const std::out_of_range& e)
            {
                LOG_ERROR(logger, "KEY OUT OF RANGE: %u", numericKey);

                mapMappingRules.clear();
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, mapMappingRules);
            }

            try
            {
                const auto rsc = mapMappingRules.emplace(numericKey, rule.value().as<std::string>());
                
                if (rsc.second == false)
                {
                    LOG_ERROR(logger, "KEYS CANNOT BE DUPLICATED: %u", numericKey);

                    mapMappingRules.clear();
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, mapMappingRules);
                }
            }
            catch (const std::bad_alloc& e)
            {
                LOG_ERROR(logger, "%s", e.what());
                return std::make_pair(rsc_e::BAD_OUT_OF_MEMORY, mapMappingRules);
            }
            catch(const std::exception& e)
            {
                LOG_ERROR(logger, "FAILED TO EMPLACE MAPPING RULE: %s", e.what());
                return std::make_pair(rsc_e::BAD_UNEXPECTED_ERROR, mapMappingRules);
            }
        }


        /**
         * @todo Map 내부의 키 값이 중복되서 들어올 시, 에러를 반환해야 함
         */
        return std::make_pair(rsc_e::GOOD, mapMappingRules);
    }

    /**
     * @return std::pair<Status, float> 
     *     @li rsc_e::GOOD 설정 값이 올바릅니다.
     *     @li Status::Code::BAD_DATA_ENCODING_INVALID 설정 값이 올바르지 않습니다.
     *     @li Status::Code::BAD_OUT_OF_RANGE 키(key) 값이 int32_t 타입으로 변환 가능한 범위를 벗어났습니다.
     */
    std::pair<rsc_e, ord_t> NodeValidator::convertToDataUnitOrderType(const std::string& value)
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
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, dataUnitOrder);
            }

            stringIndex = value.substr(2);
        }
        else
        {
            LOG_ERROR(logger, "DATA UNIT ORDER MUST START WITH 'W' OR 'B'");
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, dataUnitOrder);
        }
        
        for (char ch : stringIndex)
        {
            if (std::isdigit(ch) == false)
            {
                LOG_ERROR(logger, "INDEX MUST CONSIST OF DIGITS");
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, dataUnitOrder);
            }
        }

        int numericIndex = 0;
        try
        {
            numericIndex = std::stoi(stringIndex);
            if (numericIndex > UINT8_MAX)
            {
                LOG_ERROR(logger, "INDEX MUST BE AN 8-BIT UNSIGNED INTEGER");
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, dataUnitOrder);
            }
        }
        catch (const std::invalid_argument& e)
        {
            LOG_ERROR(logger, "INVALID INDEX ARGUMENT: %s", stringIndex.c_str());
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, dataUnitOrder);
        }
        catch (const std::out_of_range& e)
        {
            LOG_ERROR(logger, "INDEX VALUE OUT OF RANGE: %s", stringIndex.c_str());
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, dataUnitOrder);
        }

        dataUnitOrder.Index = numericIndex;
        return std::make_pair(rsc_e::GOOD, dataUnitOrder);
    }

    std::pair<rsc_e, dt_e> NodeValidator::convertToDataType(const uint8_t dataType)
    {
        switch (dataType)
        {
        case  0:
            return std::make_pair(rsc_e::GOOD, dt_e::BOOLEAN);
        case  1:
            return std::make_pair(rsc_e::GOOD, dt_e::INT8);
        case  2:
            return std::make_pair(rsc_e::GOOD, dt_e::UINT8);
        case  3:
            return std::make_pair(rsc_e::GOOD, dt_e::INT16);
        case  4:
            return std::make_pair(rsc_e::GOOD, dt_e::UINT16);
        case  5:
            return std::make_pair(rsc_e::GOOD, dt_e::INT32);
        case  6:
            return std::make_pair(rsc_e::GOOD, dt_e::UINT32);
        case  7:
            return std::make_pair(rsc_e::GOOD, dt_e::INT64);
        case  8:
            return std::make_pair(rsc_e::GOOD, dt_e::UINT64);
        case  9:
            return std::make_pair(rsc_e::GOOD, dt_e::FLOAT32);
        case 10:
            return std::make_pair(rsc_e::GOOD, dt_e::FLOAT64);
        case 11:
            return std::make_pair(rsc_e::GOOD, dt_e::STRING);
        default:
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, dt_e::BOOLEAN);
        }
    }

    std::pair<rsc_e, std::string> NodeValidator::convertToFormatString(const JsonVariant formatString)
    {
        if (formatString.isNull() == true)
        {
            LOG_VERBOSE(logger, "Format string value is not provied");
            return std::make_pair(rsc_e::GOOD_NO_DATA, std::string());
        }

        if (formatString.is<const char*>() == false)
        {
            LOG_ERROR(logger, "FORMAT STRING MUST BE A STRING");
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, std::string());
        }
        
        const char* format = formatString.as<const char*>();
        if (strlen(format) == 0)
        {
            LOG_ERROR(logger, "FORMAT STRING CANNOT BE AN EMPTY STRING");
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, std::string());
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
                    return std::make_pair(rsc_e::BAD_OUT_OF_MEMORY, std::string());
                }
                catch(const std::exception& e)
                {
                    LOG_ERROR(logger, "FAILED TO EMPLACE FORMAT SPECIFIER: %s", e.what());
                    return std::make_pair(rsc_e::BAD_UNEXPECTED_ERROR, std::string());
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
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, std::string());
        }
        else
        {
            return std::make_pair(rsc_e::GOOD, formatString.as<std::string>());
        }
        

    INVALID_SPECIFIER:
        LOG_ERROR(logger, "INVALID FORMAT STRING: %s", format);

        mVectorFormatSpecifier.clear();
        return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, std::string());
    }
}}