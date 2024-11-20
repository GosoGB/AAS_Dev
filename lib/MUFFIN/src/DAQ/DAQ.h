/**
 * @file DAQ.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief DAQ 클래스를 선언합니다.
 * 
 * @date 2024-09-26
 * @version 1.0.0
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include <map>
#include <string>

#include "IM/Node/Node.h"



namespace muffin {

    class DAQ
    {
    public:
        DAQ();
        ~DAQ();
    public:
        void AddProtocol();
        void RemoveProtocol();
    public:
        void AddNodeReference(im::Node& nodeReference);
        void RemoveNodeReference(im::Node& nodeReference);
    private:
        std::map<std::string, im::Node> mMapNode;
    };
}