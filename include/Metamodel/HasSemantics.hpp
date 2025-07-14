/**
 * @file HasSemantics.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * 
 * @note 
 * The length of 'semanticId' or the 'main semanticId' is limited to 32 bytes and it must be
 * ASCII characters due to the memory limitation.
 * 
 * @note 
 * The cardinality of 'supplementalSemanticId' is limited to 0 due to the memory limitation.
 * 
 * @date 2025-07-14
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <string>



namespace muffin { namespace aas {


    class HasSemantics
    {
    public:
        HasSemantics() {}
        ~HasSemantics() {}
        ~HasSemantics() = default;
    protected:
        std::string semanticId;
    };
}}