#include "CyclicalPubTask.h"
#include "ModbusTask.h"
#include "Protocol/Modbus/ModbusRTU.h"
#include "Protocol/MQTT/CDO.h"
#include "IM/AC/Alarm/DeprecableAlarm.h"
#include "IM/EA/DeprecableOperationTime.h"
#include "IM/EA/DeprecableProductionInfo.h"


namespace muffin {

    void StopAllTask()
    {
        mqtt::CDO& cdo = mqtt::CDO::GetInstance();
        while (cdo.Count() > 0)
        {
            delay(100);
        }
    
        StopCyclicalsMSGTask();
        StopModbusTcpTask();
        StopModbusRtuTask();
    
        AlarmMonitor& alarmMonitor = AlarmMonitor::GetInstance();
        alarmMonitor.StopTask();
        alarmMonitor.Clear();
        
        ProductionInfo& productionInfo = ProductionInfo::GetInstance();
        productionInfo.StopTask();
        productionInfo.Clear();
        
        OperationTime& operationTime = OperationTime::GetInstance();
        operationTime.StopTask();
        operationTime.Clear();

        im::NodeStore* nodeStore = im::NodeStore::CreateInstanceOrNULL();
        nodeStore->Clear();

        ModbusRtuVector.clear();
        ModbusTcpVector.clear();
    }
}