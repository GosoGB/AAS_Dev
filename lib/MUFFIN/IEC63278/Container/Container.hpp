/**
 * @file Container.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-12
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include "../Metamodel/Environment.hpp"



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
         */
        const AssetAdministrationShell* GetAssetAdministrationShell() const
        {
            if (mVectorAAS.empty())
            {
                return nullptr;
            }
            
            return mVectorAAS.front().get();
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