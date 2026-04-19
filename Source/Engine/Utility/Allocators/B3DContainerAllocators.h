//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

namespace b3d
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Memory-Internal
	 *  @{
	 */

	/** Default allocator that will be used by containers such as arrays. Performs dynamic memory allocation using the default allocator. */
	class DefaultContainerAllocator
	{
	public:
		template<class ElementType>
		class ForElementType
		{
		public:
			~ForElementType()
			{
				if(mElements)
					B3DFree(mElements);
			}

			/** Returns the memory provided by the allocator. */
			ElementType* GetElements() const { return mElements; }

			/** Returns the minimum number of elements that are always allocated regardless if Resize() is called with a smaller requested capacity. */
			u64 GetMinimumCapacity() const { return 0; }

			/** Returns true if the allocator has any dynamic allocations. */
			bool HasDynamicAllocations() const { return mElements != nullptr; }

			/**
			 * Moves the elements from @p other into the current allocator. Existing elements in the current allocator are destroyed.
			 *
			 * @param	mySize		Current number of elements in this allocator.
			 * @param	otherSize	Number of elements in the other allocator.
			 * @param	other		Allocator whose contents to move to this allocator. Memory in the allocator will be cleared.
			 */
			void Move(u64 mySize, u64 otherSize, ForElementType&& other)
			{
				B3D_ASSERT(this != &other);

				if(mElements)
				{
					for(u64 index = 0; index < mySize; ++index)
						mElements[index].~ElementType();

					B3DFree(mElements);
				}

				mElements = std::exchange(other.mElements, nullptr);
			}

			/**
			 * Resizes the allocator so it can fit more or less elements.
			 *
			 * @param	elementCount	Current number of constructed element (can be less than current capacity).
			 * @param	newCapacity		New maximum number of elements that the allocator can store.
			 */
			void Resize(u64 elementCount, u64 newCapacity)
			{
				ElementType* buffer = newCapacity > 0 ? B3DAllocateMultiple<ElementType>(newCapacity) : nullptr;

				if(buffer)
				{
					std::uninitialized_move(
						mElements,
						mElements + std::min(elementCount, newCapacity),
						buffer);
				}

				// Destoy existing elements in old memory
				if(mElements)
				{
					for(u64 index = 0; index < elementCount; ++index)
						mElements[index].~ElementType();

					B3DFree(mElements);
				}

				mElements = buffer;
			}

			ElementType* mElements = nullptr;
		};
	};

	/**
	 * Allocator that will allocate @tparam StackElementCount on the stack, avoiding dynamic allocations if element count is kept at or below this number. If the element count
	 * exceeds the stack element count, the allocator falls back on allocations using @tparam SecondaryAllocator;
	 */
	template<u32 StackElementCount, class SecondaryAllocator = DefaultContainerAllocator>
	class InlineContainerAllocator
	{
	public:
		template<class ElementType>
		class ForElementType
		{
		public:
			/** @copydoc DefaultContainerAllocator::ForElementType::GetElements */
			ElementType* GetElements() const { return mElements; }

			/** @copydoc DefaultContainerAllocator::ForElementType::GetMinimumCapacity */
			u64 GetMinimumCapacity() const { return StackElementCount; }

			/** @copydoc DefaultContainerAllocator::ForElementType::HasDynamicAllocations */
			bool HasDynamicAllocations() const { return mElements != (ElementType*)mStackStorage; }

			/** @copydoc DefaultContainerAllocator::ForElementType::Move */
			void Move(u64 mySize, u64 otherSize, ForElementType&& other)
			{
				B3D_ASSERT(this != &other);

				if(!other.HasDynamicAllocations())
				{
					// First, clean up our heap allocation
					if(HasDynamicAllocations())
					{
						// Destroy elements in our heap allocation
						for(u64 index = 0; index < mySize; ++index)
							mElements[index].~ElementType();

						mSecondaryAllocator.Resize(mySize, 0);
						mySize = 0;
					}

					// Use assignment move if we have more elements than the other array, and destroy any excess elements
					ElementType* stackElements = (ElementType*)mStackStorage;
					if(mySize > otherSize)
					{
						ElementType* newEnd = otherSize > 0 ? std::move(other.GetElements(), other.GetElements() + otherSize, stackElements) : stackElements;

						for(; newEnd != stackElements + mySize; ++newEnd)
							(*newEnd).~ElementType();
					}
					else
					{
						// Assignment move existing elements
						if(mySize > 0)
							std::move(other.GetElements(), other.GetElements() + mySize, stackElements);

						// Construct new elements
						std::uninitialized_move(other.GetElements() + mySize, other.GetElements() + otherSize, stackElements + mySize);
					}

					// Destruct source elements after moving them. This is needed because:
					// a. If elements don't support move, they will be copied, so we need to destruct originals
					// b. Even if elements support move, they still need to be destructed
					// c. Owning source array size is generally set to 0 during the move, so the elements won't be destructed when array goes out of scope
					ElementType* otherElements = other.GetElements();
					for(u64 index = 0; index < otherSize; ++index)
						otherElements[index].~ElementType();

					mElements = (ElementType*)mStackStorage;
				}
				else
				{
					// Source uses heap storage - take ownership of their allocation

					// First, destroy our existing elements if they're in stack storage
					// (mSecondaryAllocator.Move handles the case when we have dynamic allocations)
					if(!HasDynamicAllocations())
					{
						for(u64 index = 0; index < mySize; ++index)
							((ElementType*)mStackStorage)[index].~ElementType();

						mySize = 0;
					}

					mSecondaryAllocator.Move(mySize, otherSize, std::move(other.mSecondaryAllocator));
					mElements = std::exchange(other.mElements, nullptr);
				}
			}

			/** @copydoc DefaultContainerAllocator::ForElementType::Resize */
			void Resize(u64 currentSize, u64 newCapacity)
			{
				// New capacity fits on stack
				if(newCapacity <= StackElementCount)
				{
					// If current allocations are dynamic, move them to stack and free dynamic allocation
					if(HasDynamicAllocations())
					{
						std::uninitialized_move(
							mSecondaryAllocator.GetElements(),
							mSecondaryAllocator.GetElements() + currentSize,
							(ElementType*)mStackStorage);

						mSecondaryAllocator.Resize(currentSize, 0);
						mElements = (ElementType*)mStackStorage;
					}
				}
				// New capacity requires a dynamic allocation
				else
				{
					// Already have a dynamic allocation, just resize
					if(HasDynamicAllocations())
					{
						mSecondaryAllocator.Resize(currentSize, newCapacity);
						mElements = mSecondaryAllocator.GetElements();
					}
					// Allocate dynamic and move from stack
					else
					{
						mSecondaryAllocator.Resize(0, newCapacity);

						std::uninitialized_move(
							(ElementType*)mStackStorage,
							((ElementType*)mStackStorage) + currentSize,
							mSecondaryAllocator.GetElements());

						mElements = mSecondaryAllocator.GetElements();
					}
				}
			}

			std::aligned_storage_t<sizeof(ElementType), std::min(alignof(std::max_align_t), alignof(ElementType))> mStackStorage[StackElementCount];
			ElementType* mElements = (ElementType*)mStackStorage;
			typename SecondaryAllocator::template ForElementType<ElementType> mSecondaryAllocator;
		};
	};



	/** @} */
	/** @} */
} // namespace b3d
