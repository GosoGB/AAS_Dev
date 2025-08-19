/**
 * @file HelperFunctions.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-19
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#include "../../Include/Converter.hpp"
#include "../../Metamodel/Extension.hpp"
#include "../Include/HelperFunctions.hpp"



namespace muffin { namespace aas {


    template<data_type_def_xsd_e xsd>
    void updateExtensionValue(const ExtensionBase& extensionBase, JsonDocument* doc)
    {
        using obj_type = const Extension<xsd>*;
        using xsd_type = const typename xsd_type_mapper<xsd>::type*;

        xsd_type value = static_cast<obj_type>(&extensionBase)->GetValueOrNULL();
        if (value != nullptr)
        {
            doc->operator[]("value") = *value;
        }
    }


    template<data_type_def_xsd_e xsd>
    void updateQualifierValue(const QualifierBase& qualifierBase, JsonDocument* doc)
    {
        using obj_type = const Qualifier<xsd>*;
        using xsd_type = const typename xsd_type_mapper<xsd>::type*;

        xsd_type value = static_cast<obj_type>(&qualifierBase)->GetValueOrNULL();
        if (value != nullptr)
        {
            doc->operator[]("value") = *value;
        }
    }


    void processExtensionValue(const ExtensionBase& extensionBase, JsonDocument* doc)
    {
        data_type_def_xsd_e xsd;
        extensionBase.GetValueType(&xsd);

        switch (xsd)
        {
        case data_type_def_xsd_e::STRING:
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
            updateExtensionValue<data_type_def_xsd_e::STRING>(extensionBase, doc);
            return;
        
        case data_type_def_xsd_e::BOOLEAN:
            updateExtensionValue<data_type_def_xsd_e::BOOLEAN>(extensionBase, doc);
            return;
        
        case data_type_def_xsd_e::DECIMAL:
        case data_type_def_xsd_e::DOUBLE:
            updateExtensionValue<data_type_def_xsd_e::DECIMAL>(extensionBase, doc);
            return;
        
        case data_type_def_xsd_e::INTEGER:
        case data_type_def_xsd_e::LONG:
        case data_type_def_xsd_e::NEGATIVE_INTEGER:
        case data_type_def_xsd_e::NON_POSITIVE_INTEGER:
            updateExtensionValue<data_type_def_xsd_e::INTEGER>(extensionBase, doc);
            return;
        
        case data_type_def_xsd_e::FLOAT:
            updateExtensionValue<data_type_def_xsd_e::FLOAT>(extensionBase, doc);
            return;
        
        case data_type_def_xsd_e::BYTE:
            updateExtensionValue<data_type_def_xsd_e::BYTE>(extensionBase, doc);
            return;
        
        case data_type_def_xsd_e::SHORT:
            updateExtensionValue<data_type_def_xsd_e::SHORT>(extensionBase, doc);
            return;
        
        case data_type_def_xsd_e::INT:
            updateExtensionValue<data_type_def_xsd_e::INT>(extensionBase, doc);
            return;
        
        case data_type_def_xsd_e::UNSIGNED_BYTE:
            updateExtensionValue<data_type_def_xsd_e::UNSIGNED_BYTE>(extensionBase, doc);
            return;

        case data_type_def_xsd_e::UNSIGNED_SHORT:
            updateExtensionValue<data_type_def_xsd_e::UNSIGNED_SHORT>(extensionBase, doc);
            return;
            
        case data_type_def_xsd_e::UNSIGNED_INT:
            updateExtensionValue<data_type_def_xsd_e::UNSIGNED_INT>(extensionBase, doc);
            return;
            
        case data_type_def_xsd_e::UNSIGNED_LONG:
        case data_type_def_xsd_e::POSITIVE_INTEGER:
        case data_type_def_xsd_e::NON_NEGATIVE_INTEGER:
            updateExtensionValue<data_type_def_xsd_e::UNSIGNED_LONG>(extensionBase, doc);
            return;
        
        default:
            log_d("UNIMPLEMENTED DATA TYPE DEFINITION: %s", ConvertToString(xsd).c_str());
            break;
        }
    }


    void processQualifierValue(const QualifierBase& qualifierBase, JsonDocument* doc)
    {
        switch (qualifierBase.GetValueType())
        {
        case data_type_def_xsd_e::STRING:
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
            updateQualifierValue<data_type_def_xsd_e::STRING>(qualifierBase, doc);
            return;
        
        case data_type_def_xsd_e::BOOLEAN:
            updateQualifierValue<data_type_def_xsd_e::BOOLEAN>(qualifierBase, doc);
            return;
        
        case data_type_def_xsd_e::DECIMAL:
        case data_type_def_xsd_e::DOUBLE:
            updateQualifierValue<data_type_def_xsd_e::DECIMAL>(qualifierBase, doc);
            return;
        
        case data_type_def_xsd_e::INTEGER:
        case data_type_def_xsd_e::LONG:
        case data_type_def_xsd_e::NEGATIVE_INTEGER:
        case data_type_def_xsd_e::NON_POSITIVE_INTEGER:
            updateQualifierValue<data_type_def_xsd_e::INTEGER>(qualifierBase, doc);
            return;
        
        case data_type_def_xsd_e::FLOAT:
            updateQualifierValue<data_type_def_xsd_e::FLOAT>(qualifierBase, doc);
            return;
        
        case data_type_def_xsd_e::BYTE:
            updateQualifierValue<data_type_def_xsd_e::BYTE>(qualifierBase, doc);
            return;
        
        case data_type_def_xsd_e::SHORT:
            updateQualifierValue<data_type_def_xsd_e::SHORT>(qualifierBase, doc);
            return;
        
        case data_type_def_xsd_e::INT:
            updateQualifierValue<data_type_def_xsd_e::INT>(qualifierBase, doc);
            return;
        
        case data_type_def_xsd_e::UNSIGNED_BYTE:
            updateQualifierValue<data_type_def_xsd_e::UNSIGNED_BYTE>(qualifierBase, doc);
            return;

        case data_type_def_xsd_e::UNSIGNED_SHORT:
            updateQualifierValue<data_type_def_xsd_e::UNSIGNED_SHORT>(qualifierBase, doc);
            return;
            
        case data_type_def_xsd_e::UNSIGNED_INT:
            updateQualifierValue<data_type_def_xsd_e::UNSIGNED_INT>(qualifierBase, doc);
            return;
            
        case data_type_def_xsd_e::UNSIGNED_LONG:
        case data_type_def_xsd_e::POSITIVE_INTEGER:
        case data_type_def_xsd_e::NON_NEGATIVE_INTEGER:
            updateQualifierValue<data_type_def_xsd_e::UNSIGNED_LONG>(qualifierBase, doc);
            return;
        
        default:
            log_d("UNIMPLEMENTED DATA TYPE DEFINITION: %s", 
                ConvertToString(qualifierBase.GetValueType()).c_str());
            break;
        }
    }


    void processQualifierValue(JsonVariant value, QualifierBase* qualifierBase)
    {
        using xsd = data_type_def_xsd_e;

        switch (qualifierBase->GetValueType())
        {
        case data_type_def_xsd_e::STRING:
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
        {
            Qualifier<xsd::STRING>* qualifier = static_cast<Qualifier<xsd::STRING>*>(qualifierBase);
            qualifier->SetValue(value.as<const char*>());
            return;
        }
        case data_type_def_xsd_e::BOOLEAN:
        {
            Qualifier<xsd::BOOLEAN>* qualifier = static_cast<Qualifier<xsd::BOOLEAN>*>(qualifierBase);
            qualifier->SetValue(value.as<bool>());
            return;
        }
        case data_type_def_xsd_e::DECIMAL:
        case data_type_def_xsd_e::DOUBLE:
        {
            Qualifier<xsd::DECIMAL>* qualifier = static_cast<Qualifier<xsd::DECIMAL>*>(qualifierBase);
            qualifier->SetValue(value.as<double>());
            return;
        }
        case data_type_def_xsd_e::INTEGER:
        case data_type_def_xsd_e::LONG:
        case data_type_def_xsd_e::NEGATIVE_INTEGER:
        case data_type_def_xsd_e::NON_POSITIVE_INTEGER:
        {
            Qualifier<xsd::INTEGER>* qualifier = static_cast<Qualifier<xsd::INTEGER>*>(qualifierBase);
            qualifier->SetValue(value.as<int64_t>());
            return;
        }
        case data_type_def_xsd_e::FLOAT:
        {
            Qualifier<xsd::FLOAT>* qualifier = static_cast<Qualifier<xsd::FLOAT>*>(qualifierBase);
            qualifier->SetValue(value.as<float>());
            return;
        }
        case data_type_def_xsd_e::BYTE:
        {
            Qualifier<xsd::BYTE>* qualifier = static_cast<Qualifier<xsd::BYTE>*>(qualifierBase);
            qualifier->SetValue(value.as<uint8_t>());
            return;
        }
        case data_type_def_xsd_e::SHORT:
        {
            Qualifier<xsd::SHORT>* qualifier = static_cast<Qualifier<xsd::SHORT>*>(qualifierBase);
            qualifier->SetValue(value.as<int16_t>());
            return;
        }
        case data_type_def_xsd_e::INT:
        {
            Qualifier<xsd::INT>* qualifier = static_cast<Qualifier<xsd::INT>*>(qualifierBase);
            qualifier->SetValue(value.as<int32_t>());
            return;
        }
        case data_type_def_xsd_e::UNSIGNED_BYTE:
        {
            Qualifier<xsd::UNSIGNED_BYTE>* qualifier = static_cast<Qualifier<xsd::UNSIGNED_BYTE>*>(qualifierBase);
            qualifier->SetValue(value.as<uint8_t>());
            return;
        }
        case data_type_def_xsd_e::UNSIGNED_SHORT:
        {
            Qualifier<xsd::UNSIGNED_SHORT>* qualifier = static_cast<Qualifier<xsd::UNSIGNED_SHORT>*>(qualifierBase);
            qualifier->SetValue(value.as<uint16_t>());
            return;
        }   
        case data_type_def_xsd_e::UNSIGNED_INT:
        {
            Qualifier<xsd::UNSIGNED_INT>* qualifier = static_cast<Qualifier<xsd::UNSIGNED_INT>*>(qualifierBase);
            qualifier->SetValue(value.as<uint32_t>());
            return;
        }
        case data_type_def_xsd_e::UNSIGNED_LONG:
        case data_type_def_xsd_e::POSITIVE_INTEGER:
        case data_type_def_xsd_e::NON_NEGATIVE_INTEGER:
        {
            Qualifier<xsd::UNSIGNED_LONG>* qualifier = static_cast<Qualifier<xsd::UNSIGNED_LONG>*>(qualifierBase);
            qualifier->SetValue(value.as<uint64_t>());
            return;
        }
        default:
            log_d("UNIMPLEMENTED DATA TYPE DEFINITION: %s", 
                ConvertToString(qualifierBase->GetValueType()).c_str());
            break;
        }
    }


    JsonDocument ConvertToJSON(const ExtensionBase& extensionBase)
    {
        JsonDocument doc;
        doc["name"] = extensionBase.GetName();

        data_type_def_xsd_e xsd;
        if (extensionBase.GetValueType(&xsd) == true)
        {
            doc["valueType"] = ConvertToString(xsd);
            processExtensionValue(extensionBase, &doc);
        }

        if (extensionBase.GetRefersToOrNULL() != nullptr)
        {
            ASSERT((
                extensionBase.GetRefersToOrNULL()->GetType() == reference_types_e::MODEL_REFERENCE),
                "ATTRIBUTE 'refersTo' CAN ONLY HAVE 'ModelReference' TYPE");
            
            JsonArray refersTo = doc["refersTo"].to<JsonArray>();
            refersTo.add(SerializeReference(*extensionBase.GetRefersToOrNULL()));
        }

        /**
         * @todo semanticId 처리 구현해야 함
         */
        // extensionBase->SetSemanticId(nullptr);
        return doc;
    }


    JsonDocument ConvertToJSON(const QualifierBase& qualifierBase)
    {
        JsonDocument doc;
        doc["type"] = qualifierBase.GetType();

        const data_type_def_xsd_e xsd = qualifierBase.GetValueType();
        doc["valueType"] = ConvertToString(xsd);
        processQualifierValue(qualifierBase, &doc);

        if (qualifierBase.GetKindOrNULL() != nullptr)
        {
            doc["kind"] = ConvertToString(*qualifierBase.GetKindOrNULL());
        }

        if (qualifierBase.GetValueIdOrNULL() != nullptr)
        {
            const Reference* valueId = qualifierBase.GetValueIdOrNULL();
            doc["valueId"] = SerializeReference(*valueId);
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


    psram::string SerializeExtension(const ExtensionBase& extensionBase)
    {
        psram::string output;
        serializeJson(ConvertToJSON(extensionBase), output);
        return output;
    }


    psram::string SerializeQualifier(const QualifierBase& qualifierBase)
    {
        psram::string output;
        serializeJson(ConvertToJSON(qualifierBase), output);
        return output;
    }


    psram::string SerializeReference(const Reference& reference)
    {
        psram::string output;
        serializeJson(ConvertToJSON(reference), output);
        return output;
    }


    psram::unique_ptr<ExtensionBase> constructExtension(const char* name, const char* valueType)
    {
        ASSERT((valueType != nullptr), "ATTRIBUTE 'valueType' CANNOT BE NULL");

        using xsd = data_type_def_xsd_e;
        using base = ExtensionBase;
        using namespace psram;
        
        switch (ConvertToDataTypeDefXSD(valueType))
        {
        case data_type_def_xsd_e::STRING:
            return make_unique<base>(*make_unique<Extension<xsd::STRING>>(name));
        case data_type_def_xsd_e::BOOLEAN:
            return make_unique<base>(*make_unique<Extension<xsd::BOOLEAN>>(name));
        case data_type_def_xsd_e::DECIMAL:
            return make_unique<base>(*make_unique<Extension<xsd::DECIMAL>>(name));
        case data_type_def_xsd_e::INTEGER:
            return make_unique<base>(*make_unique<Extension<xsd::INTEGER>>(name));
        case data_type_def_xsd_e::DOUBLE:
            return make_unique<base>(*make_unique<Extension<xsd::DOUBLE>>(name));
        case data_type_def_xsd_e::FLOAT:
            return make_unique<base>(*make_unique<Extension<xsd::FLOAT>>(name));
        case data_type_def_xsd_e::DATE:
            return make_unique<base>(*make_unique<Extension<xsd::DATE>>(name));
        case data_type_def_xsd_e::TIME:
            return make_unique<base>(*make_unique<Extension<xsd::TIME>>(name));
        case data_type_def_xsd_e::DATETIME:
            return make_unique<base>(*make_unique<Extension<xsd::DATETIME>>(name));
        case data_type_def_xsd_e::DATETIMESTAMP:
            return make_unique<base>(*make_unique<Extension<xsd::DATETIMESTAMP>>(name));
        case data_type_def_xsd_e::G_YEAR:
            return make_unique<base>(*make_unique<Extension<xsd::G_YEAR>>(name));
        case data_type_def_xsd_e::G_MONTH:
            return make_unique<base>(*make_unique<Extension<xsd::G_MONTH>>(name));
        case data_type_def_xsd_e::G_DAY:
            return make_unique<base>(*make_unique<Extension<xsd::G_DAY>>(name));
        case data_type_def_xsd_e::G_YEAR_MONTH:
            return make_unique<base>(*make_unique<Extension<xsd::G_YEAR_MONTH>>(name));
        case data_type_def_xsd_e::G_MONTH_DAY:
            return make_unique<base>(*make_unique<Extension<xsd::G_MONTH_DAY>>(name));
        case data_type_def_xsd_e::DURATION:
            return make_unique<base>(*make_unique<Extension<xsd::DURATION>>(name));
        case data_type_def_xsd_e::YEAR_MONTH_DURATION:
            return make_unique<base>(*make_unique<Extension<xsd::YEAR_MONTH_DURATION>>(name));
        case data_type_def_xsd_e::DAYTIME_DURATION:
            return make_unique<base>(*make_unique<Extension<xsd::DAYTIME_DURATION>>(name));
        case data_type_def_xsd_e::BYTE:
            return make_unique<base>(*make_unique<Extension<xsd::BYTE>>(name));
        case data_type_def_xsd_e::SHORT:
            return make_unique<base>(*make_unique<Extension<xsd::SHORT>>(name));
        case data_type_def_xsd_e::INT:
            return make_unique<base>(*make_unique<Extension<xsd::INT>>(name));
        case data_type_def_xsd_e::LONG:
            return make_unique<base>(*make_unique<Extension<xsd::LONG>>(name));
        case data_type_def_xsd_e::UNSIGNED_BYTE:
            return make_unique<base>(*make_unique<Extension<xsd::UNSIGNED_BYTE>>(name));
        case data_type_def_xsd_e::UNSIGNED_SHORT:
            return make_unique<base>(*make_unique<Extension<xsd::UNSIGNED_SHORT>>(name));
        case data_type_def_xsd_e::UNSIGNED_INT:
            return make_unique<base>(*make_unique<Extension<xsd::UNSIGNED_INT>>(name));
        case data_type_def_xsd_e::UNSIGNED_LONG:
            return make_unique<base>(*make_unique<Extension<xsd::UNSIGNED_LONG>>(name));
        case data_type_def_xsd_e::POSITIVE_INTEGER:
            return make_unique<base>(*make_unique<Extension<xsd::POSITIVE_INTEGER>>(name));
        case data_type_def_xsd_e::NON_NEGATIVE_INTEGER:
            return make_unique<base>(*make_unique<Extension<xsd::NON_NEGATIVE_INTEGER>>(name));
        case data_type_def_xsd_e::NEGATIVE_INTEGER:
            return make_unique<base>(*make_unique<Extension<xsd::NEGATIVE_INTEGER>>(name));
        case data_type_def_xsd_e::NON_POSITIVE_INTEGER:
            return make_unique<base>(*make_unique<Extension<xsd::NON_POSITIVE_INTEGER>>(name));
        // case data_type_def_xsd_e::HEX_BINARY:
        //     return psram::make_unique<ExtensionBase>(Extension<xsd::HEX_BINARY>(name));
        // case data_type_def_xsd_e::BASE64_BINARY:
        //     return psram::make_unique<ExtensionBase>(Extension<xsd::BASE64_BINARY>(name));
        // case data_type_def_xsd_e::ANY_URI:
        //     return psram::make_unique<ExtensionBase>(Extension<xsd::ANY_URI>(name));
        // case data_type_def_xsd_e::LANG_STRING:
        //     return psram::make_unique<ExtensionBase>(Extension<xsd::LANG_STRING>(name));
        default:
            log_w("UNIMPLEMENTED DATA TYPE DEFINITION: %s", valueType);
            ASSERT((false), "UNIMPLEMENTED DATA TYPE DEFINITION: %s", valueType);
        }
    }


    psram::unique_ptr<ExtensionBase> DeserializeExtensions(const JsonArray extensions)
    {
        ASSERT((extensions.size() > 0), "THE SIZE OF 'extensions' MUST BE GREATER THAN 0");
        
        JsonObject extensionObject = extensions[0].as<JsonObject>();
        const char* name = extensionObject["name"].as<const char*>();

        psram::unique_ptr<ExtensionBase> extensionBase;
        const char* strXSD = extensionObject["valueType"].as<const char*>();
        extensionBase.reset(constructExtension(name, strXSD).get());

        if (extensionObject.containsKey("refersTo"))
        {
            Reference refersTo = DeserializeReference(extensionObject["refersTo"].as<JsonObject>());
            extensionBase->SetReference(refersTo);
        }
        
        /**
         * @todo semanticId 처리 구현해야 함
         */
        // extension->GetSemanticIdOrNULL();
        return extensionBase;
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


    psram::unique_ptr<QualifierBase> constructQualifier(const char* qualifierType, const char* valueType)
    {
        ASSERT((qualifierType != nullptr), "ATTRIBUTE 'type' CANNOT BE NULL");
        ASSERT((valueType != nullptr), "ATTRIBUTE 'valueType' CANNOT BE NULL");

        using xsd = data_type_def_xsd_e;
        using base = QualifierBase;
        using namespace psram;
        
        switch (ConvertToDataTypeDefXSD(valueType))
        {
        case data_type_def_xsd_e::STRING:
            return make_unique<base>(*make_unique<Qualifier<xsd::STRING>>(qualifierType));
        case data_type_def_xsd_e::BOOLEAN:
            return make_unique<base>(*make_unique<Qualifier<xsd::BOOLEAN>>(qualifierType));
        case data_type_def_xsd_e::DECIMAL:
            return make_unique<base>(*make_unique<Qualifier<xsd::DECIMAL>>(qualifierType));
        case data_type_def_xsd_e::INTEGER:
            return make_unique<base>(*make_unique<Qualifier<xsd::INTEGER>>(qualifierType));
        case data_type_def_xsd_e::DOUBLE:
            return make_unique<base>(*make_unique<Qualifier<xsd::DOUBLE>>(qualifierType));
        case data_type_def_xsd_e::FLOAT:
            return make_unique<base>(*make_unique<Qualifier<xsd::FLOAT>>(qualifierType));
        case data_type_def_xsd_e::DATE:
            return make_unique<base>(*make_unique<Qualifier<xsd::DATE>>(qualifierType));
        case data_type_def_xsd_e::TIME:
            return make_unique<base>(*make_unique<Qualifier<xsd::TIME>>(qualifierType));
        case data_type_def_xsd_e::DATETIME:
            return make_unique<base>(*make_unique<Qualifier<xsd::DATETIME>>(qualifierType));
        case data_type_def_xsd_e::DATETIMESTAMP:
            return make_unique<base>(*make_unique<Qualifier<xsd::DATETIMESTAMP>>(qualifierType));
        case data_type_def_xsd_e::G_YEAR:
            return make_unique<base>(*make_unique<Qualifier<xsd::G_YEAR>>(qualifierType));
        case data_type_def_xsd_e::G_MONTH:
            return make_unique<base>(*make_unique<Qualifier<xsd::G_MONTH>>(qualifierType));
        case data_type_def_xsd_e::G_DAY:
            return make_unique<base>(*make_unique<Qualifier<xsd::G_DAY>>(qualifierType));
        case data_type_def_xsd_e::G_YEAR_MONTH:
            return make_unique<base>(*make_unique<Qualifier<xsd::G_YEAR_MONTH>>(qualifierType));
        case data_type_def_xsd_e::G_MONTH_DAY:
            return make_unique<base>(*make_unique<Qualifier<xsd::G_MONTH_DAY>>(qualifierType));
        case data_type_def_xsd_e::DURATION:
            return make_unique<base>(*make_unique<Qualifier<xsd::DURATION>>(qualifierType));
        case data_type_def_xsd_e::YEAR_MONTH_DURATION:
            return make_unique<base>(*make_unique<Qualifier<xsd::YEAR_MONTH_DURATION>>(qualifierType));
        case data_type_def_xsd_e::DAYTIME_DURATION:
            return make_unique<base>(*make_unique<Qualifier<xsd::DAYTIME_DURATION>>(qualifierType));
        case data_type_def_xsd_e::BYTE:
            return make_unique<base>(*make_unique<Qualifier<xsd::BYTE>>(qualifierType));
        case data_type_def_xsd_e::SHORT:
            return make_unique<base>(*make_unique<Qualifier<xsd::SHORT>>(qualifierType));
        case data_type_def_xsd_e::INT:
            return make_unique<base>(*make_unique<Qualifier<xsd::INT>>(qualifierType));
        case data_type_def_xsd_e::LONG:
            return make_unique<base>(*make_unique<Qualifier<xsd::LONG>>(qualifierType));
        case data_type_def_xsd_e::UNSIGNED_BYTE:
            return make_unique<base>(*make_unique<Qualifier<xsd::UNSIGNED_BYTE>>(qualifierType));
        case data_type_def_xsd_e::UNSIGNED_SHORT:
            return make_unique<base>(*make_unique<Qualifier<xsd::UNSIGNED_SHORT>>(qualifierType));
        case data_type_def_xsd_e::UNSIGNED_INT:
            return make_unique<base>(*make_unique<Qualifier<xsd::UNSIGNED_INT>>(qualifierType));
        case data_type_def_xsd_e::UNSIGNED_LONG:
            return make_unique<base>(*make_unique<Qualifier<xsd::UNSIGNED_LONG>>(qualifierType));
        case data_type_def_xsd_e::POSITIVE_INTEGER:
            return make_unique<base>(*make_unique<Qualifier<xsd::POSITIVE_INTEGER>>(qualifierType));
        case data_type_def_xsd_e::NON_NEGATIVE_INTEGER:
            return make_unique<base>(*make_unique<Qualifier<xsd::NON_NEGATIVE_INTEGER>>(qualifierType));
        case data_type_def_xsd_e::NEGATIVE_INTEGER:
            return make_unique<base>(*make_unique<Qualifier<xsd::NEGATIVE_INTEGER>>(qualifierType));
        case data_type_def_xsd_e::NON_POSITIVE_INTEGER:
            return make_unique<base>(*make_unique<Qualifier<xsd::NON_POSITIVE_INTEGER>>(qualifierType));
        // case data_type_def_xsd_e::HEX_BINARY:
        //     return psram::make_unique<ExtensionBase>(Extension<xsd::HEX_BINARY>(name));
        // case data_type_def_xsd_e::BASE64_BINARY:
        //     return psram::make_unique<ExtensionBase>(Extension<xsd::BASE64_BINARY>(name));
        // case data_type_def_xsd_e::ANY_URI:
        //     return psram::make_unique<ExtensionBase>(Extension<xsd::ANY_URI>(name));
        // case data_type_def_xsd_e::LANG_STRING:
        //     return psram::make_unique<ExtensionBase>(Extension<xsd::LANG_STRING>(name));
        default:
            log_w("UNIMPLEMENTED DATA TYPE DEFINITION: %s", valueType);
            ASSERT((false), "UNIMPLEMENTED DATA TYPE DEFINITION: %s", valueType);
        }
    }


    psram::unique_ptr<QualifierBase> DeserializeQualifiers(const JsonArray qualifiers)
    {
        ASSERT((qualifiers.size() > 0), "THE SIZE OF 'qualifiers' MUST BE GREATER THAN 0");
        
        JsonObject qualifierObject = qualifiers[0].as<JsonObject>();
        const char* qualifierType = qualifierObject["type"].as<const char*>();
        ASSERT((qualifierType != nullptr), "ATTRIBUTE 'type' CANNOT BE NULL");
        const char* valueType = qualifierObject["valueType"].as<const char*>();
        ASSERT((valueType != nullptr), "ATTRIBUTE 'valueType' CANNOT BE NULL");
        
        psram::unique_ptr<QualifierBase> qualifier = constructQualifier(qualifierType, valueType);
        
        if (qualifierObject.containsKey("kind"))
        {
            const char* qualifierKind = qualifierObject["kind"].as<const char*>();
            qualifier->SetKind(ConvertToQualifierKindType(qualifierKind));
        }
        
        if (qualifierObject.containsKey("valueId"))
        {
            JsonObject valueId = qualifierObject["valueId"].as<JsonObject>();
            qualifier->SetValueID(DeserializeReference(valueId));
        }
        
        if (qualifierObject.containsKey("value"))
        {
            processQualifierValue(qualifierObject["value"].as<JsonVariant>(), qualifier.get());
        }

        return qualifier;
    }


    psram::unique_ptr<SubmodelElement> DeserializeSubmodelElements(const JsonArray submodelElements)
    {
        ASSERT((submodelElements.size() > 0), "THE SIZE OF 'qualifiers' MUST BE GREATER THAN 0");
        
        // SubmodelElement::Clone();
        // SubmodelElement::GetCategoryOrNull();
        // SubmodelElement::GetExtensionOrNull();
        // SubmodelElement::GetIdShortOrNull();
        // SubmodelElement::GetKind();
        // SubmodelElement::GetModelType();
        // SubmodelElement::GetQualifierOrNULL();
        // SubmodelElement::GetSemanticIdOrNULL();

        for (const JsonObject submodelElement : submodelElements)
        {
            const char* modelType = submodelElement["modelType"].as<const char*>();
            ASSERT((modelType != nullptr), "ATTRIBUTE 'modelType' CANNOT BE NULL");
            
            const char* qualifierType = submodelElement["type"].as<const char*>();
            ASSERT((qualifierType != nullptr), "ATTRIBUTE 'type' CANNOT BE NULL");
            const char* valueType = submodelElement["valueType"].as<const char*>();
            ASSERT((valueType != nullptr), "ATTRIBUTE 'valueType' CANNOT BE NULL");
        }
        
        psram::unique_ptr<QualifierBase> qualifier = constructQualifier(qualifierType, valueType);
        
        if (qualifierObject.containsKey("kind"))
        {
            const char* qualifierKind = qualifierObject["kind"].as<const char*>();
            qualifier->SetKind(ConvertToQualifierKindType(qualifierKind));
        }
        
        if (qualifierObject.containsKey("valueId"))
        {
            JsonObject valueId = qualifierObject["valueId"].as<JsonObject>();
            qualifier->SetValueID(DeserializeReference(valueId));
        }
        
        if (qualifierObject.containsKey("value"))
        {
            processQualifierValue(qualifierObject["value"].as<JsonVariant>(), qualifier.get());
        }

        return qualifier;
    }
}}