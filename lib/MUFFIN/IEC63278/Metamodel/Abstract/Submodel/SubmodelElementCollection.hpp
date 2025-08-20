/**
 * @file SubmodelElementCollection.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief
 * A submodel element collection is a kind of struct, i.e. a logical encapsulation of multiple
 * named values. It has a fixed number of submodel elements.
 * 
 * @date 2025-08-19
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include "./SubmodelElement.hpp"

#include "Common/PSRAM.hpp"



namespace muffin { namespace aas {


    class SubmodelElementCollection : public SubmodelElement
    {
        using SubmodelElements = psram::vector<psram::unique_ptr<SubmodelElement>>;
    public:
        SubmodelElementCollection()
        {
            mModelType = key_types_e::SubmodelElementCollection;
        }

        SubmodelElementCollection(const SubmodelElementCollection& other)
            : SubmodelElement(other)
        {
            mValue.reserve(other.mValue.size());
            for (const auto& element : other.mValue)
            {
                if (element)
                {
                    mValue.push_back(element->Clone());
                }
            }
        }

        ~SubmodelElementCollection() noexcept override = default;

    public:
        psram::unique_ptr<SubmodelElement> Clone() const override
        {
            return psram::make_unique<SubmodelElementCollection>(*this);
        }

        void Add(psram::unique_ptr<SubmodelElement> submodelElement)
        {
            mValue.emplace_back(std::move(submodelElement));
        }

    public:
        using iterator = typename SubmodelElements::iterator;
        using const_iterator = typename SubmodelElements::const_iterator;

        iterator begin() noexcept { return mValue.begin(); }
        const_iterator begin() const noexcept { return mValue.begin(); }
        const_iterator cbegin() const noexcept { return mValue.cbegin(); }

        iterator end() noexcept { return mValue.end(); }
        const_iterator end() const noexcept { return mValue.end(); }
        const_iterator cend() const noexcept { return mValue.cend(); }

        size_t size() const noexcept { return mValue.size(); }
        bool empty() const noexcept { return mValue.empty(); }

        SubmodelElement* operator[](size_t index) { return mValue[index].get(); }
        const SubmodelElement* operator[](size_t index) const { return mValue[index].get(); }
    private:
        SubmodelElements mValue;
    };
}}