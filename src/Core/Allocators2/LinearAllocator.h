#pragma once

#include "Allocator2.h"

namespace arcane
{

class LinearAllocator : public Allocator2
{
public:
    LinearAllocator(size_t size, void *start);
    //virtual ~LinearAllocator();

    //NoOp - Use rewind() or clear()
    void deallocate(void *p) override final;

    void *getStart() const;
    void *getMark() const;

    virtual void rewind(void *mark) = 0;
    virtual void clear() = 0;

protected:
    void *m_start;
    void *m_current_pos;
};

} // namespace arcane
