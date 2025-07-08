#ifndef EIP_TYPES_H
#define EIP_TYPES_H

#include <Network/Ethernet/W5500/EthernetClient.h>
#include <IPAddress.h>

struct EIPSession 
{
    muffin::w5500::EthernetClient* client = nullptr;
    uint32_t sessionHandle = 0;
    uint32_t connectionID = 0;
    IPAddress targetIP;
    uint16_t targetPort = 44818;
    bool connected = false;
};

#endif
