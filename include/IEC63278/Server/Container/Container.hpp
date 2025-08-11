/**
 * @file Container.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-07
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include "../../Metamodel/AssetAdministrationShell.hpp"

#include "Common/PSRAM.hpp"



namespace muffin { namespace aas {


    class Container
    {
    public:
        Container() = default;
        Container(const Container& other) = default;
        Container(Container&& other) noexcept = default;
        ~Container() noexcept = default;
    
    public:
        using iterator = typename psram::vector<AssetAdministrationShell>::iterator;
        using const_iterator = typename psram::vector<AssetAdministrationShell>::const_iterator;
        
        iterator begin() noexcept { return mVectorAAS.begin(); }
        const_iterator begin() const noexcept { return mVectorAAS.begin(); }
        const_iterator cbegin() const noexcept { return mVectorAAS.cbegin(); }

        iterator end() noexcept { return mVectorAAS.end(); }
        const_iterator end() const noexcept { return mVectorAAS.end(); }
        const_iterator cend() const noexcept { return mVectorAAS.cend(); }

        size_t size() const noexcept { return mVectorAAS.size(); }
        bool empty() const noexcept { return mVectorAAS.empty(); }

        AssetAdministrationShell& operator[](size_t index) { return mVectorAAS[index]; }
        const AssetAdministrationShell& operator[](size_t index) const { return mVectorAAS[index]; }

    private:
        psram::vector<AssetAdministrationShell> mVectorAAS;
    };
}}
    
// public:
//     Status Create(const jvs::config::Node* cin);
//     Status Remove(const std::string& nodeID);
//     void Clear();
//     std::pair<Status, Node*> GetNodeReference(const std::string& nodeID);
// private:
//     std::map<std::string, Node*> mMapNode;