/**
 * @file HasSemantics.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * Element that can have a semantic definition plus some supplemental semantic definitions.
 * 
 * @note
 * If there is a supplemental semantic ID (HasSemantics/supplementalSemanticId) defined then there 
 * shall be also a main semantic ID (HasSemantics/semanticId).
 * [Source: Constraint AASd-118]
 * 
 * @note
 * The cardinality of attribute 'supplementalSemanticId' is limited to 0 due to memory usage.
 * 
 * @todo
 * To manage memory, you would need to consider limiting the length of 'semanticId' to 32 bytes.
 * And also you would need to limit it to ASCII characters only.
 *
 * @date 2025-07-29
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <memory>

#include "../Reference.hpp"

#include "Common/PSRAM.hpp"



namespace muffin { namespace aas {


    class HasSemantics
    {
    public:
        HasSemantics() = default;
        HasSemantics(psram::unique_ptr<Reference> semanticID)
            : mSemanticID(std::move(semanticID))
        {}
        
        HasSemantics(const Reference& semanticID)
            : mSemanticID(psram::make_unique<Reference>(semanticID))
        {}

        HasSemantics(const HasSemantics& other)
        {
            if (other.mSemanticID != nullptr)
            {
                mSemanticID = psram::make_unique<Reference>(*other.mSemanticID);
            }
        }

        HasSemantics(HasSemantics&& other) noexcept = default;

        HasSemantics& operator=(const HasSemantics& other)
        {
            if (this != &other)
            {
                if (other.mSemanticID)
                {
                    mSemanticID = psram::make_unique<Reference>(*other.mSemanticID);
                }
                else
                {
                    mSemanticID.reset();
                }
            }
            return *this;
        }

        HasSemantics& operator=(HasSemantics&& other) noexcept = default;

        virtual ~HasSemantics() = default;

    public:
        void SetSemanticID(psram::unique_ptr<Reference> semanticID)
        {
            mSemanticID = std::move(semanticID);
        }

        void SetSemanticID(const Reference& semanticID)
        {
            mSemanticID = psram::make_unique<Reference>(semanticID);
        }

        const Reference* GetSemanticID() const noexcept
        {
            return mSemanticID.get();
        }

    protected:
        psram::unique_ptr<Reference> mSemanticID;
    };
}}