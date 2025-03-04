/**
 * @file CRC32.h
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief CRC32 체크섬을 계산하는 클래스를 선언합니다.
 * 
 * @date 2025-01-20
 * @version 1.2.2
 * 
 * @copyright Copyright (c) Edgecross Inc. 2024-2025
 */




#pragma once

#include <string>



namespace muffin {

    class CRC32
    {
    public:
        CRC32() {}
        virtual ~CRC32() {}
    public:
        /**
         * @brief CRC32 테이블을 초기화합니다.
         */
        void Init();

        /**
         * @brief Total CRC32 값을 계산합니다.
         * @note 본 함수를 호출한 이후에 추가적으로 Calculate 함수를 
         *       호출하는 경우 Total CRC32 값은 부정확합니다.
         */
        void Teardown();

        /**
         * @brief 주어진 데이터에 대한 CRC32 값을 계산합니다.
         * @note Teardown 함수가 호출되기 전까지 mTotalChecksum
         *       변수의 값은 지속적으로 업데이트 됩니다.
         */
        uint32_t Calculate(const std::string& data);

        /**
         * @brief 주어진 데이터에 대한 CRC32 값을 계산합니다.
         * @note Teardown 함수가 호출되기 전까지 mTotalChecksum
         *       변수의 값은 지속적으로 업데이트 됩니다.
         */
        uint32_t Calculate(const size_t size, uint8_t data[]);

        /**
         * @brief Total CRC32 값을 반환합니다.
         */
        uint32_t RetrieveTotalChecksum() const;

        void Reset();
    private:
        uint32_t mCRC32Table[256];
        uint32_t mTotalChecksum = 0xFFFFFFFF;
        uint32_t mChunkChecksum = 0xFFFFFFFF;
    };
}