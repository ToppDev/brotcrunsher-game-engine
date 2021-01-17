#pragma once

#include "../../Utilities/PointerMath.h"
#include "../../Utilities/DataTypes.h"
#include "../../Utilities/Debug.h"

#include <memory>

namespace arcane
{

static const uint8_t DEFAULT_ALIGNMENT = 8;

class Allocator2
{
public:
    Allocator2(size_t size);

    virtual ~Allocator2();

    virtual void *allocate(size_t size, uint8_t alignment = DEFAULT_ALIGNMENT) = 0;

    virtual void deallocate(void *ptr) = 0;

    size_t getSize() const;
    size_t getUsedMemory() const;
    size_t getNumAllocations() const;

protected:
    Allocator2(const Allocator2 &);
    Allocator2 &operator=(Allocator2 &);

    size_t m_size;
    size_t m_used_memory;
    size_t m_num_allocations;
};

namespace allocator
{

template <class T, class... Args>
T *allocateNew(Allocator2 &allocator, Args &&... args);

template <class T>
void deallocateDelete(Allocator2 &allocator, T *object);

template <class T, class... Args>
T *allocateArray(Allocator2 &allocator, size_t length, Args &&... args);

template <class T>
T *allocateArrayNoConstruct(Allocator2 &allocator, size_t length);

template <class T>
void deallocateArray(Allocator2 &allocator, T *array);

template <class T>
void deallocateArrayNoDestruct(Allocator2 &allocator, T *array);

}; // namespace allocator

#include "Allocator2.inl"
}; // namespace arcane
