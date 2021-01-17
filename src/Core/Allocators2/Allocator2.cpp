#include "Allocator2.h"

using namespace arcane;

Allocator2::Allocator2(size_t size)
    : m_size(size), m_used_memory(0), m_num_allocations(0)
{
    ASSERT(size > 0);
}

Allocator2::~Allocator2()
{
    ASSERT(m_num_allocations == 0 && m_used_memory == 0);

    m_size = 0;
}

size_t Allocator2::getSize() const
{
    return m_size;
}

size_t Allocator2::getUsedMemory() const
{
    return m_used_memory;
}

size_t Allocator2::getNumAllocations() const
{
    return m_num_allocations;
}