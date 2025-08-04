/**
 * @file SubmodelElements.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief
 * A submodel element is an element suitable for the description and differentiation of assets.
 * It is recommended to add a semanticId to a SubmodelElement.
 * 
 * @date 2025-08-01
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include "./Referable.hpp"
#include "./HasKind.hpp"
#include "./HasSemantics.hpp"
#include "./Qualifiable.hpp"
#include "./HasDataSpecification.hpp"



namespace muffin { namespace aas {


    class SubmodelElements : public Referable, 
    {
        SubmodelElements() = default;
        virtual ~SubmodelElements() noexcept = default;
    };
}}