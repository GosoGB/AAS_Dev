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
 * @date 2025-08-16
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

    protected:
        HasExtensions(const HasExtensions& other)
        {
            if (other.mExtension)
            {
                mExtension = other.mExtension->Clone();
            }
        }

    public:
        HasExtensions& operator=(const HasExtensions& other) = delete;

        HasExtensions(HasExtensions&& other) noexcept = default;
        HasExtensions& operator=(HasExtensions&& other) noexcept = default;
        
        virtual ~HasExtensions() noexcept = default;

    public:
        void SetExtension(psram::unique_ptr<ExtensionBase> extension)
        {
            mExtension = std::move(extension);
        }
        
        template<data_type_def_xsd_e xsd>
        void SetExtension(const Extension<xsd>& extension)
        {
            mExtension = psram::make_unique<Extension<xsd>>(extension);
        }

        ExtensionBase* GetExtensionOrNull() const
        {
            return mExtension.get();
        }

    protected:
        psram::unique_ptr<ExtensionBase> mExtension;
    };
}}