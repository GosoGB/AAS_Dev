/**
 * @file PSRAM.hpp
 * @author Lee, Sang-jin (lsj31@edgecross.ai)
 * 
 * @brief 
 * Provides custom allocators and deleters for using PSRAM with STL containers and smart pointers.
 * 
 * @note
 * This implementation follows a no-throw policy, returning nullptr on allocation failure.
 * 
 * @date 2025-07-29
 * @version 0.0.1
 * 
 * @copyright Copyright (c) 2025 EdgeCross Inc.
 */




#if (MODEL == MT11)

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <esp_heap_caps.h>



namespace muffin { namespace psram {


    inline void* allocate(size_t size)
    {
        return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
    }

    inline void deallocate(void* p)
    {
        free(p);
    }

    template <typename T>
    struct Deleter
    {
        void operator()(T* p) const
        {
            if (p)
            {
                p->~T();
                deallocate(p);
            }
        }
    };

    template <class T>
    struct Allocator
    {
        typedef T value_type;

        Allocator() = default;
        template <class U> constexpr Allocator(const Allocator<U>&) noexcept {}

        T* allocate(std::size_t n)
        {
            if (n > (std::size_t(-1) / sizeof(T)))
            {
                return nullptr;
            }

            void* p = psram::allocate(n * sizeof(T));
            if (!p)
            {
                return nullptr;
            }

            return static_cast<T*>(p);
        }

        void deallocate(T* p, std::size_t /*n*/) noexcept
        {
            psram::deallocate(p);
        }
    };

    template <class T, class U> bool operator==(const Allocator<T>&, const Allocator<U>&)
    {
        return true;
    }

    template <class T, class U> bool operator!=(const Allocator<T>&, const Allocator<U>&)
    {
        return false;
    }

    // --- Type Aliases for convenience ---
    template<typename T> using unique_ptr = std::unique_ptr<T, Deleter<T>>;
    template<typename T> using vector = std::vector<T, Allocator<T>>;
    using string = std::basic_string<char, std::char_traits<char>, Allocator<char>>;


    // --- Helper function for creation ---
    template <typename T, typename... Args>
    unique_ptr<T> make_unique(Args&&... args)
    {
        void* mem = allocate(sizeof(T));

        if (!mem)
        {
            return nullptr;
        }

        return unique_ptr<T>(new (mem) T(std::forward<Args>(args)...));
    }
}}


#endif