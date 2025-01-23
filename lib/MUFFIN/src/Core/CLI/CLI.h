/**
 * @file CLI.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * @brief 
 * @version 1.2.2
 * @date 2025-01-23
 * 
 * @copyright Copyright (c) 2025
 * 
 */



#pragma once

#include "Common/Status.h"
#include "DataFormat/JSON/JSON.h"

namespace muffin {

    class CommandLineInterface
    {
    public:
        CommandLineInterface() {}
        virtual ~CommandLineInterface() {}
    public:
        Status Init();
    private:
        Status configureNetworkInterface();
        Status configureLTE();
        Status configureEthernet();
        bool isValidIpFormat(const std::string &ip, const bool& isSubnetmask = false);
    private:
        std::string getSerialInput();
        JsonDocument mJarvisJson;
        Status saveJarvisJson();

    };
}