/**
 * @file Environment.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-11
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

#include "./Abstract/Submodel/Submodel.hpp"
#include "./AssetAdministrationShell.hpp"

#include "Common/PSRAM.hpp"



namespace muffin { namespace aas {


    class Environment
    {
    public:
        Environment() = default;
        Environment(const Environment& other) = default;
        Environment(Environment&& other) noexcept = default;
        ~Environment() noexcept = default;
    public:
        void AddAssetAdministrationShell(const AssetAdministrationShell& aas)
        {
            if (mVectorAAS.size() == 2)
            {
                log_e("ABORTED: REACHED CAPACITY LIMIT");
                return;
            }
            
            mVectorAAS.emplace_back(aas);
        }

        void RemoveAssetAdministrationShell(const Identifier& id)
        {
            for (auto it = mVectorAAS.begin(); it != mVectorAAS.end(); ++it)
            {
                if (id == it->GetID())
                {
                    it = mVectorAAS.erase(it);
                    return;
                }
            }
        }

    public:
        void AddSubmodel(const Submodel& submodel)
        {
            if (mVectorSubmodel.size() == 5)
            {
                log_e("ABORTED: REACHED CAPACITY LIMIT");
                return;
            }
            
            mVectorSubmodel.emplace_back(submodel);
        }

        void RemoveSubmodel(const Identifier& id)
        {
            for (auto it = mVectorSubmodel.begin(); it != mVectorSubmodel.end(); ++it)
            {
                if (id == it->GetID())
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
        psram::vector<AssetAdministrationShell> mVectorAAS;
        psram::vector<Submodel> mVectorSubmodel;
    };
}}