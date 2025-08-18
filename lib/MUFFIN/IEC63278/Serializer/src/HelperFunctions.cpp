/**
 * @file HelperFunctions.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-18
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#include "../Include/HelperFunctions.hpp"



namespace muffin { namespace aas {


    JsonDocument ConvertToJSON(const ExtensionBase& extensionBase)
    {
        JsonDocument doc;
        doc["name"] = extensionBase.GetName();

        data_type_def_xsd_e xsd;
        if (extensionBase.GetValueType(&xsd) == true)
        {
            doc["valueType"] = ConvertToString(xsd);
        }

        switch (xsd)
        {
        case data_type_def_xsd_e::STRING:
        {
            using ExtensionType = const Extension<data_type_def_xsd_e::STRING>*;
            ExtensionType extension = static_cast<ExtensionType>(&extensionBase);
            psram::string value;
            if (extension->GetValue(&value) == true)
            {
                doc["value"] = value;
            }
            break;
        }

        case data_type_def_xsd_e::BOOLEAN:
        case data_type_def_xsd_e::DECIMAL:
        case data_type_def_xsd_e::INTEGER:
        case data_type_def_xsd_e::DOUBLE:
        case data_type_def_xsd_e::FLOAT:
        case data_type_def_xsd_e::DATE:
        case data_type_def_xsd_e::TIME:
        case data_type_def_xsd_e::DATETIME:
        case data_type_def_xsd_e::DATETIMESTAMP:
        case data_type_def_xsd_e::G_YEAR:
        case data_type_def_xsd_e::G_MONTH:
        case data_type_def_xsd_e::G_DAY:
        case data_type_def_xsd_e::G_YEAR_MONTH:
        case data_type_def_xsd_e::G_MONTH_DAY:
        case data_type_def_xsd_e::DURATION:
        case data_type_def_xsd_e::YEAR_MONTH_DURATION:
        case data_type_def_xsd_e::DAYTIME_DURATION:
        case data_type_def_xsd_e::BYTE:
        case data_type_def_xsd_e::SHORT:
        case data_type_def_xsd_e::INT:
        case data_type_def_xsd_e::LONG:
        case data_type_def_xsd_e::UNSIGNED_BYTE:
        case data_type_def_xsd_e::UNSIGNED_SHORT:
        case data_type_def_xsd_e::UNSIGNED_INT:
        case data_type_def_xsd_e::UNSIGNED_LONG:
        case data_type_def_xsd_e::POSITIVE_INTEGER:
        case data_type_def_xsd_e::NON_NEGATIVE_INTEGER:
        case data_type_def_xsd_e::NEGATIVE_INTEGER:
        case data_type_def_xsd_e::NON_POSITIVE_INTEGER:
        case data_type_def_xsd_e::HEX_BINARY:
        case data_type_def_xsd_e::BASE64_BINARY:
        case data_type_def_xsd_e::ANY_URI:
        case data_type_def_xsd_e::LANG_STRING:
            log_w("GIVEN DATA TYPE DEFINITION IS NOT IMPLEMENTED YET: %s", ConvertToString(xsd).c_str());
            // deliberately omitted the break to make it fall through to the error log below.
        default:
            log_e("UNDEFINED DATA TYPE DEFINITION: %s", ConvertToString(xsd).c_str());
            break;
        }

        Reference reference(ReferenceTypes::EXTERNAL_REFERENCE, psram::vector<Key>());
        if (extensionBase.GetRefersTo(&reference) == true)
        {
            ASSERT((reference.GetType() == reference_types_e::MODEL_REFERENCE),
                    "ATTRIBUTE 'refersTo' CAN ONLY HAVE 'ModelReference' TYPE");
            doc["refersTo"] = SerializeReference(reference);
        }

        return doc;
    }


    JsonDocument ConvertToJSON(const Reference& reference)
    {
        JsonDocument doc;
        doc["type"] = ConvertToString(reference.GetType());

        const Reference* referredSemanticID = reference.GetReferredSemanticID();
        log_d("referredSemanticID: %p", referredSemanticID);
        if (referredSemanticID != nullptr)
        {
            doc["referredSemanticId"] = ConvertToJSON(*referredSemanticID);
        }

        JsonArray arrKeys = doc["keys"].to<JsonArray>();
        psram::vector<Key> keys = reference.GetKeys();
        for (size_t idx = 0; idx < keys.size(); ++idx)
        {
            JsonObject key = arrKeys.add<JsonObject>();
            key["type"] =  ConvertToString(keys[idx].GetType());
            key["value"] = keys[idx].GetValue();
        }

        return doc;
    }


    psram::string SerializeReference(const Reference& reference)
    {
        psram::string output;
        serializeJson(ConvertToJSON(reference), output);
        return output;
    }


    psram::string SerializeExtension(const ExtensionBase& extensionBase)
    {
        psram::string output;
        serializeJson(ConvertToJSON(extensionBase), output);
        return output;
    }


    Reference DeserializeReference(const JsonObject reference)
    {
        const char* type = reference["type"].as<const char*>();
        ASSERT((type != nullptr), "ATTRIBUTE 'type' CANNOT BE NULL");
        
        const reference_types_e referenceType = strcmp(type, "ModelReference") == 0
            ? reference_types_e::MODEL_REFERENCE
            : reference_types_e::EXTERNAL_REFERENCE;

        const size_t numKeys = reference["keys"].size();
        ASSERT((numKeys > 0), "THE SIZE OF 'keys' MUST BE GREATER THAN 0");

        psram::vector<Key> keys;
        keys.reserve(numKeys);

        for (const auto& key : reference["keys"].as<JsonArray>())
        {
            ASSERT((key.containsKey("type")), "KEY 'type' CANNOT BE MISSING");
            ASSERT((key.containsKey("value")), "KEY 'value' CANNOT BE MISSING");

            const char* strType = key["type"].as<const char*>();
            ASSERT((strType != nullptr), "THE VALUE OF 'type' CANNOT BE NULL");
            const key_types_e type = ConvertToKeyType(strType);

            const char* value = key["value"].as<const char*>();
            ASSERT((value != nullptr), "THE VALUE OF 'value' CANNOT BE NULL");

            keys.emplace_back(type, value);
        }
        ASSERT((keys.empty() == false), "THE SIZE OF 'keys' CANNOT BE ZERO");
        Reference _reference(referenceType, std::move(keys));
        
        if (reference.containsKey("referredSemanticId"))
        {
            _reference.SetReferredSemanticID(
                psram::make_unique<Reference>(
                    DeserializeReference(reference["referredSemanticId"].as<JsonObject>())
                )
            );
        }
        
        return _reference;
    }
}}