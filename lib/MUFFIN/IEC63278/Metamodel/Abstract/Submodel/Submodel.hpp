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
 * @date 2025-08-12
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <cstddef> 
#include <memory>

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
        using SubmodelElements = psram::vector<psram::unique_ptr<SubmodelElement>>;

    public:
        Submodel(const psram::string& id) : Identifiable(id) {}
        Submodel(const psram::string& id, SubmodelElements&& submodelElements)
            : Identifiable(id)
            , mSubmodelElements(std::move(submodelElements))
        {}

        Submodel(const Submodel& other) = delete;
        Submodel(Submodel&& other) noexcept = default;
        
        Submodel& operator=(const Submodel& other) = delete;
        Submodel& operator=(Submodel&& other) noexcept = default;
            
        virtual ~Submodel() noexcept = default;
    
    public:
        void AddSubmodelElement(psram::unique_ptr<SubmodelElement> element)
        {
            mSubmodelElements.emplace_back(std::move(element));
        }

    public:
        using iterator = typename SubmodelElements::iterator;
        using const_iterator = typename SubmodelElements::const_iterator;

        iterator begin() noexcept { return mSubmodelElements.begin(); }
        const_iterator begin() const noexcept { return mSubmodelElements.begin(); }
        const_iterator cbegin() const noexcept { return mSubmodelElements.cbegin(); }

        iterator end() noexcept { return mSubmodelElements.end(); }
        const_iterator end() const noexcept { return mSubmodelElements.end(); }
        const_iterator cend() const noexcept { return mSubmodelElements.cend(); }

        size_t size() const noexcept { return mSubmodelElements.size(); }
        bool empty() const noexcept { return mSubmodelElements.empty(); }

        SubmodelElement* operator[](size_t index) { return mSubmodelElements[index].get(); }
        const SubmodelElement* operator[](size_t index) const
        {
            return mSubmodelElements[index].get();
        }

    private:
        SubmodelElements mSubmodelElements;
    };
}}