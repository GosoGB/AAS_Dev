/**
 * @file Base64.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @date 2025-08-21
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#pragma once

#include "Common/PSRAM.hpp"



namespace muffin {


    psram::string DecodeBase64(const char* base64)
    {
        psram::string safe_base64(base64);
        for (char &c : safe_base64)
        {
            if (c == '-') c = '+';
            else if (c == '_') c = '/';
        }

        while (safe_base64.length() % 4)
        {
            safe_base64 += '=';
        }
        const char* safe_base64_cstr = safe_base64.c_str();

        static const uint8_t b64_table[256] =
        {
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 62, 64, 64, 64, 63,
           52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
           64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
           15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
           64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
           41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
        };
        const size_t len = safe_base64.length();
        if (len % 4 != 0)
        {
            return "";
        }
 
        psram::string decoded;
        decoded.reserve(len / 4 * 3);
        for (size_t idx = 0; idx < len; idx += 4)
        {
            uint32_t tmp = 
            (
                (b64_table[safe_base64_cstr[idx]]       << 18) | 
                (b64_table[safe_base64_cstr[idx + 1]]   << 12) | 
                (b64_table[safe_base64_cstr[idx + 2]]   << 6)  | 
                (b64_table[safe_base64_cstr[idx + 3]])
            );

            decoded += (tmp >> 16) & 0xFF;
            if (safe_base64_cstr[idx + 2] != '=')
            {
                decoded += (tmp >> 8) & 0xFF;
            }

            if (safe_base64_cstr[idx + 3] != '=')
            {
                decoded += tmp & 0xFF;
            }
        }
        
        return decoded;
    }

    
    psram::string EncodeBase64(const psram::string& data, const bool urlSafe = true)
    {
        static const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
 
        psram::string ret;
        int i = 0;
        int j = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];
        const char* bytes_to_encode = data.c_str();
        size_t in_len = data.length();

        ret.reserve(in_len * 4 / 3 + 4);

        while (in_len--)
        {
            char_array_3[i++] = *(bytes_to_encode++);
            if (i == 3)
            {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for(i = 0; (i <4) ; i++)
                {
                    ret += base64_chars[char_array_4[i]];
                }
                i = 0;
            }
        }

        if (i)
        {
            for(j = i; j < 3; j++)
                char_array_3[j] = '\0';

            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (j = 0; (j < i + 1); j++)
                ret += base64_chars[char_array_4[j]];

            if (urlSafe == false)
            {
                while((i++ < 3))
                {
                    ret += '=';
                }
            }
        }

        return ret;
    }
}