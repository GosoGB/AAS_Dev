/**
 * @file Qualifier.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief
 * A qualifier is a type-value-pair that makes additional statements w.r.t. the value of the 
 * element.
 * 
 * @note
 * If both, the value and the valueId of a Qualifier are present then the value needs to be 
 * identical to the value of the referenced coded value in Qualifier/valueId.
 * [Constraint AASd-006]
 * 
 * @note
 * The value of Qualifier/value shall be consistent to the data type as defined in Qualifier/
 * valueType.
 * [Constraint AASd-020]
 * 
 * @date 2025-08-01
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include "./HasSemantics.hpp"
#include "../Reference.hpp"
#include "../TypeDefinitions.hpp"

#include "Common/PSRAM.hpp"



namespace muffin { namespace aas {


    using QualifierType = std::string;


    template<data_type_def_xsd_e xsd>
    class Qualifier : public HasSemantics
    {
    public:
        Qualifier(const QualifierType& type, const data_type_def_xsd_e valueType)
            : mType(type)
            , mValueType(valueType)
        {
            mValueType = get_xsd_type_from_cpp<T>();
        }

        virtual ~Qualifier() noexcept = default;

    protected:
        psram::unique_ptr<qualifier_kind_e> mKind;
        QualifierType mType;
        data_type_def_xsd_e mValueType;
        typename xsd_type_mapper<xsd>::type mValue;
        psram::unique_ptr<Reference> mValueID;
    };
}}