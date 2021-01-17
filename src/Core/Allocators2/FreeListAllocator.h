#pragma once

#include "../../Utilities/PointerMath.h"
#include "../../Utilities/DataTypes.h"
#include "../../Utilities/Debug.h"

#include <vector>

namespace arcane
{

class FreeListAllocator
{
public:
    template <typename T>
    class AllocatorPointer
    {
        friend class FreeListAllocator;

    private:
        FreeListAllocator *m_parent;
        size_t m_handle_index;

    public:
        AllocatorPointer()
            : m_parent(nullptr), m_handle_index(0)
        {
        }

        AllocatorPointer(FreeListAllocator *parent, size_t handleIndex)
            : m_parent(parent), m_handle_index(handleIndex)
        {
        }

    public:                // Operators
        template <class U> // Operator for casting with template
        AllocatorPointer(AllocatorPointer<U> arg)
            : m_parent(arg.getParent()), m_handle_index(arg.getHandleIndex())
        {
        }

        T *getRaw()
        {
            return static_cast<T *>(m_parent->m_handle_table[m_handle_index]);
        }

        FreeListAllocator *getParent()
        {
            return m_parent;
        }

        size_t getHandleIndex()
        {
            return m_handle_index;
        }

        AllocatorPointer<T> &operator+=(const size_t &rhs)
        {
            m_parent->m_handle_table[m_handle_index] = static_cast<T *>(m_parent->m_handle_table[m_handle_index]) + rhs;

            return *this;
        }
        AllocatorPointer<T> &operator-=(const size_t &rhs)
        {
            m_parent->m_handle_table[m_handle_index] = static_cast<T *>(m_parent->m_handle_table[m_handle_index]) - rhs;

            return *this;
        }

        friend AllocatorPointer<T> operator+(AllocatorPointer<T> lhs, const size_t &rhs)
        {
            lhs += rhs;
            return lhs;
        }
        friend AllocatorPointer<T> operator-(AllocatorPointer<T> lhs, const size_t &rhs)
        {
            lhs -= rhs;
            return lhs;
        }

        explicit operator size_t *() const
        {
            return reinterpret_cast<size_t *>(static_cast<T *>(m_parent->m_handle_table[m_handle_index]));
        }

        T *operator->()
        {
            return static_cast<T *>(m_parent->m_handle_table[m_handle_index]);
        }

        const T *operator->() const
        {
            return static_cast<T *>(m_parent->m_handle_table[m_handle_index]);
        }

        T &operator*()
        {
            return *static_cast<T *>(m_parent->m_handle_table[m_handle_index]);
        }

        const T &operator*() const
        {
            return *static_cast<T *>(m_parent->m_handle_table[m_handle_index]);
        }

        T &operator[](int index)
        {
            return *(static_cast<T *>(m_parent->m_handle_table[m_handle_index]) + index);
        }

        const T &operator[](int index) const
        {
            return *(static_cast<T *>(m_parent->m_handle_table[m_handle_index]) + index);
        }

        bool operator==(void *ptr) const
        {
            return static_cast<T *>(m_parent->m_handle_table[m_handle_index]) == ptr;
        }

        bool operator!=(void *ptr) const
        {
            return static_cast<T *>(m_parent->m_handle_table[m_handle_index]) != ptr;
        }

        bool operator>(void *ptr) const
        {
            return static_cast<T *>(m_parent->m_handle_table[m_handle_index]) > ptr;
        }

        bool operator<(void *ptr) const
        {
            return static_cast<T *>(m_parent->m_handle_table[m_handle_index]) < ptr;
        }

        bool operator>=(void *ptr) const
        {
            return static_cast<T *>(m_parent->m_handle_table[m_handle_index]) >= ptr;
        }

        bool operator<=(void *ptr) const
        {
            return static_cast<T *>(m_parent->m_handle_table[m_handle_index]) <= ptr;
        }

        bool operator==(AllocatorPointer<T> ptr) const
        {
            return static_cast<T *>(m_parent->m_handle_table[m_handle_index]) == ptr.getRaw();
        }

        bool operator!=(AllocatorPointer<T> ptr) const
        {
            return static_cast<T *>(m_parent->m_handle_table[m_handle_index]) != ptr.getRaw();
        }

        bool operator>(AllocatorPointer<T> ptr) const
        {
            return static_cast<T *>(m_parent->m_handle_table[m_handle_index]) > ptr.getRaw();
        }

        bool operator<(AllocatorPointer<T> ptr) const
        {
            return static_cast<T *>(m_parent->m_handle_table[m_handle_index]) < ptr.getRaw();
        }

        bool operator>=(AllocatorPointer<T> ptr) const
        {
            return static_cast<T *>(m_parent->m_handle_table[m_handle_index]) >= ptr.getRaw();
        }

        bool operator<=(AllocatorPointer<T> ptr) const
        {
            return static_cast<T *>(m_parent->m_handle_table[m_handle_index]) <= ptr.getRaw();
        }
    };

    FreeListAllocator(size_t size, void *start, size_t handle_table_length);
    ~FreeListAllocator();

    FreeListAllocator::AllocatorPointer<byte> allocate(size_t size, uint8_t alignment);

    void deallocate(FreeListAllocator::AllocatorPointer<byte> *p);

    template <class T, class... Args>
    FreeListAllocator::AllocatorPointer<T> allocateNew(Args &&... args);

    template <class T>
    void deallocateDelete(FreeListAllocator::AllocatorPointer<T> object);

    template <class T, class... Args>
    FreeListAllocator::AllocatorPointer<T> allocateArray(size_t length, Args &&... args);

    template <class T>
    FreeListAllocator::AllocatorPointer<T> allocateArrayNoConstruct(size_t length);

    template <class T>
    void deallocateArray(FreeListAllocator::AllocatorPointer<T> array);

    template <class T>
    void deallocateArrayNoDestruct(FreeListAllocator::AllocatorPointer<T> array);

    bool needsDefragmentation();

    bool defragment();

    size_t getSize() const;
    size_t getUsedMemory() const;
    size_t getNumAllocations() const;

private:
    struct AllocationHeader
    {
        size_t size;
        uint8_t adjustment;
    };

    struct FreeBlock
    {
        size_t size;
        FreeBlock *next;
    };

    static_assert(sizeof(AllocationHeader) >= sizeof(FreeBlock), "sizeof(AllocationHeader) < sizeof(FreeBlock)");

    FreeListAllocator(const FreeListAllocator &);
    FreeListAllocator &operator=(const FreeListAllocator &);

    void findBestFitFreeBlock(const size_t size, const size_t alignment, uint8_t &adjustment, FreeBlock *&bestFitPrev, FreeBlock *&bestFit, size_t &totalFreeSpace);

    byte *m_start;
    size_t m_size;
    size_t m_used_memory;
    size_t m_num_allocations;

    FreeBlock *m_freeBlocks;

    std::vector<size_t> m_unusedHandleStack;

    size_t m_handle_table_length;
    void **m_handle_table;
};

}; // namespace arcane

#include "FreeListAllocator.inl"