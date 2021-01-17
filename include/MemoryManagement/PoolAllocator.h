#pragma once

#include "main.h"
#include <memory> // Sollte gekapselt sein (Fremd API)

namespace bbe
{

template <typename T>
union PoolChunk {
    T value;
    PoolChunk<T> *nextPoolChunk;

    PoolChunk(){};
    ~PoolChunk(){};
};

template <typename T, typename Allocator = std::allocator<PoolChunk<T>>>
class PoolAllocator
{
private:
    static const size_t POOL_ALLOCATOR_DEFAULT_SIZE = 1024;
    size_t m_openAllocations = 0;

    size_t m_size = 0;

    PoolChunk<T> *m_data = nullptr;
    PoolChunk<T> *m_head = nullptr;

    Allocator *m_parentAllocator = nullptr;
    bool m_needsToDeleteParentAllocator = false;

public:
    explicit PoolAllocator(size_t size = POOL_ALLOCATOR_DEFAULT_SIZE, Allocator *parentAllocator = nullptr)
        : m_size(size), m_parentAllocator(parentAllocator)
    {
        if (parentAllocator == nullptr)
        {
            m_parentAllocator = new Allocator();
            m_needsToDeleteParentAllocator = true;
        }

        m_data = m_parentAllocator->allocate(m_size);
        m_head = m_data;

        for (size_t i = 0; i < m_size - 1; i++)
        {
            m_data[i].nextPoolChunk = std::addressof(m_data[i + 1]);
        }
        m_data[m_size - 1].nextPoolChunk = nullptr;
    }

    PoolAllocator(const PoolAllocator &other) = delete;            // Copy Constructor
    PoolAllocator(PoolAllocator &&other) = delete;                 // Move Constructor
    PoolAllocator &operator=(const PoolAllocator &other) = delete; // Copy Assignment
    PoolAllocator &operator=(PoolAllocator &&other) = delete;      // Move Assignment

    ~PoolAllocator()
    {
        if (m_openAllocations != 0)
        {
            // TODO: Error Handling
            DEBUG_BREAK;
        }

        m_parentAllocator->deallocate(m_data, m_size);

        if (m_needsToDeleteParentAllocator)
        {
            delete m_parentAllocator;
        }

        m_data = nullptr;
        m_head = nullptr;
    }

    template <typename... arguments>
    T *allocate(arguments &&... args)
    {
        if (m_head == nullptr)
        {
            DEBUG_BREAK;
            return nullptr;
        }

        m_openAllocations++;

        PoolChunk<T> *poolChunk = m_head;
        m_head = m_head->nextPoolChunk;

        T *retVal = new (std::addressof(poolChunk->value)) T(std::forward<arguments>(args)...);
        return retVal;
    }

    void deallocate(T *data)
    {
        // TODO: What if data is not part of m_data
        m_openAllocations--;

        data->~T();
        PoolChunk<T> *poolChunk = reinterpret_cast<PoolChunk<T> *>(data);
        poolChunk->nextPoolChunk = m_head;
        m_head = poolChunk;
    }
};

} // namespace bbe