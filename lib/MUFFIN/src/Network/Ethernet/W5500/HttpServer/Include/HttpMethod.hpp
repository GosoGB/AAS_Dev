/**
 * @file HttpMethod.hpp
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

#include <http_parser.h>



namespace muffin { namespace w5500 {

    typedef enum http_method http_method_e;
    #define HTTP_ANY_METHOD ((http_method_e)(255))
}}