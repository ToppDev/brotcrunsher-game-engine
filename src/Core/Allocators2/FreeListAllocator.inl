#pragma once

#include "FreeListAllocator.h"

namespace arcane
{

template <class T, class... Args>
FreeListAllocator::AllocatorPointer<T> FreeListAllocator::allocateNew(Args &&... args)
{
    auto p = static_cast<FreeListAllocator::AllocatorPointer<T>>(allocate(sizeof(T), alignof(T)));

    if (p == nullptr)
    {
        return FreeListAllocator::AllocatorPointer<T>(this, 0);
    }

    new (p.getRaw()) T(std::forward<Args>(args)...);

    return p;
}

template <class T>
void FreeListAllocator::deallocateDelete(FreeListAllocator::AllocatorPointer<T> object)
{
    ASSERT(object != nullptr);

    object->~T();
    deallocate((FreeListAllocator::AllocatorPointer<byte> *)&object);
}

template <class T, class... Args>
FreeListAllocator::AllocatorPointer<T> FreeListAllocator::allocateArray(size_t length, Args &&... args)
{
    ASSERT(length > 0);

    uint8_t header_size = sizeof(size_t) / sizeof(T);

    if (sizeof(size_t) % sizeof(T) > 0)
    {
        header_size += 1;
    }

    //Allocate extra space to store array length in the bytes before the array
    auto p = static_cast<FreeListAllocator::AllocatorPointer<T>>(allocate(sizeof(T) * (length + header_size), alignof(T)));

    if (p == nullptr)
    {
        return FreeListAllocator::AllocatorPointer<T>(this, 0);
    }

    p += header_size;

    *(((size_t *)p) - 1) = length;

    for (size_t i = 0; i < length; i++)
    {
        new (std::addressof(p[i])) T(std::forward<Args>(args)...);
    }

    return p;
}

template <class T>
FreeListAllocator::AllocatorPointer<T> FreeListAllocator::allocateArrayNoConstruct(size_t length)
{
    ASSERT(length > 0);

    uint8_t header_size = sizeof(size_t) / sizeof(T);

    if (sizeof(size_t) % sizeof(T) > 0)
    {
        header_size += 1;
    }

    //Allocate extra space to store array length in the bytes before the array
    auto p = static_cast<FreeListAllocator::AllocatorPointer<T>>(allocate(sizeof(T) * (length + header_size), alignof(T)));

    if (p == nullptr)
    {
        return FreeListAllocator::AllocatorPointer<T>(this, 0);
    }

    p += header_size;

    *(((size_t *)p) - 1) = length;

    return p;
}

template <class T>
void FreeListAllocator::deallocateArray(FreeListAllocator::AllocatorPointer<T> array)
{
    ASSERT(array != nullptr);

    size_t length = *(((size_t *)array) - 1);

    for (size_t i = length; i > 0; i--)
    {
        array[i - 1].~T();
    }

    //Calculate how much extra memory was allocated to store the length before the array
    uint8_t header_size = sizeof(size_t) / sizeof(T);

    if (sizeof(size_t) % sizeof(T) > 0)
    {
        header_size += 1;
    }

    FreeListAllocator::AllocatorPointer<T> temp = array - header_size;
    deallocate((FreeListAllocator::AllocatorPointer<byte> *)&temp);
}

template <class T>
void FreeListAllocator::deallocateArrayNoDestruct(FreeListAllocator::AllocatorPointer<T> array)
{
    ASSERT(array != nullptr);

    //Calculate how much extra memory was allocated to store the length before the array
    uint8_t header_size = sizeof(size_t) / sizeof(T);

    if (sizeof(size_t) % sizeof(T) > 0)
    {
        header_size += 1;
    }

    FreeListAllocator::AllocatorPointer<T> temp = array - header_size;
    deallocate((FreeListAllocator::AllocatorPointer<byte> *)&temp);
}

}; // namespace arcane