/**
 * @file SubmodelElement.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief
 * A submodel element is an element suitable for the description and differentiation of assets.
 * It is recommended to add a semanticId to a SubmodelElement.
 * 
 * @date 2025-08-16
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


    class SubmodelElement;


    class ISubmodelElement : public Referable, public HasKind, public HasSemantics, 
                             public Qualifiable, public HasDataSpecification
    {
    public:
        virtual psram::unique_ptr<SubmodelElement> Clone() const = 0;
        virtual key_types_e GetModelType() const noexcept = 0;
    };


    class SubmodelElement : public ISubmodelElement
    {
    public:
        SubmodelElement() = default;
        SubmodelElement(const SubmodelElement&) = default;
        SubmodelElement(SubmodelElement&&) noexcept = default;

        SubmodelElement& operator=(const SubmodelElement&) = delete;
        SubmodelElement& operator=(SubmodelElement&&) noexcept = default;
        
        virtual ~SubmodelElement() noexcept = default;
    
    public:
        psram::unique_ptr<SubmodelElement> Clone() const override
        {
            ASSERT(false, "MUST BE OVERRIDDEN IN DERIVED CLASSES");
            return nullptr;
        }

        key_types_e GetModelType() const noexcept override
        {
            return mModelType;
        }

        void SetModelType(const key_types_e modelType)
        {
            mModelType = modelType;
        }
    
    protected:
        key_types_e mModelType;
    };
}}