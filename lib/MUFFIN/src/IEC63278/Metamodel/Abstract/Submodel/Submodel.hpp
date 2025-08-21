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
 * @date 2025-08-16
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <algorithm>
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
        Submodel(psram::string&& id) : Identifiable(std::move(id)) {}

        Submodel(const Submodel& other) = delete;
        Submodel& operator=(const Submodel& other) = delete;
        
        Submodel(Submodel&& other) noexcept = default;
        Submodel& operator=(Submodel&& other) noexcept = default;
            
        virtual ~Submodel() noexcept = default;


    public:
        Submodel Clone() const
        {
            Submodel clone(mID);
            clone.SetKind(mKind);

            if (mCategory)
            {
                clone.SetCategory(mCategory->c_str());
            }

            if (mIdShort)
            {
                clone.SetIdShort(*mIdShort);
            }

            if (mExtension)
            {
                clone.SetExtension(mExtension->Clone());
            }
            
            if (mQualifier)
            {
                clone.SetQualifier(mQualifier->Clone());
            }
            
            if (mSemanticID)
            {
                clone.SetSemanticID(psram::make_unique<Reference>(*mSemanticID));
            }
            
            clone.mSubmodelElements.reserve(this->mSubmodelElements.size());
            for (const auto& element : this->mSubmodelElements)
            {
                if (element) {
                    clone.AddSubmodelElement(element->Clone());
                }
            }

            return clone;
        }

    public:
        void AddSubmodelElement(psram::unique_ptr<SubmodelElement> element)
        {
            mSubmodelElements.emplace_back(std::move(element));
        }

        const SubmodelElement* GetElementWithIdShort(const psram::string& idShort) const
        {
            auto it = std::find_if(
                mSubmodelElements.cbegin(), 
                mSubmodelElements.cend(), 
                [&idShort](const psram::unique_ptr<SubmodelElement>& element)
                {
                    return element->GetIdShortOrNull();
                }
            );

            return (it != mSubmodelElements.cend()) ? it->get() : nullptr;
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