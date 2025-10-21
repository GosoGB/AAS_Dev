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


    static uint32_t sRealtimeMonitorTimer = millis();
    static uint32_t sJobProgressTimer = millis();
    static uint32_t sConfigurationTimer = millis();


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

    private:
        static void updateProperty(const data_type_def_xsd_e xsd, const im::var_data_t& data, DataElement* dataElement)
        {
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
            case data_type_def_xsd_e::ANY_URI:
            {
                const data_type_def_xsd_e _xsd = data_type_def_xsd_e::STRING;
                Property<_xsd>* property = static_cast<Property<_xsd>*>(dataElement);
                property->SetValue(data.Value.String.Data);
                break;
            }

            case data_type_def_xsd_e::BOOLEAN:
            {
                const data_type_def_xsd_e _xsd = data_type_def_xsd_e::BOOLEAN;
                Property<_xsd>* property = static_cast<Property<_xsd>*>(dataElement);
                property->SetValue(data.Value.Boolean);
                break;
            }

            case data_type_def_xsd_e::DECIMAL:
            case data_type_def_xsd_e::DOUBLE:
            {
                const data_type_def_xsd_e _xsd = data_type_def_xsd_e::DOUBLE;
                Property<_xsd>* property = static_cast<Property<_xsd>*>(dataElement);
                property->SetValue(data.Value.Float64);
                break;
            }

            case data_type_def_xsd_e::INTEGER:
            case data_type_def_xsd_e::LONG:
            case data_type_def_xsd_e::NEGATIVE_INTEGER:
            case data_type_def_xsd_e::NON_POSITIVE_INTEGER:
            {
                const data_type_def_xsd_e _xsd = data_type_def_xsd_e::INTEGER;
                Property<_xsd>* property = static_cast<Property<_xsd>*>(dataElement);
                switch (data.DataType)
                {
                case jvs::dt_e::INT8:
                    property->SetValue(data.Value.Int8);
                    break;

                case jvs::dt_e::INT16:
                    property->SetValue(data.Value.Int16);
                    break;
                
                case jvs::dt_e::INT32:
                    property->SetValue(data.Value.Int32);
                    break;
                    
                case jvs::dt_e::INT64:
                    property->SetValue(data.Value.Int64);
                    break;

                default:
                    break;
                }
                
                break;
            }

            case data_type_def_xsd_e::FLOAT:
            {
                const data_type_def_xsd_e _xsd = data_type_def_xsd_e::FLOAT;
                Property<_xsd>* property = static_cast<Property<_xsd>*>(dataElement);

                switch (data.DataType)
                {
                case jvs::dt_e::FLOAT32:
                    property->SetValue(data.Value.Float32);
                    break;
                
                case jvs::dt_e::FLOAT64:
                    property->SetValue(data.Value.Float64);
                    break;
                
                default:
                    break;
                }
                break;
            }

            case data_type_def_xsd_e::BYTE:
            {
                const data_type_def_xsd_e _xsd = data_type_def_xsd_e::BYTE;
                Property<_xsd>* property = static_cast<Property<_xsd>*>(dataElement);
                property->SetValue(data.Value.Int8);
                // LOG_DEBUG(logger, "idShort: %s ---> value: %d", property->GetIdShortOrNull(), data.Value.Int16);
                break;
            }

            case data_type_def_xsd_e::SHORT:
            {
                const data_type_def_xsd_e _xsd = data_type_def_xsd_e::SHORT;
                Property<_xsd>* property = static_cast<Property<_xsd>*>(dataElement);
                // LOG_DEBUG(logger, "idShort: %s ---> value: %d", property->GetIdShortOrNull(), data.Value.Int16);
                property->SetValue(data.Value.Int16);
                break;
            }

            case data_type_def_xsd_e::INT:
            {
                const data_type_def_xsd_e _xsd = data_type_def_xsd_e::INT;
                Property<_xsd>* property = static_cast<Property<_xsd>*>(dataElement);
                // LOG_DEBUG(logger, "idShort: %s ---> value: %d", property->GetIdShortOrNull(), data.Value.Int32);
                property->SetValue(data.Value.Int32);
                break;
            }

            case data_type_def_xsd_e::UNSIGNED_BYTE:
            {
                const data_type_def_xsd_e _xsd = data_type_def_xsd_e::UNSIGNED_BYTE;
                Property<_xsd>* property = static_cast<Property<_xsd>*>(dataElement);
                property->SetValue(data.Value.UInt8);
                // LOG_DEBUG(logger, "idShort: %s ---> value: %u", property->GetIdShortOrNull(), data.Value.UInt8);
                break;
            }

            case data_type_def_xsd_e::UNSIGNED_SHORT:
            {
                const data_type_def_xsd_e _xsd = data_type_def_xsd_e::UNSIGNED_SHORT;
                Property<_xsd>* property = static_cast<Property<_xsd>*>(dataElement);
                property->SetValue(data.Value.UInt16);
                // LOG_DEBUG(logger, "idShort: %s ---> value: %u", property->GetIdShortOrNull(), data.Value.UInt16);
                break;
            }

            case data_type_def_xsd_e::UNSIGNED_INT:
            {
                const data_type_def_xsd_e _xsd = data_type_def_xsd_e::UNSIGNED_INT;
                Property<_xsd>* property = static_cast<Property<_xsd>*>(dataElement);
                // LOG_DEBUG(logger, "idShort: %s ---> value: %u", property->GetIdShortOrNull(), data.Value.UInt32);
                property->SetValue(data.Value.UInt32);
                break;
            }

            case data_type_def_xsd_e::UNSIGNED_LONG:
            case data_type_def_xsd_e::POSITIVE_INTEGER:
            case data_type_def_xsd_e::NON_NEGATIVE_INTEGER:
            {
                const data_type_def_xsd_e _xsd = data_type_def_xsd_e::UNSIGNED_LONG;
                Property<_xsd>* property = static_cast<Property<_xsd>*>(dataElement);
                property->SetValue(data.Value.UInt64);
                // LOG_DEBUG(logger, "idShort: %s ---> value: %llu", property->GetIdShortOrNull(), data.Value.UInt64);
                break;
            }
            
            default:
                // LOG_DEBUG(logger, "idShort: %s", dataElement->GetIdShortOrNull());
                break;
            }
        }

        static void handleRealTimeMonitoring(const psram::string& idShort, const im::Node& node, SubmodelElementCollection& smc)
        {   
            for (auto& element : smc)
            {
                DataElement* dataElement = static_cast<DataElement*>(element.get());
                if (idShort != dataElement->GetIdShortOrNull())
                {
                    continue;
                }

                const key_types_e type = dataElement->GetModelType();
                if (type != key_types_e::PROPERTY)
                {
                    continue;
                }
                
                im::var_data_t data = node.VariableNode.RetrieveData();
                const data_type_def_xsd_e xsd = dataElement->GetValueType();
                updateProperty(xsd, data, dataElement);
            }

            if (uint32_t(millis() - sRealtimeMonitorTimer) < CYCLICAL_TIMER)
            {
                return;
            }
            sRealtimeMonitorTimer = millis();

            SubmodelsSerializer submodelSerializer;
            psram::string json = submodelSerializer.EncodeProperty(SM_ID_OPERATIONAL_DATA, "RealTimeMonitoring");
        
            mqtt::Message msg(mqtt::topic_e::AAS_OPERATIONALDATA_RTM, json.c_str());
            mqtt::cdo.Store(msg);
        }

        static void handleJobProgress(const psram::string& idShort, const im::Node& node, SubmodelElementCollection& smc)
        {   
            for (auto& element : smc)
            {
                DataElement* dataElement = static_cast<DataElement*>(element.get());
                if (idShort != dataElement->GetIdShortOrNull())
                {
                    continue;
                }

                const key_types_e type = dataElement->GetModelType();
                if (type != key_types_e::PROPERTY)
                {
                    continue;
                }
                
                im::var_data_t data = node.VariableNode.RetrieveData();
                const data_type_def_xsd_e xsd = dataElement->GetValueType();
                updateProperty(xsd, data, dataElement);
            }

            if (uint32_t(millis() - sJobProgressTimer) < CYCLICAL_TIMER)
            {
                return;
            }
            sJobProgressTimer = millis();

            SubmodelsSerializer submodelSerializer;
            psram::string json = submodelSerializer.EncodeProperty(SM_ID_OPERATIONAL_DATA, "JobProgress");
        
            mqtt::Message msg(mqtt::topic_e::AAS_OPERATIONALDATA_JP, json.c_str());
            mqtt::cdo.Store(msg);
        }

        static void handleConfiguration(const psram::string& idShort, const im::Node& node, SubmodelElementCollection& smc)
        {
            for (auto& element : smc)
            {
                DataElement* dataElement = static_cast<DataElement*>(element.get());
                if (idShort != dataElement->GetIdShortOrNull())
                {
                    continue;
                }

                const key_types_e type = dataElement->GetModelType();
                if (type != key_types_e::PROPERTY)
                {
                    continue;
                }
                
                im::var_data_t data = node.VariableNode.RetrieveData();
                const data_type_def_xsd_e xsd = dataElement->GetValueType();
                updateProperty(xsd, data, dataElement);
            }

            if (uint32_t(millis() - sConfigurationTimer) < CYCLICAL_TIMER)
            {
                return;
            }
            sConfigurationTimer = millis();

            SubmodelsSerializer submodelSerializer;
            psram::string json = submodelSerializer.EncodeProperty(SM_ID_CONFIGURATION, "BasicConfiguration");
        
            mqtt::Message msg(mqtt::topic_e::AAS_CONFIGURATION, json.c_str());
            mqtt::cdo.Store(msg);
        }

    public:
        void AddEntry(const char* nodeId, const char* idShort)
        {
            mMap.emplace(nodeId, idShort);
            LOG_DEBUG(logger, "%s: %s", nodeId, idShort);
        }

        static void implTask(void* pvParams)
        {
            Container* container = Container::GetInstance();
            const Submodel* smOperationalData = container->GetSubmodelByID(SM_ID_OPERATIONAL_DATA);
            const Submodel* smConfiguration = container->GetSubmodelByID(SM_ID_CONFIGURATION);

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

                    for (auto& sme : *smOperationalData)
                    {
                        if (sme->GetModelType() != key_types_e::SubmodelElementCollection)
                        {
                            continue;
                        }
                        
                        SubmodelElementCollection* smc = static_cast<SubmodelElementCollection*>(sme.get());
                        if (strcmp("RealTimeMonitoring", smc->GetIdShortOrNull()) == 0)
                        {
                            handleRealTimeMonitoring(pair.second, *result.second, *smc);
                        }
                        else if (strcmp("JobProgress", smc->GetIdShortOrNull()) == 0)
                        {
                            handleJobProgress(pair.second, *result.second, *smc);
                        }
                    }

                    for (auto& sme : *smConfiguration)
                    {
                        if (sme->GetModelType() != key_types_e::SubmodelElementCollection)
                        {
                            continue;
                        }
                        
                        SubmodelElementCollection* smc = static_cast<SubmodelElementCollection*>(sme.get());
                        if (strcmp("BasicConfiguration", smc->GetIdShortOrNull()) == 0)
                        {
                            handleConfiguration(pair.second, *result.second, *smc);
                        }
                    }
                }

                vTaskDelay(pdMS_TO_TICKS(1000));
            }
        }

        void Start()
        {
            LOG_INFO(logger, "Starting AAS Client Task");
            vTaskDelay(pdMS_TO_TICKS(10000));

            xTaskCreatePinnedToCore(
                implTask,
                "AasClientTask",
                10240,
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