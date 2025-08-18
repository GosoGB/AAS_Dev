/**
 * @file HelperFunctions.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-18
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



    psram::unique_ptr<QualifierBase> DeserializeQualifiers(const JsonArray qualifiers)
    {
        ;
    }
}}