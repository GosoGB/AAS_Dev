/**
 * @file HasDataSpecification.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief
 * Element that can be extended by using data specification templates. A data specification 
 * template defines a named set of additional attributes an element may or shall have. The data 
 * specifications used are explicitly specified with their global ID.
 * 
 * @note
 * The cardinality of attribute 'dataSpecification' is limited to 0..1 due to memory usage.
 * 
 * @todo
 * JSON file regarding to the 'DataSpecification' made by AASX Package Explorer is incomplete and
 * CANNOT PASS the test engine provided by IDTA. Thus, DataSpecification is not going to be provided
 * until we have resource for further research.
 * 
 * @date 2025-08-18
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <memory>
#include <utility>

#include "../Reference.hpp"

#include "Common/Assert.hpp"
#include "Common/PSRAM.hpp"



namespace muffin { namespace aas {


    class HasDataSpecification
    {
    public:
        HasDataSpecification() = default;
        // HasDataSpecification(const Reference& dataSpecification)
        //     : mDataSpecification(psram::make_unique<Reference>(dataSpecification))
        // {
        //     ASSERT((mDataSpecification->GetType() == reference_types_e::EXTERNAL_REFERENCE), 
        //         "ATTRIBUTE 'dataSpecification' MUST BE A GLOBAL REFERENCE");
        // }

        // HasDataSpecification(psram::unique_ptr<Reference> dataSpecification)
        //     : mDataSpecification(std::move(dataSpecification))
        // {
        //     ASSERT((mDataSpecification->GetType() == reference_types_e::EXTERNAL_REFERENCE), 
        //         "ATTRIBUTE 'dataSpecification' MUST BE A GLOBAL REFERENCE");
        // }

        HasDataSpecification(const HasDataSpecification& other) = default;
        // HasDataSpecification(const HasDataSpecification& other)
        // {
        //     if (other.mDataSpecification)
        //     {
        //         mDataSpecification = psram::make_unique<Reference>(*other.mDataSpecification);
        //     }
        // }

        HasDataSpecification(HasDataSpecification&& other) noexcept = default;

        HasDataSpecification& operator=(HasDataSpecification& other) = default;
        // HasDataSpecification& operator=(const HasDataSpecification& other)
        // {
        //     if (this != &other)
        //     {
        //         if (other.mDataSpecification)
        //         {
        //             mDataSpecification = psram::make_unique<Reference>(*other.mDataSpecification);
        //         }
        //         else
        //         {
        //             mDataSpecification.reset();
        //         }
        //     }
        //     return *this;
        // }

        HasDataSpecification& operator=(HasDataSpecification&& other) noexcept = default;

        virtual ~HasDataSpecification() = default;

    public:
        // void SetDataSpecification(const Reference& dataSpecification)
        // {
        //     mDataSpecification = psram::make_unique<Reference>(dataSpecification);
        // }

        // void SetDataSpecification(psram::unique_ptr<Reference> dataSpecification)
        // {
        //     mDataSpecification = std::move(dataSpecification);
        // }

        // const Reference* GetDataSpecificationOrNULL() const
        // {
        //     return mDataSpecification ? mDataSpecification.get() : nullptr;
        // }
        
    protected:
        // psram::unique_ptr<Reference> mDataSpecification;
    };
}}