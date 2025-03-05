/**
 * @file NodeValidator.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief Node 설정 정보가 유효한지 검사하는 클래스를 정의합니다.
 * 
 * @date 2025-02-26
 * @version 1.2.13
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include "Common/Assert.h"
#include "Common/Logger/Logger.h"
#include "JARVIS/Config/Information/Node.h"
#include "NodeValidator.h"



namespace muffin { namespace jvs {

    NodeValidator::NodeValidator()
        : mAddressType(rsc_e::UNCERTAIN, adtp_e::NUMERIC)
        , mAddress(rsc_e::UNCERTAIN, addr_u())
        , mModbusArea(rsc_e::UNCERTAIN, mb_area_e::COILS)
        , mBitIndex(rsc_e::UNCERTAIN, 0)
        , mAddressQuantity(rsc_e::UNCERTAIN, 0)
        , mNumericScale(rsc_e::UNCERTAIN, scl_e::NEGATIVE_1)
        , mNumericOffset(rsc_e::UNCERTAIN, 0.0f)
        , mDataUnitOrders(rsc_e::UNCERTAIN, std::vector<DataUnitOrder>())
        , mDataTypes(rsc_e::UNCERTAIN, std::vector<muffin::jvs::dt_e>())
        , mFormatString(rsc_e::UNCERTAIN, std::string())
        , mPatternUID(std::regex(R"(^(?:[PAE][A-Za-z0-9!@#$%^&*()_+=-]{3}|(?:DI|DO|MD)[A-Za-z0-9!@#$%^&*()_+=-]{2})$)"))
    {
        memset(mNodeID, '\0', sizeof(mNodeID));
        memset(mUID,    '\0', sizeof(mUID));
    }

    std::pair<rsc_e, std::string> NodeValidator::Inspect(const JsonArray arrayCIN, cin_vector* outVector)
    {
        ASSERT((outVector != nullptr), "OUTPUT PARAMETER <outVector> CANNOT BE A NULL POINTER");
        ASSERT((arrayCIN.isNull() == false), "OUTPUT PARAMETER <arrayCIN> CANNOT BE NULL");
        ASSERT((arrayCIN.size() != 0), "INPUT PARAMETER <arrayCIN> CANNOT BE 0 IN LENGTH");

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

            strncpy(mNodeID, json["id"].as<const char*>(), sizeof(mNodeID));
            if (strlen(mNodeID) != 4)
            {
                char message[64] = {'\0'};
                snprintf(message, 64, "INVALID NODE ID: %s", json["id"].as<const char*>());
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }

            convertToAdressType(json["adtp"].as<uint8_t>());
            if (mAddressType.first != rsc_e::GOOD)
            {
                char message[64] = {'\0'};
                snprintf(message, 64, "UNDEFINED OR UNSUPPORTED ADDRESS TYPE: %s", json["adtp"].as<const char*>());
                return std::make_pair(mAddressType.first, message);
            }
            
            convertToAddress(json["addr"].as<JsonVariant>());
            if (mAddress.first != rsc_e::GOOD)
            {
                char message[64] = {'\0'};
                snprintf(message, 64, "INVALID NODE ADDRESS: %s, NODE ID: %s", json["adtp"].as<const char*>(),
                                                                               mNodeID);
                return std::make_pair(mAddress.first, message);
            }
            
            mDataTypes = processDataTypes(json["dt"].as<JsonArray>());
            if (mDataTypes.first != rsc_e::GOOD)
            {
                char message[64] = {'\0'};
                snprintf(message, 64, "INVALID DATA TYPES FOR NODE ID: %s", mNodeID);
                return std::make_pair(mDataTypes.first, message);
            }

            strncpy(mUID, json["uid"].as<const char*>(), sizeof(mUID));
            if (std::regex_match(mUID, mPatternUID) == false)
            {
                char message[64] = {'\0'};
                snprintf(message, 64, "INVALID UID: %s", json["uid"].as<const char*>());
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }

            mIsEventType = json["event"].as<bool>();
            /*Node 설정 형식에서 필수로 입력해야 하는 속성은 모두 유효합니다.*/

            convertToModbusArea(json["area"].as<JsonVariant>());
            if (mModbusArea.first != rsc_e::GOOD && 
                mModbusArea.first != rsc_e::GOOD_NO_DATA)
            {
                char message[128] = {'\0'};
                snprintf(message, 128, "FAILED TO CONVERT TO MODBUS AREA: %s, NODE ID: %s", json["area"].as<const char*>(), mNodeID);
                return std::make_pair(mModbusArea.first, message);
            }

            convertToBitIndex(json["bit"].as<JsonVariant>());
            if (mBitIndex.first != rsc_e::GOOD && 
                mBitIndex.first != rsc_e::GOOD_NO_DATA)
            {
                char message[128] = {'\0'};
                snprintf(message, 128, "FAILED TO CONVERT TO BIT INDEX: %s, NODE ID: %s", json["bit"].as<const char*>(), mNodeID);
                return std::make_pair(mBitIndex.first, message);
            }

            convertToAddressQuantity(json["qty"].as<JsonVariant>());
            if (mAddressQuantity.first != rsc_e::GOOD && 
                mAddressQuantity.first != rsc_e::GOOD_NO_DATA)
            {
                char message[128] = {'\0'};
                snprintf(message, 128, "FAILED TO CONVERT TO ADDRESS QUANTITY: %s, NODE ID: %s", json["qty"].as<const char*>(), mNodeID);
                return std::make_pair(mAddressQuantity.first, message);
            }

            convertToNumericScale(json["scl"].as<JsonVariant>());
            if (mNumericScale.first != rsc_e::GOOD && 
                mNumericScale.first != rsc_e::GOOD_NO_DATA)
            {
                char message[128] = {'\0'};
                snprintf(message, 128, "FAILED TO CONVERT TO NUMERIC SCALE: %s, NODE ID: %s", json["scl"].as<const char*>(), mNodeID);
                return std::make_pair(mNumericScale.first, message);
            }

            convertToNumericOffset(json["ofst"].as<JsonVariant>());
            if (mNumericOffset.first != rsc_e::GOOD && 
                mNumericOffset.first != rsc_e::GOOD_NO_DATA)
            {
                char message[128] = {'\0'};
                snprintf(message, 128, "FAILED TO CONVERT TO NUMERIC OFFSET: %s, NODE ID: %s", json["ofst"].as<const char*>(), mNodeID);
                return std::make_pair(mNumericOffset.first, message);
            }

            mDataUnitOrders = processDataUnitOrders(json["ord"].as<JsonVariant>());
            if (mDataUnitOrders.first != rsc_e::GOOD && mDataUnitOrders.first != rsc_e::GOOD_NO_DATA)
            {
                char message[128] = {'\0'};
                snprintf(message, 128, "FAILED TO CONVERT TO DATA UNIT ORDERS: %s, NODE ID: %s", json["ord"].as<const char*>(), mNodeID);
                return std::make_pair(mDataUnitOrders.first, message);
            }

            convertToFormatString(json["fmt"].as<JsonVariant>());
            if (mFormatString.first != rsc_e::GOOD && mFormatString.first != rsc_e::GOOD_NO_DATA)
            {
                char message[128] = {'\0'};
                snprintf(message, 128, "FAILED TO CONVERT TO FORMAT STRING: %s, NODE ID: %s", json["fmt"].as<const char*>(), mNodeID);
                return std::make_pair(mFormatString.first, message);
            }

            std::pair<rsc_e, std::string> result = validateModbusArea();
            if (result.first != rsc_e::GOOD)
            {
                LOG_ERROR(logger, "INVALID MODBUS AREA CONFIG: %s", result.second.c_str());
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

            memset(mNodeID, '\0', sizeof(mNodeID));
            mAddressType      = std::make_pair(rsc_e::UNCERTAIN, adtp_e::NUMERIC);
            mAddress          = std::make_pair(rsc_e::UNCERTAIN, addr_u());
            mModbusArea       = std::make_pair(rsc_e::UNCERTAIN, mb_area_e::COILS);
            mBitIndex         = std::make_pair(rsc_e::UNCERTAIN, 0);
            mAddressQuantity  = std::make_pair(rsc_e::UNCERTAIN, 0);
            mNumericScale     = std::make_pair(rsc_e::UNCERTAIN, scl_e::NEGATIVE_1);
            mNumericOffset    = std::make_pair(rsc_e::UNCERTAIN, 0.0f);
            mDataUnitOrders   = std::make_pair(rsc_e::UNCERTAIN, std::vector<DataUnitOrder>());
            mDataTypes        = std::make_pair(rsc_e::UNCERTAIN, std::vector<muffin::jvs::dt_e>());
            mFormatString     = std::make_pair(rsc_e::UNCERTAIN, std::string());
            memset(mUID, '\0', sizeof(mUID));
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
        isValid &= json.containsKey("ord");
        isValid &= json.containsKey("dt");
        isValid &= json.containsKey("fmt");
        isValid &= json.containsKey("uid");
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
        isValid &= json["event"].isNull()  == false;
        isValid &= json["id"].is<const char*>();
        isValid &= json["adtp"].is<uint8_t>();
        isValid &= json["addr"].is<JsonVariant>();
        isValid &= json["dt"].is<JsonArray>();
        isValid &= json["uid"].is<const char*>();
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
            char message[128] = {'\0'};
            snprintf(message, 128, "Modbus memory area is not enabled, Node ID: %s", mNodeID);
            return std::make_pair(rsc_e::GOOD, message);
        }
        
        if (mAddressType.second != adtp_e::NUMERIC)
        {
            char message[128] = {'\0'};
            snprintf(message, 128, "ADDRESS TYPE MUST BE NUMERIC TO ENABLE MODBUS PROTOCOL, NODE ID: %s", mNodeID);
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }
        
    /** @todo 나중에 Modbus 확장 주소로 설정해야 할 때 구현해야 합니다.
     *  @code {.cpp}
     *  if (mAddress.second.Numeric > 0x270F)
     *  {
     *      const std::string message = "EXTENDED REGISTER ADDRESS FOR MODBUS PROTOCOL IS NOT SUPPORTED, NODE ID:  "+ mNodeID;
     *      return std::make_pair(rsc_e::BAD_UNSUPPORTED_CONFIGURATION, message);
     *  }    
     * @endcode
     */

        if (mModbusArea.second == mb_area_e::COILS || mModbusArea.second == mb_area_e::DISCRETE_INPUT)
        {
            if (mBitIndex.first == rsc_e::GOOD)
            {
                char message[128] = {'\0'};
                snprintf(message, 128, "MODBUS AREA CANNOT BE CONFIGURED AS \"Coils\" OR \"Discrete Inputs\" WITH BIT INDEX, NODE ID: %s", mNodeID);
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
            
            if (mAddressQuantity.first == rsc_e::GOOD)
            {
                char message[128] = {'\0'};
                snprintf(message, 128, "MODBUS AREA CANNOT BE CONFIGURED AS \"Coils\" OR \"Discrete Inputs\" WITH ADDRESS QUANTITY, NODE ID: %s", mNodeID);
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
            
            if (mNumericScale.first == rsc_e::GOOD)
            {
                char message[128] = {'\0'};
                snprintf(message, 128, "MODBUS AREA CANNOT BE CONFIGURED AS \"Coils\" OR \"Discrete Inputs\" WITH NUMERIC SCALE, NODE ID: %s", mNodeID);
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
            
            if (mNumericOffset.first == rsc_e::GOOD)
            {
                char message[128] = {'\0'};
                snprintf(message, 128, "MODBUS AREA CANNOT BE CONFIGURED AS \"Coils\" OR \"Discrete Inputs\" WITH NUMERIC OFFSET, NODE ID: %s", mNodeID);
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
            
            if (mDataUnitOrders.first == rsc_e::GOOD)
            {
                char message[128] = {'\0'};
                snprintf(message, 128, "MODBUS AREA CANNOT BE CONFIGURED AS \"Coils\" OR \"Discrete Inputs\" WITH DATA UNIT ORDERS, NODE ID: %s", mNodeID);
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
            
            if (mDataTypes.second.size() != 1)
            {
                char message[128] = {'\0'};
                snprintf(message, 128, "MODBUS AREA CANNOT BE CONFIGURED AS \"Coils\" OR \"Discrete Inputs\" IF MORE THAN ONE DATA TYPE IS PROVIDED, NODE ID: %s", mNodeID);
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
            
            if (mDataTypes.second.front() != dt_e::BOOLEAN)
            {
                char message[128] = {'\0'};
                snprintf(message, 128, "MODBUS AREA CANNOT BE CONFIGURED AS \"Coils\" OR \"Discrete Inputs\" IF DATA TYPE IS NOT BOOLEAN, NODE ID: %s", mNodeID);
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
            
            if (mFormatString.first == rsc_e::GOOD)
            {
                char message[128] = {'\0'};
                snprintf(message, 128, "MODBUS AREA CANNOT BE CONFIGURED AS \"Coils\" OR \"Discrete Inputs\" WITH FORMAT STRING, NODE ID: %s", mNodeID);
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }

            return std::make_pair(rsc_e::GOOD, "GOOD");
        }
        else
        {
            if(mBitIndex.first == rsc_e::GOOD)
            {
                if (mAddressQuantity.first == rsc_e::GOOD)
                {
                    char message[256] = {'\0'};
                    snprintf(message, 256, "MODBUS AREA CANNOT BE CONFIGURED AS \"Input Registers\" OR \"Holding Registers\" WITH BOTH BIT INDEX AND ADDRESS QUANTITY, NODE ID: %s", mNodeID);
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
                }
            }
            else if (mAddressQuantity.first == rsc_e::GOOD_NO_DATA)
            {
                char message[128] = {'\0'};
                snprintf(message, 128, "MODBUS AREA CANNOT BE CONFIGURED AS \"Input Registers\" OR \"Holding Registers\" WITHOUT ADDRESS QUANTITY, NODE ID: %s", mNodeID);
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
            
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
            char message[128] = {'\0'};
            snprintf(message, 128, "Bit index is not enabled, NODE ID: %s", mNodeID);
            return std::make_pair(rsc_e::GOOD, message);
        }

        if (mAddressQuantity.first == rsc_e::GOOD)
        {
            char message[128] = {'\0'};
            snprintf(message, 128, "BIT INDEX CANNOT BE CONFIGURED WITH ADDRESS QUANTITY, NODE ID: %s", mNodeID);
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }
        
        if (mNumericScale.first == rsc_e::GOOD)
        {
            char message[128] = {'\0'};
            snprintf(message, 128, "BIT INDEX CANNOT BE CONFIGURED WITH NUMERIC SCALE, NODE ID: %s", mNodeID);
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }
        
        if (mNumericOffset.first == rsc_e::GOOD)
        {
            char message[128] = {'\0'};
            snprintf(message, 128, "BIT INDEX CANNOT BE CONFIGURED WITH NUMERIC OFFSET, NODE ID: %s", mNodeID);
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        if (mFormatString.first == rsc_e::GOOD)
        {
            char message[128] = {'\0'};
            snprintf(message, 128, "BIT INDEX CANNOT BE CONFIGURED WITH FORMAT STRING, NODE ID: %s", mNodeID);
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        if (mDataTypes.second.size() != 1)
        {
            char message[128] = {'\0'};
            snprintf(message, 128, "BIT INDEX CANNOT BE CONFIGURED WITH MORE THAN ONE DATA TYPE, NODE ID: %s", mNodeID);
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        if (mDataUnitOrders.first == rsc_e::GOOD)
        {
            if (mDataUnitOrders.second.size() != 1)
            {
                char message[128] = {'\0'};
                snprintf(message, 128, "BIT INDEX CANNOT BE CONFIGURED WITH MORE THAN ONE DATA UNIT ORDER, NODE ID: %s", mNodeID);
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
        }

        if (mModbusArea.first == rsc_e::GOOD)
        {
            if (mModbusArea.second == mb_area_e::COILS || mModbusArea.second == mb_area_e::DISCRETE_INPUT)
            {
                char message[128] = {'\0'};
                snprintf(message, 128, "BIT INDEX CANNOT BE CONFIGURED WITH MODBUS AREA \"Coils\" OR \"Discrete Inputs\", NODE ID: %s", mNodeID);
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
        }

        switch (mDataTypes.second.front())
        {
            case dt_e::INT8:
            case dt_e::UINT8:
                if (mBitIndex.second > 7)
                {
                    char message[128] = {'\0'};
                    snprintf(message, 128, "[NODE ID] %s, BIT INDEX OUT OF RANGE FOR 8-BIT INTEGER: %u", mNodeID, mBitIndex.second);
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
                }
                break;
            case dt_e::INT16:
            case dt_e::UINT16:
                if (mBitIndex.second > 15)
                {
                    char message[128] = {'\0'};
                    snprintf(message, 128, "[NODE ID] %s, BIT INDEX OUT OF RANGE FOR 16-BIT INTEGER: %u", mNodeID, mBitIndex.second);
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
                }
                break;
            case dt_e::INT32:
            case dt_e::UINT32:
                if (mBitIndex.second > 31)
                {
                    char message[128] = {'\0'};
                    snprintf(message, 128, "[NODE ID] %s, BIT INDEX OUT OF RANGE FOR 32-BIT INTEGER: %u", mNodeID, mBitIndex.second);
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
                }
                break;
            case dt_e::INT64:
            case dt_e::UINT64:
                if (mBitIndex.second > 63)
                {
                    char message[128] = {'\0'};
                    snprintf(message, 128, "[NODE ID] %s, BIT INDEX OUT OF RANGE FOR 64-BIT INTEGER: %u", mNodeID, mBitIndex.second);
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
                }
                break;
            default:
                char message[128] = {'\0'};
                snprintf(message, 128, "[NODE ID] %s, INVALID DATA TYPE FOR BIT INDEX: %u", mNodeID, static_cast<uint8_t>(mDataTypes.second.front()));
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

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
                char message[128] = {'\0'};
                snprintf(message, 128, "NUMERIC ADDRESS QUANTITY CANNOT BE CONFIGURED IF ADDRESS TYPE IS NOT NUMERIC, NODE ID: %s", mNodeID);
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
            else
            {
                char message[128] = {'\0'};
                snprintf(message, 128, "Numeric address quantity is not enabled, NODE ID: %s", mNodeID);
                return std::make_pair(rsc_e::GOOD, message);
            }
        }
        
        if (mAddressQuantity.first == rsc_e::GOOD_NO_DATA)
        {
            if (mBitIndex.first == rsc_e::GOOD)
            {
                char message[128] = {'\0'};
                snprintf(message, 128, "Address quantity is null by bit index config, NODE ID: %s", mNodeID);
                return std::make_pair(rsc_e::GOOD_NO_DATA, message);
            }

            if (mModbusArea.first == rsc_e::GOOD)
            {
                if (mModbusArea.second == mb_area_e::COILS || mModbusArea.second == mb_area_e::DISCRETE_INPUT)
                {
                    char message[128] = {'\0'};
                    snprintf(message, 128, "Address quantity is null by Modbus area config, NODE ID: %s", mNodeID);
                    return std::make_pair(rsc_e::GOOD_NO_DATA, message);
                }
                else
                {
                    char message[128] = {'\0'};
                    snprintf(message, 128, "NUMERIC ADDRESS QUANTITY MUST BE CONFIGURED IF MODBUS AREA \"INPUT REGISTERS\" OR \"HOLDING REGISTERS\", NODE ID: %s", mNodeID);
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
                }
            }

            // qty , bit , modbus area도 없는 애들은 여기로 빠진다.
            char message[128] = {'\0'};
            snprintf(message, 128, "NUMERIC ADDRESS QUANTITY MUST BE CONFIGURED IF ADDRESS TYPE IS NUMERIC, NODE ID: %s", mNodeID);
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        if (mBitIndex.first == rsc_e::GOOD)
        {
            char message[128] = {'\0'};
            snprintf(message, 128, "NUMERIC ADDRESS QUANTITY CANNOT BE CONFIGURED IF BIT INDEX IS ENABLED, NODE ID: %s", mNodeID);
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        if (mModbusArea.first == rsc_e::GOOD)
        {
            if (mModbusArea.second == mb_area_e::COILS || mModbusArea.second == mb_area_e::DISCRETE_INPUT)
            {
                char message[128] = {'\0'};
                snprintf(message, 128, "NUMERIC ADDRESS QUANTITY CANNOT BE CONFIGURED WITH MODBUS AREA \"Coils\" OR \"Discrete Inputs\", NODE ID: %s", mNodeID);
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
        }

        if ((mAddressQuantity.second > 1))
        {
            for (auto dataType : mDataTypes.second)
            {
                if (dataType == dt_e::BOOLEAN)
                {
                    char message[128] = {'\0'};
                    snprintf(message, 128, "NUMERIC ADDRESS QUANTITY LENGTH MUST BE 1 IF THERE'S A BOOLEAN DATA TYPE, NODE ID: %s", mNodeID);
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
                }
            }
        }
        
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
            return std::make_pair(rsc_e::GOOD, "Numeric scale is not enabled");
        }
        
        if (mDataTypes.second.size() != 1)
        {
            char message[128] = {'\0'};
            snprintf(message, 128, "NUMERIC SCALE CANNOT BE CONFIGURED WITH MORE THAN ONE DATA TYPES, NODE ID: %s", mNodeID);
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        if (mDataUnitOrders.first == rsc_e::GOOD)
        {
            if (mDataUnitOrders.second.size() != 1)
            {
                char message[128] = {'\0'};
                snprintf(message, 128, "NUMERIC SCALE CANNOT BE CONFIGURED WITH MORE THAN ONE DATA UNIT ORDERS, NODE ID: %s", mNodeID);
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
            char message[128] = {'\0'};
            snprintf(message, 128, "NUMERIC SCALE CANNOT BE CONFIGURED IF BIT INDEX IS ENABLED, NODE ID: %s", mNodeID);
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        if (mModbusArea.first == rsc_e::GOOD)
        {
            if (mModbusArea.second == mb_area_e::COILS || mModbusArea.second == mb_area_e::DISCRETE_INPUT)
            {
                char message[128] = {'\0'};
                snprintf(message, 128, "NUMERIC SCALE CANNOT BE CONFIGURED WITH MODBUS AREA \"Coils\" OR \"Discrete Inputs\", NODE ID: %s", mNodeID);
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
        }

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
            return std::make_pair(rsc_e::GOOD, "Numeric offset is not enabled");
        }

        if (mDataTypes.second.size() != 1)
        {
            char message[128] = {'\0'};
            snprintf(message, 128, "NUMERIC OFFSET CANNOT BE CONFIGURED WITH MORE THAN ONE DATA TYPES, NODE ID: %s", mNodeID);
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        if (mDataUnitOrders.first == rsc_e::GOOD)
        {
            if (mDataUnitOrders.second.size() != 1)
            {
                char message[128] = {'\0'};
                snprintf(message, 128, "NUMERIC OFFSET CANNOT BE CONFIGURED WITH MORE THAN ONE DATA UNIT ORDERS, NODE ID: %s", mNodeID);
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
            char message[128] = {'\0'};
            snprintf(message, 128, "NUMERIC OFFSET CANNOT BE CONFIGURED IF BIT INDEX IS ENABLED, NODE ID: %s", mNodeID);
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        if (mModbusArea.first == rsc_e::GOOD)
        {
            if (mModbusArea.second == mb_area_e::COILS || mModbusArea.second == mb_area_e::DISCRETE_INPUT)
            {
                char message[128] = {'\0'};
                snprintf(message, 128, "NUMERIC OFFSET CANNOT BE CONFIGURED WITH MODBUS AREA \"Coils\" OR \"Discrete Inputs\", NODE ID: %s", mNodeID);
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
        }

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
            char message[64] = {'\0'};
            snprintf(message, 64, "Data unit orders are not enabled, NODE ID: %s", mNodeID);
            return std::make_pair(rsc_e::GOOD, message);
        }
        
        if (mBitIndex.first == rsc_e::GOOD)
        {
            char message[128] = {'\0'};
            snprintf(message, 128, "DATA UNIT ORDERS CANNOT BE CONFIGURED IF BIT INDEX IS ENABLED, NODE ID: %s", mNodeID);
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        if (mModbusArea.first == rsc_e::GOOD)
        {
            if (mModbusArea.second == mb_area_e::COILS || mModbusArea.second == mb_area_e::DISCRETE_INPUT)
            {
                char message[128] = {'\0'};
                snprintf(message, 128, "DATA UNIT ORDERS CANNOT BE CONFIGURED WITH MODBUS AREA \"Coils\" OR \"Discrete Inputs\", NODE ID: %s", mNodeID);
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
        }
        
        if (mDataUnitOrders.second.size() != mDataTypes.second.size())
        {
            char message[128] = {'\0'};
            snprintf(message, 128, "DATA UNIT ORDERS MUST HAVE EQUAL LENGTH OF ELEMENTS WITH DATA TYPES, NODE ID: %s", mNodeID);
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        for (const auto& dataUnitOrder : mDataUnitOrders.second)
        {
            std::set<uint8_t> setIndex;

            for (const auto& orderType : dataUnitOrder)
            {
                if(orderType.DataUnit == data_unit_e::BYTE)
                {
                    continue;
                }

                const auto result = setIndex.emplace(orderType.Index);
                if (result.second == false)
                {
                    char message[128] = {'\0'};
                    snprintf(message, 128, "DATA UNIT ORDER INDICES CANNOT BE DUPLICATED, NODE ID: %s", mNodeID);
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
                    
                }
            }
        }

        return std::make_pair(rsc_e::GOOD, "GOOD");
    }
    
    /**
     * @return Status
     *     @li rsc_e::GOOD Data Types 설정이 없거나, 설정 정보가 유효합니다.
     *     @li Status::Code::BAD_CONFIGURATION_ERROR 다른 Node 속성을 고려했을 때 선결조건 또는 관계가 유효하지 않습니다.
     */
    std::pair<rsc_e, std::string> NodeValidator::validateDataTypes()
    {
        if (mDataTypes.second.size() == 1 && mDataTypes.second.at(0) == dt_e::STRING)
        {
            return std::make_pair(rsc_e::GOOD, "GOOD");
        }

        if (mDataUnitOrders.first == rsc_e::GOOD)
        {
            if (mDataTypes.second.size() != mDataUnitOrders.second.size())
            {
                char message[128] = {'\0'};
                snprintf(message, 128, "DATA TYPES MUST HAVE EQUAL LENGTH OF ELEMENTS WITH DATA UNIT ORDERS, NODE ID: %s", mNodeID);
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
                        dataTypeSize = 8;
                        break;
                    case dt_e::STRING:
                        dataTypeSize = mDataUnitOrders.second.at(i).RetrieveTotalSize();
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
                    char message[128] = {'\0'};
                    snprintf(message, 128, "DATA TYPE SIZE DOES NOT MATCH WITH THE SUM OF DATA UNIT ORDERS, NODE ID: %s", mNodeID);
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
                }
            }
        }
        
        if (mModbusArea.first == rsc_e::GOOD && mDataUnitOrders.first != rsc_e::GOOD)
        {
            if (mModbusArea.second == mb_area_e::COILS || mModbusArea.second == mb_area_e::DISCRETE_INPUT)
            {
                if (mDataTypes.second.size() != 1)
                {
                    char message[128] = {'\0'};
                    snprintf(message, 128, "DATA TYPE MUST HAVE ONE ELEMENT WITH MODBUS AREA \"Coils\" OR \"Discrete Inputs\", NODE ID: %s", mNodeID);
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
                }
                else if (mDataTypes.second.front() != dt_e::BOOLEAN)
                {
                    char message[128] = {'\0'};
                    snprintf(message, 128, "DATA TYPE MUST BE \"BOOLEAN\" WITH MODBUS AREA \"Coils\" OR \"Discrete Inputs\", NODE ID: %s", mNodeID);
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

                for (uint8_t i = 0; i < mDataTypes.second.size(); ++i)
                {
                    switch (mDataTypes.second.at(i))
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
                            char message[128] = {'\0'};
                            snprintf(message, 128, "DATA TYPE \"BOOLEAN\" CANNOT BE CONFIGURED WITH MODBUS AREA USING REGISTERS, NODE ID: %s", mNodeID);
                            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
                    }
                }

                if (totalRegisterSize > sumDataTypeSize)
                {
                    char message[128] = {'\0'};
                    snprintf(message, 128, "TOTAL SIZE OF MODBUS REGISTERS DOES NOT MATCH WITH THE SUM OF EACH DATA TYPES, NODE ID: %s", mNodeID);
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
                }
                else if (totalRegisterSize < sumDataTypeSize)
                {
                    /**
                     * @todo 오류는 아니지만 경고 메시지를 만들어야함
                     */
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, "TOTAL REGISTER SIZE DOES NOT MATCH DATA TYPE SIZE");
                }
                else
                {
                    //정상 조건
                }
            }
        }

        return std::make_pair(rsc_e::GOOD, "GOOD");
    }

    /**
     * @return Status
     *     @li rsc_e::GOOD Format String 설정이 없거나, 설정 정보가 유효합니다.
     *     @li Status::Code::BAD_CONFIGURATION_ERROR 다른 Node 속성을 고려했을 때 선결조건 또는 관계가 유효하지 않습니다.
     */
    std::pair<rsc_e, std::string> NodeValidator::validateFormatString()
    {
        if (mDataTypes.second.size() > 1)
        {
            if (mFormatString.first != rsc_e::GOOD)
            {
                char message[128] = {'\0'};
                snprintf(message, 128, "FORMAT STRING MUST BE CONFIGURED IF DATA TYPE SIZE IS 2 OR MORE, NODE ID: %s", mNodeID);
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
        }

        if (mFormatString.first == rsc_e::GOOD_NO_DATA)
        {
            return std::make_pair(rsc_e::GOOD, "Format string is not enabled");
        }
        
        if (mBitIndex.first == rsc_e::GOOD)
        {
            char message[128] = {'\0'};
            snprintf(message, 128, "FORMAT STRING CANNOT BE CONFIGURED IF BIT INDEX IS ENABLED, NODE ID: %s", mNodeID);
            return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
        }

        if (mModbusArea.first == rsc_e::GOOD)
        {
            if (mModbusArea.second == mb_area_e::COILS || mModbusArea.second == mb_area_e::DISCRETE_INPUT)
            {
                char message[128] = {'\0'};
                snprintf(message, 128, "FORMAT STRING CANNOT BE CONFIGURED WITH MODBUS AREA \"Coils\" OR \"Discrete Inputs\", NODE ID: %s", mNodeID);
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
            }
        }

        if (mVectorFormatSpecifier.size() != mDataTypes.second.size())
        {
            char message[128] = {'\0'};
            snprintf(message, 128, "THE NUMBER OF FORMAT SPECIFIERS MUST BE EQUAL TO THE NUMBER OF DATA TYPES, NODE ID: %s", mNodeID);
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
                    char message[128] = {'\0'};
                    snprintf(message, 128, "INVALID DATA TYPE FOR FORMAT SPECIFIER \"32-BIT INTEGER\", NODE ID: %s", mNodeID);
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
                }
                break;
            case fmt_spec_e::INTEGER_64:
                if (dataType != dt_e::INT8  && dataType != dt_e::INT16  && dataType != dt_e::INT32 && dataType != dt_e::INT64)
                {
                    char message[128] = {'\0'};
                    snprintf(message, 128, "INVALID DATA TYPE FOR FORMAT SPECIFIER \"64-BIT INTEGER\", NODE ID: %s", mNodeID);
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
                }
                break;
            case fmt_spec_e::UNSIGNED_INTEGER_32:
                if (dataType != dt_e::UINT8 && dataType != dt_e::UINT16 && dataType != dt_e::UINT32)
                {
                    char message[128] = {'\0'};
                    snprintf(message, 128, "INVALID DATA TYPE FOR FORMAT SPECIFIER \"UNSIGNED 32-BIT INTEGER\", NODE ID: %s", mNodeID);
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
                }
                break;
            case fmt_spec_e::UNSIGNED_INTEGER_64:
                if (dataType != dt_e::UINT8 && dataType != dt_e::UINT16 && dataType != dt_e::UINT32 && dataType != dt_e::UINT64)
                {
                    char message[128] = {'\0'};
                    snprintf(message, 128, "INVALID DATA TYPE FOR FORMAT SPECIFIER \"UNSIGNED 64-BIT INTEGER\", NODE ID: %s", mNodeID);
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
                }
                break;
            case fmt_spec_e::FLOATING_POINT_64:
                if (dataType != dt_e::FLOAT32 && dataType != dt_e::FLOAT64)
                {
                    char message[128] = {'\0'};
                    snprintf(message, 128, "INVALID DATA TYPE FOR FORMAT SPECIFIER \"64-BIT FLOATING POINT\", NODE ID: %s", mNodeID);
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
                }
                break;
            case fmt_spec_e::CHARACTER:
            case fmt_spec_e::STRING:
                if (dataType != dt_e::STRING)
                {
                    char message[128] = {'\0'};
                    snprintf(message, 128, "INVALID DATA TYPE FOR FORMAT SPECIFIER \"STRING\", NODE ID: %s", mNodeID);
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
                }
                break;
            case fmt_spec_e::HEX_LOWERCASE:
            case fmt_spec_e::HEX_UPPERCASE:
                if (dataType == dt_e::BOOLEAN || dataType == dt_e::FLOAT32 || dataType == dt_e::FLOAT64 || dataType == dt_e::STRING)
                {
                    char message[128] = {'\0'};
                    snprintf(message, 128, "INVALID DATA TYPE FOR FORMAT SPECIFIER \"HEXA CODE\", NODE ID: %s", mNodeID);
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, message);
                }
                break;
            default:
                char message[128] = {'\0'};
                snprintf(message, 128, "[NODE ID] %s, INVALID DATA TYPE FOR FORMAT SPECIFIER: %u", mNodeID, static_cast<uint8_t>(specifier));
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
            LOG_ERROR(logger, "TOO MANY ELEMENTS FOR A DATA TYPE ARRAY: %u", length);
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
                vectorDataTypes.clear();
                vectorDataTypes.shrink_to_fit();
                LOG_ERROR(logger, "DATA TYPE MUST BE AN 8-BIT UNSIGNED INTEGER");
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, vectorDataTypes);
            }
            
            const auto retDT = convertToDataType(dataType.as<uint8_t>());
            if (retDT.first != rsc_e::GOOD)
            {
                vectorDataTypes.clear();
                vectorDataTypes.shrink_to_fit();
                LOG_ERROR(logger, "FAILED TO CONVERT TO DATA TYPE");
                return std::make_pair(retDT.first, vectorDataTypes);
            }
            
            try
            {
                vectorDataTypes.emplace_back(retDT.second);
            }
            catch(const std::exception& e)
            {
                vectorDataTypes.clear();
                vectorDataTypes.shrink_to_fit();
                LOG_ERROR(logger, "FAILED TO EMPLACE DATA TYPE: %s", e.what());
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
            return std::make_pair(rsc_e::GOOD_NO_DATA, vectorDataUnitOrder);
        }
        else if (dataUnitOrders.is<JsonArray>() == false)
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
            LOG_ERROR(logger, "FAILED TO RESERVE DATA UNIT ORDERS: %s", e.what());
            return std::make_pair(rsc_e::BAD_UNEXPECTED_ERROR, vectorDataUnitOrder);
        }

        for (auto subarrayDataUnitOrders : arrayDataUnitOrders)
        {
            if (subarrayDataUnitOrders.is<JsonArray>() == false)
            {
                vectorDataUnitOrder.clear();
                vectorDataUnitOrder.shrink_to_fit();
                LOG_ERROR(logger, "ELEMENT ARRAY MUST A JSON ARRAY");
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, vectorDataUnitOrder);
            }
            
            JsonArray elementsArray = subarrayDataUnitOrders.as<JsonArray>();
            const size_t elementArraySize = elementsArray.size();
            if (elementArraySize == 0)
            {
                vectorDataUnitOrder.clear();
                vectorDataUnitOrder.shrink_to_fit();
                LOG_ERROR(logger, "SUB-ARRAY OF DATA UNIT ORDERS CANNOT BE EMPTY");
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, vectorDataUnitOrder);
            }
            else if (elementArraySize > UINT8_MAX)
            {
                vectorDataUnitOrder.clear();
                vectorDataUnitOrder.shrink_to_fit();
                LOG_ERROR(logger, "THE SUB-ARRAY OF DATA UNIT ORDERS HAS TOO MANY ELEMENTS: %u", elementArraySize);
                return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, vectorDataUnitOrder);
            }
            
            DataUnitOrder dataUnitOrder(elementArraySize);
            for (auto element : elementsArray)
            {
                if (element.is<const char*>() == false)
                {
                    vectorDataUnitOrder.clear();
                    vectorDataUnitOrder.shrink_to_fit();
                    LOG_ERROR(logger, "ELEMENT OF DATA UNIT ORDERS MUST BE A STRING");
                    return std::make_pair(rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE, vectorDataUnitOrder);
                }

                const auto retConvert = convertToDataUnitOrderType(element.as<const char*>());
                if (retConvert.first != rsc_e::GOOD)
                {
                    vectorDataUnitOrder.clear();
                    vectorDataUnitOrder.shrink_to_fit();
                    LOG_ERROR(logger, "INVALID ELEMENT FOR DATA UNIT ORDER: %s", element.as<const char*>());
                    return std::make_pair(retConvert.first, vectorDataUnitOrder);
                }
                
                Status ret = dataUnitOrder.EmplaceBack(retConvert.second);
                if (ret != Status::Code::GOOD)
                {
                    vectorDataUnitOrder.clear();
                    vectorDataUnitOrder.shrink_to_fit();
                    LOG_ERROR(logger, "FAILED TO EMPLACE DATA UNIT ORDER ELEMENT: %s", ret.c_str());
                    
                    if (ret == Status::Code::BAD_OUT_OF_MEMORY)
                    {
                        return std::make_pair(rsc_e::BAD_OUT_OF_MEMORY, vectorDataUnitOrder);
                    }
                    else
                    {
                        return std::make_pair(rsc_e::BAD_UNEXPECTED_ERROR, vectorDataUnitOrder);
                    }
                }
            }

            try
            {
                vectorDataUnitOrder.emplace_back(dataUnitOrder);
            }
            catch(const std::exception& e)
            {
                vectorDataUnitOrder.clear();
                vectorDataUnitOrder.shrink_to_fit();
                LOG_ERROR(logger, "FAILED TO EMPLACE DATA UNIT ORDER: %s", e.what());
                return std::make_pair(rsc_e::BAD_UNEXPECTED_ERROR, vectorDataUnitOrder);
            }
        }

        return std::make_pair(rsc_e::GOOD, vectorDataUnitOrder);
    }
    
    void NodeValidator::convertToAdressType(const uint8_t type)
    {
        switch (type)
        {
        case 0:
            mAddressType.first   = rsc_e::GOOD;
            mAddressType.second  = adtp_e::NUMERIC;
            return;
        case 1:
        case 2:
        case 3:
            mAddressType.first = rsc_e::BAD_UNSUPPORTED_CONFIGURATION;
            return;
        default:
            mAddressType.first = rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
            return;
        }
    }

    void NodeValidator::convertToAddress(JsonVariant address)
    {
        ASSERT((mAddressType.first == rsc_e::GOOD), "ADDRESS TYPE MUST BE CONFIGURED IN ADVANCE");
        
        switch (mAddressType.second)
        {
        case adtp_e::NUMERIC:
            if (address.is<uint32_t>() == false)
            {
                mAddress.first = rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
                return;
            }
            else
            {
                mAddress.first = rsc_e::GOOD;
                mAddress.second.Numeric = address.as<uint32_t>();
                return;
            }
        case adtp_e::STRING:
        case adtp_e::BYTE_STRING:
        case adtp_e::GUID:
            mAddress.first = rsc_e::BAD_UNSUPPORTED_CONFIGURATION;
            return;
        default:
            mAddress.first = rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
            return;
        }
    }

    /**
     * @return std::pair<Status, mb_area_e> 
     *     @li rsc_e::GOOD 설정 값이 올바릅니다.
     *     @li rsc_e::GOOD_NO_DATA 설정 값이 NULL 입니다.
     *     @li Status::Code::BAD_DATA_ENCODING_INVALID 설정 값이 올바르지 않습니다.
     */
    void NodeValidator::convertToModbusArea(JsonVariant modbusArea)
    {
        if (modbusArea.isNull() == true)
        {
            mModbusArea.first = rsc_e::GOOD_NO_DATA;
            return;
        }

        if (modbusArea.is<uint8_t>() == false)
        {
            mModbusArea.first = rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
            return;
        }
        
        switch (modbusArea.as<uint8_t>())
        {
        case 1:
            mModbusArea.second = mb_area_e::COILS;
            break;
        case 2:
            mModbusArea.second = mb_area_e::DISCRETE_INPUT;
            break;
        case 3:
            mModbusArea.second = mb_area_e::INPUT_REGISTER;
            break;
        case 4:
            mModbusArea.second = mb_area_e::HOLDING_REGISTER;
            break;
        default:
            mModbusArea.first = rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
            return;
        }

        mModbusArea.first = rsc_e::GOOD;
    }

    /**
     * @return std::pair<Status, uint8_t> 
     *     @li rsc_e::GOOD 설정 값이 올바릅니다.
     *     @li rsc_e::GOOD_NO_DATA 설정 값이 NULL 입니다.
     *     @li Status::Code::BAD_DATA_ENCODING_INVALID 설정 값이 올바르지 않습니다.
     */
    void NodeValidator::convertToBitIndex(JsonVariant bitIndex)
    {
        if (bitIndex.isNull() == true)
        {
            mBitIndex.first = rsc_e::GOOD_NO_DATA;
            return;
        }

        if (bitIndex.is<uint8_t>() == false)
        {
            mBitIndex.first = rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
            return;
        }
        
        mBitIndex.first   = rsc_e::GOOD;
        mBitIndex.second  = bitIndex.as<uint8_t>();
    }

    /**
     * @return std::pair<Status, uint8_t> 
     *     @li rsc_e::GOOD 설정 값이 올바릅니다.
     *     @li rsc_e::GOOD_NO_DATA 설정 값이 NULL 입니다.
     *     @li Status::Code::BAD_DATA_ENCODING_INVALID 설정 값이 올바르지 않습니다.
     */
    void NodeValidator::convertToAddressQuantity(JsonVariant addressQuantity)
    {
        if (addressQuantity.isNull() == true)
        {
            mAddressQuantity.first = rsc_e::GOOD_NO_DATA;
            return;
        }

        if (addressQuantity.is<uint8_t>() == false)
        {
            mAddressQuantity.first = rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
            return;
        }
        
        if (addressQuantity.as<uint8_t>() == 0)
        {
            mAddressQuantity.first = rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
            return;
        }

        mAddressQuantity.first   = rsc_e::GOOD;
        mAddressQuantity.second  = addressQuantity.as<uint8_t>();
    }

    /**
     * @return std::pair<Status, scl_e> 
     *     @li rsc_e::GOOD 설정 값이 올바릅니다.
     *     @li rsc_e::GOOD_NO_DATA 설정 값이 NULL 입니다.
     *     @li Status::Code::BAD_DATA_ENCODING_INVALID 설정 값이 올바르지 않습니다.
     */
    void NodeValidator::convertToNumericScale(JsonVariant numericScale)
    {
        if (numericScale.isNull() == true)
        {
            mNumericScale.first = rsc_e::GOOD_NO_DATA;
            return;
        }

        if (numericScale.is<int8_t>() == false)
        {
            mNumericScale.first = rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
            return;
        }
        
        switch (numericScale.as<int8_t>())
        {
        case -3:
            mNumericScale.second = scl_e::NEGATIVE_3;
            break;
        case -2:
            mNumericScale.second = scl_e::NEGATIVE_2;
            break;
        case -1:
            mNumericScale.second = scl_e::NEGATIVE_1;
            break;
        case 1:
            mNumericScale.second = scl_e::POSITIVE_1;
            break;
        case 2:
            mNumericScale.second = scl_e::POSITIVE_2;
            break;
        case 3:
            mNumericScale.second = scl_e::POSITIVE_3;
            break;
        default:
            mNumericScale.first = rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
            return;
        }

        mNumericScale.first = rsc_e::GOOD;
    }

    /**
     * @return std::pair<Status, float> 
     *     @li rsc_e::GOOD 설정 값이 올바릅니다.
     *     @li rsc_e::GOOD_NO_DATA 설정 값이 NULL 입니다.
     *     @li Status::Code::BAD_DATA_ENCODING_INVALID 설정 값이 올바르지 않습니다.
     */
    void NodeValidator::convertToNumericOffset(JsonVariant numericOffset)
    {
        if (numericOffset.isNull() == true)
        {
            mNumericOffset.first = rsc_e::GOOD_NO_DATA;
            return;
        }

        if (numericOffset.is<float>() == false)
        {
            mNumericOffset.first = rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
            return;
        }

        mNumericOffset.first = rsc_e::GOOD;
        mNumericOffset.second = numericOffset.as<float>();
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
        dataUnitOrder.DataUnit = data_unit_e::BYTE;
        dataUnitOrder.ByteOrder = byte_order_e::LOWER;
        dataUnitOrder.Index = 0;

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

    void NodeValidator::convertToFormatString(const JsonVariant formatString)
    {
        if (formatString.isNull() == true)
        {
            mFormatString.first = rsc_e::GOOD_NO_DATA;
            return;
        }

        if (formatString.is<const char*>() == false)
        {
            mFormatString.first = rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
            return;
        }
        
        const char* format = formatString.as<const char*>();
        if (strlen(format) == 0)
        {
            mFormatString.first = rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
            return;
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
                    mFormatString.first = rsc_e::BAD_OUT_OF_MEMORY;
                    return;
                }
                catch(const std::exception& e)
                {
                    mFormatString.first = rsc_e::BAD_UNEXPECTED_ERROR;
                    return;
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
            mFormatString.first = rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
            return;
        }
        else
        {
            mFormatString.first   = rsc_e::GOOD;
            mFormatString.second  = formatString.as<const char*>();
            return;
        }
        
    INVALID_SPECIFIER:
        mVectorFormatSpecifier.clear();
        mFormatString.first = rsc_e::BAD_INVALID_FORMAT_CONFIG_INSTANCE;
    }
}}