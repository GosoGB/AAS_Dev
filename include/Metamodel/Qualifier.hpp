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
 * @date 2025-08-04
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include "./Abstract/HasSemantics.hpp"
#include "./Reference.hpp"
#include "./TypeDefinitions.hpp"

#include "Common/PSRAM.hpp"



namespace muffin { namespace aas {

    
    class QualifierBase
    {
    public:
        virtual ~QualifierBase() noexcept = default;
        virtual void SetKind(const qualifier_kind_e kind) = 0;
        virtual void SetValueID(const Reference& valueId) = 0;
        virtual const qualifier_kind_e* GetKind() const noexcept = 0;
        virtual QualifierType GetType() const noexcept = 0;
        virtual data_type_def_xsd_e GetValueType() const noexcept = 0;
        virtual const Reference* GetValueID() const noexcept = 0;
        virtual psram::unique_ptr<QualifierBase> Clone() const = 0;
    };
    
    
    using QualifierType = std::string;
    

    template<data_type_def_xsd_e xsd>
    class Qualifier : public QualifierBase, public HasSemantics
    {
    public:
        Qualifier(const QualifierType& type)
            : mType(type)
        {
            mValueType = get_xsd_type_from_cpp<xsd>();
        }

        Qualifier(const Qualifier& other)
        {
            if (other.mKind != nullptr)
            {
                mKind = psram::make_unique<qualifier_kind_e>(*other.mKind);
            }
            
            if (other.mValue != nullptr)
            {
                mValue = psram::make_unique<typename xsd_type_mapper<xsd>::type>(*other.mValue);
            }
            
            if (other.mValueID != nullptr)
            {
                mValueID = psram::make_unique<Reference>(*other.mValueID);
            }
        }

        Qualifier(Qualifier&& other) noexcept = default;

        Qualifier& operator=(const Qualifier& other)
        {
            if (this != &other)
            {
                if (other.mKind)
                {
                    mKind = psram::make_unique<qualifier_kind_e>(*other.mKind);
                }
                else
                {
                    mkind.reset();
                }
                
                if (other.mValue)
                {
                    mValue = psram::make_unique<typename xsd_type_mapper<xsd>::type>(*other.mValue);
                }
                else
                {
                    mValue.reset();
                }
                
                if (other.mValueID)
                {
                    mValueID = psram::make_unique<Reference>(*other.mValueID);
                }
                else
                {
                    mValueID.reset();
                }
            }
            return *this;
        }

        Qualifier& operator=(Qualifier&& other) noexcept = default;

        virtual ~Qualifier() noexcept = default;

        psram::unique_ptr<QualifierBase> Clone() const override
        {
            return psram::make_unique<Qualifier<xsd>>(*this);
        }

    public:
        void SetKind(const qualifier_kind_e kind) override
        {
            mKind = psram::make_unique<qualifier_kind_e>(kind);
        }

        void SetValue(const typename xsd_type_mapper<xsd>::type& value)
        {
            mValue = value;
        }

        void SetValueID(const Reference& valueId) override
        {
            mValueID = psram::make_unique<Reference>(valueId);
        }

    public:
        const qualifier_kind_e* GetKind() const override noexcept
        {
            return mKind.get();
        }

        QualifierType GetType() const override noexcept
        {
            return mType;
        }

        data_type_def_xsd_e GetValueType() const override noexcept
        {
            return mValueType;
        }

        const typename xsd_type_mapper<xsd>::type* GetValue() const noexcept
        {
            return mValue.get();
        }

        const Reference* GetValueID() const override noexcept
        {
            return mValueID.get();
        }

    protected:
        psram::unique_ptr<qualifier_kind_e> mKind;
        QualifierType mType;
        data_type_def_xsd_e mValueType;
        psram::unique_ptr<typename xsd_type_mapper<xsd>::type> mValue;
        psram::unique_ptr<Reference> mValueID;
    };
}}