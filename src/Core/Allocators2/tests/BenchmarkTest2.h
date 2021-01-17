#pragma once

#include "../../../MainTest.h"

#include <time.h> // timespec
#include <vector>
#include <iostream>
#include <stdlib.h> /* srand, rand */

#include "../Allocator2.h" // base class allocator
#include "../FreeListAllocator.h"
#include "../FixedLinearAllocator.h"

namespace arcane
{

class CAllocator2 : public Allocator2
{
public:
    CAllocator2()
        : Allocator2(1)
    {
    }

    ~CAllocator2()
    {
    }

    void *allocate(size_t size, uint8_t alignment) override
    {
        // m_used can't be updated, because malloc doesn't store this info
        // and we don't wanna store it here outselves
        m_num_allocations++;

        return new byte[size];
    }

    void deallocate(void *ptr) override
    {
        m_num_allocations--;
        delete[](byte *) ptr;
    }
};

struct BenchmarkResults2
{
    long nOperations;
    double elapsedTime;
    float operationsPerSec;
    float timePerOperation;
};

class Benchmark2
{
public:
    Benchmark2(const unsigned int nOperations)
        : m_nOperations(nOperations)
    {
    }

    BenchmarkResults2 SingleAllocationFreeList(FreeListAllocator *allocator, const std::size_t size, const std::size_t alignment)
    {
        FreeListAllocator::AllocatorPointer<byte> addresses[m_nOperations];
        int operations = 0;

        setTimer(m_start);
        while (operations < m_nOperations)
        {
            addresses[operations] = allocator->allocate(size, alignment);
            //ASSERT(addresses[operations] != nullptr);
            ++operations;
        }
        setTimer(m_end);

        --operations;
        while (operations >= 0)
        {
            allocator->deallocate(&addresses[operations]);
            --operations;
        }

        BenchmarkResults2 results = buildResults(m_nOperations, calculateElapsedTime());

        return results;
    }
    BenchmarkResults2 SingleFreeFreeList(FreeListAllocator *allocator, const std::size_t size, const std::size_t alignment)
    {
        FreeListAllocator::AllocatorPointer<byte> addresses[m_nOperations];
        int operations = 0;

        setTimer(m_start);
        while (operations < m_nOperations)
        {
            addresses[operations] = allocator->allocate(size, alignment);
            //ASSERT(addresses[operations] != nullptr);
            ++operations;
        }
        --operations;
        while (operations >= 0)
        {
            allocator->deallocate(&addresses[operations]);
            --operations;
        }
        setTimer(m_end);

        BenchmarkResults2 results = buildResults(m_nOperations, calculateElapsedTime());

        return results;
    }
    template <typename T>
    BenchmarkResults2 SingleAllocationObjectsFreeList(FreeListAllocator *allocator, const std::size_t count)
    {
        FreeListAllocator::AllocatorPointer<T> addresses[m_nOperations];
        int operations = 0;

        setTimer(m_start);
        while (operations < m_nOperations)
        {
            addresses[operations] = allocator->allocateArray<T>(count);
            //ASSERT(addresses[operations] != nullptr);
            ++operations;
        }
        setTimer(m_end);

        --operations;
        while (operations >= 0)
        {
            allocator->deallocateArray(addresses[operations]);
            --operations;
        }

        BenchmarkResults2 results = buildResults(m_nOperations, calculateElapsedTime());

        return results;
    }
    template <typename T>
    BenchmarkResults2 SingleFreeObjectsFreeList(FreeListAllocator *allocator, const std::size_t count)
    {
        FreeListAllocator::AllocatorPointer<T> addresses[m_nOperations];
        int operations = 0;

        setTimer(m_start);
        while (operations < m_nOperations)
        {
            addresses[operations] = allocator->allocateArray<T>(count);
            //ASSERT(addresses[operations] != nullptr);
            ++operations;
        }
        --operations;
        while (operations >= 0)
        {
            allocator->deallocateArray(addresses[operations]);
            --operations;
        }
        setTimer(m_end);

        BenchmarkResults2 results = buildResults(m_nOperations, calculateElapsedTime());

        return results;
    }
    template <typename T>
    BenchmarkResults2 SingleAllocationNoCFreeList(FreeListAllocator *allocator, const std::size_t count)
    {
        FreeListAllocator::AllocatorPointer<T> addresses[m_nOperations];
        int operations = 0;

        setTimer(m_start);
        while (operations < m_nOperations)
        {
            addresses[operations] = allocator->allocateArrayNoConstruct<T>(count);
            //ASSERT(addresses[operations] != nullptr);
            ++operations;
        }
        setTimer(m_end);

        --operations;
        while (operations >= 0)
        {
            allocator->deallocateArrayNoDestruct(addresses[operations]);
            --operations;
        }

        BenchmarkResults2 results = buildResults(m_nOperations, calculateElapsedTime());

        return results;
    }
    template <typename T>
    BenchmarkResults2 SingleFreeObjectsNoCFreeList(FreeListAllocator *allocator, const std::size_t count)
    {
        FreeListAllocator::AllocatorPointer<T> addresses[m_nOperations];
        int operations = 0;

        setTimer(m_start);
        while (operations < m_nOperations)
        {
            addresses[operations] = allocator->allocateArrayNoConstruct<T>(count);
            //ASSERT(addresses[operations] != nullptr);
            ++operations;
        }
        --operations;
        while (operations >= 0)
        {
            allocator->deallocateArrayNoDestruct(addresses[operations]);
            --operations;
        }
        setTimer(m_end);

        BenchmarkResults2 results = buildResults(m_nOperations, calculateElapsedTime());

        return results;
    }

    BenchmarkResults2 SingleAllocation(Allocator2 *allocator, const std::size_t size, const std::size_t alignment)
    {
        void *addresses[m_nOperations];
        int operations = 0;

        setTimer(m_start);
        while (operations < m_nOperations)
        {
            addresses[operations] = allocator->allocate(size, alignment);
            ++operations;
        }
        setTimer(m_end);

        --operations;
        while (operations >= 0)
        {
            allocator->deallocate(addresses[operations]);
            --operations;
        }

        BenchmarkResults2 results = buildResults(m_nOperations, calculateElapsedTime());

        return results;
    }
    BenchmarkResults2 SingleFree(Allocator2 *allocator, const std::size_t size, const std::size_t alignment)
    {
        void *addresses[m_nOperations];
        int operations = 0;

        setTimer(m_start);
        while (operations < m_nOperations)
        {
            addresses[operations] = allocator->allocate(size, alignment);
            ++operations;
        }
        --operations;
        while (operations >= 0)
        {
            allocator->deallocate(addresses[operations]);
            --operations;
        }
        setTimer(m_end);

        BenchmarkResults2 results = buildResults(m_nOperations, calculateElapsedTime());

        return results;
    }
    template <typename T>
    BenchmarkResults2 SingleAllocationObjects(Allocator2 *allocator, const std::size_t count)
    {
        T *addresses[m_nOperations];
        int operations = 0;

        setTimer(m_start);
        while (operations < m_nOperations)
        {
            addresses[operations] = allocator::allocateArray<T>(*allocator, count);
            ++operations;
        }
        setTimer(m_end);

        --operations;
        while (operations >= 0)
        {
            allocator::deallocateArray(*allocator, addresses[operations]);
            --operations;
        }

        BenchmarkResults2 results = buildResults(m_nOperations, calculateElapsedTime());

        return results;
    }
    template <typename T>
    BenchmarkResults2 SingleFreeObjects(Allocator2 *allocator, const std::size_t count)
    {
        T *addresses[m_nOperations];
        int operations = 0;

        setTimer(m_start);
        while (operations < m_nOperations)
        {
            addresses[operations] = allocator::allocateArray<T>(*allocator, count);
            ++operations;
        }
        --operations;
        while (operations >= 0)
        {
            allocator::deallocateArray(*allocator, addresses[operations]);
            --operations;
        }
        setTimer(m_end);

        BenchmarkResults2 results = buildResults(m_nOperations, calculateElapsedTime());

        return results;
    }
    template <typename T>
    BenchmarkResults2 SingleAllocationNoC(Allocator2 *allocator, const std::size_t count)
    {
        T *addresses[m_nOperations];
        int operations = 0;

        setTimer(m_start);
        while (operations < m_nOperations)
        {
            addresses[operations] = allocator::allocateArrayNoConstruct<T>(*allocator, count);
            ++operations;
        }
        setTimer(m_end);

        --operations;
        while (operations >= 0)
        {
            allocator::deallocateArrayNoDestruct(*allocator, addresses[operations]);
            --operations;
        }

        BenchmarkResults2 results = buildResults(m_nOperations, calculateElapsedTime());

        return results;
    }
    template <typename T>
    BenchmarkResults2 SingleFreeObjectsNoC(Allocator2 *allocator, const std::size_t count)
    {
        T *addresses[m_nOperations];
        int operations = 0;

        setTimer(m_start);
        while (operations < m_nOperations)
        {
            addresses[operations] = allocator::allocateArrayNoConstruct<T>(*allocator, count);
            ++operations;
        }
        --operations;
        while (operations >= 0)
        {
            allocator::deallocateArrayNoDestruct(*allocator, addresses[operations]);
            --operations;
        }
        setTimer(m_end);

        BenchmarkResults2 results = buildResults(m_nOperations, calculateElapsedTime());

        return results;
    }

private:
    void setTimer(timespec &timer)
    {
        clock_gettime(CLOCK_REALTIME, &timer);
    }
    double calculateElapsedTime() const
    {
        timespec temp;
        if ((m_end.tv_nsec - m_start.tv_nsec) < 0)
        {
            temp.tv_sec = m_end.tv_sec - m_start.tv_sec - 1;
            temp.tv_nsec = 1e9 + m_end.tv_nsec - m_start.tv_nsec;
        }
        else
        {
            temp.tv_sec = m_end.tv_sec - m_start.tv_sec;
            temp.tv_nsec = m_end.tv_nsec - m_start.tv_nsec;
        }

        const double time_sec = (double)temp.tv_sec;
        const double time_nsec = (double)temp.tv_nsec;
        const double time_msec = (time_sec * 1e3) + (time_nsec / 1e6);

        return time_msec;
    }
    const BenchmarkResults2 buildResults(const unsigned int nOperations, const double elapsedTime) const
    {
        BenchmarkResults2 results;

        results.nOperations = nOperations;
        results.elapsedTime = elapsedTime;
        results.operationsPerSec = results.nOperations / results.elapsedTime;
        results.timePerOperation = results.elapsedTime / results.nOperations;

        return results;
    }

private:
    unsigned int m_nOperations;
    timespec m_start, m_end;
};

struct BenchmarkTest2 : testing::Test
{
    Benchmark2 *benchmark;
    const std::vector<std::size_t> ALLOCATION_SIZES{32, 64, 256, 512, 1024, 2048, 4096};
    const std::vector<std::size_t> ALIGNMENTS{8, 8, 8, 8, 8, 8, 8};
    const uint32_t ALLOCATION_SIZE = 2048;
    const uint32_t ALLOCATION_OBJECT_COUNT = 51;
    const uint32_t ALIGNMENT = 8;
    const uint32_t ALLOCATION_AMOUNT = 1e4;

    BenchmarkTest2()
    {
        benchmark = new Benchmark2(ALLOCATION_AMOUNT);
    }

    virtual ~BenchmarkTest2()
    {
        delete benchmark;
    }
};

static double cElapsedTimeAlloc2;
static double cElapsedTimeAlloc2Free;
static double cElapsedTimeAlloc2Object;
static double cElapsedTimeAlloc2FreeObject;
static double cElapsedTimeAlloc2NoC;
static double cElapsedTimeAlloc2FreeNoC;

TEST_F(BenchmarkTest2, CAllocator2)
{
    CAllocator2 *allocator = new CAllocator2();

    BenchmarkResults2 resultAllocation = benchmark->SingleAllocation(allocator, ALLOCATION_SIZE, ALIGNMENT);
    BenchmarkResults2 resultAllocFree = benchmark->SingleFree(allocator, ALLOCATION_SIZE, ALIGNMENT);

    BenchmarkResults2 resultAllocationObject = benchmark->SingleAllocationObjects<TestClass>(allocator, ALLOCATION_OBJECT_COUNT);
    BenchmarkResults2 resultAllocFreeObject = benchmark->SingleFreeObjects<TestClass>(allocator, ALLOCATION_OBJECT_COUNT);

    BenchmarkResults2 resultAllocationNoC = benchmark->SingleAllocationNoC<TestClass>(allocator, ALLOCATION_OBJECT_COUNT);
    BenchmarkResults2 resultAllocFreeNoC = benchmark->SingleFreeObjectsNoC<TestClass>(allocator, ALLOCATION_OBJECT_COUNT);

    GTEST_COUT << "Allocation : " << ALLOCATION_SIZE << " byte (" << resultAllocation.nOperations << "x): " << resultAllocation.elapsedTime << "ms" << std::endl;
    GTEST_COUT << "Alloc/Free : " << ALLOCATION_SIZE << " byte (" << resultAllocFree.nOperations << "x): " << resultAllocFree.elapsedTime << "ms" << std::endl;
    GTEST_COUT << "AllocNoC:    " << sizeof(TestClass) * ALLOCATION_OBJECT_COUNT << " byte (" << resultAllocationNoC.nOperations << "x): " << resultAllocationNoC.elapsedTime << "ms" << std::endl;
    GTEST_COUT << "Free NoC:    " << sizeof(TestClass) * ALLOCATION_OBJECT_COUNT << " byte (" << resultAllocFreeNoC.nOperations << "x): " << resultAllocFreeNoC.elapsedTime << "ms" << std::endl;
    GTEST_COUT << "AllocObject: " << sizeof(TestClass) * ALLOCATION_OBJECT_COUNT << " byte (" << resultAllocationObject.nOperations << "x): " << resultAllocationObject.elapsedTime << "ms" << std::endl;
    GTEST_COUT << "Free Object: " << sizeof(TestClass) * ALLOCATION_OBJECT_COUNT << " byte (" << resultAllocFreeObject.nOperations << "x): " << resultAllocFreeObject.elapsedTime << "ms" << std::endl;

    cElapsedTimeAlloc2 = resultAllocation.elapsedTime;
    cElapsedTimeAlloc2Free = resultAllocFree.elapsedTime;
    cElapsedTimeAlloc2Object = resultAllocationObject.elapsedTime;
    cElapsedTimeAlloc2FreeObject = resultAllocFreeObject.elapsedTime;
    cElapsedTimeAlloc2NoC = resultAllocationNoC.elapsedTime;
    cElapsedTimeAlloc2FreeNoC = resultAllocFreeNoC.elapsedTime;

    delete allocator;
}

TEST_F(BenchmarkTest2, FreeListAllocator)
{
    const size_t MEMORY_SIZE = ALLOCATION_SIZE * ALLOCATION_AMOUNT + 200 + 48 * ALLOCATION_AMOUNT;
    void *memory = new byte[MEMORY_SIZE];
    ASSERT(memory && "Could not allocate memory from system!");

    FreeListAllocator *allocator = new (memory) FreeListAllocator(MEMORY_SIZE - sizeof(FreeListAllocator),
                                                                  pointer_math::add(memory, sizeof(FreeListAllocator)), ALLOCATION_AMOUNT + 1);

    // Somehow here the time skyrockets for the next test
    benchmark->SingleAllocationFreeList(allocator, ALLOCATION_SIZE, ALIGNMENT);

    BenchmarkResults2 resultAllocation = benchmark->SingleAllocationFreeList(allocator, ALLOCATION_SIZE, ALIGNMENT);
    BenchmarkResults2 resultAllocFree = benchmark->SingleFreeFreeList(allocator, ALLOCATION_SIZE, ALIGNMENT);

    BenchmarkResults2 resultAllocationObject = benchmark->SingleAllocationObjectsFreeList<TestClass>(allocator, ALLOCATION_OBJECT_COUNT);
    BenchmarkResults2 resultAllocFreeObject = benchmark->SingleFreeObjectsFreeList<TestClass>(allocator, ALLOCATION_OBJECT_COUNT);

    BenchmarkResults2 resultAllocationNoC = benchmark->SingleAllocationNoCFreeList<TestClass>(allocator, ALLOCATION_OBJECT_COUNT);
    BenchmarkResults2 resultAllocFreeNoC = benchmark->SingleFreeObjectsNoCFreeList<TestClass>(allocator, ALLOCATION_OBJECT_COUNT);

    GTEST_COUT << "Allocation : " << ALLOCATION_SIZE << " byte (" << resultAllocation.nOperations << "x): " << resultAllocation.elapsedTime << "ms" << std::endl;
    GTEST_COUT << "Alloc/Free : " << ALLOCATION_SIZE << " byte (" << resultAllocFree.nOperations << "x): " << resultAllocFree.elapsedTime << "ms" << std::endl;
    GTEST_COUT << "AllocNoC:    " << sizeof(TestClass) * ALLOCATION_OBJECT_COUNT << " byte (" << resultAllocationNoC.nOperations << "x): " << resultAllocationNoC.elapsedTime << "ms" << std::endl;
    GTEST_COUT << "Free NoC:    " << sizeof(TestClass) * ALLOCATION_OBJECT_COUNT << " byte (" << resultAllocFreeNoC.nOperations << "x): " << resultAllocFreeNoC.elapsedTime << "ms" << std::endl;
    GTEST_COUT << "AllocObject: " << sizeof(TestClass) * ALLOCATION_OBJECT_COUNT << " byte (" << resultAllocationObject.nOperations << "x): " << resultAllocationObject.elapsedTime << "ms" << std::endl;
    GTEST_COUT << "Free Object: " << sizeof(TestClass) * ALLOCATION_OBJECT_COUNT << " byte (" << resultAllocFreeObject.nOperations << "x): " << resultAllocFreeObject.elapsedTime << "ms" << std::endl;

    EXPECT_LE(resultAllocation.elapsedTime, cElapsedTimeAlloc2);
    EXPECT_LE(resultAllocFree.elapsedTime, cElapsedTimeAlloc2Free);
    EXPECT_LE(resultAllocationNoC.elapsedTime, cElapsedTimeAlloc2NoC);
    EXPECT_LE(resultAllocFreeNoC.elapsedTime, cElapsedTimeAlloc2FreeNoC);
    EXPECT_LE(resultAllocationObject.elapsedTime, cElapsedTimeAlloc2Object);
    EXPECT_LE(resultAllocFreeObject.elapsedTime, cElapsedTimeAlloc2FreeObject);

    delete allocator;
}

TEST_F(BenchmarkTest2, FixedLinearAllocator)
{
    const size_t MEMORY_SIZE = ALLOCATION_SIZE * ALLOCATION_AMOUNT + sizeof(FixedLinearAllocator) + 32 * ALLOCATION_AMOUNT;
    void *memory = new byte[MEMORY_SIZE];

    ASSERT(memory && "Could not allocate memory from system!");

    FixedLinearAllocator *allocator = new (memory) FixedLinearAllocator(MEMORY_SIZE - sizeof(FixedLinearAllocator),
                                                                        pointer_math::add(memory, sizeof(FixedLinearAllocator)));

    BenchmarkResults2 resultAllocation = benchmark->SingleAllocation(allocator, ALLOCATION_SIZE, ALIGNMENT);
    allocator->clear();
    BenchmarkResults2 resultAllocFree = benchmark->SingleFree(allocator, ALLOCATION_SIZE, ALIGNMENT);
    allocator->clear();

    // Somehow here the time skyrockets for the next test
    benchmark->SingleAllocationNoC<TestClass>(allocator, ALLOCATION_OBJECT_COUNT);
    allocator->clear();

    BenchmarkResults2 resultAllocationNoC = benchmark->SingleAllocationNoC<TestClass>(allocator, ALLOCATION_OBJECT_COUNT);
    allocator->clear();
    BenchmarkResults2 resultAllocFreeNoC = benchmark->SingleFreeObjectsNoC<TestClass>(allocator, ALLOCATION_OBJECT_COUNT);
    allocator->clear();

    BenchmarkResults2 resultAllocationObject = benchmark->SingleAllocationObjects<TestClass>(allocator, ALLOCATION_OBJECT_COUNT);
    allocator->clear();
    BenchmarkResults2 resultAllocFreeObject = benchmark->SingleFreeObjects<TestClass>(allocator, ALLOCATION_OBJECT_COUNT);
    allocator->clear();

    GTEST_COUT << "Allocation : " << ALLOCATION_SIZE << " byte (" << resultAllocation.nOperations << "x): " << resultAllocation.elapsedTime << "ms" << std::endl;
    GTEST_COUT << "Alloc/Free : " << ALLOCATION_SIZE << " byte (" << resultAllocFree.nOperations << "x): " << resultAllocFree.elapsedTime << "ms" << std::endl;
    GTEST_COUT << "AllocNoC:    " << sizeof(TestClass) * ALLOCATION_OBJECT_COUNT << " byte (" << resultAllocationNoC.nOperations << "x): " << resultAllocationNoC.elapsedTime << "ms" << std::endl;
    GTEST_COUT << "Free NoC:    " << sizeof(TestClass) * ALLOCATION_OBJECT_COUNT << " byte (" << resultAllocFreeNoC.nOperations << "x): " << resultAllocFreeNoC.elapsedTime << "ms" << std::endl;
    GTEST_COUT << "AllocObject: " << sizeof(TestClass) * ALLOCATION_OBJECT_COUNT << " byte (" << resultAllocationObject.nOperations << "x): " << resultAllocationObject.elapsedTime << "ms" << std::endl;
    GTEST_COUT << "Free Object: " << sizeof(TestClass) * ALLOCATION_OBJECT_COUNT << " byte (" << resultAllocFreeObject.nOperations << "x): " << resultAllocFreeObject.elapsedTime << "ms" << std::endl;

    EXPECT_LE(resultAllocation.elapsedTime, cElapsedTimeAlloc2);
    EXPECT_LE(resultAllocFree.elapsedTime, cElapsedTimeAlloc2Free);
    EXPECT_LE(resultAllocationNoC.elapsedTime, cElapsedTimeAlloc2NoC);
    EXPECT_LE(resultAllocFreeNoC.elapsedTime, cElapsedTimeAlloc2FreeNoC);
    EXPECT_LE(resultAllocationObject.elapsedTime, cElapsedTimeAlloc2Object);
    EXPECT_LE(resultAllocFreeObject.elapsedTime, cElapsedTimeAlloc2FreeObject);

    delete allocator;
}

} // namespace arcane