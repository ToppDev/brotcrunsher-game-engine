#pragma once

#include "LinearAllocator.h"

namespace arcane
{

class FixedLinearAllocator : public LinearAllocator
{
public:
    FixedLinearAllocator(size_t size, void *start);
    virtual ~FixedLinearAllocator();

    virtual void *allocate(size_t size, uint8_t alignment = DEFAULT_ALIGNMENT) override;

    virtual void rewind(void *mark) override;

    void clear() override final;

private:
    FixedLinearAllocator(const FixedLinearAllocator &);
    FixedLinearAllocator &operator=(const FixedLinearAllocator &);
};

}; // namespace arcane
