/**
 * @file Client.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 
 * @date 2025-10-17
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include "../Container/Container.hpp"
#include "../Metamodel/Abstract/Submodel/Property.hpp"
#include "../Metamodel/Abstract/Submodel/SubmodelElementCollection.hpp"
#include "../include/Converter.hpp"
#include "../Serializer/Serializer.hpp"

#include "Common/PSRAM.hpp"
#include "Common/Logger/Logger.h"
#include "IM/Custom/FirmwareVersion/FirmwareVersion.h"
#include "IM/Node/NodeStore.h"
#include "Protocol/MQTT/CDO.h"



namespace muffin { namespace aas {


    class Client
    {
    public:
        Client(Client const&) = delete;
        void operator=(Client const&) = delete;
        static Client* GetInstance()
        {
            return &mInstance;
        }
    private:
        Client() = default;
        ~Client() noexcept = default;
    private:
        static Client mInstance;

    public:
        void AddEntry(const char* nodeId, const char* idShort)
        {
            mMap.emplace(nodeId, idShort);

            for (auto& e : mMap)
            {
                LOG_DEBUG(logger, "%s: %s", e.first.c_str(), e.second.c_str());
            }
        }

        static void implTask(void* pvParams)
        {
            Container* container = Container::GetInstance();
            const Submodel* submodel = container->GetSubmodelByID(SUBMODEL_ID);

            while (true)
            {
                im::NodeStore& nodeStore = im::NodeStore::GetInstance();
                for (const auto& pair : mMap)
                {
                    const auto result = nodeStore.GetNodeReference(pair.first.c_str());
                    if (result.first.ToCode() != Status::Code::GOOD)
                    {
                        LOG_ERROR(logger, "FAILED TO GET A REFERENCE TO NODE: %s", pair.first.c_str())
                        continue;
                    }

                    for (auto& sme : *submodel)
                    {
                        if (sme->GetModelType() != key_types_e::SubmodelElementCollection)
                        {
                            continue;
                        }
                        
                        SubmodelElementCollection* smc = static_cast<SubmodelElementCollection*>(sme.get());
                        if (strcmp("RealTimeMonitoring", smc->GetIdShortOrNull()) != 0)
                        {
                            continue;
                        }

                        for (auto& element : *smc)
                        {
                            DataElement* dataElement = static_cast<DataElement*>(element.get());
                            if (pair.second != dataElement->GetIdShortOrNull())
                            {
                                continue;
                            }

                            const key_types_e type = dataElement->GetModelType();
                            if (type != key_types_e::PROPERTY)
                            {
                                continue;
                            }
                            
                            im::var_data_t data = result.second->VariableNode.RetrieveData();
                            const data_type_def_xsd_e xsd = dataElement->GetValueType();
                            switch (xsd)
                            {
                            case data_type_def_xsd_e::FLOAT:
                            {
                                const data_type_def_xsd_e _xsd = data_type_def_xsd_e::FLOAT;
                                Property<_xsd>* property = static_cast<Property<_xsd>*>(dataElement);
                                property->SetValue(data.Value.Float32);
                                LOG_DEBUG(logger, "%s: %.2f", pair.second.c_str(), *property->GetValue());
                                break;
                            }
                            
                            default:
                                break;
                            }
                        }
                    }
                }
                
                SubmodelsSerializer submodelSerializer;
                psram::string json = submodelSerializer.EncodeProperty(SUBMODEL_ID, "RealTimeMonitoring");
            
                mqtt::Message msg(mqtt::topic_e::AAS_MQTT, json.c_str());
                mqtt::cdo.Store(msg);

                vTaskDelay(pdMS_TO_TICKS(10 * 1000));
            }
        }

        void Start()
        {
            LOG_INFO(logger, "Starting AAS Client Task");
            vTaskDelay(pdMS_TO_TICKS(5000));

            xTaskCreatePinnedToCore(
                implTask,
                "AasClientTask",
                8192,
                NULL,
                0,
                NULL,
                0
            );
        }

    private:
        /**
         * @param #1: node ID
         * @param #2: idShort
         */
        static psram::map<psram::string, psram::string> mMap;
    };

    
    Client Client::mInstance;
    psram::map<psram::string, psram::string> Client::mMap;
}}