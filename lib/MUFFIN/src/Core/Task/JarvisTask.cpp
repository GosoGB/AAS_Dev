/**
 * @file JarvisTask.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 수신한 JARIVS 설정 정보를 검증하여 유효하다면 적용하는 태스크를 정의합니다.
 * 
 * @date 2025-05-28
 * @version 1.4.0
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
#include "Core/Task/MelsecTask.h"
#include "Core/Task/PubTask.h"
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
#include "Protocol/Melsec/Melsec.h"
#include "Protocol/MQTT/CIA.h"
#include "Protocol/MQTT/CatMQTT/CatMQTT.h"
#include "Protocol/MQTT/LwipMQTT/LwipMQTT.h"
#include "Protocol/MQTT/Include/Helper.h"
#include "Protocol/SPEAR/SPEAR.h"

#include "Storage/ESP32FS/ESP32FS.h"
#include "Network/Ethernet/EthernetFactory.h"
#include "Network/Ethernet/W5500/W5500.h"
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
                break;
            }
        }

#if defined(MT10) || defined(MT11) || defined(MB10) 
        for (auto& pair : *jarvis)
        {
            const jvs::cfg_key_e key = pair.first;
            if (key == jvs::cfg_key_e::ETHERNET)
            {
                InitEthernetService();
                break;
            }
        }
#endif

#if defined(MT11)
        applyEthernet();
#endif

    #if defined(MT10) || defined(MT11) || defined(MB10) 
        for (auto& pair : *jarvis)
        {
            const jvs::cfg_key_e key = pair.first;
            if (key == jvs::cfg_key_e::MELSEC)
            {
                applyMelsecCIN(pair.second);
                for (auto it = pair.second.begin(); it != pair.second.end(); ++it)
                {
                    delete *it;
                }
                break;
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
            case jvs::cfg_key_e::MELSEC:
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
    #if defined(MODLINK_L) || defined(ML10) 
        ASSERT((vectorRS485CIN.size() == 1), "THERE MUST BE ONLY ONE RS-485 CIN FOR MODLINK-L AND ML10");
        jvs::config::Rs485* cin = Convert.ToRS485CIN(vectorRS485CIN[0]);
        if (cin->GetPortIndex().second == jvs::prt_e::PORT_2)
        {
            RS485_LINK1 = new(std::nothrow) RS485Class(Serial2, 17, -1, -1);
            if (RS485_LINK1 == nullptr)
            {
                LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR RS485 INTERFACE");
            }
        }

    #elif defined(MT10) || defined(MB10)
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
    #elif defined(MT11)
        {
            jvs::config::Rs485* cin = Convert.ToRS485CIN(vectorRS485CIN[0]);
            if (cin->GetPortIndex().second == jvs::prt_e::PORT_3)
            {   
                uint8_t TX_PIN  = 38;
                
                RS485_LINK2 = new(std::nothrow) RS485Class(Serial2, TX_PIN, -1, -1);
                if (RS485_LINK2 == nullptr)
                {
                    LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR RS485 INTERFACE");
                }
            }
            else if (cin->GetPortIndex().second == jvs::prt_e::PORT_2)
            {   
                uint8_t TX_PIN  = 17;
              
                /**
                 * @todo 현재 CATM1 시리얼 포트와 HardwareSerial UART 핀이 곂치기 때문에 반드시 테스트 후 처리 해야함 @김주성 @이상진
                 * 
                 */
                RS485_LINK1 = new(std::nothrow) RS485Class(Serial2, TX_PIN, -1, -1);
                if (RS485_LINK1 == nullptr)
                {
                    LOG_ERROR(logger, "FAILED TO ALLOCATE MEMORY FOR RS485 INTERFACE");
                }
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

    #if defined(MODLINK_L) || defined(ML10) || defined(MT11)
        ModbusRtuVector.clear();
        mConfigVectorMbRTU.clear();
        
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
            mConfigVectorMbRTU.emplace_back(*cin);
            ModbusRtuVector.emplace_back(*modbusRTU);
        }

        LOG_INFO(logger, "Configured Modbus RTU protocol, mConfigVectorMbRTU size : %d",mConfigVectorMbRTU.size());

        StartModbusRtuTask();
        StartTaskMSG();
    #else
        ModbusRtuVector.clear();
        mConfigVectorMbRTU.clear();
        
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
            mConfigVectorMbRTU.emplace_back(*cin);
            ModbusRtuVector.emplace_back(*modbusRTU);
        }

        LOG_INFO(logger, "Configured Modbus RTU protocol, mConfigVectorMbRTU size : %d",mConfigVectorMbRTU.size());
        
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
        StartTaskMSG();
    #endif
        
    }

    void applyModbusTcpCIN(std::vector<jvs::config::Base*>& vectorModbusTCPCIN)
    {
    #if defined(MODLINK_L) || defined(ML10)
        ASSERT((true), "ETHERNET MUST BE CONFIGURE FOR MODLINK-B AND MT10");
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
        mConfigVectorMbTCP.clear();
        ModbusTcpVector.clear();
        
        for (auto& modbusTCPCIN : vectorModbusTCPCIN)
        {
            jvs::config::ModbusTCP* cin = static_cast<jvs::config::ModbusTCP*>(modbusTCPCIN);
        
    #if defined(MT11)
            const jvs::if_e eth = cin->GetEthernetInterface().second;
            switch (eth)
            {
            case jvs::if_e::EMBEDDED:
            {
                const auto retSocketID = ethernet->GetAvailableSocketId();
                if (retSocketID.first.ToCode() != Status::Code::GOOD)
                {
                    LOG_ERROR(logger,"[ETH0] NO AVAILABLE SOCKET");
                    continue;
                }
                LOG_DEBUG(logger,"SOCKET ID : %d",static_cast<uint8_t>(retSocketID.second));
                if (w5500::embededEthernetClient == nullptr)
                {
                    w5500::embededEthernetClient = new w5500::EthernetClient(*ethernet, w5500::sock_id_e::SOCKET_0);
                    embededModbusTCPClient = new ModbusTCPClient(*w5500::embededEthernetClient);
                }
                applyModbusTcpConfig(ethernet, retSocketID.second, cin);
                break;
            }
            case jvs::if_e::LINK_01:
            {
                const auto retSocketID = link1W5500->GetAvailableSocketId();
                if (retSocketID.first.ToCode() != Status::Code::GOOD)
                {
                    LOG_ERROR(logger,"[ETH1] NO AVAILABLE SOCKET");
                    continue;
                }
                if (w5500::link1EthernetClient == nullptr)
                {
                    w5500::link1EthernetClient = new w5500::EthernetClient(*link1W5500, w5500::sock_id_e::SOCKET_0);
                    link1ModbusTCPClient = new ModbusTCPClient(*w5500::link1EthernetClient);
                }
                applyModbusTcpConfig(link1W5500, retSocketID.second, cin);
                break;
            }
            default:
                break;
            }
    #else
            ModbusTCP* modbusTCP = new ModbusTCP();
            Status ret = modbusTCP->Config(cin);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO CONFIGURE MODBUS TCP");
                return;
            }
            mConfigVectorMbTCP.emplace_back(*cin);
            ModbusTcpVector.emplace_back(*modbusTCP);
    #endif
        }

        StartModbusTcpTask();
        StartTaskMSG();
    }

    void applyMelsecCIN(std::vector<jvs::config::Base*>& vectorMelsecCIN)
    {
    #if defined(MODLINK_L) || defined(ML10)
        ASSERT((true), "ETHERNET MUST BE CONFIGURE FOR MODLINK-B AND MT10");
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

        mConfigVectorMelsec.clear();
        MelsecVector.clear();

        for (auto& melsecCIN : vectorMelsecCIN)
        {
            jvs::config::Melsec* cin = static_cast<jvs::config::Melsec*>(melsecCIN);

    #if defined(MT11)
            const jvs::if_e eth = cin->GetEthernetInterface().second;
            switch (eth)
            {
            case jvs::if_e::EMBEDDED:
            {
               /**
                 * @todo Melsec은 1번 소캣을 고정으로 사용하도록 임시로 두었음 @김주성
                 * 
                 */
                if (embededMelsecClient == nullptr)
                {
                    embededMelsecClient = new MelsecClient(*ethernet, w5500::sock_id_e::SOCKET_1);
                }
                
                applyMelsecConfig(embededMelsecClient, cin);
                break;
            }
            case jvs::if_e::LINK_01:
            {
                if (link1MelsecClient == nullptr)
                {
                    link1MelsecClient = new MelsecClient(*link1W5500, w5500::sock_id_e::SOCKET_1);
                }

                applyMelsecConfig(link1MelsecClient, cin);
                break;
            }
            default:
                break;
            }
    #else
            Melsec* melsec = new Melsec();
            Status ret = melsec->Config(cin);
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "FAILED TO CONFIGURE MELSEC");
                return;
            }
            mConfigVectorMelsec.emplace_back(*cin);
            MelsecVector.emplace_back(*melsec);
    #endif
        }

        StartMelsecTask();
        StartTaskMSG();

    }

#if defined(MT11)
    void applyEthernet()
    {
        if (jvs::config::embeddedEthernet != nullptr)
        {
            ethernet->Config(jvs::config::embeddedEthernet);
            ethernet->Connect();   
        }

        if (jvs::config::link1Ethernet != nullptr)
        {
            link1W5500 = new(std::nothrow) W5500(w5500::if_e::LINK_01);
            Status ret = link1W5500->Init();
            if (ret != Status::Code::GOOD)
            {
                LOG_ERROR(logger, "LINK1 W5500 INIT ERROR: %s",ret.c_str());
            }

            link1W5500->Config(jvs::config::link1Ethernet);
            link1W5500->Connect();
            
        }
    }

    void applyModbusTcpConfig(W5500* eth, w5500::sock_id_e id, jvs::config::ModbusTCP* cin)
    {
        ModbusTCP* modbusTCP = new ModbusTCP(*eth, id);
        Status ret = modbusTCP->Config(cin);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CONFIGURE MODBUS TCP");
            return;
        }
        mConfigVectorMbTCP.emplace_back(*cin);

        if (id == w5500::sock_id_e::SOCKET_0)
        {
            switch (cin->GetEthernetInterface().second)
            {
            case jvs::if_e::EMBEDDED:
                modbusTCP->SetModbusTCPClient(embededModbusTCPClient, w5500::embededEthernetClient);
                break;
            case jvs::if_e::LINK_01:
                modbusTCP->SetModbusTCPClient(link1ModbusTCPClient, w5500::link1EthernetClient);
                break;
            default:
                break;
            }

            ModbusTcpVectorDynamic.emplace_back(*modbusTCP);
        }
        else
        {
            ModbusTcpVector.emplace_back(*modbusTCP);
        }
    }

    void applyMelsecConfig(MelsecClient* client, jvs::config::Melsec* cin)
    {
        Melsec* melsec = new Melsec();
        melsec->SetClient(client);

        Status ret = melsec->Config(cin);
        if (ret != Status::Code::GOOD)
        {
            LOG_ERROR(logger, "FAILED TO CONFIGURE MELSEC");
            return;
        }
        mConfigVectorMelsec.emplace_back(*cin);
        MelsecVector.emplace_back(*melsec);
        
    }
#endif
}