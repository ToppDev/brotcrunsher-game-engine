#include "FreeListAllocator.h"

#include <cstring>

#include <iostream>

using namespace arcane;

FreeListAllocator::FreeListAllocator(size_t size, void *start, size_t handle_table_length)
	: m_start((byte *)start), m_size(size), m_used_memory(0), m_num_allocations(0), m_freeBlocks((FreeBlock *)start)
{
	ASSERT(size > sizeof(FreeBlock));

	m_freeBlocks->size = size;
	m_freeBlocks->next = nullptr;

	m_handle_table_length = handle_table_length;
	m_handle_table = new void *[m_handle_table_length];
	memset(m_handle_table, 0, sizeof(void *) * m_handle_table_length);

	for (size_t i = m_handle_table_length - 1; i > 0; i--)
	{
		m_unusedHandleStack.push_back(i);
	}
}

FreeListAllocator::~FreeListAllocator()
{
	if (m_handle_table != nullptr)
	{
		delete[] m_handle_table;
		m_handle_table = nullptr;
	}

	ASSERT(m_num_allocations == 0 && m_used_memory == 0);

	m_size = 0;
}

FreeListAllocator::AllocatorPointer<byte> FreeListAllocator::allocate(size_t size, uint8_t alignment)
{
	ASSERT(size != 0 && alignment != 0);

	if (m_unusedHandleStack.empty())
	{
		return FreeListAllocator::AllocatorPointer<byte>(this, 0);
	}

	FreeBlock *best_fit_prev;
	FreeBlock *best_fit;
	uint8_t best_fit_adjustment;
	size_t total_free_space;

	findBestFitFreeBlock(size, alignment, best_fit_adjustment, best_fit_prev, best_fit, total_free_space);

	if (best_fit == nullptr)
	{
		// Memory is fragmented and we need to clean up first (This can take time!)
		if (total_free_space >= size + alignment)
		{
			// TODO: Call defragmentator till a large enough block is generated
			findBestFitFreeBlock(size, alignment, best_fit_adjustment, best_fit_prev, best_fit, total_free_space);
			ASSERT(best_fit && "Defragmenting or findBestFitFreeBlock is not working correctly.");
		}
		else // Not enough memory available
		{
			return FreeListAllocator::AllocatorPointer<byte>(this, 0);
		}
	}

	size_t best_fit_total_size = size + best_fit_adjustment;

	//If allocations in the remaining memory will be impossible
	if (best_fit->size - best_fit_total_size <= sizeof(AllocationHeader))
	{
		//Increase allocation size instead of creating a new FreeBlock
		best_fit_total_size = best_fit->size;

		// Remove empty FreeBlock from the list
		if (best_fit_prev != nullptr)
		{
			best_fit_prev->next = best_fit->next;
		}
		else
		{
			m_freeBlocks = best_fit->next;
		}
	}
	else
	{
		//Prevent new block from overwriting best fit block info
		ASSERT(best_fit_total_size > sizeof(FreeBlock));

		//Else create a new FreeBlock containing remaining memory
		FreeBlock *new_block = (FreeBlock *)(pointer_math::add(best_fit, best_fit_total_size));
		new_block->size = best_fit->size - best_fit_total_size;
		new_block->next = best_fit->next;

		if (best_fit_prev != nullptr)
		{
			best_fit_prev->next = new_block;
		}
		else
		{
			m_freeBlocks = new_block;
		}
	}

	byte *aligned_address = reinterpret_cast<byte *>(best_fit) + best_fit_adjustment;

	AllocationHeader *header = (AllocationHeader *)(aligned_address - sizeof(AllocationHeader));

	//std::cout << "best_fit:           " << (void *)best_fit << std::endl;
	//std::cout << "aligned_address:    " << (void *)aligned_address << std::endl;
	//std::cout << "header:             " << (void *)header << std::endl;
	//std::cout << "header->size:       " << best_fit_total_size << std::endl;
	//std::cout << "header->adjustment: " << (size_t)best_fit_adjustment << std::endl
	//		  << std::endl;

	header->size = best_fit_total_size;
	header->adjustment = best_fit_adjustment;

	ASSERT(pointer_math::isAligned(header));

	m_used_memory += best_fit_total_size;
	m_num_allocations++;

	ASSERT(pointer_math::alignForwardAdjustment(reinterpret_cast<void *>(aligned_address), alignment) == 0);

	size_t index = m_unusedHandleStack.back();
	m_unusedHandleStack.pop_back();
	m_handle_table[index] = aligned_address;

	return FreeListAllocator::AllocatorPointer<byte>(this, index);
}

void FreeListAllocator::deallocate(FreeListAllocator::AllocatorPointer<byte> *p)
{
	ASSERT(p != nullptr && (*p) != nullptr);

	AllocationHeader *header = (AllocationHeader *)pointer_math::subtract((*p).getRaw(), sizeof(AllocationHeader));

	byte *block_start = reinterpret_cast<byte *>((*p).getRaw()) - header->adjustment;
	size_t block_size = header->size;
	byte *block_end = block_start + block_size;

	FreeBlock *prev_free_block = nullptr;
	FreeBlock *free_block = m_freeBlocks;

	// Find first FreeBlock, which is behind the current allocation
	while (free_block != nullptr)
	{
		if ((byte *)free_block >= block_end)
			break;

		prev_free_block = free_block;
		free_block = free_block->next;
	}

	// FreeBlock is at the start of the list
	if (prev_free_block == nullptr)
	{
		prev_free_block = (FreeBlock *)block_start;
		prev_free_block->size = block_size;
		prev_free_block->next = m_freeBlocks;

		m_freeBlocks = prev_free_block;
	}
	else if ((byte *)prev_free_block + prev_free_block->size == block_start)
	{
		prev_free_block->size += block_size; // Previous Free Block ends at our allocation, so just merge them
	}
	else
	{
		// Create new Free Block and put it into the list
		FreeBlock *temp = (FreeBlock *)block_start;
		temp->size = block_size;
		temp->next = prev_free_block->next;

		prev_free_block->next = temp;

		prev_free_block = temp;
	}

	//std::cout << "m_freeBlocks:             " << (void *)m_freeBlocks << std::endl;
	//std::cout << "m_freeBlocks->size:       " << m_freeBlocks->size << std::endl;
	//std::cout << "m_freeBlocks->next:       " << (void *)m_freeBlocks->next << std::endl;
	//if (m_freeBlocks->next)
	//	std::cout << "m_freeBlocks->next-size:  " << m_freeBlocks->next->size << std::endl;

	ASSERT(prev_free_block != nullptr);

	// Check if newly created free blocks can be merged (Coalescence)
	if ((byte *)prev_free_block + prev_free_block->size == (byte *)prev_free_block->next)
	{
		prev_free_block->size += prev_free_block->next->size;
		prev_free_block->next = prev_free_block->next->next;
	}

	m_num_allocations--;
	m_used_memory -= block_size;

	m_unusedHandleStack.push_back((*p).m_handle_index);
	(*p).m_handle_index = 0;
}

bool FreeListAllocator::needsDefragmentation()
{
	// No FreeBlock left or only one and at the end
	if (m_freeBlocks == nullptr || (m_freeBlocks->next == nullptr && ((byte *)m_freeBlocks + m_freeBlocks->size) == (m_start + m_size)))
	{
		return false;
	}
	return true;
}

bool FreeListAllocator::defragment()
{
	if (!needsDefragmentation())
	{
		return false;
	}

	/* FreeBlock* next_free_block = m_freeBlocks->next;

	byte *newAddr = (byte *)m_freeBlocks;
	byte *oldAddr = (byte *)pointer_math::add(newAddr, m_freeBlocks->size);

	uint8_t oldAdjustment = pointer_math::alignForwardAdjustment(oldAddr, alignof(AllocationHeader));
	AllocationHeader *oldHeader = (AllocationHeader *)(oldAddr + oldAdjustment);
	byte *oldAlignedAddress = (byte *)pointer_math::add(oldHeader, sizeof(AllocationHeader));

	uint8_t newAdjustment = pointer_math::alignForwardAdjustment(newAddr, alignof(AllocationHeader));
	AllocationHeader *newHeader = (AllocationHeader *)(newAddr + newAdjustment);
	byte *newAlignedAddress = (byte *)pointer_math::add(newHeader, sizeof(AllocationHeader));
    */
	//newHeader->size = oldHeader->size;
	//newHeader->adjustment = oldHeader->adjustment;

	return true;
}

size_t FreeListAllocator::getSize() const
{
	return m_size;
}

size_t FreeListAllocator::getUsedMemory() const
{
	return m_used_memory;
}

size_t FreeListAllocator::getNumAllocations() const
{
	return m_num_allocations;
}

void FreeListAllocator::findBestFitFreeBlock(const size_t size, const size_t alignment, uint8_t &adjustment,
											 FreeBlock *&bestFitPrev, FreeBlock *&bestFit, size_t &totalFreeSpace)
{
	FreeBlock *prev_free_block = nullptr;
	FreeBlock *free_block = m_freeBlocks;

	FreeBlock *best_fit_prev = nullptr;
	FreeBlock *best_fit = nullptr;
	uint8_t best_fit_adjustment = 0;
	size_t total_free_space = 0;

	while (free_block != nullptr)
	{
		//Calculate adjustment needed to keep object correctly aligned
		uint8_t adjustment = pointer_math::alignForwardAdjustmentWithHeader<AllocationHeader>(free_block, alignment);

		size_t total_size = size + adjustment;

		total_free_space += free_block->size;

		//If its an exact match use this free block
		if (free_block->size == total_size)
		{
			best_fit_prev = prev_free_block;
			best_fit = free_block;
			best_fit_adjustment = adjustment;

			break;
		}

		//If its a better fit switch
		if (free_block->size > total_size && (best_fit == nullptr || free_block->size < best_fit->size))
		{
			best_fit_prev = prev_free_block;
			best_fit = free_block;
			best_fit_adjustment = adjustment;
		}

		prev_free_block = free_block;
		free_block = free_block->next;
	}

	bestFit = best_fit;
	bestFitPrev = best_fit_prev;
	adjustment = best_fit_adjustment;
	totalFreeSpace = total_free_space;
}