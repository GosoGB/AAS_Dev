/**
 * @file psramAllocator.h
 * @author Kim, Joo-sung (joosung5732@edgecross.ai)
 * 
 * @brief MT11인 경우 PSRAM을 사용할 수 있으며 기존 STD 컨테이너를 PSRAM에 올리는 커스텀 allocator 입니다.
 * 
 * @date 2025-07-28
 * @version 1.5.01
 * 
 * @copyright Copyright Edgecross Inc. (c) 2025
 */




// psram_allocator.h
#pragma once
#include <esp_heap_caps.h>
#include <new>
#include <cstddef>
#include <cstdint>

template <class T>
struct PsramAllocator 
{
  using value_type = T;

  PsramAllocator() noexcept {}
  template <class U>
  PsramAllocator(const PsramAllocator<U>&) noexcept {}

  T* allocate(std::size_t n) 
  {
    void* p = heap_caps_malloc(n * sizeof(T), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!p) throw std::bad_alloc();
    return static_cast<T*>(p);
  }

  void deallocate(T* p, std::size_t) noexcept 
  {
    heap_caps_free(p);
  }
};

template <class T, class U>
bool operator==(const PsramAllocator<T>&, const PsramAllocator<U>&) { return true; }
template <class T, class U>
bool operator!=(const PsramAllocator<T>&, const PsramAllocator<U>&) { return false; }

// helper alias
template <typename T>
using psramVector = std::vector<T, PsramAllocator<T>>;
