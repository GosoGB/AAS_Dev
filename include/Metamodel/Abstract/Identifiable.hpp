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
 * This class inherits from the @class 'Referable' and shouldn't be instantiated directly.
 *
 * @date 2025-07-14
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once



namespace muffin { namespace aas {


    class Identifiable
    {
    private:
        /* data */
    public:
        Identifiable(/* args */);
        ~Identifiable();
    };
}}