/**
 * @file Container.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-14
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <string.h>

#include "../Metamodel/Environment.hpp"

#include "Common/Assert.hpp"



namespace muffin { namespace aas {


    class Container : public Environment
    {
    public:
        Container(Container const&) = delete;
        void operator=(Container const&) = delete;
        static Container* GetInstance();
    private:
        Container() = default;
        ~Container() noexcept = default;
    private:
        static Container* mInstance;
    
    public:
        /**
         * @todo temporary implementation, need to be replaced with actual implementation
         * @todo make an instance using deep copy and then return it to the caller
         */
        const AssetAdministrationShell* GetAasByID(const Identifier& id) const
        {
            for (const auto& aas : mVectorAAS)
            {
                const Identifier retrievedId = aas->GetID();
                if (retrievedId == id)
                {
                    return aas.get();
                }
            }
            return nullptr;
        }


        psram::vector<AssetAdministrationShell> GetAllAAS() const
        {
            psram::vector<AssetAdministrationShell> vectorAAS;
            vectorAAS.reserve(mVectorAAS.size());

            for (const auto& aas : mVectorAAS)
            {
                vectorAAS.emplace_back(aas->Clone());
            }

            return vectorAAS;
        }


        /**
         * @todo temporary implementation, need to be replaced with actual implementation
         * @todo make an instance using deep copy and then return it to the caller
         */
        const Submodel* GetSubmodelByID(const Identifier& id) const
        {
            for (const auto& submodel : mVectorSubmodel)
            {
                const Identifier retrievedId = submodel->GetID();
                if (retrievedId == id)
                {
                    return submodel.get();
                }
            }
            return nullptr;
        }


        psram::vector<Submodel> GetAllSubmodels() const
        {
            psram::vector<Submodel> vectorSubmodels;
            vectorSubmodels.reserve(mVectorSubmodel.size());
            for (const auto& submodel : mVectorSubmodel)
            {
                vectorSubmodels.emplace_back(submodel->Clone());
            }
            return vectorSubmodels;
        }

        // using iterator = typename psram::vector<AssetAdministrationShell>::iterator;
        // using const_iterator = typename psram::vector<AssetAdministrationShell>::const_iterator;
        
        // iterator begin() noexcept { return mVectorAAS.begin(); }
        // iterator begin() noexcept { return mVectorSubmodel.begin(); }

        // const_iterator begin() const noexcept { return mVectorAAS.begin(); }
        // const_iterator cbegin() const noexcept { return mVectorAAS.cbegin(); }

        // iterator end() noexcept { return mVectorAAS.end(); }
        // const_iterator end() const noexcept { return mVectorAAS.end(); }
        // const_iterator cend() const noexcept { return mVectorAAS.cend(); }

        // iterator erase(const_iterator __position) { return begin() + (__position - cbegin()); }

        // size_t size() const noexcept { return mVectorAAS.size(); }
        // bool empty() const noexcept { return mVectorAAS.empty(); }

        // AssetAdministrationShell& operator[](size_t index) { return mVectorAAS[index]; }
        // const AssetAdministrationShell& operator[](size_t index) const { return mVectorAAS[index]; }
    };
}}