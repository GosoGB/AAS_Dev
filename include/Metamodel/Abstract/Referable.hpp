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
 * @date 2025-07-14
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <memory>
#include <string>

#include "./HasExtensions.hpp"

#include "Common/Assert.hpp"



namespace muffin { namespace aas {


    class Referable : public HasExtensions
    {
    public:
        Referable() {}
        ~Referable() {}
        ~Referable() = default;
    public:
        void SetCategory(const std::string& category)
        {
            ASSERT(
                (
                    [&] (const std::string& cat)
                    {
                        return ((cat != "CONSTANT") && (cat != "PARAMETER") && (cat != "VARIABLE"));
                    } (category)
                ), "INVALID CATEGORY FOR A 'Referable': %s", category.c_str()
            );

            this->category = std::make_shared<std::string>(category);
        }

        void SetIdShort(const std::string& idShort) 
        {
            ASSERT((idShort.length() > 0), "'idShort' MUST NOT BE EMPTY");
            ASSERT((idShort.length() < 129), "'idShort' MUST BE SMALLER THAN 128 BYTES");
            
            this->idShort = std::make_shared<std::string>(idShort);
        }
    protected:
        std::weak_ptr<std::string> category;
        std::weak_ptr<std::string> idShort;
    };
}}