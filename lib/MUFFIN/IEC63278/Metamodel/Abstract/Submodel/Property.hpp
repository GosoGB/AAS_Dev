/**
 * @file Property.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief
 * A property is a data element that has a single value.
 * 
 * @note
 * If both, the Property/value and the Property/valueId are present then the value of Property/
 * value needs to be identical to the value of the referenced coded value in Property/valueId.
 * [Constraint AASd-007]
 * 
 * @date 2025-08-11
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include "../../TypeDefinitions.hpp"
#include "../../Utility/XsdTypeMapper.hpp"
#include "./DataElement.hpp"
#include <esp32-hal-log.h>


namespace muffin { namespace aas {


    template<data_type_def_xsd_e xsd>
    class Property : public DataElement
    {
    public:
        Property() : mValueType(xsd) {}
        ~Property() noexcept = default;

    public:
        void SetValue(const typename xsd_type_mapper<xsd>::type& value)
        {
            if (likely(mValue))
            {
                *mValue = value;
            }
            else
            {
                mValue = psram::make_unique<typename xsd_type_mapper<xsd>::type>(value);
            }
        }

        void SetValueID(const Reference& valueId)
        {
            mValueID = psram::make_unique<Reference>(valueId);
        }

    public:
        data_type_def_xsd_e GetValueType() const noexcept
        {
            return mValueType;
        }
        
        typename xsd_type_mapper<xsd>::type* GetValue() const noexcept
        {
            return mValue.get();
        }

        const Reference* GetValueID() const noexcept
        {
            return mValueID.get();
        }

    private:
        data_type_def_xsd_e mValueType;
        psram::unique_ptr<typename xsd_type_mapper<xsd>::type> mValue;
        psram::unique_ptr<Reference> mValueID;
    };
}}