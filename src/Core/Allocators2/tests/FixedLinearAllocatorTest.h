#include "../../../MainTest.h"

#include "../FixedLinearAllocator.h"

namespace arcane
{

struct FixedLinearAllocatorTest : testing::Test
{
    void *m_memory;
    FixedLinearAllocator *m_allocator;
    const size_t MEMORY_SIZE = 1024 + sizeof(FixedLinearAllocator);

    FixedLinearAllocatorTest()
    {
        m_memory = new byte[MEMORY_SIZE];

        ASSERT(m_memory && "Could not allocate m_memory from system!");

        m_allocator = new (m_memory) FixedLinearAllocator(MEMORY_SIZE - sizeof(FixedLinearAllocator),
                                                          pointer_math::add(m_memory, sizeof(FixedLinearAllocator)));
    }

    virtual ~FixedLinearAllocatorTest()
    {
        delete m_allocator;
    }
};

TEST_F(FixedLinearAllocatorTest, AllocateIntegers)
{
    auto p1 = allocator::allocateNew<uint32_t>(*m_allocator, 242);
    //GTEST_COUT << "Allocations: " << m_allocator->getNumAllocations() << " - Size: " << m_allocator->getUsedMemory() << "/" << m_allocator->getSize() << std::endl;
    EXPECT_EQ(*p1, 242);

    allocator::deallocateDelete(*m_allocator, p1);
    //GTEST_COUT << "Allocations: " << m_allocator->getNumAllocations() << " - Size: " << m_allocator->getUsedMemory() << "/" << m_allocator->getSize() << std::endl;
    m_allocator->clear();
    //GTEST_COUT << "Allocations: " << m_allocator->getNumAllocations() << " - Size: " << m_allocator->getUsedMemory() << "/" << m_allocator->getSize() << std::endl;
}

TEST_F(FixedLinearAllocatorTest, AllocateTestObjects)
{
    auto o1 = allocator::allocateNew<TestClass>(*m_allocator, 1, "Test 1");
    //GTEST_COUT << "Allocations: " << m_allocator->getNumAllocations() << " - Size: " << m_allocator->getUsedMemory() << "/" << m_allocator->getSize() << std::endl;

    auto o2 = allocator::allocateArray<TestClass>(*m_allocator, 2, 23, "Test");
    o2[0].m_count = 2;
    o2[0].m_name = "Test 2";
    o2[1].m_count = 3;
    o2[1].m_name = "Test 3";
    //GTEST_COUT << "Allocations: " << m_allocator->getNumAllocations() << " - Size: " << m_allocator->getUsedMemory() << "/" << m_allocator->getSize() << std::endl;

    void *mark = m_allocator->getMark();
    auto o3 = allocator::allocateArray<TestClass>(*m_allocator, 2, 45, "Test");
    o3[0].m_count = 4;
    o3[0].m_name = "Test 4";
    o3[1].m_count = 5;
    o3[1].m_name = "Test 5";
    //GTEST_COUT << "Allocations: " << m_allocator->getNumAllocations() << " - Size: " << m_allocator->getUsedMemory() << "/" << m_allocator->getSize() << std::endl;

    auto o4 = allocator::allocateNew<TestClass>(*m_allocator, 6, "Test 6");
    //GTEST_COUT << "Allocations: " << m_allocator->getNumAllocations() << " - Size: " << m_allocator->getUsedMemory() << "/" << m_allocator->getSize() << std::endl;

    allocator::deallocateDelete(*m_allocator, o4);
    allocator::deallocateArray(*m_allocator, o3);
    //GTEST_COUT << "Allocations: " << m_allocator->getNumAllocations() << " - Size: " << m_allocator->getUsedMemory() << "/" << m_allocator->getSize() << std::endl;
    m_allocator->rewind(mark);
    //GTEST_COUT << "Allocations: " << m_allocator->getNumAllocations() << " - Size: " << m_allocator->getUsedMemory() << "/" << m_allocator->getSize() << std::endl;

    auto o5 = allocator::allocateNew<TestClass>(*m_allocator, 7, "Test 7");
    //GTEST_COUT << "Allocations: " << m_allocator->getNumAllocations() << " - Size: " << m_allocator->getUsedMemory() << "/" << m_allocator->getSize() << std::endl;

    auto o6 = allocator::allocateArray<TestClass>(*m_allocator, 30);
    //GTEST_COUT << "Allocations: " << m_allocator->getNumAllocations() << " - Size: " << m_allocator->getUsedMemory() << "/" << m_allocator->getSize() << std::endl;

    EXPECT_EQ(o1->m_count, 1);
    EXPECT_EQ(o1->m_name, "Test 1");
    EXPECT_EQ(o2[0].m_count, 2);
    EXPECT_EQ(o2[0].m_name, "Test 2");
    EXPECT_EQ(o2[1].m_count, 3);
    EXPECT_EQ(o2[1].m_name, "Test 3");
    EXPECT_EQ(o5->m_count, 7);
    EXPECT_EQ(o5->m_name, "Test 7");
    EXPECT_EQ(o6, nullptr);

    allocator::deallocateDelete(*m_allocator, o1);
    allocator::deallocateArray(*m_allocator, o2);
    allocator::deallocateDelete(*m_allocator, o5);
    m_allocator->clear();
}

} // namespace arcane