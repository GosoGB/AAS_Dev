/**
 * @file Value.hpp
 * @author Kim, Gi-baek (gibaek0806@edgecross.ai)
 * 
 * @brief 
 * AAS 메타모델의 ValueDataType을 위한 다형적 값 래퍼 클래스를 정의합니다.
 * 
 * @note
 * 이 파일은 DataTypeDefXsd에 따라 다양한 실제 데이터 타입을 런타임에 처리하기 위해
 * 템플릿과 객체 지향의 다형성을 이용합니다. IValue 인터페이스를 기반으로,
 * Value<Xsd> 템플릿 클래스가 실제 데이터 타입의 값을 저장합니다.
 * 
 * @date 2025-12-03
 * @version 0.1.0
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */
#pragma once

#include "IEC63278/Metamodel/TypeDefinitions.hpp"
#include "IEC63278/Metamodel/Utility/XsdTypeMapper.hpp"
#include "Common/PSRAM.hpp"
#include "IEC63278/Metamodel/PrimitiveTypes.hpp"

namespace muffin { namespace aas {

    /**
     * @brief 다형적 값을 위한 추상 기본 인터페이스
     */
    class IValue 
    {
    public:
        virtual ~IValue() = default;

        /**
         * @brief 현재 값 객체를 복제하여 새로운 unique_ptr을 반환합니다.
         * @return ValueDataType 복제된 값 객체
         */
        virtual ValueDataType Clone() const = 0;

        /**
         * @brief 이 값이 나타내는 AAS 데이터 타입을 반환합니다.
         * @return data_type_def_xsd_e 열거형 값
         */
        virtual data_type_def_xsd_e GetDataType() const = 0;
    };

    /**
     * @brief 특정 XSD 타입의 값을 저장하는 템플릿 클래스
     * @tparam Xsd 컴파일 시간에 결정되는 AAS 데이터 타입
     */
    template <data_type_def_xsd_e Xsd>
    class Value : public IValue 
    {
    public:
        // xsd_type_mapper를 통해 XSD 타입에 해당하는 C++ 타입을 가져옵니다.
        using type = typename xsd_type_mapper<Xsd>::type;

        /**
         * @brief 값을 인자로 받아 Value 객체를 생성합니다.
         * @param value 저장할 값
         */
        explicit Value(const type& value) : mValue(value) {}
        explicit Value(type&& value) : mValue(std::move(value)) {}

        /**
         * @brief 객체를 복제합니다.
         * @return ValueDataType 복제된 객체를 가리키는 unique_ptr
         */
        ValueDataType Clone() const override 
        {
            return psram::make_unique<Value<Xsd>>(mValue);
        }

        /**
         * @brief 객체의 데이터 타입을 반환합니다.
         * @return data_type_def_xsd_e 템플릿 인자로 사용된 데이터 타입
         */
        data_type_def_xsd_e GetDataType() const override 
        {
            return Xsd;
        }

        /**
         * @brief 저장된 값을 '읽기 전용'으로 반환합니다.
         * @return const type& 저장된 값에 대한 const 참조
         */
        const type& GetValue() const 
        {
            return mValue;
        }

        /**
         * @brief 새로운 값으로 교체합니다.
         * @param newValue 교체할 새로운 값
         */
        void SetValue(const type& newValue)
        {
            mValue = newValue;
        }

    private:
        type mValue; // 실제 값이 저장되는 멤버 변수
    };

    /**
     * @brief Value 객체를 생성하는 헬퍼 함수
     * @tparam Xsd 생성할 값의 AAS 데이터 타입
     * @param value 생성자에 전달할 값
     * @return ValueDataType 생성된 객체를 가리키는 unique_ptr
     */
    template <data_type_def_xsd_e Xsd>
    ValueDataType MakeValue(typename Value<Xsd>::type&& value)
    {
        return psram::make_unique<Value<Xsd>>(std::move(value));
    }

    template <data_type_def_xsd_e Xsd>
    ValueDataType MakeValue(const typename Value<Xsd>::type& value)
    {
        return psram::make_unique<Value<Xsd>>(value);
    }
}}