#include "LinearAllocator.h"

using namespace arcane;

LinearAllocator::LinearAllocator(size_t size, void *start)
    : Allocator2(size), m_start(start), m_current_pos(start)
{
}

void LinearAllocator::deallocate(void *p)
{
    ASSERT(p && "Deallocation of nullptr is not allowed!");
    //ASSERT("Cannot call deallocate on Linear Allocators" && false);
}

void *LinearAllocator::getStart() const
{
    return m_start;
}

void *LinearAllocator::getMark() const
{
    return m_current_pos;
}
