/**
 * @file SpecificAssetId.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * A specific asset ID describes a generic supplementary identifying attribute of the asset.
 * The specific asset ID is not necessarily globally unique.
 * 
 * @note
 * This class inherits from the @class 'HasSemantics'.
 *
 * @date 2025-07-28
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <string>
#include <utility>

#include "./Abstract/HasSemantics.hpp"
#include "./Reference.hpp"

#include "Common/PSRAM.hpp"



namespace muffin { namespace aas {


    class SpecificAssetId : public HasSemantics
    {
    public:
        SpecificAssetId(const psram::string& name,
                        const psram::string& value,
                        const Reference& externalSubjectId)
            : mName(name)
            , mValue(value)
            , mExternalSubjectId(psram::make_unique<Reference>(externalSubjectId))
        {}

        SpecificAssetId(const SpecificAssetId& other)
            : HasSemantics(other),
              mName(other.mName),
              mValue(other.mValue),
              mExternalSubjectId(psram::make_unique<Reference>(*other.mExternalSubjectId))
        {}

        SpecificAssetId(SpecificAssetId&& other) noexcept = default;

        SpecificAssetId& operator=(const SpecificAssetId& other)
        {
            if (this != &other)
            {
                HasSemantics::operator=(other);
                mName = other.mName;
                mValue = other.mValue;
                mExternalSubjectId = psram::make_unique<Reference>(*other.mExternalSubjectId);
            }
            return *this;
        }

        SpecificAssetId& operator=(SpecificAssetId&& other) noexcept = default;

        ~SpecificAssetId() noexcept override = default;

    public:
        const psram::string& GetName() const noexcept
        {
            return mName;
        }

        const psram::string& GetValue() const noexcept
        {
            return mValue;
        }

        Reference* GetExternalSubjectId() const noexcept
        {
            return mExternalSubjectId.get();
        }

    private:
        psram::string mName;
        psram::string mValue;
        psram::unique_ptr<Reference> mExternalSubjectId;
    };
}}