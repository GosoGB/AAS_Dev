/**
 * @file HasKind.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief
 * An element with a kind is an element that can either represent a template or an instance.
 * Default for an element is that it is representing an instance.
 * 
 * @date 2025-08-01
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include "../TypeDefinitions.hpp"



namespace muffin { namespace aas {


    class HasKind
    {
    public:
        HasKind() : mKind(modeling_kind_e::INSTANCE) {};
        HasKind(const modeling_kind_e kind) : mKind(kind) {};
        virtual ~HasKind() noexcept = default;
    protected:
        modeling_kind_e mKind;
    };
}}