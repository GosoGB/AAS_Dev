#ifndef CIP_PATH_H
#define CIP_PATH_H

#include <string>
#include <vector>

// TAG 이름을 CIP 경로(Symbolic Segment)로 인코딩
inline std::vector<uint8_t> encodeTagPath(const std::string& tagName) {
    std::vector<uint8_t> path;
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

// CIP 경로를 문자열 TAG 이름으로 디코딩 (디버깅용)
inline std::string decodeTagPath(const std::vector<uint8_t>& path) {
    if (path.size() < 2 || path[0] != 0x91) return "";
    uint8_t len = path[1];
    return std::string(path.begin() + 2, path.begin() + 2 + len);
}

// Class/Instance/Attribute 기반 CIP Logical Path 생성 , 1756-pm020_-en-p.pdf page 14
// 8-bit, 16-bit 대응
inline std::vector<uint8_t> buildLogicalPath(uint16_t classID, uint16_t instanceID, uint16_t attributeID) {
    std::vector<uint8_t> path;

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


// 배열 인덱스를 지정하는 Logical Segment 생성
// 1756-pm020_-en-p.pdf page 14
inline std::vector<uint8_t> buildElementSelector(uint32_t index) {
    std::vector<uint8_t> seg;
    if (index <= 0xFF) {
        seg.push_back(0x28); // 8-bit Element ID
        seg.push_back(static_cast<uint8_t>(index));
    } else if (index <= 0xFFFF) {
        seg.push_back(0x29); // 16-bit Element ID
        seg.push_back(index & 0xFF);
        seg.push_back((index >> 8) & 0xFF);
    } else {
        seg.push_back(0x2A); // 32-bit Element ID
        seg.push_back(index & 0xFF);
        seg.push_back((index >> 8) & 0xFF);
        seg.push_back((index >> 16) & 0xFF);
        seg.push_back((index >> 24) & 0xFF);
    }
    return seg;
}

#endif