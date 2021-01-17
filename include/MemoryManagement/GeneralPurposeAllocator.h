#pragma once

#include "main.h"
#include "Util/UtilMath.h"
#include "DataStructures/List.h"
#include "Util/DataTypes.h"
#include "DataStructures/Stack.h"

#include <cstring>

namespace bbe
{

namespace INTERNAL
{

class GeneralPurposeAllocatorFreeChunk
{
public:
    byte *m_addr;
    size_t m_size;
    GeneralPurposeAllocatorFreeChunk(byte *addr, size_t size)
        : m_addr(addr), m_size(size)
    {
    }

    bool touches(const GeneralPurposeAllocatorFreeChunk &other) const
    {
        if (m_addr + m_size == other.m_addr)
        {
            return true;
        }

        if (other.m_addr + other.m_size == m_addr)
        {
            return true;
        }

        return false;
    }

    bool operator>(const GeneralPurposeAllocatorFreeChunk &other) const
    {
        return m_addr > other.m_addr;
    }
    bool operator<(const GeneralPurposeAllocatorFreeChunk &other) const
    {
        return m_addr < other.m_addr;
    }
    bool operator>=(const GeneralPurposeAllocatorFreeChunk &other) const
    {
        return m_addr >= other.m_addr;
    }
    bool operator<=(const GeneralPurposeAllocatorFreeChunk &other) const
    {
        return m_addr <= other.m_addr;
    }
    bool operator==(const GeneralPurposeAllocatorFreeChunk &other) const
    {
        return m_addr == other.m_addr;
    }
    bool operator!=(const GeneralPurposeAllocatorFreeChunk &other) const
    {
        return m_addr != other.m_addr;
    }

    template <typename T, typename... arguments>
    T *allocateObject(size_t amountOfObjects = 1, arguments &&... args)
    {
        // Static assert will be translated from compiler, so no runtime losses
        static_assert(alignof(T) <= 128, "Max alignment of 128 was exceeded!"); // byte = 256 --> can only align objects with half the alignment

        byte *allocationLocation = (byte *)nextMultiple(alignof(T), ((size_t)m_addr) + 1); // +1: save offset in front
        size_t amountOfBytes = amountOfObjects * sizeof(T);

        byte *newAddr = allocationLocation + amountOfBytes;

        if (newAddr <= m_addr + m_size)
        {
            byte offset = (byte)(allocationLocation - m_addr);
            allocationLocation[-1] = offset;

            T *returnPointer = reinterpret_cast<T *>(allocationLocation);
            m_size -= newAddr - m_addr;
            m_addr = newAddr;

            for (size_t i = 0; i < amountOfObjects; i++)
            {
                T *object = std::addressof(returnPointer[i]);
                new (object) T(std::forward<arguments>(args)...); // Objecte anlegen und Konstruktor aufrufen
            }
            return returnPointer;
        }
        else
        {
            return nullptr;
        }
    }
};

} // namespace INTERNAL

class GeneralPurposeAllocator
{
public:
    template <typename T>
    class GeneralPurposeAllocatorPointer
    {
        friend class GeneralPurposeAllocator;

    private:
        size_t m_handleIndex;
        size_t m_size;
        GeneralPurposeAllocator *m_pparent;

    public:
        GeneralPurposeAllocatorPointer(GeneralPurposeAllocator *parent, size_t handleIndex, size_t size)
            : m_pparent(parent), m_handleIndex(handleIndex), m_size(size)

        {
        }

        T *operator->()
        {
            return static_cast<T *>(m_pparent->m_handleTable[m_handleIndex]);
        }

        const T *operator->() const
        {
            return static_cast<T *>(m_pparent->m_handleTable[m_handleIndex]);
        }

        T &operator*()
        {
            return *static_cast<T *>(m_pparent->m_handleTable[m_handleIndex]);
        }

        const T &operator*() const
        {
            return *static_cast<T *>(m_pparent->m_handleTable[m_handleIndex]);
        }

        T &operator[](int index)
        {
            return *(static_cast<T *>(m_pparent->m_handleTable[m_handleIndex]) + index);
        }

        const T &operator[](int index) const
        {
            return *(static_cast<T *>(m_pparent->m_handleTable[m_handleIndex]) + index);
        }

        T *getRaw()
        {
            return static_cast<T *>(m_pparent->m_handleTable[m_handleIndex]);
        }

        bool operator==(void *ptr) const
        {
            return static_cast<T *>(m_pparent->m_handleTable[m_handleIndex]) == ptr;
        }

        bool operator!=(void *ptr) const
        {
            return static_cast<T *>(m_pparent->m_handleTable[m_handleIndex]) != ptr;
        }
    };

    class GeneralPurposeAllocatorRelocatable
    {
        friend class GeneralPurposeAllocator;

    private:
        GeneralPurposeAllocator *m_pparent;
        union {
            size_t m_handleIndex;
            void *m_paddr;
        };
        bool m_usedAddr = false;
        size_t m_amountOfObjects = 0;

        byte *(GeneralPurposeAllocatorRelocatable::*relocate)(void *);

        template <typename T>
        byte *relocateTemplate(void *newAddr)
        {
            static_assert(alignof(T) <= 128, "Max alignment of 128 was exceeded!");

            byte *allocationLocation = (byte *)nextMultiple(alignof(T), ((size_t)newAddr) + 1);
            size_t m_amountOfBytes = m_amountOfObjects * sizeof(T);

            byte offset = (byte)(allocationLocation - (byte *)newAddr);
            allocationLocation[-1] = offset;

            T *oldData = static_cast<T *>(m_pparent->m_handleTable[m_handleIndex]);
            T *newData = reinterpret_cast<T *>(allocationLocation);

            if (std::is_trivially_move_constructible<T>::value)
            {
                // Copy N bytes of SRC to DEST, guaranteeing
                //      correct behavior for overlapping strings.
                std::memmove(newData, oldData, m_amountOfBytes);
            }
            else if (allocationLocation + sizeof(T) < (byte *)oldData) // not overlapping
            {
                for (int i = 0; i < m_amountOfObjects; i++)
                {
                    new (std::addressof(newData[i])) T(std::move(oldData[i]));
                    std::addressof(oldData[i])->~T();
                }
            }
            else
            {
                // Daten Temporär ablegen auf dem Stack (Stack kann nicht dynamisch allokieren, deshalb immer nur einzelne Objekte moven)
                byte tempByteArr[sizeof(T) + alignof(T)];

                T *tempObj = reinterpret_cast<T *>(nextMultiple((size_t)alignof(T), (size_t)tempByteArr));

                for (int i = 0; i < m_amountOfObjects; i++)
                {
                    new (tempObj) T(std::move(oldData[i]));
                    std::addressof(oldData[i])->~T();

                    T *object = std::addressof(newData[i]);
                    new (object) T(std::move(*tempObj));
                    tempObj->~T();
                }
            }

            m_pparent->m_handleTable[m_handleIndex] = allocationLocation;
            return static_cast<byte *>(allocationLocation + m_amountOfBytes);
        }

        template <typename T>
        GeneralPurposeAllocatorRelocatable(GeneralPurposeAllocator *parent, size_t handleIndex, size_t amountOfObjects, T *t) // t only for compiler, to know what type
            : m_pparent(parent), m_handleIndex(handleIndex), m_amountOfObjects(amountOfObjects), m_usedAddr(false)
        {
            relocate = &GeneralPurposeAllocatorRelocatable::relocateTemplate<T>;
        }

        GeneralPurposeAllocatorRelocatable(GeneralPurposeAllocator *parent, void *addr)
            : m_pparent(parent), m_paddr(addr), m_usedAddr(true)
        {
        }

        GeneralPurposeAllocatorRelocatable(GeneralPurposeAllocator *parent, size_t handleIndex)
            : m_pparent(parent), m_handleIndex(handleIndex), m_usedAddr(false)
        {
        }

        byte *operator()(void *newAddr)
        {
            return (this->*relocate)(newAddr);
        }

        void *getAddr() const
        {
            if (m_usedAddr)
            {
                return m_paddr;
            }
            else
            {
                return m_pparent->m_handleTable[m_handleIndex];
            }
        }

    public:
        bool operator>(const GeneralPurposeAllocatorRelocatable &other) const
        {
            return getAddr() > other.getAddr();
        }
        bool operator<(const GeneralPurposeAllocatorRelocatable &other) const
        {
            return getAddr() < other.getAddr();
        }
        bool operator>=(const GeneralPurposeAllocatorRelocatable &other) const
        {
            return getAddr() >= other.getAddr();
        }
        bool operator<=(const GeneralPurposeAllocatorRelocatable &other) const
        {
            return getAddr() <= other.getAddr();
        }
        bool operator==(const GeneralPurposeAllocatorRelocatable &other) const
        {
            return getAddr() == other.getAddr();
        }
        bool operator!=(const GeneralPurposeAllocatorRelocatable &other) const
        {
            return getAddr() != other.getAddr();
        }
    };

private:
    static const size_t GENERAL_PURPOSE_ALLOCATOR_DEFAULT_SIZE = 1024;

    byte *m_data;
    size_t m_size;

    List<INTERNAL::GeneralPurposeAllocatorFreeChunk, true> m_freeChunks;

    size_t m_lengthOfHanfleTable;
    void **m_handleTable;
    Stack<size_t> m_unusedHandleStack;
    List<GeneralPurposeAllocatorRelocatable, true> m_allocatedBlocks;

public:
    explicit GeneralPurposeAllocator(size_t size = GENERAL_PURPOSE_ALLOCATOR_DEFAULT_SIZE, size_t lengthOfHandleTable = GENERAL_PURPOSE_ALLOCATOR_DEFAULT_SIZE / 4)
        : m_size(size), m_lengthOfHanfleTable(lengthOfHandleTable)
    {
        m_data = new byte[m_size];
        m_freeChunks.pushBack(INTERNAL::GeneralPurposeAllocatorFreeChunk(m_data, m_size));

        m_handleTable = new void *[m_lengthOfHanfleTable];
        memset(m_handleTable, 0, sizeof(void *) * m_lengthOfHanfleTable);

        for(size_t i = lengthOfHandleTable - 1; i > 0; i--)
        {
            m_unusedHandleStack.push(i);
        }
    }

    GeneralPurposeAllocator(const GeneralPurposeAllocator &other) = delete;
    GeneralPurposeAllocator(GeneralPurposeAllocator &&other) = delete;
    GeneralPurposeAllocator &operator=(const GeneralPurposeAllocator &other) = delete;
    GeneralPurposeAllocator &operator=(GeneralPurposeAllocator &&other) = delete;

    ~GeneralPurposeAllocator()
    {
        if (m_data != nullptr)
        {
            delete[] m_data;
            m_data = nullptr;
        }

        if (m_handleTable != nullptr)
        {
            delete[] m_handleTable;
            m_handleTable = nullptr;
        }
    }

    template <typename T, typename... arguments>
    GeneralPurposeAllocatorPointer<T> allocateObjects(size_t amountOfObjects = 1, arguments &&... args)
    {
        static_assert(alignof(T) <= 128, "Max alignment of 128 was exceeded!");

        for (size_t i = 0; i < m_freeChunks.getLength(); i++)
        {
            T *data = m_freeChunks[i].allocateObject<T>(amountOfObjects, std::forward<arguments>(args)...);
            if (data != nullptr && m_unusedHandleStack.hasDataLeft())
            {
                // Remove empty free chunks
                if (m_freeChunks[i].m_size == 0)
                {
                    m_freeChunks.removeIndex(i);
                }

                size_t index = m_unusedHandleStack.pop();
                m_handleTable[index] = data;
                m_allocatedBlocks.pushBack(GeneralPurposeAllocatorRelocatable(this, index, amountOfObjects, data));

                return GeneralPurposeAllocatorPointer<T>(this, index, amountOfObjects);
            }
        }

        return GeneralPurposeAllocatorPointer<T>(this, 0, 0); // Or defragment the space
    }

    template <typename T>
    void deallocateObjects(GeneralPurposeAllocatorPointer<T> pointer)
    {
        for (size_t i = 0; i < pointer.m_size; i++)
        {
            std::addressof(pointer[i])->~T();
        }

        byte *bytePointer = reinterpret_cast<byte *>(m_handleTable[pointer.m_handleIndex]);
        size_t amountOfBytes = sizeof(T) * pointer.m_size;
        byte offset = bytePointer[-1];

        INTERNAL::GeneralPurposeAllocatorFreeChunk gpafc(bytePointer - offset, amountOfBytes + offset);

        INTERNAL::GeneralPurposeAllocatorFreeChunk *p_gpafc = &gpafc;

        INTERNAL::GeneralPurposeAllocatorFreeChunk *left;
        INTERNAL::GeneralPurposeAllocatorFreeChunk *right;

        m_freeChunks.getNeighbors(*p_gpafc, left, right);

        bool didMerge = false;

        if (left != nullptr)
        {
            if (left->touches(*p_gpafc))
            {
                left->m_size += p_gpafc->m_size;
                p_gpafc = left;
                didMerge = true;
            }
        }
        if (right != nullptr)
        {
            if (right->touches(*p_gpafc))
            {
                if (didMerge)
                {
                    p_gpafc->m_size += right->m_size;
                    m_freeChunks.removeSingle(*right);
                }
                else
                {
                    right->m_size += p_gpafc->m_size;
                    right->m_addr = p_gpafc->m_addr;
                }
                didMerge = true;
            }
        }

        if (!didMerge)
        {
            m_freeChunks.pushBack(gpafc);
        }

        m_unusedHandleStack.push(pointer.m_handleIndex);
        if (!m_allocatedBlocks.removeSingle(GeneralPurposeAllocatorRelocatable(this, pointer.m_handleIndex)))
        {
            DEBUG_BREAK;
        }
        pointer.m_handleIndex = 0;
    }

    bool needsDefragmentation()
    {
        // Keine free chunks
        if (m_freeChunks.getLength() == 0   // oder nur noch einer und dieser am Ende
                || (m_freeChunks.getLength() == 1 && (m_freeChunks[0].m_addr + m_freeChunks[0].m_size) == (m_data + m_size)))
        {
            return false;
        }
        return true;
    }

    bool defragment()
    {
        if (!needsDefragmentation())
        {
            return false;
        }

        byte* addr = m_freeChunks[0].m_addr;
        GeneralPurposeAllocatorRelocatable *left;
        GeneralPurposeAllocatorRelocatable *right;
        GeneralPurposeAllocatorRelocatable indexLocator(this, addr); // Tue so, als wäre FreeChung eine Allokierung, um Nachbarn zu finden
        m_allocatedBlocks.getNeighbors(indexLocator, left, right);

        byte oldOffset = static_cast<byte*>(right->getAddr())[-1];
        byte *newAddr = (*right)(addr);
        byte newOffset = static_cast<byte*>(right->getAddr())[-1];

        m_freeChunks[0].m_addr = newAddr;
        if (newOffset != oldOffset)
        {
            m_freeChunks[0].m_size += oldOffset;
            m_freeChunks[0].m_size -= newOffset;
        }

        if (m_freeChunks.getLength() > 1)
        {
            if(m_freeChunks[0].touches(m_freeChunks[1]))
            {
                m_freeChunks[0].m_size += m_freeChunks[1].m_size;
                m_freeChunks.removeIndex(1);
            }
        }

        return true;
    }
};

} // namespace bbe