/**
 * @file JarvisTask.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 수신한 JARIVS 설정 정보를 검증하여 유효하다면 적용하는 태스크를 정의합니다.
 * 
 * @date 2025-01-23
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#include "Preferences.h"
#include "Common/Assert.h"
#include "Common/Status.h"
#include "Common/Time/TimeUtils.h"
#include "Common/Convert/ConvertClass.h"
#include "Core/Core.h"
#include "Core/Task/ModbusTask.h"
#include "Core/Task/CyclicalPubTask.h"
#include "DataFormat/JSON/JSON.h"

#include "IM/Custom/Constants.h"
#include "IM/Custom/MacAddress/MacAddress.h"
#include "IM/Node/NodeStore.h"
#include "IM/AC/Alarm/DeprecableAlarm.h"
#include "IM/EA/DeprecableProductionInfo.h"
#include "IM/EA/DeprecableOperationTime.h"

#include "JARVIS/JARVIS.h"
#include "JARVIS/Config/Operation/Operation.h"
#include "JarvisTask.h"
#include "Protocol/MQTT/CDO.h"
#include "Protocol/HTTP/CatHTTP/CatHTTP.h"
#include "Protocol/HTTP/Include/TypeDefinitions.h"
#include "Protocol/Modbus/Include/TypeDefinitions.h"
#include "Protocol/Modbus/Include/ArduinoRS485/src/ArduinoRS485.h"
#include "Protocol/Modbus/ModbusRTU.h"
#include "Protocol/Modbus/ModbusTCP.h"
#include "Protocol/MQTT/CIA.h"
#include "Protocol/MQTT/CatMQTT/CatMQTT.h"
#include "Protocol/MQTT/LwipMQTT/LwipMQTT.h"
#include "Protocol/MQTT/Include/Helper.h"
#include "Protocol/SPEAR/SPEAR.h"

#include "Storage/ESP32FS/ESP32FS.h"
#include "Network/Ethernet/Ethernet.h"
#include "Storage/CatFS/CatFS.h"
#include "Protocol/HTTP/LwipHTTP/LwipHTTP.h"

#include "ServiceSets/NetworkServiceSet/InitializeNetworkService.h"

muffin::mqtt::IMQTT* mqttClient = nullptr;
muffin::http::IHTTP* httpClient = nullptr;



namespace muffin {

    static bool s_HasNode = false;


    /**
     * @todo 상태 코드를 반환하도록 코드를 수정해야 합니다.
     */
    void ApplyJarvisTask()
    {
        for (auto& pair : *jarvis)
        {
            const jvs::cfg_key_e key = pair.first;
            if (key == jvs::cfg_key_e::NODE)
            {
                applyNodeCIN(pair.second);
                s_HasNode = true;

                for (auto it = pair.second.begin(); it != pair.second.end(); ++it)
                {
                    delete *it;
                }
                break;
            }
        }
    #if defined(MODLINK_T2) || defined(MODLINK_B)
        for (auto& pair : *jarvis)
        {
            const jvs::cfg_key_e key = pair.first;
            if (key == jvs::cfg_key_e::ETHERNET)
            {
                InitEthernetService();
            }
        }
    #endif

        for (auto& pair : *jarvis)
        {
            const jvs::cfg_key_e key = pair.first;

            switch (key)
            {
            case jvs::cfg_key_e::ALARM:
                applyAlarmCIN(pair.second);
                for (auto it = pair.second.begin(); it != pair.second.end(); ++it)
                {
                    delete *it;
                }
                break;
            case jvs::cfg_key_e::OPERATION_TIME:
                applyOperationTimeCIN(pair.second);
                for (auto it = pair.second.begin(); it != pair.second.end(); ++it)
                {
                    delete *it;
                }
                break;
            case jvs::cfg_key_e::RS485:
                applyRS485CIN(pair.second);
                break;
            case jvs::cfg_key_e::PRODUCTION_INFO:
                applyProductionInfoCIN(pair.second);
                for (auto it = pair.second.begin(); it != pair.second.end(); ++it)
                {
                    delete *it;
                }
                break;
            case jvs::cfg_key_e::MODBUS_RTU:
                {
                    for (auto cin : *jarvis)
                    {
                        if (cin.first == jvs::cfg_key_e::RS485)
                        {
                            jvs::config::Rs485* rs485CIN = static_cast<jvs::config::Rs485*>(cin.second[0]);
                            applyModbusRtuCIN(pair.second, rs485CIN);
                            for (auto it = pair.second.begin(); it != pair.second.end(); ++it)
                            {
                                delete *it;
                            }
                            break;
                        }
                    }
                }
                break;
            case jvs::cfg_key_e::MODBUS_TCP:
                applyModbusTcpCIN(pair.second);
                for (auto it = pair.second.begin(); it != pair.second.end(); ++it)
                {
                    delete *it;
                }
                break;
            case jvs::cfg_key_e::NODE:
            case jvs::cfg_key_e::LTE_CatM1:
            case jvs::cfg_key_e::OPERATION:
            case jvs::cfg_key_e::RS232:
            case jvs::cfg_key_e::WIFI4:
            case jvs::cfg_key_e::ETHERNET:
                break;
            
            default:
                ASSERT(false, "UNIMPLEMENTED CONFIGURATION SERVICES");
                break;
            }
        }
    }

    void applyAlarmCIN(std::vector<jvs::config::Base*>& vectorAlarmCIN)
    {
        AlarmMonitor& alarmMonitor = AlarmMonitor::GetInstance();
        for (auto cin : vectorAlarmCIN)
        {
            alarmMonitor.Add(static_cast<jvs::config::Alarm*>(cin));
        }
        alarmMonitor.StartTask();
    }
    
    void applyNodeCIN(std::vector<jvs::config::Base*>& vectorNodeCIN)
    {
        im::NodeStore* nodeStore = im::NodeStore::CreateInstanceOrNULL();
        for (auto& baseCIN : vectorNodeCIN)
        {
            jvs::config::Node* nodeCIN = static_cast<jvs::config::Node*>(baseCIN);
            nodeStore->Create(nodeCIN);
        }
    }
    
    void applyOperationTimeCIN(std::vector<jvs::config::Base*>& vectorOperationTimeCIN)
    {
        OperationTime& operationTime = OperationTime::GetInstance();
        for (auto cin : vectorOperationTimeCIN)
        {
            operationTime.Config(static_cast<jvs::config::OperationTime*>(cin));
        }
        operationTime.StartTask();
    }
    
    void applyProductionInfoCIN(std::vector<jvs::config::Base*>& vectorProductionInfoCIN)
    {
        ProductionInfo& productionInfo = ProductionInfo::GetInstance();
        for (auto cin : vectorProductionInfoCIN)
        {
            productionInfo.Config(static_cast<jvs::config::Production*>(cin));
        }
        productionInfo.StartTask();
    }

    void applyRS485CIN(std::vector<jvs::config::Base*>& vectorRS485CIN)
    {
    #if defined(MODLINK_L) || defined(MODLINK_ML10)
        ASSERT((vectorRS485CIN.size() == 1), "THERE MUST BE ONLY ONE RS-485 CIN FOR MODLINK-L AND MODLINK-ML10");
        jvs::config::Rs485* cin = Convert.ToRS485CIN(vectorRS485CIN[0]);
        if (cin->GetPortIndex().second == jvs::prt_e::PORT_2)
        {
            RS485 = new(std::nothrow) RS485Class(Serial2, 17, -1, -1);
            if (RS485 == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR RS485 INTERFACE");
            }
        }

    #elif defined(MODLINK_T2) || defined(MODLINK_B)
        for (auto& Rs485CIN : vectorRS485CIN)
        {
            size_t count = 0;
            while (count < 5)
            {
                Status ret = spear.SetJarvisLinkConfig(Rs485CIN, jvs::cfg_key_e::RS485);
                if (ret == Status(Status::Code::GOOD))
                {
                    break;
                count++;
                }
                delay(100);
            }            
        }

    #endif
    }

    void applyModbusRtuCIN(std::vector<jvs::config::Base*>& vectorModbusRTUCIN, jvs::config::Rs485* rs485CIN)
    {
        if (s_HasNode == false)
        {
            LOG_WARNING(logger, "NO NODE");
            return;
        }
        
        if (jvs::config::operation.GetPlanExpired().second == true)
        {
            LOG_WARNING(logger, "EXPIRED SERVICE PLAN: MODBUS TASK IS NOT GOING TO START");
            return;
        }

    #if defined(MODLINK_L) || defined(MODLINK_ML10)
        ModbusRtuVector.clear();
        mVectorModbusRTU.clear();
        
        for (auto& modbusRTUCIN : vectorModbusRTUCIN)
        {
            ModbusRTU* modbusRTU = new ModbusRTU();
            modbusRTU->SetPort(rs485CIN);
            jvs::config::ModbusRTU* cin = static_cast<jvs::config::ModbusRTU*>(modbusRTUCIN);
            modbusRTU->mPort = cin->GetPort().second;
            Status ret = modbusRTU->Config(cin);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO CONFIGURE MODBUS RTU");
                return;
            }
            mVectorModbusRTU.emplace_back(*cin);
            ModbusRtuVector.emplace_back(*modbusRTU);
        }

        LOG_INFO(logger, "Configured Modbus RTU protocol, mVectorModbusRTU size : %d",mVectorModbusRTU.size());

        SetPollingInterval(jvs::config::operation.GetIntervalPolling().second);
        StartModbusRtuTask();
        StartTaskCyclicalsMSG(jvs::config::operation.GetIntervalServer().second);
    #else
        ModbusRtuVector.clear();
        mVectorModbusRTU.clear();
        
        std::set<jvs::prt_e> link;
        for (auto& modbusRTUCIN : vectorModbusRTUCIN)
        {
            ModbusRTU* modbusRTU = new ModbusRTU();
            jvs::config::ModbusRTU* cin = static_cast<jvs::config::ModbusRTU*>(modbusRTUCIN);
            modbusRTU->mPort = cin->GetPort().second;
            link.insert(modbusRTU->mPort);
            Status ret = modbusRTU->Config(cin);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO CONFIGURE MODBUS RTU");
                return;
            }
            mVectorModbusRTU.emplace_back(*cin);
            ModbusRtuVector.emplace_back(*modbusRTU);
        }

        LOG_INFO(logger, "Configured Modbus RTU protocol, mVectorModbusRTU size : %d",mVectorModbusRTU.size());
        
        size_t count = 0;
        while (count < 5)
        {
            Status ret = spear.SetJarvisProtocolConfig(link);
            if (ret == Status(Status::Code::GOOD))
            {
                break;
            }
            count++;
            delay(100);
        }

        StartModbusRtuTask();
        StartTaskCyclicalsMSG(jvs::config::operation.GetIntervalServer().second);
    #endif
        
    }

    void applyModbusTcpCIN(std::vector<jvs::config::Base*>& vectorModbusTCPCIN)
    {
    #if defined(MODLINK_L) || defined(MODLINK_ML10)
        ASSERT((true), "ETHERNET MUST BE CONFIGURE FOR MODLINK-B AND MODLINK-T2");
    #endif

        if (s_HasNode == false)
        {
            LOG_WARNING(logger, "NO NODE");
            return;
        }

        if (jvs::config::operation.GetPlanExpired().second == true)
        {
            LOG_WARNING(logger, "EXPIRED SERVICE PLAN: MODBUS TASK IS NOT GOING TO START");
            return;
        }
        ModbusTcpVector.clear();
        mVectorModbusTCP.clear();
        
        for (auto& modbusTCPCIN : vectorModbusTCPCIN)
        {
            ModbusTCP* modbusTCP = new ModbusTCP();
            jvs::config::ModbusTCP* cin = static_cast<jvs::config::ModbusTCP*>(modbusTCPCIN);
            Status ret = modbusTCP->Config(cin);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO CONFIGURE MODBUS TCP");
                return;
            }
            mVectorModbusTCP.emplace_back(*cin);
            ModbusTcpVector.emplace_back(*modbusTCP);
        }

        SetPollingInterval(jvs::config::operation.GetIntervalPolling().second);
        StartModbusTcpTask();
        StartTaskCyclicalsMSG(jvs::config::operation.GetIntervalServer().second);
    }
}