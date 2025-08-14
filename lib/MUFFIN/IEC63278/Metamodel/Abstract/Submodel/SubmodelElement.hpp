/**
 * @file SubmodelElement.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief
 * A submodel element is an element suitable for the description and differentiation of assets.
 * It is recommended to add a semanticId to a SubmodelElement.
 * 
 * @date 2025-08-14
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

#include "Common/PSRAM.hpp"



namespace muffin { namespace aas {


    class SubmodelElement : public Referable, public HasKind, public HasSemantics, 
                            public Qualifiable, public HasDataSpecification
    {
    protected:
        SubmodelElement(const SubmodelElement&) = default;

    public:
        SubmodelElement() = default;
        SubmodelElement(SubmodelElement&&) = default;

        SubmodelElement& operator=(const SubmodelElement&) = delete;
        SubmodelElement& operator=(SubmodelElement&&) = default;
        
        virtual ~SubmodelElement() noexcept = default;
    
    public:
        virtual psram::unique_ptr<SubmodelElement> Clone() const = 0;
    };
}}