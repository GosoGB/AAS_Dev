/**
 * @file URI.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * 
 * @date 2025-09-08
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include <string>
#include <vector>

#include "Common/Assert.hpp"
#include "Common/Logger/Logger.h"



namespace muffin { namespace w5500 {


    class URI
    {
    public:
        URI(const char* uri) : mURI(uri) {}
        URI(const std::string& uri) : mURI(uri) {}
        ~URI() noexcept = default;
    public:
        URI* Clone() const
        {
            return new URI(mURI);
        }
    
        std::string GetURI() const
        {
            return mURI;
        }

        virtual void InitPathArgs(__attribute__((unused)) std::vector<std::string>* pathArgs)
        {
            ASSERT((pathArgs != nullptr), "BAD PARAM: OUTPUT PARAMETER CANNOT BE NULL");
            LOG_VERBOSE(logger, "This function does not do actual work");
        }

        virtual bool CanHandle(const std::string &requestURI, __attribute__((unused)) std::vector<std::string> &pathArgs)
        {
            return mURI == requestURI;
        }
    private:
        const std::string mURI;
    };
}}