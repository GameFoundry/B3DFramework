//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DUtilityPrerequisites.h"
#include "Allocators/B3DStackAllocator.h"

using namespace b3d;

namespace
{
	/** Per-thread stack state, valid before any dynamic initialization. */
	class StackAllocatorState : INonCopyable
	{
		/** A backing block; blocks after the current one are empty and available for reuse. */
		struct MemoryBlock
		{
			explicit MemoryBlock(u32 capacity) : Capacity(capacity) {}

			u32 Capacity;
			u32 UsedBytes = 0;
			MemoryBlock* PreviousBlock = nullptr;
			MemoryBlock* NextBlock = nullptr;
		};

		/** Releases cached storage when the thread exits. */
		class ThreadCleanup : INonCopyable
		{
		public:
			explicit ThreadCleanup(StackAllocatorState& state) : mState(state) {}

			~ThreadCleanup()
			{
				mState.mIsThreadExiting = true;
				mState.Release();
			}

		private:
			StackAllocatorState& mState;
		};

	public:
		constexpr StackAllocatorState() = default;

		u8* Allocate(u32 amount)
		{
			B3D_ASSERT(amount <= std::numeric_limits<u32>::max() - sizeof(u32));
			amount += sizeof(u32);

			if(amount > mRemainingBytes)
				AllocateBlock(amount);

			u8* data = reinterpret_cast<u8*>(mCurrentBlock + 1) + mCurrentBlock->UsedBytes;
			mCurrentBlock->UsedBytes += amount;
			mRemainingBytes -= amount;

			std::memcpy(data, &amount, sizeof(amount));
			return data + sizeof(u32);
		}

		void Deallocate(u8* data)
		{
			data -= sizeof(u32);

			u32 amount;
			std::memcpy(&amount, data, sizeof(amount));

			B3D_ASSERT(mCurrentBlock != nullptr && amount <= mCurrentBlock->UsedBytes);
			mCurrentBlock->UsedBytes -= amount;
			mRemainingBytes += amount;

			B3D_ASSERT(reinterpret_cast<u8*>(mCurrentBlock + 1) + mCurrentBlock->UsedBytes == data && "Out of order stack deallocation detected. Deallocations need to happen in order opposite of allocations.");

			if(mCurrentBlock->UsedBytes == 0)
			{
				MemoryBlock* emptyBlock = mCurrentBlock;
				MemoryBlock* previousBlock = emptyBlock->PreviousBlock;

				// Merge adjacent empty blocks into one reusable block.
				if(emptyBlock->NextBlock != nullptr)
				{
					u64 totalCapacity = 0;
					for(MemoryBlock* block = emptyBlock; block != nullptr; block = block->NextBlock)
						totalCapacity += block->Capacity;

					if(totalCapacity <= std::numeric_limits<u32>::max())
					{
						if(previousBlock != nullptr)
							previousBlock->NextBlock = nullptr;
						mCurrentBlock = previousBlock;

						while(emptyBlock != nullptr)
						{
							MemoryBlock* nextBlock = emptyBlock->NextBlock;
							B3DDelete(emptyBlock);
							emptyBlock = nextBlock;
						}

						AllocateBlock((u32)totalCapacity);
					}
				}

				if(previousBlock != nullptr)
					mCurrentBlock = previousBlock;

				mRemainingBytes = mCurrentBlock->Capacity - mCurrentBlock->UsedBytes;

				// TLS destructors can allocate after the cleanup guard has already run.
				if(mIsThreadExiting && mCurrentBlock->UsedBytes == 0)
					Release();
			}
		}

	private:
		MemoryBlock* mCurrentBlock = nullptr;
		u32 mRemainingBytes = 0;
		bool mIsThreadExiting = false;

		void AllocateBlock(u32 wantedSize)
		{
			if(!mIsThreadExiting)
			{
				thread_local ThreadCleanup cleanup(*this);
				(void)cleanup;
			}

			const u32 capacity = std::max(wantedSize, 1024u * 1024u);
			MemoryBlock* block = mCurrentBlock == nullptr ? nullptr : mCurrentBlock->NextBlock;
			while(block != nullptr && block->Capacity < capacity)
				block = block->NextBlock;

			if(block != nullptr)
			{
				block->PreviousBlock->NextBlock = block->NextBlock;
				if(block->NextBlock != nullptr)
					block->NextBlock->PreviousBlock = block->PreviousBlock;
			}
			else
			{
				void* storage = B3DAllocate(sizeof(MemoryBlock) + (size_t)capacity);
				block = new(storage) MemoryBlock(capacity);
			}

			block->PreviousBlock = mCurrentBlock;
			block->NextBlock = mCurrentBlock == nullptr ? nullptr : mCurrentBlock->NextBlock;

			if(block->NextBlock != nullptr)
				block->NextBlock->PreviousBlock = block;

			if(mCurrentBlock != nullptr)
				mCurrentBlock->NextBlock = block;

			mCurrentBlock = block;
			mRemainingBytes = block->Capacity;
		}

		void Release()
		{
			B3D_ASSERT((mCurrentBlock == nullptr || (mCurrentBlock->UsedBytes == 0 && mCurrentBlock->PreviousBlock == nullptr)) && "Not all blocks were released before shutting down the stack allocator.");

			MemoryBlock* block = mCurrentBlock;
			while(block != nullptr && block->PreviousBlock != nullptr)
				block = block->PreviousBlock;

			while(block != nullptr)
			{
				MemoryBlock* nextBlock = block->NextBlock;
				B3DDelete(block);
				block = nextBlock;
			}

			mCurrentBlock = nullptr;
			mRemainingBytes = 0;
		}
	};

	static_assert(std::is_trivially_destructible_v<StackAllocatorState>);
	B3D_THREADLOCAL StackAllocatorState gThreadStack;
}

u8* MemStack::Alloc(u32 amount)
{
	return gThreadStack.Allocate(amount);
}

void MemStack::DeallocLast(u8* data)
{
	gThreadStack.Deallocate(data);
}
