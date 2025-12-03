/**
 * @file PrimitiveTypes.hpp
 * @author Kim, Gi-baek (gibaek0806@edgecross.ai)
 * 
 * @brief 
 * AAS 메타모델 표준에 명시된 기본 데이터 타입(Primitive Data Types)을 정의합니다.
 * 
 * @note
 * 이 파일은 AAS 메타모델의 다른 모든 클래스를 구성하는 기본 빌딩 블록을 제공하며,
 * 타입 별칭(using)을 사용하여 표준과의 일관성을 유지하고 가독성을 높입니다.
 * 
 * @date 2025-12-02
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <cstddef>
#include <memory>
#include "Common/PSRAM.hpp"

namespace muffin { namespace aas {

    class IValue; // Forward declaration

    // Primitive Type Aliases from AAS Metamodel v3.1.2
    // A central place for all primitive type definitions based on standard types.


    using ValueDataType    = psram::unique_ptr<IValue>;
    using Identifier       = psram::string;        // string with max 2048 characters
    using LabelType        = psram::string;        // string with max 64 characters
    using NameType         = psram::string;        // string with max 128 characters
    using QualifierType    = NameType;           // string with max 128 characters
    using VersionType      = psram::string;        // string with max 4 characters
    using RevisionType     = psram::string;      // string with max 4 characters
    using ContentType      = psram::string;        // string with max 128 characters (MIME type)
    using PathType         = Identifier;        // string with max 2048 characters (URI)
    using MessageTopicType = psram::string;      // string with max 255 characters
    using DateTimeUtc      = psram::string;      // dateTime for UTC
    using BlobType         = psram::vector<uint8_t>; // base64binary


    // Structure for strings with language tags based on RDF data type 'langString'
    struct LangString {
        psram::string Language;
        psram::string Text;
    };

    using LangStringSet           = psram::vector<LangString>;
    using MultiLanguageNameType   = LangStringSet; // Each 'text' has max 128 chars.
    using MultiLanguageTextType   = LangStringSet; // Each 'text' has max 1023 chars.
}}
