/**
 * @file MimeTable.hpp
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

#include <cstdint>



namespace muffin { namespace w5500 { namespace mime {


    typedef enum class MimeTypeEnum
        : uint8_t
    {
        HTML,
        HTM,
        CSS,
        TXT,
        JS,
        JSON,
        PNG,
        GIF,
        JPG,
        ICO,
        SVG,
        TTF,
        OTF,
        WOFF,
        WOFF2,
        EOT,
        SFNT,
        XML,
        PDF,
        ZIP,
        GZ,
        APP_CACHE,
        NONE,
        MAX_TYPE
    } type_e;

    typedef struct MimeEntryType
    {
        const char EndsWith[16];
        const char MimeType[32];
    } entry_t;


    extern const entry_t mimeTable[static_cast<uint8_t>(type_e::MAX_TYPE)];
}}}