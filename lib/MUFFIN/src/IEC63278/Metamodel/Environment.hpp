/**
 * @file Environment.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-12
 * @version 0.0.1
 * 
 * @note
 * The cardinality of attribute 'assetAdministrationShell' is limited to 0..2 due to memory usage.
 * 
 * @note
 * The cardinality of attribute 'submodel' is limited to 0..5 due to memory usage.
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <esp32-hal-log.h>
#include <memory>

#include "./Abstract/Submodel/Submodel.hpp"
#include "./AssetAdministrationShell.hpp"

#include "Common/PSRAM.hpp"



namespace muffin { namespace aas {


    class Environment
    {
    public:
        Environment() = default;
        
        Environment(const Environment& other) = delete;
        Environment(Environment&& other) noexcept = default;
        
        Environment& operator=(const Environment& other) = delete;
        Environment& operator=(Environment&& other) noexcept = default;

        ~Environment() noexcept = default;

    public:
        void AddAssetAdministrationShell(psram::unique_ptr<AssetAdministrationShell> aas)
        {
            if (mVectorAAS.size() == 2)
            {
                log_e("ABORTED: REACHED CAPACITY LIMIT");
                return;
            }
            
            mVectorAAS.emplace_back(std::move(aas));
        }

        void RemoveAssetAdministrationShell(const Identifier& id)
        {
            for (auto it = mVectorAAS.begin(); it != mVectorAAS.end(); ++it)
            {
                // it is an iterator to a psram::unique_ptr, so dereference it twice.
                if (id == (*it)->GetID())
                {
                    it = mVectorAAS.erase(it);
                    return;
                }
            }
        }

    public:
        void AddSubmodel(psram::unique_ptr<Submodel> submodel)
        {
            if (mVectorSubmodel.size() == 5)
            {
                log_e("ABORTED: REACHED CAPACITY LIMIT");
                return;
            }
            
            mVectorSubmodel.emplace_back(std::move(submodel));
        }

        void RemoveSubmodel(const Identifier& id)
        {
            for (auto it = mVectorSubmodel.begin(); it != mVectorSubmodel.end(); ++it)
            {
                // it is an iterator to a psram::unique_ptr, so dereference it twice.
                if (id == (*it)->GetID())
                {
                    it = mVectorSubmodel.erase(it);
                    return;
                }
            }
        }

    public:
        void Clear()
        {
            mVectorAAS.clear();
            mVectorSubmodel.clear();
        }

    protected:
        psram::vector<psram::unique_ptr<AssetAdministrationShell>> mVectorAAS;
        psram::vector<psram::unique_ptr<Submodel>> mVectorSubmodel;
    };
}}