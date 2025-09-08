/**
 * @file MimeTable.cpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-09-08
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#include "../Include/MimeTable.hpp"



namespace muffin { namespace w5500 { namespace mime {

    
    const entry_t mimeTable[static_cast<uint8_t>(type_e::MAX_TYPE)] =
    {
        { ".html", "text/html" },
        { ".htm", "text/html" },
        { ".css", "text/css" },
        { ".txt", "text/plain" },
        { ".js", "application/javascript" },
        { ".json", "application/json" },
        { ".png", "image/png" },
        { ".gif", "image/gif" },
        { ".jpg", "image/jpeg" },
        { ".ico", "image/x-icon" },
        { ".svg", "image/svg+xml" },
        { ".ttf", "application/x-font-ttf" },
        { ".otf", "application/x-font-opentype" },
        { ".woff", "application/font-woff" },
        { ".woff2", "application/font-woff2" },
        { ".eot", "application/vnd.ms-fontobject" },
        { ".sfnt", "application/font-sfnt" },
        { ".xml", "text/xml" },
        { ".pdf", "application/pdf" },
        { ".zip", "application/zip" },
        { ".gz", "application/x-gzip" },
        { ".appcache", "text/cache-manifest" },
        { "", "application/octet-stream" } 
    };
}}}