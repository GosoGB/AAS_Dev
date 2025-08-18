/**
 * @file Qualifiable.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief
 * The value of a qualifiable element may be further qualified by one or more qualifiers.
 * 
 * @date 2025-08-18
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include "../Qualifier.hpp"

#include "Common/PSRAM.hpp"



namespace muffin { namespace aas {


    class Qualifiable
    {
    public:
        Qualifiable() = default;
        Qualifiable(psram::unique_ptr<QualifierBase> qualifier)
            : mQualifier(std::move(qualifier))
        {}

        Qualifiable(const Qualifiable& other)
        {
            if (other.mQualifier != nullptr)
            {
                mQualifier = other.mQualifier->Clone();
            }
        }

        Qualifiable& operator=(const Qualifiable& other)
        {
            if (this != &other)
            {
                if (other.mQualifier != nullptr)
                {
                    mQualifier = other.mQualifier->Clone();
                }
                else
                {
                    mQualifier.reset();
                }
            }
            return *this;
        }
        
        Qualifiable(Qualifiable&&) noexcept = default;
        virtual ~Qualifiable() noexcept = default;
    
    public:
        void SetQualifier(psram::unique_ptr<QualifierBase> qualifier)
        {
            mQualifier = std::move(qualifier);
        }

        template<data_type_def_xsd_e xsd>
        void SetQualifier(const Qualifier<xsd>& qualifier)
        {
            mQualifier = psram::make_unique<Qualifier<xsd>>(qualifier);
        }

    public:
        const QualifierBase* GetQualifierOrNULL() const noexcept
        {
            return mQualifier ? mQualifier.get() : nullptr;
        }

    protected:
        psram::unique_ptr<QualifierBase> mQualifier;
    };
}}