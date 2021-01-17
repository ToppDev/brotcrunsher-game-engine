#include "../../../MainTest.h"

#include "../FreeListAllocator.h"

namespace arcane
{

struct FreeListAllocatorTest : testing::Test
{
    void *m_memory;
    FreeListAllocator *m_allocator;
    const size_t MEMORY_SIZE = 1024 + sizeof(FreeListAllocator);
    const size_t HANDLE_STACK_SIZE = 5;

    FreeListAllocatorTest()
    {
        m_memory = new byte[MEMORY_SIZE];

        ASSERT(m_memory && "Could not allocate m_memory from system!");

        m_allocator = new (m_memory) FreeListAllocator(MEMORY_SIZE - sizeof(FreeListAllocator),
                                                       pointer_math::add(m_memory, sizeof(FreeListAllocator)), HANDLE_STACK_SIZE);
    }

    virtual ~FreeListAllocatorTest()
    {
        EXPECT_EQ(m_allocator->getNumAllocations(), 0);
        EXPECT_EQ(m_allocator->getUsedMemory(), 0);
        delete m_allocator;
    }
};

TEST_F(FreeListAllocatorTest, Allocate)
{
    auto p1 = static_cast<FreeListAllocator::AllocatorPointer<char>>(m_allocator->allocate(sizeof(char) * 10, alignof(char)));
    void *pt1 = (void *)p1.getRaw();
    //GTEST_COUT << (void *)p1.getRaw() << ": " << p1.getRaw() << std::endl;
    strcpy(p1.getRaw(), "Test Str");
    void *pt2 = (void *)p1.getRaw();
    //GTEST_COUT << (void *)p1.getRaw() << ": " << p1.getRaw() << std::endl;
    EXPECT_STREQ(p1.getRaw(), "Test Str");
    EXPECT_EQ(pt1, pt2);
    //GTEST_COUT << "Allocations: " << m_allocator->getNumAllocations() << " - Size: " << m_allocator->getUsedMemory() << "/" << m_allocator->getSize() << std::endl;

    m_allocator->deallocate((FreeListAllocator::AllocatorPointer<byte> *)&p1);
    EXPECT_EQ((void *)p1.getRaw(), nullptr);
    //GTEST_COUT << (void *)p1.getRaw() << std::endl;
    //GTEST_COUT << "Allocations: " << m_allocator->getNumAllocations() << " - Size: " << m_allocator->getUsedMemory() << "/" << m_allocator->getSize() << std::endl;
}

TEST_F(FreeListAllocatorTest, AllocateIntegers)
{
    auto p1 = m_allocator->allocateNew<uint32_t>(242);
    auto p2 = m_allocator->allocateNew<uint32_t>(300);
    //GTEST_COUT << "Allocations: " << m_allocator->getNumAllocations() << " - Size: " << m_allocator->getUsedMemory() << "/" << m_allocator->getSize() << std::endl;
    EXPECT_EQ(*p1, 242);
    EXPECT_EQ(*p2, 300);
    *p2 = 400;
    EXPECT_EQ(*p1, 242);
    EXPECT_EQ(*p2, 400);

    m_allocator->deallocateDelete(p1);
    m_allocator->deallocateDelete(p2);
    //GTEST_COUT << "Allocations: " << m_allocator->getNumAllocations() << " - Size: " << m_allocator->getUsedMemory() << "/" << m_allocator->getSize() << std::endl;
}

TEST_F(FreeListAllocatorTest, AllocateTestObjects)
{
    auto o1 = m_allocator->allocateNew<TestClass>(1, "Test 1");
    //o1->print();
    //GTEST_COUT << "Allocations: " << m_allocator->getNumAllocations() << " - Size: " << m_allocator->getUsedMemory() << "/" << m_allocator->getSize() << std::endl;

    auto o2 = m_allocator->allocateArray<TestClass>(2, 23, "Test");
    o2[0].m_count = 2;
    o2[0].m_name = "Test 2";
    o2[1].m_count = 3;
    o2[1].m_name = "Test 3";
    //o2[0].print();
    //o2[1].print();
    //GTEST_COUT << "Allocations: " << m_allocator->getNumAllocations() << " - Size: " << m_allocator->getUsedMemory() << "/" << m_allocator->getSize() << std::endl;

    auto o3 = m_allocator->allocateArray<TestClass>(30);
    //GTEST_COUT << "Allocations: " << m_allocator->getNumAllocations() << " - Size: " << m_allocator->getUsedMemory() << "/" << m_allocator->getSize() << std::endl;

    EXPECT_EQ(o1->m_count, 1);
    EXPECT_EQ(o1->m_name, "Test 1");
    EXPECT_EQ(o2[0].m_count, 2);
    EXPECT_EQ(o2[0].m_name, "Test 2");
    EXPECT_EQ(o2[1].m_count, 3);
    EXPECT_EQ(o2[1].m_name, "Test 3");
    EXPECT_EQ(o3, nullptr);

    m_allocator->deallocateDelete(o1);
    //GTEST_COUT << "Allocations: " << m_allocator->getNumAllocations() << " - Size: " << m_allocator->getUsedMemory() << "/" << m_allocator->getSize() << std::endl;
    m_allocator->deallocateArray(o2);
    //GTEST_COUT << "Allocations: " << m_allocator->getNumAllocations() << " - Size: " << m_allocator->getUsedMemory() << "/" << m_allocator->getSize() << std::endl;
}

TEST_F(FreeListAllocatorTest, AllocateTestObjectsNoConstruct)
{
    auto o1 = m_allocator->allocateArrayNoConstruct<TestClass>(2);
    //GTEST_COUT << o1[0].m_count << " - " << o1[0].m_name << std::endl;
    //GTEST_COUT << o1[1].m_count << " - " << o1[1].m_name << std::endl;
    new (std::addressof(o1[0])) TestClass(1, "Test 1");
    o1[1].m_count = 2;
    EXPECT_EQ(o1[0].m_count, 1);
    EXPECT_EQ(o1[0].m_name, "Test 1");
    EXPECT_EQ(o1[1].m_count, 2);
    //GTEST_COUT << o1[0].m_count << " - " << o1[0].m_name << std::endl;
    //GTEST_COUT << o1[1].m_count << " - " << o1[1].m_name << std::endl;
    o1[0].m_name = "Test 1 Modified";
    EXPECT_EQ(o1[0].m_name, "Test 1 Modified");
    //GTEST_COUT << o1[0].m_count << " - " << o1[0].m_name << std::endl;

    o1[0].~TestClass();
    m_allocator->deallocateArrayNoDestruct(o1);
}

TEST_F(FreeListAllocatorTest, HandleStackSizeTest)
{
    auto o1 = m_allocator->allocateArray<TestClass>(2);
    auto o2 = m_allocator->allocateArray<TestClass>(2);
    auto o3 = m_allocator->allocateArray<TestClass>(2);
    auto o4 = m_allocator->allocateNew<TestClass>();
    auto o5 = m_allocator->allocateNew<TestClass>();

    EXPECT_EQ(o5.getRaw(), nullptr);

    m_allocator->deallocateArray(o1);
    m_allocator->deallocateArray(o2);
    m_allocator->deallocateArray(o3);
    m_allocator->deallocateDelete(o4);
}

TEST_F(FreeListAllocatorTest, AllocateExactlyFull)
{
    auto o1 = m_allocator->allocateArray<char>(MEMORY_SIZE - 24 - sizeof(FreeListAllocator));
    //GTEST_COUT << "Allocations: " << m_allocator->getNumAllocations() << " - Size: " << m_allocator->getUsedMemory() << "/" << m_allocator->getSize() << std::endl;

    auto o2 = m_allocator->allocateNew<uint32_t>(1);

    EXPECT_EQ(o2, nullptr);

    m_allocator->deallocateArray(o1);
}

TEST_F(FreeListAllocatorTest, Defragmentation)
{
    auto o1 = m_allocator->allocateArray<TestClass>(2, 12, "Test 12");
    auto o2 = m_allocator->allocateArrayNoConstruct<char>(3);
    auto o3 = m_allocator->allocateNew<TestClass>(3, "Test 3");
    //GTEST_COUT << "Allocations: " << m_allocator->getNumAllocations() << " - Size: " << m_allocator->getUsedMemory() << "/" << m_allocator->getSize() << std::endl;

    m_allocator->deallocateArrayNoDestruct(o2);
    //GTEST_COUT << "Allocations: " << m_allocator->getNumAllocations() << " - Size: " << m_allocator->getUsedMemory() << "/" << m_allocator->getSize() << std::endl;

    m_allocator->defragment();

    m_allocator->deallocateArray(o1);
    m_allocator->deallocateDelete(o3);
}

} // namespace arcane