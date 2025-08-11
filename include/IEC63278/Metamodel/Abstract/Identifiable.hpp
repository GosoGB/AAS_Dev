/**
 * @file Identifiable.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief An element that has a globally unique identifier.
 * 
 * @details
 * An identifiable element is a referable with a globally unique identifier (Identifier).
 * To reference an identifiable only the global ID (Identifiable/id) shall be used because 
 * the idShort is not unique for an identifiable. Identifiables may have administrative 
 * information like version etc.
 * 
 * @note
 * The cardinality of attribute 'administration' is limited to 0 due to memory usage.
 * 
 * @note
 * This class inherits from the @class 'Referable' and shouldn't be instantiated directly.
 * 
 * @note
 * Identifier type changed. Before struct class with two attributes: id and idType. 
 * Now string data type only.
 * [Source: Details of the Asset Administration Shell Part 1 Version 3.0RC02, p.200]
 *
 * @date 2025-07-28
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <string>
#include <utility>

#include "./Referable.hpp"

#include "Common/Assert.hpp"
#include "Common/PSRAM.hpp"



namespace muffin { namespace aas {
    

    using Identifier = psram::string;
    
    class Identifiable : public Referable
    {
    public:
        explicit Identifiable(psram::string id)
            : Referable()
            , mID(std::move(id))
        {
            ASSERT(mID.empty() == false, "IDENTIFIER CANNOT BE EMPTY");
        }

        Identifiable(const Identifiable& other) = default;

        ~Identifiable() noexcept override = default;

        Identifiable& operator=(const Identifiable& other)
        {
            if (this != &other)
            {
                mID = other.mID;
            }
            return *this;
        }

        bool operator==(const Identifiable& other) const
        {
            return mID == other.mID;
        }

    public:
        Identifier GetID() const noexcept
        {
            return mID;
        }
    
    protected:
        Identifier mID;
    };
}}