/**
 * @file Reference.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * Reference to either a model element of the same or another AAS or to an external
 * entity. A reference is an ordered list of keys.
 * 
 * @details
 * A model reference is an ordered list of keys, each keys referencing an element. The complete list 
 * of keys may for example be concatenated to a path that then gives unique access to an element. 
 * A global reference is a reference to an external entity.
 * 
 * The cardinality of member variables is as follows:
 *  - @var type: 1
 *  - @var referredSemanticId: 0..1
 *  - @var keys: 1..*
 * 
 * @date 2025-08-18
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <memory>
#include <utility>
#include <vector>

#include "./Key.hpp"
#include "./TypeDefinitions.hpp"

#include "Common/Assert.hpp"
#include "Common/PSRAM.hpp"



namespace muffin { namespace aas {


    class Reference
    {
    public:
        Reference(const reference_types_e type, const psram::vector<Key>& keys)
            : mType(type)
            , mKeys(keys)
        {
            ASSERT((mKeys.empty() == false), "REFERENCE KEY CANNOT BE EMPTY");
        }

        Reference(const reference_types_e type, psram::vector<Key>&& keys)
            : mType(type)
            , mKeys(std::move(keys))
        {
            ASSERT((mKeys.empty() == false), "REFERENCE KEY CANNOT BE EMPTY");
        }

        Reference(const Reference& other)
            : mType(other.mType)
            , mKeys(other.mKeys)
            , mReferredSemanticID(nullptr)
        {
            if (other.mReferredSemanticID)
            {
                mReferredSemanticID = psram::make_unique<Reference>(*other.mReferredSemanticID);
            }
        }

        Reference& operator=(const Reference& other)
        {
            if (this != &other)
            {
                mType = other.mType;
                mKeys = other.mKeys;
                mReferredSemanticID = other.mReferredSemanticID
                                        ? psram::make_unique<Reference>(*other.mReferredSemanticID)
                                        : nullptr;
            }
            return *this;
        }
        
        Reference(Reference&& other) noexcept = default;
        Reference& operator=(Reference&& other) noexcept = default;

        ~Reference() noexcept = default;

        bool operator==(const Reference& other) const
        {
            if (mType != other.mType || mKeys != other.mKeys)
            {
                return false;
            }

            const bool thisHasId = (mReferredSemanticID != nullptr);
            const bool otherHasId = (other.mReferredSemanticID != nullptr);

            if (thisHasId != otherHasId)
            {
                return false;
            }

            if (thisHasId == false)
            {
                return true;
            }

            return *mReferredSemanticID == *other.mReferredSemanticID;
        }

        bool operator!=(const Reference& other) const
        {
            return !(*this == other);
        }

        Reference Clone() const
        {
            return *this;
        }
        
    public:
        void SetReferredSemanticID(psram::unique_ptr<Reference> referredSemanticID)
        {
            mReferredSemanticID = std::move(referredSemanticID);
        }

    public:
        reference_types_e GetType() const noexcept
        {
            return mType;
        }

        const Reference* GetReferredSemanticID() const noexcept
        {
            return mReferredSemanticID ? mReferredSemanticID.get() : nullptr;
        }

        const psram::vector<Key>& GetKeys() const noexcept
        {
            return mKeys;
        }

    private:
        reference_types_e mType;
        psram::unique_ptr<Reference> mReferredSemanticID;
        psram::vector<Key> mKeys;
    };
}}