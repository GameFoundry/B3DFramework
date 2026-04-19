//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

namespace b3d
{
	/** @addtogroup Containers
	 *  @{
	 */

	// TODO - Ideally we have a lock-free pool allocator to go along with this queue

	/** Threading safety options for TQueue. */
	enum class QueueThreadingPolicy
	{
		MPSC, /**< Multiple producer threads can write to the queue, one consumer thread can read from the queue safely. */
		SPSC, /**< Single producer thread can write to the queue, one consumer thread can read from the queue safely. */
		SingleThread /**< No threading safety, should be only used by a single thread. */
	};

	/** Lock-free queue implemented as a linked list. Supports multiple producer/single consumer, single producer/single consume and thread unsafe modes. */
	template<typename T, QueueThreadingPolicy ThreadingPolicy = QueueThreadingPolicy::SingleThread, typename AllocatorTag = DefaultAllocatorTag>
	class TQueue final : public INonCopyable
	{
	public:
		TQueue()
		{
			Node* const root = new(MemoryAllocator<AllocatorTag>::AllocateAligned(sizeof(Node), alignof(Node))) Node;
			mHead.store(root, std::memory_order_relaxed);
			mTail = root;
		}

		~TQueue()
		{
			Node* next = mTail->Next.load(std::memory_order_relaxed);
			MemoryAllocator<AllocatorTag>::FreeAligned(mTail);

			while (next != nullptr)
			{
				mTail = next;
				next = mTail->Next.load(std::memory_order_relaxed);

				((T*)&mTail->Value)->~T();
				MemoryAllocator<AllocatorTag>::FreeAligned(mTail);
			}
		}

		/** Adds an entry to the back of the queue. */
		template <typename... ArgumentTypes>
		void Enqueue(ArgumentTypes&&... arguments)
		{
			Node* const node = new(MemoryAllocator<AllocatorTag>::AllocateAligned(sizeof(Node), alignof(Node))) Node;
			new ((void*)&node->Value) T(std::forward<ArgumentTypes>(arguments)...);

			if constexpr(ThreadingPolicy == QueueThreadingPolicy::MPSC)
			{
				Node* const previous = mHead.exchange(node, std::memory_order_acq_rel);
				previous->Next.store(node, std::memory_order_release);
			}
			else if constexpr(ThreadingPolicy == QueueThreadingPolicy::SPSC)
			{
				Node* const head = mHead.load(std::memory_order_relaxed);
				head->Next.store(node, std::memory_order_release);
				mHead = node;
			}
			else
			{
				Node* const head = mHead.load(std::memory_order_relaxed);
				head->Next.store(node, std::memory_order_relaxed);
				mHead = node;
			}
		}

		/** Removes an entry from the front of the queue. */
		TOptional<T> Dequeue()
		{
			Node* next;

			if constexpr(ThreadingPolicy == QueueThreadingPolicy::SingleThread)
				next = mTail->Next.load(std::memory_order_relaxed);
			else
				next = mTail->Next.load(std::memory_order_acquire);

			if (next == nullptr)
				return {};

			T* valuePointer = (T*)&next->Value;
			TOptional<T> result{ std::move(*valuePointer) };

			valuePointer->~T();
			MemoryAllocator<AllocatorTag>::FreeAligned(mTail);

			mTail = next;
			return result;
		}

		/** Removes an entry from the front of the queue. */
		bool Dequeue(T& outElement)
		{
			TOptional<T> localElement = Dequeue();
			if (localElement.IsSet())
			{
				outElement = std::move(localElement.GetValue());
				return true;
			}

			return false;
		}

		/** Retrieves an entry from the front of the queue, without removing it. Returns null if the queue is empty. */
		T* Peek() const
		{
			Node* next;

			if constexpr(ThreadingPolicy == QueueThreadingPolicy::SingleThread)
				next = mTail->Next.load(std::memory_order_relaxed);
			else
				next = mTail->Next.load(std::memory_order_acquire);

			if (next == nullptr)
				return nullptr;

			return (T*)&next->Value;
		}

		/** Returns true if the queue contains no entries. */
		bool IsEmpty() const
		{
			if constexpr(ThreadingPolicy == QueueThreadingPolicy::SingleThread)
				return mTail->Next.load(std::memory_order_relaxed) == nullptr;
			
			return mTail->Next.load(std::memory_order_acquire) == nullptr;
		}

	private:
		struct Node
		{
			std::atomic<Node*> Next{ nullptr };
			std::aligned_storage_t<sizeof(T), std::min(alignof(std::max_align_t), alignof(T))> Value;
		};

	private:
		std::atomic<Node*> mHead;
		alignas(64) Node* mTail; // Aligned to next cache line to avoid conflicts with mHead
	};

	/** @} */
} // namespace b3d
