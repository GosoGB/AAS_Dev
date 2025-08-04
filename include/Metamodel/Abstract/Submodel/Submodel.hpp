/**
 * @file Submodel.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief
 * A submodel defines a specific aspect of the asset represented by the AAS.
 * 
 * @details
 * A submodel is used to structure the digital representation and technical functionality of an 
 * Administration Shell into distinguishable parts. Each submodel refers to a well-defined domain 
 * or subject matter. Submodels can become standardized and, thus, become submodels templates.
 * 
 * @date 2025-08-04
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <cstddef> 

#include "../Identifiable.hpp"
#include "../HasKind.hpp"
#include "../HasSemantics.hpp"
#include "../Qualifiable.hpp"
#include "../HasDataSpecification.hpp"
#include "./SubmodelElement.hpp"

#include "Common/PSRAM.hpp"



namespace muffin { namespace aas {


    class Submodel : public Identifiable, public HasKind, public HasSemantics, 
                     public Qualifiable,  public HasDataSpecification
    {
    public:
        Submodel(const psram::string& id)
            : Identifiable(id)
        {}

        Submodel(const psram::string& id, const psram::vector<SubmodelElement>& submodelElement)
            : Identifiable(id)
            , mSubmodelElement(submodelElement)
        {}

        Submodel(const psram::string& id, psram::vector<SubmodelElement>&& submodelElement)
            : Identifiable(id)
            , mSubmodelElement(std::move(submodelElement))
        {}

        Submodel(const Submodel& other) = default;
        Submodel(Submodel&& other) noexcept = default;

        void operator=(const Submodel& other)
        {
            if (this != &other)
            {
                mSubmodelElement = other.mSubmodelElement;
            }
        }

        Submodel& operator=(Submodel&& other) noexcept = default;
            
        virtual ~Submodel() noexcept = default;
    
    public:
        using iterator = typename psram::vector<SubmodelElement>::iterator;
        using const_iterator = typename psram::vector<SubmodelElement>::const_iterator;

        iterator begin() noexcept { return mSubmodelElement.begin(); }
        const_iterator begin() const noexcept { return mSubmodelElement.begin(); }
        const_iterator cbegin() const noexcept { return mSubmodelElement.cbegin(); }

        iterator end() noexcept { return mSubmodelElement.end(); }
        const_iterator end() const noexcept { return mSubmodelElement.end(); }
        const_iterator cend() const noexcept { return mSubmodelElement.cend(); }

        size_t size() const noexcept { return mSubmodelElement.size(); }
        bool empty() const noexcept { return mSubmodelElement.empty(); }

        SubmodelElement& operator[](size_t index) { return mSubmodelElement[index]; }
        const SubmodelElement& operator[](size_t index) const { return mSubmodelElement[index]; }

    private:
        psram::vector<SubmodelElement> mSubmodelElement;
    };
}}