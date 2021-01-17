#include "FixedLinearAllocator.h"

using namespace arcane;

FixedLinearAllocator::FixedLinearAllocator(size_t size, void *start)
    : LinearAllocator(size, start)
{
}

FixedLinearAllocator::~FixedLinearAllocator()
{
}

void *FixedLinearAllocator::allocate(size_t size, uint8_t alignment)
{
    ASSERT(size != 0 && alignment != 0);

    byte *allocationLocation = reinterpret_cast<byte *>(pointer_math::alignForward(m_current_pos, alignment));
    byte *newHeadPointer = allocationLocation + size;

    if (newHeadPointer <= (byte *)m_start + m_size)
    {
        m_used_memory += size;

        m_current_pos = newHeadPointer;
        return allocationLocation;
    }
    else
    {
        return nullptr;
    }
}

void FixedLinearAllocator::rewind(void *p)
{
    ASSERT(m_start <= p);
    
    if(m_current_pos <= p)
    {
        return;
    }

    m_current_pos = p;
    m_used_memory = (uintptr_t)m_current_pos - (uintptr_t)m_start;
}

void FixedLinearAllocator::clear()
{
    m_num_allocations = 0;
    m_used_memory = 0;

    m_current_pos = m_start;
}
