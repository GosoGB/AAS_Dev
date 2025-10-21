/**
 * @file SubmodelsSerializer.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-19
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#include "../../Container/Container.hpp"
#include "../../Include/Converter.hpp"
#include "../Include/HelperFunctions.hpp"
#include "../Include/SubmodelsSerializer.hpp"



namespace muffin { namespace aas {

    
    psram::string SubmodelsSerializer::Encode(const Identifier& id)
    {
        Container* container = Container::GetInstance();
        const Submodel* submodel = container->GetSubmodelByID(id);
        if (submodel == nullptr)
        {
            log_d("Submodel Not found with id: '%s'", id.c_str());
            return psram::string();
        }

        psram::string output;
        JsonDocument doc = encode(*submodel);
        serializeJson(doc, output);
        return output;
    }

    
    psram::string SubmodelsSerializer::EncodeProperty(const Identifier& id, const psram::string& idShort)
    {
        Container* container = Container::GetInstance();
        const Submodel* submodel = container->GetSubmodelByID(id);
        if (submodel == nullptr)
        {
            log_d("Submodel Not found with id: '%s'", id.c_str());
            return psram::string();
        }

        psram::string output;
        const SubmodelElement* property = submodel->GetElementWithIdShort(idShort);

        JsonDocument doc = SerializeSubmodelElement(*property);
        serializeJson(doc, output);
        return output;
    }


    psram::string SubmodelsSerializer::EncodeAll()
    {
        Container* container = Container::GetInstance();
        const auto vectorSubmodels = container->GetAllSubmodels();

        JsonDocument doc;
        JsonArray arraySubmodels = doc.to<JsonArray>();

        for (const auto& submodel : vectorSubmodels)
        {
            JsonDocument docSubmodel = encode(submodel);
            arraySubmodels.add(docSubmodel);
        }

        psram::string output;
        serializeJson(doc, output);
        return output;
    }


    JsonDocument SubmodelsSerializer::encode(const Submodel& submodel)
    {
        JsonDocument doc;
        doc["modelType"] = "Submodel";
        doc["id"] = submodel.GetID();
        doc["kind"] = ConvertToString(submodel.GetKind());

        if (submodel.GetIdShortOrNull() != nullptr)
        {
            doc["idShort"] = submodel.GetIdShortOrNull();
        }

        if (submodel.GetSemanticIdOrNULL() != nullptr)
        {
            doc["semanticId"] = SerializeReference(*submodel.GetSemanticIdOrNULL());
        }

        if (submodel.GetCategoryOrNull() != nullptr)
        {
            doc["category"] = submodel.GetCategoryOrNull();
        }
        
        if (submodel.GetExtensionOrNull() != nullptr)
        {
            JsonArray extensions = doc["extensions"].to<JsonArray>();
            extensions.add(SerializeExtension(*submodel.GetExtensionOrNull()));
        }
        
        if (submodel.GetQualifierOrNULL() != nullptr)
        {
            JsonArray qualifiers = doc["qualifiers"].to<JsonArray>();
            qualifiers.add(SerializeQualifier(*submodel.GetQualifierOrNULL()));
        }

        if (submodel.size() == 0)
        {
            return doc;
        }
        
        JsonArray submodelElements = doc["submodelElements"].to<JsonArray>();
        for (const auto& submodelElement : submodel)
        {
            submodelElements.add(SerializeSubmodelElement(*submodelElement.get()));
        }
        
        return doc;
    }
}}