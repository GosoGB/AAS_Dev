/**
 * @file HasExtensions.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief
 * Element that can be extended by proprietary extensions.
 * 
 * @note
 * Extensions are proprietary, i.e. they do not support global interoperability.
 * 
 * @note
 * The cardinality of attribute 'extension' is limited to 0..1 due to memory usage.
 *
 * @date 2025-07-28
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <memory>

#include "../Extension.hpp"

#include "Common/Assert.hpp"
#include "Common/PSRAM.hpp"



namespace muffin { namespace aas {


    class HasExtensions
    {
    public:
        HasExtensions() = default;
        HasExtensions(psram::unique_ptr<ExtensionBase> extension)
            : mExtension(std::move(extension))
        {}

        HasExtensions(const HasExtensions& other)
        {
            if (other.mExtension != nullptr)
            {
                mExtension = other.mExtension->Clone();
            }
        }

        HasExtensions& operator=(const HasExtensions& other)
        {
            if (this != &other)
            {
                if (other.mExtension != nullptr)
                {
                    mExtension = other.mExtension->Clone();
                }
                else
                {
                    mExtension.reset();
                }
            }
            return *this;
        }

        virtual ~HasExtensions() noexcept = default;

    public:
        void SetExtension(psram::unique_ptr<ExtensionBase> extension)
        {
            mExtension = std::move(extension);
        }
        
        template<data_type_def_xsd_e xsd>
        void SetExtension(const Extension<xsd>& extension)
        {
            mExtension = psram::make_unique<Extension<T>>(extension);
        }

        ExtensionBase* GetExtension() const
        {
            return mExtension.get();
        }

    protected:
        psram::unique_ptr<ExtensionBase> mExtension;
    };
}}