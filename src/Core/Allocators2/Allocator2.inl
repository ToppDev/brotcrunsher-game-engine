#pragma once

namespace allocator
{

template <class T, class... Args>
T *allocateNew(Allocator2 &allocator, Args &&... args)
{
    T *p = reinterpret_cast<T *>(allocator.allocate(sizeof(T), alignof(T)));

    if (p == nullptr)
    {
        return nullptr;
    }

    return new (p) T(std::forward<Args>(args)...);
}

template <class T>
void deallocateDelete(Allocator2 &allocator, T *object)
{
    ASSERT(object != nullptr);

    object->~T();
    allocator.deallocate(object);
}

template <class T, class... Args>
T *allocateArray(Allocator2 &allocator, size_t length, Args &&... args)
{
    ASSERT(length > 0);

    uint8_t header_size = sizeof(size_t) / sizeof(T);

    if (sizeof(size_t) % sizeof(T) > 0)
    {
        header_size += 1;
    }

    //Allocate extra space to store array length in the bytes before the array
    T *p = reinterpret_cast<T *>(allocator.allocate(sizeof(T) * (length + header_size), alignof(T)));

    if (p == nullptr)
    {
        return nullptr;
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
T *allocateArrayNoConstruct(Allocator2 &allocator, size_t length)
{
    ASSERT(length > 0);

    uint8_t header_size = sizeof(size_t) / sizeof(T);

    if (sizeof(size_t) % sizeof(T) > 0)
    {
        header_size += 1;
    }

    //Allocate extra space to store array length in the bytes before the array
    T *p = reinterpret_cast<T *>(allocator.allocate(sizeof(T) * (length + header_size), alignof(T)));

    if (p == nullptr)
    {
        return nullptr;
    }

    p += header_size;

    *(((size_t *)p) - 1) = length;

    return p;
}

template <class T>
void deallocateArray(Allocator2 &allocator, T *array)
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

    allocator.deallocate(array - header_size);
}

template <class T>
void deallocateArrayNoDestruct(Allocator2 &allocator, T *array)
{
    ASSERT(array != nullptr);

    //Calculate how much extra memory was allocated to store the length before the array
    uint8_t header_size = sizeof(size_t) / sizeof(T);

    if (sizeof(size_t) % sizeof(T) > 0)
    {
        header_size += 1;
    }

    allocator.deallocate(array - header_size);
}

}; // namespace allocator
