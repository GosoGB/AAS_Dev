/**
 * @file SubmodelElement.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief
 * A submodel element is an element suitable for the description and differentiation of assets.
 * It is recommended to add a semanticId to a SubmodelElement.
 * 
 * @date 2025-08-04
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include "../Referable.hpp"
#include "../HasKind.hpp"
#include "../HasSemantics.hpp"
#include "../Qualifiable.hpp"
#include "../HasDataSpecification.hpp"



namespace muffin { namespace aas {


    class SubmodelElement : public Referable, public HasKind, public HasSemantics, 
                            public Qualifiable, public HasDataSpecification
    {
    public:
        SubmodelElement() = default;
        
        SubmodelElement(const SubmodelElement&) = delete;
        SubmodelElement(SubmodelElement&&) = default;

        SubmodelElement& operator=(const SubmodelElement&) = delete;
        SubmodelElement& operator=(SubmodelElement&&) = default;
        
        virtual ~SubmodelElement() = default;
    };
}}