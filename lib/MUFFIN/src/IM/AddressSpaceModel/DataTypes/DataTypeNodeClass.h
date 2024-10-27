/**
 * @file DataTypeNodeClass.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief OPC UA 표준의 DataType Node Class를 선언합니다.
 * 
 * @date 2024-10-27
 * @version 0.0.1
 * 
 * @ref https://reference.opcfoundation.org/Core/Part3/v104/docs/5.8.3#Table16
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024
 */




#pragma once

#include "BaseNodeClass.h"



namespace muffin { namespace im {

    class DataTypeNodeClass : public BaseNodeClass
    {
    public:
        DataTypeNodeClass();
        ~DataTypeNodeClass();
    private:
        const bool mIsAbstract;
        // DataTypeDefinition
    };
}}