/**
 * @file DataElement.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief
 * A data element is a submodel element that is not further composed out of other submodel elements.
 * A data element is a submodel element that has a value. The type of value differs for different 
 * subtypes of data elements.
 * 
 * @note
 * For data elements category (inherited by Referable) shall be one of the following values: 
 * CONSTANT, PARAMETER or VARIABLE. Default: VARIABLE
 * [Constraint AASd-090]
 * 
 * @date 2025-08-14
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include "./SubmodelElement.hpp"



namespace muffin { namespace aas {


    class DataElement : public SubmodelElement
    {
    protected:
        DataElement(const DataElement& other) = default;

    public:
        DataElement() = default;
        ~DataElement() override = default;
    };
}}