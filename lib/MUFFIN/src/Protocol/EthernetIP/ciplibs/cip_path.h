#if defined(MT11)

#ifndef CIP_PATH_H
#define CIP_PATH_H

#include <string>
#include <vector>
#include <regex>
#include "Common/PSRAM.hpp"


// 배열 인덱스를 지정하는 Logical Segment 생성
// 1756-pm020_-en-p.pdf page 14의 규칙에 따름, * 다른 제조사일 경우 확인 필요
inline muffin::psram::vector<uint8_t> buildElementSelector(uint32_t index) 
{
    muffin::psram::vector<uint8_t> seg;
    if (index <= 0xFF) 
    {
        seg.push_back(0x28); // 8-bit Element ID
        seg.push_back(static_cast<uint8_t>(index));
    } 
    else if (index <= 0xFFFF) 
    {
        seg.push_back(0x29); // 16-bit Element ID
        seg.push_back(0x00); // Reserved
        seg.push_back(index & 0xFF);
        seg.push_back((index >> 8) & 0xFF);
    } 
    else 
    {
        seg.push_back(0x2A); // 32-bit Element ID
        seg.push_back(0x00); // Reserved
        seg.push_back(index & 0xFF);
        seg.push_back((index >> 8) & 0xFF);
        seg.push_back((index >> 16) & 0xFF);
        seg.push_back((index >> 24) & 0xFF);
    }
    return seg;
}

// TAG 이름을 CIP 경로(Symbolic Segment)로 인코딩
inline muffin::psram::vector<uint8_t> encodeTagPath(const std::string& tagName) 
{
    muffin::psram::vector<uint8_t> path;
    path.push_back(0x91);   // Symbolic Segment type
    path.push_back(tagName.length());
    for (char c : tagName) {
        path.push_back(static_cast<uint8_t>(c));
    }
    if (tagName.length() % 2 != 0) {
        path.push_back(0x00);  // Padding
    }
    return path;
}

inline muffin::psram::vector<uint8_t> encodeTagPathWithMultiIndex(const std::string& tag) 
{
    muffin::psram::vector<uint8_t> path;
    std::string token;
    std::stringstream ss(tag);

    while (std::getline(ss, token, '.')) 
    {
        std::string base;
        muffin::psram::vector<uint32_t> indices;
        size_t pos = 0;
        while (pos < token.length()) 
        {
            if (token[pos] == '[') 
            {
                size_t end = token.find(']', pos);
                if (end == std::string::npos) break;
                std::string idx_str = token.substr(pos + 1, end - pos - 1);
                indices.push_back(atoi(idx_str.c_str()));
                pos = end + 1;
            } 
            else 
            {
                base += token[pos];
                ++pos;
            }
        }

        // Add base tag name
        muffin::psram::vector<uint8_t> seg = encodeTagPath(base);
        path.insert(path.end(), seg.begin(), seg.end());

        // Add indices
        for (uint32_t idx : indices) 
        {
            muffin::psram::vector<uint8_t> idx_seg = buildElementSelector(idx);
            path.insert(path.end(), idx_seg.begin(), idx_seg.end());
        }
    }

    return path;
}

// inline muffin::psram::vector<uint8_t> encodeTagPathWithMultiIndex(const std::string& tag) 
// {
//     muffin::psram::vector<uint8_t> path;
//     std::string tagName;
//     muffin::psram::vector<uint32_t> indices;

//     // 태그 이름과 인덱스를 직접 분리
//     size_t pos = 0;
//     while (pos < tag.length()) 
//     {
//         if (tag[pos] == '[') 
//         {
//             // 인덱스 시작
//             size_t end = tag.find(']', pos);
//             if (end == std::string::npos) break;  // 잘못된 형식
//             std::string index_str = tag.substr(pos + 1, end - pos - 1);
//             indices.push_back(static_cast<uint32_t>(atoi(index_str.c_str())));
//             pos = end + 1;
//         } 
//         else 
//         {
//             tagName += tag[pos];
//             ++pos;
//         }
//     }

//     // 심볼 세그먼트 인코딩
//     path = encodeTagPath(tagName);

//     // 인덱스 세그먼트 추가
//     for (uint32_t idx : indices) 
//     {
//         muffin::psram::vector<uint8_t> seg = buildElementSelector(idx);
//         path.insert(path.end(), seg.begin(), seg.end());
//     }

//     return path;
// }


// CIP 경로를 문자열 TAG 이름으로 디코딩 (디버깅용)
inline std::string decodeTagPath(const muffin::psram::vector<uint8_t>& path) {
    if (path.size() < 2 || path[0] != 0x91) return "";
    uint8_t len = path[1];
    return std::string(path.begin() + 2, path.begin() + 2 + len);
}

// Class/Instance/Attribute 기반 CIP Logical Path 생성 , 1756-pm020_-en-p.pdf page 14
// 8-bit, 16-bit 대응
// Class/Instance/Attribute 기반 CIP Logical Path 생성 , 1756-pm020_-en-p.pdf page 14
// 8-bit, 16-bit 대응
inline muffin::psram::vector<uint8_t> buildLogicalPath(uint16_t classID, uint16_t instanceID, uint16_t attributeID) {
    muffin::psram::vector<uint8_t> path;

    // Class
    if (classID <= 0xFF) {
        path.push_back(0x20);   
        path.push_back(static_cast<uint8_t>(classID));
    } else {
        path.push_back(0x21); // 16-bit
        path.push_back(static_cast<uint8_t>(classID & 0xFF));
        path.push_back(static_cast<uint8_t>((classID >> 8) & 0xFF));
    }

    // Instance
    if (instanceID <= 0xFF) {
        path.push_back(0x24);   
        path.push_back(static_cast<uint8_t>(instanceID));
    } else {
        path.push_back(0x25); // 16-bit
        path.push_back(static_cast<uint8_t>(instanceID & 0xFF));
        path.push_back(static_cast<uint8_t>((instanceID >> 8) & 0xFF));
    }

    // Attribute
    if (attributeID <= 0xFF) {
        path.push_back(0x30);
        path.push_back(static_cast<uint8_t>(attributeID));
    } else {
        path.push_back(0x31); // 16-bit
        path.push_back(static_cast<uint8_t>(attributeID & 0xFF));
        path.push_back(static_cast<uint8_t>((attributeID >> 8) & 0xFF));
    }
    return path;
}


#endif

#endif