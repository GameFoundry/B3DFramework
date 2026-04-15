//************************************ B3D Framework - Copyright 2024 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Debug/B3DDebug.h"
#include "ECS/B3DEntity.h"
#include "ECS/B3DUtility.h"

#include <iterator>

#include "Utility/B3DTypeId.h"

namespace b3d::ecs
{
	/** @addtogroup ECS-Internal
	 *  @{
	 */

	// Note: Based on EnTT (https://github.com/skypjack/entt)

	/** Iterator that allows iteration over a container whose contents are contiguous in memory. */
	template<typename ContainerType>
	struct TContiguousContainerIterator final
	{
		using value_type = typename ContainerType::value_type;
		using pointer = typename ContainerType::const_pointer;
		using reference = typename ContainerType::const_reference;
		using difference_type = typename ContainerType::difference_type;
		using const_pointer = typename ContainerType::const_pointer;
		using const_reference = typename ContainerType::const_reference;
		using iterator_category = std::random_access_iterator_tag;

		TContiguousContainerIterator() = default;
		TContiguousContainerIterator(const ContainerType& container, u64 index)
			: mContainer(&container), mIndex(index) { }

		TContiguousContainerIterator& operator++()
		{
			++mIndex;
			return *this;
		}

		TContiguousContainerIterator& operator--()
		{
			B3D_ENSURE(mIndex > 0);

			--mIndex;
			return *this;
		}

		TContiguousContainerIterator& operator+=(u64 value)
		{
			mIndex += value;
			return *this;
		}

		TContiguousContainerIterator& operator-=(u64 value)
		{
			B3D_ENSURE(mIndex >= value);

			mIndex -= value;
			return *this;
		}

		TContiguousContainerIterator operator+(u64 value) const
		{
			TContiguousContainerIterator copy = *this;
			return (copy += value);
		}

		TContiguousContainerIterator operator-(u64 value) const
		{
			TContiguousContainerIterator copy = *this;
			return (copy -= value);
		}

		const_reference operator[](u64 value) const { return (*mContainer)[mIndex + value]; }
		const_pointer operator->() const { return std::addressof(operator[](0)); }
		const_reference operator*() const { return operator[](0); }

		u64 Index() const { return mIndex; }

	private:
		const ContainerType* mContainer = nullptr;
		u64 mIndex = 0;
	};

	template<typename ContainerType>
	i64 operator-(const TContiguousContainerIterator<ContainerType>& lhs, const TContiguousContainerIterator<ContainerType>& rhs)
	{
		return rhs.Index() - lhs.Index();
	}

	template<typename ContainerType>
	bool operator==(const TContiguousContainerIterator<ContainerType>& lhs, const TContiguousContainerIterator<ContainerType>& rhs)
	{
		return lhs.Index() == rhs.Index();
	}

	template<typename ContainerType>
	bool operator!=(const TContiguousContainerIterator<ContainerType>& lhs, const TContiguousContainerIterator<ContainerType>& rhs)
	{
		return !(lhs == rhs);
	}

	template<typename ContainerType>
	bool operator<(const TContiguousContainerIterator<ContainerType>& lhs, const TContiguousContainerIterator<ContainerType>& rhs)
	{
		return lhs.index() > rhs.index();
	}

	template<typename ContainerType>
	bool operator>(const TContiguousContainerIterator<ContainerType>& lhs, const TContiguousContainerIterator<ContainerType>& rhs)
	{
		return rhs < lhs;
	}

	template<typename ContainerType>
	bool operator<=(const TContiguousContainerIterator<ContainerType>& lhs, const TContiguousContainerIterator<ContainerType>& rhs)
	{
		return !(lhs > rhs);
	}

	template<typename ContainerType>
	bool operator>=(const TContiguousContainerIterator<ContainerType>& lhs, const TContiguousContainerIterator<ContainerType>& rhs)
	{
		return !(lhs < rhs);
	}

	/** @} */

	/** @addtogroup ECS
	 *  @{
	 */

	/** Determines how are entries treated when they are removed from a sparse set. */
	enum class SparseSetDeletePolicy
	{
		/**
		 * Entry will be deleted in place. Its value will be replaced with an invalid entity. Next time a new entry is allocated the entry may be re-used,
		 * in which case its entity version will be incremented. This policy is usually not suggested as it prevents fast iteration over the contents
		 * (as checks need to be made for deleted entries), but may be required in case the stored object does not support move operations.
		 */
		InPlace,

		/**
		 * Entry will be swapped with the last entry in the container, and then removed form the container. Entries will never be re-used.
		 * This is usually used for component storage.
		 */
		SwapAndErase,

		/**
		 * Entry will be swapped with the last entry in the container. This results in all valid entries being stored in the first part of the container,
		 * and all invalid entries stored in the last part of the container. When iterating such a container you need to retrieve the number of valid
		 * entries and only iterate up to that point, rather than the whole container. Deleted entries may be re-used, in which case the entity
		 * version will be incremented. This is usually used for entity storage.
		 */
		SwapOnly
	};

	/** @} */

	/** @addtogroup ECS-Internal
	 *  @{
	 */

	/**
	 * Similar to an array, but saves memory by not allocating data if there are large gaps in the stored indices. i.e. while a regular array would
	 * require you to allocate an array of size 100 000 to store an entry at index 99 999, sparse set will only allocate a single page of data.
	 *
	 * Internally the set works by allocating two arrays:
	 *  - Sparse: Stores lookup from the user-provided index to the internal packed array index. Uses pages to avoid allocating memory for unused index ranges.
	 *  - Packed: Stores all the data in one contiguous array.
	 *
	 * Packed array can be iterated as efficiently as a regular array, while lookups based on index come with a cost of an additional layer of
	 * indirection (packed[sparse[index]]), as well as some arithmetic for page calculation.
	 *
	 * @note	Page size is defined by B3D_ECS_SPARSE_SET_PAGE_SIZE. If you are storing indices that are close in page set size increments
	 *			note that memory use may be high. You may reduce memory usage by reducing the page size.
	 */
	class SparseSet
	{
		using SparseContainerType = TArray<TArrayView<Entity>>;
		using PackedContainerType = TArray<Entity>;
		static constexpr u32 SparsePageSize = B3D_ECS_SPARSE_SET_PAGE_SIZE;

		static_assert(SparsePageSize > 0 && (SparsePageSize & SparsePageSize - 1) == 0, "SparsePageSize must be a power of two value.");

	public:
		using Iterator = TContiguousContainerIterator<PackedContainerType>;
		using ConstIterator = Iterator;
		using ReverseIterator = std::reverse_iterator<Iterator>;
		using ConstReverseIterator = std::reverse_iterator<ConstIterator>;

		static constexpr u64 kMaximumEntryCount = Entity::kIdentifierMask;

		SparseSet(TypeHash elementTypeHash = B3DGetTypeHash<void>())
			: mElementTypeHash(elementTypeHash)
		{ }

		virtual ~SparseSet()
		{
			FreeSparsePages();
		}

		/** Returns a hash value that represents the element type stored in this set. */
		TypeHash GetElementTypeHash() const { return mElementTypeHash; }

		Entity operator[](u64 index)
		{
			return mPackedEntities[index];
		}

		/**
		 * Adds a new entity to the set and returns an iterator of the added entity.
		 * Caller must ensure not to add an entity with an index that's already in the set.
		 */
		Iterator Add(Entity entity)
		{
			Iterator iterator = AddInternal(entity, false);
			OnWasAdded(*iterator);
			return iterator;
		}

		/**
		 * Adds all entities from the provided iterator range to the set, and returns a pointer to the last added entity.
		 * Caller must ensure not to add an entity with an index that's already in the set.
		 */
		Iterator Add(Iterator first, Iterator last)
		{
			Iterator iterator = End();
			for(; first != last; ++first)
			{
				iterator = AddInternal(*first, true);
				OnWasAdded(*iterator);
			}

			return iterator;
		}

		/** Removes an entity from the set. Caller must ensure that entity is actually part of the set. */
		void Erase(Entity entity)
		{
			OnWillRemove(entity);
			EraseInternal(entity);
		}

		/** Removes all entities from the provided iterator range from the set. Caller must ensure that entity is actually part of the set. */
		void Erase(Iterator first, Iterator last)
		{
			for(; first != last; ++first)
			{
				OnWillRemove(*first);
				EraseInternal(*first);
			}
		}

		/** Removes an entity from the set if it exists, otherwise does nothing. Returns true if entity was removed. */
		bool EraseIfValid(Entity entity)
		{
			if(Contains(entity))
			{
				Erase(entity);
				return true;
			}

			return false;
		}

		/**
		 * Removes all existing entities from the provided iterator range from the set. Ignores entities that are not part of the set.
		 * Returns number of entities that were removed.
		 */
		u64 EraseIfValid(Iterator first, Iterator last)
		{
			u64 removedEntryCount = 0;

			while(first != last)
			{
				while(first != last && !Contains(*first))
					++first;

				const auto iterator = first;

				while(first != last && Contains(*first))
					++first;

				removedEntryCount += (u64)std::distance(iterator, first);
				Erase(iterator, first);
			}

			return removedEntryCount;
		}

		/** Removes everything from the set and clears the internal arrays. */
		virtual void Clear()
		{
			ClearInternal();
		}

		/** Removes all invalid entities from the set. Only relevant for sets using in-place deletion policy. */
		virtual void ClearInvalid()
		{
		}

		/** Shrinks the memory use of the set to accomodate the currently assigned entries, without any reserve for new entries. */
		virtual void Shrink()
		{
			const u64 maximumPageCount = (u64)mSparseIndices.Size();
			u64 createdPageCount = 0;

			SparseContainerType newSparseIndices;
			newSparseIndices.Reserve(maximumPageCount);

			for(const auto& entity : mPackedEntities)
			{
				if(entity == kInvalidEntity)
					continue;

				const u64 entityIdentifier = entity.GetIdentifier();
				const u64 sparsePage = GetSparsePage(entityIdentifier);

				if(mSparseIndices[sparsePage].IsEmpty())
					continue;

				if(sparsePage >= (u64)newSparseIndices.Size())
					newSparseIndices.Resize(sparsePage + 1);

				newSparseIndices[sparsePage] = std::exchange(mSparseIndices[sparsePage], {});
				createdPageCount++;

				// Early exit
				if(createdPageCount == maximumPageCount)
					break;
			}

			FreeSparsePages();
			mSparseIndices = std::move(newSparseIndices);

			mSparseIndices.Shrink();
			mPackedEntities.Shrink();
		}

		/** Reserves internal memory in order to fit @p capacity entries. */
		virtual void Reserve(u64 capacity)
		{
			mPackedEntities.Reserve(capacity);
		}

		/** Returns how many entries can fit into the internal memory. */
		virtual u64 Capacity() const
		{
			return mPackedEntities.Capacity();
		}

		/** Returns the current version of the provided entity, or invalid version if entity is not part of this set. */
		Entity::VersionType GetVersion(Entity entity) const
		{
			Entity* const sparseEntry = GetSparseEntryPointer(entity);
			if(sparseEntry == nullptr)
				return ((Entity)kInvalidEntity).GetVersion();
			
			return sparseEntry->GetVersion();
		}

		/** Updates the version for the entity with identifier as specified by @p entity. New version is taken from the provided @p entity parameter. */
		void UpdateVersion(Entity entity)
		{
			Entity& sparseEntry = GetSparseEntryReference(entity);
			sparseEntry = Entity(sparseEntry.GetIdentifier(), entity.GetVersion());

			const u64 packedEntryIndex = sparseEntry.GetIdentifier();
			mPackedEntities[packedEntryIndex] = entity;
		}

		/** Returns the iterator to an entity with the provided identifier and version. Returns an iterator past the edge of the set if no entity is found. */
		Iterator Find(Entity entity) const
		{
			return Contains(entity) ? GetIterator(entity) : End();
		}

		/** Returns true if the set contains an entity with the provided identifier and version. */
		bool Contains(Entity entity) const
		{
			Entity* const sparseEntry = GetSparseEntryPointer(entity);
			if(sparseEntry == nullptr)
				return false;

			if(sparseEntry->GetVersion() != entity.GetVersion())
				return false;

			return true;
		}

		/** Returns an index of an entity in the internal packed array. If the entity is not part of the set, behaviour is undefined. */
		u64 GetPackedIndex(Entity entity) const
		{
			return GetSparseEntryReference(entity).GetIdentifier();
		}

		/** Returns the number of entities in the set. Note for in-place and swap-only deletion policies this will also count the number of invalid entries. */
		u64 Size() const { return mPackedEntities.Size(); }

		/** Returns true if no entries are stored in the set. */
		bool IsEmpty() const { return mPackedEntities.Empty(); }

		/** Returns raw pointer to the internal packed entity array. */
		const Entity* Data() const { return mPackedEntities.Data(); }

		/** Swaps the location of the provided entities in the packed array, as well as the relevant payload (data associated with the entities), if any. */
		virtual void Swap(Entity lhs, Entity rhs) = 0;

		/** Returns the current delete policy. See SparseSetDeletePolicy. */
		virtual SparseSetDeletePolicy GetDeletePolicy() const { return SparseSetDeletePolicy::SwapAndErase; }

		/**
		 * When using in-place deletion policy returns the packed index to the first invalid element.
		 * When using swap-only deletion policy returns the number of valid entities in the set.
		 * Not relevant if using swap-and-erase deletion policy.
		 */
		virtual u64 GetFirstFreeElementPackedIndex() const { return kMaximumEntryCount; }

		/** Returns an iterator to the start of the internal packed entity array. */
		Iterator Begin() const { return Iterator(mPackedEntities, 0); }
		ConstIterator Cbegin() const { return Begin(); }

		/** Returns an iterator to the past the end of the internal packed entity array. */
		Iterator End() const { return Iterator(mPackedEntities, Size()); }
		ConstIterator Cend() const { return End(); }

		/** Returns a reverse iterator to the last element of the internal packed entity array. */
		ReverseIterator Rbegin() const { return std::make_reverse_iterator(End()); }
		ConstReverseIterator Crbegin() const { return Rbegin(); }

		/** Returns a reverse iterator before the first element of the internal packed entity array. */
		ReverseIterator Rend() const { return std::make_reverse_iterator(Begin()); }
		ConstReverseIterator Crend() const { return Rend(); }

		// For std compatibility
		using iterator = Iterator;
		using const_iterator = ConstIterator;
		using reverse_iterator = ReverseIterator;
		using const_reverse_iterator = ConstReverseIterator;

		iterator begin() const { return Begin(); }
		const_iterator cbegin() const { return Cbegin(); }

		iterator end() const { return End(); }
		const_iterator cend() const { return Cend(); }

		reverse_iterator rbegin() const { return Rbegin(); }
		const_reverse_iterator crbegin() const { return Crbegin(); }

		reverse_iterator rend() const { return Rend(); }
		const_reverse_iterator crend() const { return Crend(); }

		Event<void(Entity), ThreadUnsafe> OnWasAdded; /**< Triggers any time a new entity is added to the set. */
		Event<void(Entity), ThreadUnsafe> OnWillRemove; /**< Triggers right before an entity is removed from the set. */
		Event<void(Entity), ThreadUnsafe> OnWasUpdated; /**< Triggers when an existing component is updated or replaced. Must be explicitly triggered by the caller. */
	protected:
		/**
		 * Adds a new entity to the set.
		 *
		 * @param	entity					Entity to add.
		 * @param	forceAddAtEnd			Only relevant when using in-place deletion policy. When true it will add an entity at the end of the packaged data array,
		 *									rather than re-using the first available invalid entity entry.
		 * @return							Iterator to the added entity.
		 */
		virtual Iterator AddInternal(Entity entity, bool forceAddAtEnd) { return End(); }

		/** Removed an entity from the sparse set. Entity must be a part of the sparse set. */
		virtual void EraseInternal(Entity entity) { }

		/** Removes everything from the set and clears the internal arrays. */
		void ClearInternal()
		{
			FreeSparsePages();
			mPackedEntities.Clear();
		}

		/**
		 * Swaps the location of two entities.
		 *
		 * @param	lhsPackedIndex		Index within the packed array of the first element to swap.
		 * @param	rhsPackedIndex		Index within the packed array of the second element to swap.
		 */
		void SwapEntities(u64 lhsPackedIndex, u64 rhsPackedIndex)
		{
			Entity& lhsPackedEntry = mPackedEntities[lhsPackedIndex];
			Entity& rhsPackedEntry = mPackedEntities[rhsPackedIndex];

			Entity& lhsSparseEntry = GetSparseEntryReference(lhsPackedEntry);
			Entity& rhsSparseEntry = GetSparseEntryReference(rhsPackedEntry);

			lhsSparseEntry = Entity((Entity::Traits::IdentifierType)rhsPackedIndex, lhsPackedEntry.GetVersion());
			rhsSparseEntry = Entity((Entity::Traits::IdentifierType)lhsPackedIndex, rhsPackedEntry.GetVersion());

			std::swap(lhsPackedEntry, rhsPackedEntry);
		}

		/**
		 * Retrieves an existing entry from the sparse array, or adds a new entry if one doesn't already exist. Note that returned entity
		 * is not a regular entity, but rather its identifier is an index into the packed array.
		 */
		Entity& GetOrCreateSparseEntryReference(Entity entity)
		{
			const u64 entityIdentifier = entity.GetIdentifier();
			const u64 sparsePage = GetSparsePage(entityIdentifier);

			if(sparsePage >= mSparseIndices.Size())
				mSparseIndices.Resize(sparsePage + 1);

			if(mSparseIndices[sparsePage].IsEmpty())
			{
				mSparseIndices[sparsePage] = TArrayView<Entity>(B3DAllocateMultiple<Entity>(SparsePageSize), SparsePageSize);
				std::uninitialized_fill(mSparseIndices[sparsePage].Data(), mSparseIndices[sparsePage].Data() + SparsePageSize, kNullEntity);
			}

			return mSparseIndices[sparsePage][GetSparseIndexWithinPage(entityIdentifier)];
		}

		/**
		 * Retrieves an existing entry from the sparse array. Note that returned entity is not a regular entity, but rather its
		 * identifier is an index into the packed array. Caller must ensure the entity is part of the set before calling.
		 */
		Entity& GetSparseEntryReference(Entity value) const
		{
			const u64 entityIdentifier = value.GetIdentifier();
			return const_cast<Entity&>(mSparseIndices[GetSparsePage(entityIdentifier)][GetSparseIndexWithinPage(entityIdentifier)]);
		}

		/**
		 * Attempts to retrieve an existing entry from the sparse array, or null if one cannot be found. Note that returned entity
		 * is not a regular entity, but rather its identifier is an index into the packed array. 
		 */
		Entity* GetSparseEntryPointer(Entity value) const
		{
			const u64 entityIdentifier = value.GetIdentifier();
			const u64 sparsePage = GetSparsePage(entityIdentifier);

			if(sparsePage < mSparseIndices.Size() && !mSparseIndices[sparsePage].IsEmpty())
				return const_cast<Entity*>(&mSparseIndices[sparsePage][GetSparseIndexWithinPage(entityIdentifier)]);

			return nullptr;
		}

		/** Returns an iterator to the provided entity. Caller must ensure the entity is part of the set before calling. */
		Iterator GetIterator(Entity entity) const
		{
			return Iterator(mPackedEntities, (i64)GetPackedIndex(entity));
		}

		/** Frees any sparse pages that don't contain any entries. */
		void FreeSparsePages()
		{
			for(auto&& page : mSparseIndices)
			{
				if(!page.IsEmpty())
				{
					std::destroy(page.Data(), page.Data() + SparsePageSize);
					B3DFree(page.Data());
					page = {};
				}
			}
			mSparseIndices.Clear();
		}

		/** Converts an index into the packed array into an entity identifier. */
		static constexpr Entity::IdentifierType GetPackedIndexAsEntryIdentifier(u64 packedIndex)
		{
			return (Entity::IdentifierType)packedIndex;
		}

		/** Calculates an index within a page, for an entity with the provided identifier. */
		static constexpr u64 GetSparseIndexWithinPage(u64 entityIdentifier)
		{
			return entityIdentifier & (SparsePageSize - 1);
		}

		/** Calculates the page at which to store the entity with the provided identifier. */
		static constexpr u64 GetSparsePage(u64 entityIdentifier)
		{
			return entityIdentifier / SparsePageSize;
		}

		SparseContainerType mSparseIndices; /**< List of pages that map entity identifier into packed array indices. */

		// Note: Might consider paging this. It won't be continous anymore, making it harder to iterate, but adding entries might prevent expensive resizes if there's a lot of entries
		PackedContainerType mPackedEntities; /**< Packed array of entities. */

		TypeHash mElementTypeHash = 0;
	};

	/** @} */

	/** @addtogroup ECS
	 *  @{
	 */

	/** Implements features of SparseSet that depend on a particular deletion policy. */
	template<SparseSetDeletePolicy DeletePolicy>
	class TSparseSet : public SparseSet
	{
		using Super = SparseSet;
	public:
		static constexpr SparseSetDeletePolicy kDeletePolicy = DeletePolicy;

		TSparseSet()
			:SparseSet(B3DGetTypeHash<void>())
		{ }

		TSparseSet(TypeHash elementTypeHash)
			:SparseSet(elementTypeHash)
		{ }

		~TSparseSet() override = default;

		void Clear() override
		{
			if constexpr(DeletePolicy == SparseSetDeletePolicy::InPlace)
			{
				for(auto it = Super::Begin(); it != Super::End(); ++it)
				{
					if(*it != kInvalidEntity)
						Super::OnWillRemove(*it);
				}
			}
			else if constexpr(DeletePolicy == SparseSetDeletePolicy::SwapOnly)
			{
				const Iterator end = Super::Begin() + GetFirstFreeElementPackedIndex();
				for(auto it = Super::Begin(); it != end; ++it)
					Super::OnWillRemove(*it);
				
			}
			else
			{
				for(auto it = Super::Begin(); it != Super::End(); ++it)
					Super::OnWillRemove(*it);
			}

			ClearInternal();
		}

		void ClearInvalid() override
		{
			ClearInvalidInternal<TSparseSet, &TSparseSet::MoveOrSwapPayload>();
		}

		void Swap(Entity lhs, Entity rhs) override
		{
			SwapInternal<TSparseSet, &TSparseSet::MoveOrSwapPayload>(lhs, rhs);
		}

		/**
		 * Sorts the entity packed data to match the order of the provided entities.
		 * Note sorting operation will fail if using in-place deletion policy and invalid (deleted) entries are present.
		 *
		 * @param	first	Iterator pointing to the first entity.
		 * @param	last	Iterator pointing to the last entity.
		 * @return			Iterator one past the last entity that was sorted (begin() if nothing was sorted).
		 */
		template<typename It>
		Iterator SortAs(It first, It last)
		{
			return SortAsInternal<TSparseSet, &TSparseSet::MoveOrSwapPayload>(first, last);
		}

		/**
		 * Sorts the first @p count entities using the provided comparison function.
		 * Note sorting operation will fail if using in-place deletion policy and invalid (deleted) entries are present.
		 *
		 * @param	count		Number of entities to sort.
		 * @param	predicate	Function used to sort the entities.
		 */
		template<typename ComparisonFunction = std::less<>>
		void SortN(u64 count, ComparisonFunction predicate = ComparisonFunction{})
		{
			return SortNInternal<TSparseSet, &TSparseSet::MoveOrSwapPayload, ComparisonFunction>(count, std::move(predicate));
		}

		/**
		 * Sorts all entities using the provided comparison function.
		 * Note sorting operation will fail if using in-place deletion policy and invalid (deleted) entries are present.
		 */
		template<typename ComparisonFunction = std::less<>>
		void Sort(ComparisonFunction predicate = ComparisonFunction{})
		{
			return SortInternal<TSparseSet, &TSparseSet::MoveOrSwapPayload, ComparisonFunction>(std::move(predicate));
		}

		SparseSetDeletePolicy GetDeletePolicy() const override { return DeletePolicy; }
		u64 GetFirstFreeElementPackedIndex() const override { return mFreeListHead; }

	protected:
		Iterator AddInternal(Entity entity, bool forceAddAtEnd) override
		{
			Entity& sparseSetEntry = GetOrCreateSparseEntryReference(entity);
			u64 packedEntryIndex = mPackedEntities.Size();

			// Add entity at first available spot, or at the end if requested
			if constexpr(DeletePolicy == SparseSetDeletePolicy::InPlace)
			{
				B3D_ENSURE(sparseSetEntry == kNullEntity);

				// If there is a free spot (i.e. invalid (deleted) entity), and not forcing add to end, add there
				if(mFreeListHead != kMaximumEntryCount && !forceAddAtEnd)
				{
					packedEntryIndex = mFreeListHead;
					sparseSetEntry = Entity(GetPackedIndexAsEntryIdentifier(mFreeListHead), entity.GetVersion());
					mFreeListHead = (u64)(std::exchange(mPackedEntities[packedEntryIndex], entity).GetIdentifier());
				}
				// Otherwise add at the end of the packed entities array
				else
				{
					mPackedEntities.Add(entity);
					sparseSetEntry = Entity(GetPackedIndexAsEntryIdentifier(packedEntryIndex), entity.GetVersion());
				}
			}
			// Always add at the end of the packed entities array, as all existing entries are valid
			else if constexpr(DeletePolicy == SparseSetDeletePolicy::SwapAndErase)
			{
				mPackedEntities.Add(entity);
				B3D_ENSURE(sparseSetEntry == kNullEntity);
				sparseSetEntry = Entity(GetPackedIndexAsEntryIdentifier(packedEntryIndex), entity.GetVersion());
			}
			else
			{
				if(sparseSetEntry == kNullEntity)
				{
					mPackedEntities.Add(entity);
					sparseSetEntry = Entity(GetPackedIndexAsEntryIdentifier(packedEntryIndex), entity.GetVersion());
				}
				else
				{
					B3D_ENSURE(!(sparseSetEntry.GetIdentifier() < mFreeListHead));
					sparseSetEntry = sparseSetEntry.GetAsNextVersion();
				}

				packedEntryIndex = mFreeListHead++;
				SwapEntities(sparseSetEntry.GetIdentifier(), packedEntryIndex);
			}

			return Begin() + packedEntryIndex;
		}

		void EraseInternal(Entity entity) override
		{
			if constexpr(DeletePolicy == SparseSetDeletePolicy::InPlace)
			{
				// Sparse entry is set to a null value, while packed entry points to the next free packed entry, and its marked as invalid via its version
				const u64 packedEntryIndex = std::exchange(GetSparseEntryReference(entity), kNullEntity).GetIdentifier();
				mPackedEntities[packedEntryIndex] = Entity(GetPackedIndexAsEntryIdentifier(std::exchange(mFreeListHead, packedEntryIndex)), ((Entity)kInvalidEntity).GetVersion());
			}
			else if constexpr(DeletePolicy == SparseSetDeletePolicy::SwapAndErase)
			{
				// Set last sparse entry so it points to the packed index of the entry that was removed, swap packed entries
				Entity& sparseEntryToRemove = GetSparseEntryReference(entity);
				Entity& lastSparseEntry = GetSparseEntryReference(mPackedEntities.Back());

				const u64 packedEntryToRemoveIndex = sparseEntryToRemove.GetIdentifier();
				lastSparseEntry = Entity(GetPackedIndexAsEntryIdentifier(packedEntryToRemoveIndex), mPackedEntities.Back().GetVersion());
				mPackedEntities[packedEntryToRemoveIndex] = mPackedEntities.Back();

				sparseEntryToRemove = kNullEntity;
				mPackedEntities.Pop();
			}
			else
			{
				const u64 packedEntryIndex = GetPackedIndex(entity);
				UpdateVersion(entity.GetAsNextVersion());

				mFreeListHead -= (packedEntryIndex < mFreeListHead);
				SwapEntities(packedEntryIndex, mFreeListHead);
			}
		}

		/** Same as SparseSet::ClearInternal but also resets the free list head. */
		void ClearInternal()
		{
			Super::ClearInternal();
			mFreeListHead = DeletePolicy != SparseSetDeletePolicy::SwapOnly ? Super::kMaximumEntryCount : 0;
		}

		/**
		 * Helper for ClearInvalid(). Clears all invalid entities, and allows the caller to provide a function
		 * that also clears the associated payload data, if any. @p MoveOrSwapPayload accepts two parameters,
		 * packed index from which to move the payload, and packed index where to move the payload, respectively.
		 */
		template<typename T, void(T::*MoveOrSwapPayload)(u64, u64)>
		void ClearInvalidInternal()
		{
			if constexpr(DeletePolicy != SparseSetDeletePolicy::InPlace)
				return;

			u64 validPackedEntryIndex = mPackedEntities.Size();
			u64 freePackedEntryIndex = std::exchange(mFreeListHead, kMaximumEntryCount);

			// Find first valid entry
			for(; validPackedEntryIndex > 0 && mPackedEntities[validPackedEntryIndex - 1] == kInvalidEntity; --validPackedEntryIndex) { }

			while(freePackedEntryIndex != kMaximumEntryCount)
			{
				if(freePackedEntryIndex < validPackedEntryIndex)
				{
					--validPackedEntryIndex;

					// Move the free entry to the back
					Entity& fromPackedEntry = mPackedEntities[validPackedEntryIndex];
					Entity& toPackedEntry = mPackedEntities[freePackedEntryIndex];

					(((T*)this)->*MoveOrSwapPayload)(validPackedEntryIndex, freePackedEntryIndex);
					GetSparseEntryReference(fromPackedEntry) = Entity((Entity::Traits::IdentifierType)freePackedEntryIndex, toPackedEntry.GetVersion());

					// Find next free entry
					freePackedEntryIndex = mPackedEntities[freePackedEntryIndex].GetIdentifier();

					toPackedEntry = fromPackedEntry;

					// Find next valid entry
					for(; validPackedEntryIndex > 0 && mPackedEntities[validPackedEntryIndex - 1] == kInvalidEntity; --validPackedEntryIndex) { }
				}
				// Already at the end
				else
				{
					// Find next free entry
					freePackedEntryIndex = mPackedEntities[freePackedEntryIndex].GetIdentifier();
				}
			}

			mPackedEntities.Erase(mPackedEntities.begin() + validPackedEntryIndex, mPackedEntities.end());
		}

		/**
		 * Helper for SortAs(). Allows the caller to provide a function that also moves the associated payload data.
		 * @p MoveOrSwapPayload accepts two parameters, packed index from which to move the payload, and packed
		 * index where to move the payload, respectively.
		 */ 
		template<typename T, void(T::*MoveOrSwapPayload)(u64, u64), typename It>
		Iterator SortAsInternal(It first, It last)
		{
			if(!B3D_ENSURE(GetDeletePolicy() != SparseSetDeletePolicy::InPlace || mFreeListHead == Super::kMaximumEntryCount))
				return Begin();

			auto localIterator = Begin();
			const u64 validPackedEntryCount = (GetDeletePolicy() == SparseSetDeletePolicy::SwapOnly) ? mFreeListHead : mPackedEntities.size();

			for(const auto localLast = Begin() + validPackedEntryCount; (localIterator != localLast) && (first != last); ++first)
			{
				if(auto entity = *first; Contains(entity))
				{
					if(auto localEntity = *localIterator; localEntity != entity)
					{
						const u64 packedIndexFrom = GetPackedIndex(localEntity);
						const u64 packedIndexTo = GetPackedIndex(entity);

						(((T*)this)->*MoveOrSwapPayload)(packedIndexFrom, packedIndexTo);
						SwapEntities(packedIndexFrom, packedIndexTo);
					}

					++localIterator;
				}
			}

			return localIterator;
		}

		/**
		 * Helper for SortN(). Allows the caller to provide a function that also moves the associated payload data.
		 * @p MoveOrSwapPayload accepts two parameters, packed index from which to move the payload, and packed
		 * index where to move the payload, respectively.
		 */ 
		template<typename T, void(T::*MoveOrSwapPayload)(u64, u64), typename ComparisonFunction = std::less<>>
		void SortNInternal(u64 count, ComparisonFunction predicate = ComparisonFunction{})
		{
			if(!B3D_ENSURE(GetDeletePolicy() != SparseSetDeletePolicy::InPlace || mFreeListHead == Super::kMaximumEntryCount))
				return;

			if(!B3D_ENSURE(count <= mPackedEntities.Size()))
				return;

			std::sort(mPackedEntities.Begin(), mPackedEntities.Begin() + count, std::move(predicate));

			for(u64 rootPackedIndexToCheck = 0; rootPackedIndexToCheck < count; ++rootPackedIndexToCheck)
			{
				u64 packedIndexToCheck = rootPackedIndexToCheck;
				u64 originalPackedIndex = GetPackedIndex(mPackedEntities[packedIndexToCheck]);

				while(packedIndexToCheck != originalPackedIndex)
				{
					const u64 nextPackedIndex = GetPackedIndex(mPackedEntities[originalPackedIndex]);
					const Entity entity = mPackedEntities[packedIndexToCheck];

					(((T*)this)->*MoveOrSwapPayload)(originalPackedIndex, nextPackedIndex);

					// Make sure the sparse entry points to the new index
					GetSparseEntryReference(entity) = Entity(GetPackedIndexAsEntryIdentifier(packedIndexToCheck), mPackedEntities[packedIndexToCheck].GetVersion());
					packedIndexToCheck = std::exchange(originalPackedIndex, nextPackedIndex);
				}
			}
		}

		/**
		 * Helper for Sort(). Allows the caller to provide a function that also moves the associated payload data.
		 * @p MoveOrSwapPayload accepts two parameters, packed index from which to move the payload, and packed
		 * index where to move the payload, respectively.
		 */ 
		template<typename T, void(T::*MoveOrSwapPayload)(u64, u64), typename ComparisonFunction = std::less<>>
		void SortInternal(ComparisonFunction predicate = ComparisonFunction{})
		{
			const u64 validPackedEntryCount = (GetDeletePolicy() == SparseSetDeletePolicy::SwapOnly) ? mFreeListHead : mPackedEntities.size();
			SortNInternal<T, MoveOrSwapPayload>(validPackedEntryCount, std::move(predicate));
		}

		/**
		 * Helper for Swap(). Allows the caller to provide a function that also moves the associated payload data.
		 * @p MoveOrSwapPayload accepts two parameters, packed index from which to move the payload, and packed
		 * index where to move the payload, respectively.
		 */ 
		template<typename T, void(T::*MoveOrSwapPayload)(u64, u64)>
		void SwapInternal(Entity lhs, Entity rhs)
		{
			const u64 packedIndexLhs = GetPackedIndex(lhs);
			const u64 packedIndexRhs = GetPackedIndex(rhs);

			(((T*)this)->*MoveOrSwapPayload)(packedIndexLhs, packedIndexRhs);
			SwapEntities(packedIndexLhs, packedIndexRhs);
		}

	private:
		/** Default implementation that doesn't do anything as there is no payload to move. */
		void MoveOrSwapPayload(u64, u64)
		{
			// Do nothing
		}

		/**
		 * For in-place delete policy points to first free entry, or kMaximumEntryCount if no free entries.
		 * For swap-only delete policy points to the first free entry, where all other elements are sequentially following the first free element. This value corresponds to valid entry count.
		 * For swap-and-erase delete policy this value is not used.
		 */
		u64 mFreeListHead = DeletePolicy != SparseSetDeletePolicy::SwapOnly ? Super::kMaximumEntryCount : 0;
	};

	/** @} */

	/** @addtogroup ECS-Internal
	 *  @{
	 */

	/** Checks that all provided ECS storage types use InPlace deletion policy. */
	template<typename... StorageType>
	static constexpr bool TAllTypesUseInPlaceDelete = ((sizeof...(StorageType) == 1u) && ... && (StorageType::kDeletePolicy == SparseSetDeletePolicy::InPlace));

	template<typename Type, typename = void>
	struct StorageForType;

	/**
	 * Returns the appropriate storage type for use for the specified type. Some of the things taken into account:
	 * - If provided type is empty, storage with no payload array is used (e.g. for tags)
	 * - If provided type is Entity, storage that can be used for generating new entities is used
	 * - Otherwise a storage is used that can contain payload of Type. Constness of Type is taken account
	 */
	template<typename Type>
	using TStorageType = typename StorageForType<Type>::StorageType;

	/** @} */
} // namespace b3d::ecs
