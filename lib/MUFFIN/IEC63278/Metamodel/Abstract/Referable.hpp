/**
 * @file Referable.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * An element that is referable by its idShort. This ID is not globally unique.
 * This ID is unique within the name space of the element.
 * 
 * @details
 * The metamodel distinguishes between elements that are identifiable, referable or none of both. 
 * Referable elements can be referenced via the idShort. Not every element of the metamodel is 
 * referable. There are elements that are just attributes of a referable.
 *
 * @note
 * For non-identifiable referables the idShort shall be unique in its namespace.
 * The name space is defined as follows: 
 *  - 'The parent element(s) an element is part of and that is either 
 *     referable or identifiable is the name space of the element'
 * [source: Constraint AASd-022]
 * 
 * @note
 * This class inherits from the @class 'HasExtensions' and shouldn't be instantiated directly.
 * 
 * @date 2025-08-12
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <memory>
#include <string>

#include "./HasExtensions.hpp"

#include "Common/Assert.hpp"
#include "Common/PSRAM.hpp"



namespace muffin { namespace aas {


    class Referable : public HasExtensions
    {
    public:
        Referable() = default;

        Referable(const Referable&) = delete;
        Referable& operator=(const Referable&) = delete;
        
        Referable(Referable&&) = default;
        Referable& operator=(Referable&&) = default;
        
        ~Referable() noexcept override = default;

    public:
        void SetCategory(const psram::string& category)
        {
            ASSERT(
                ((category != "CONSTANT") && (category != "PARAMETER") && (category != "VARIABLE")),
                "INVALID CATEGORY FOR A 'Referable': %s", category.c_str()
            );

            if (mCategory == nullptr)
            {
                mCategory = psram::make_unique<psram::string>(category);
            }
            else
            {
                *mCategory = category;
            }
        }

        void SetIdShort(const psram::string& idShort) 
        {
            ASSERT((idShort.length() > 0), "'idShort' MUST NOT BE EMPTY");
            ASSERT((idShort.length() < 129), "'idShort' MUST BE SMALLER THAN 128 BYTES");

            if (mIdShort == nullptr)
            {
                mIdShort = psram::make_unique<psram::string>(idShort);
            }
            else
            {
                *mIdShort = idShort;
            }
        }

    public:
        const char* GetCategoryOrNull() const noexcept
        {
            return mCategory->c_str();
        }
        
        const char* GetIdShortOrNull() const noexcept
        {
            return mIdShort->c_str();
        }

    protected:
        psram::unique_ptr<psram::string> mCategory;
        psram::unique_ptr<psram::string> mIdShort;
    };
}}