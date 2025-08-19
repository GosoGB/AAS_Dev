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
    public:
        DataElement() = default;
        DataElement(const DataElement& other) = default;
        DataElement(DataElement&& other) = default;
        ~DataElement() override = default;
    
    public:
        key_types_e GetModelType() const noexcept override
        {
            return key_types_e::DATA_ELEMENT;
        }

        virtual data_type_def_xsd_e GetValueType() const noexcept
        {
            return mValueType;
        }
    protected:
        data_type_def_xsd_e mValueType = data_type_def_xsd_e::ANY_URI;
    };
}}