//************************************ bs::framework - Copyright 2024 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsUtilityPrerequisites.h"
#include "Debug/BsDebug.h"

#include <iterator>

namespace bs::ecs
{
	/** @addtogroup General
	 *  @{
	 */

	// Note: Based on EnTT (https://github.com/skypjack/entt)

	// TODO - Move to utility library
	template<typename Type>
	constexpr std::enable_if_t<std::is_unsigned_v<Type>, u32> CountBits(Type value)
	{
		return value ? ((u32)(value & 1) + CountBits(static_cast<Type>(value >> 1))) : 0;
	}
	template<typename>
	struct TEntityTypeTraits;

	template<>
	struct TEntityTypeTraits<u32>
	{
		using StorageType = u32;
		using IdentifierType = u32;
		using VersionType = u16;

		static constexpr u32 kIdentifierBitCount = 20;
		static constexpr u32 kVersionBitCount  = 12;
		static constexpr StorageType kIdentifierMask = (1 << kIdentifierBitCount) - 1;
		static constexpr StorageType kVersionMask = (1 << kVersionBitCount) - 1;
	};

	template<>
	struct TEntityTypeTraits<u64>
	{
		using StorageType = u64;
		using IdentifierType = u64;
		using VersionType = u32;

		static constexpr u32 kIdentifierBitCount = 32;
		static constexpr u32 kVersionBitCount  = 32;
	};

	template<typename Type>
	struct TEntity
	{
		using Traits = TEntityTypeTraits<Type>;
		using StorageType = typename Traits::StorageType;
		using IdentifierType = typename Traits::IdentifierType;
		using VersionType = typename Traits::VersionType;

		static constexpr StorageType kIdentifierMask = (1 << Traits::kIdentifierBitCount) - 1;
		static constexpr StorageType kVersionMask = (1 << Traits::kVersionBitCount) - 1;

		constexpr TEntity() = default;
		constexpr TEntity(IdentifierType identifier, VersionType version)
		{
			if constexpr(Traits::kVersionMask == 0u)
				IdentifierAndVersion = (Type)(identifier & Traits::kIdentifierMask);
			else
				IdentifierAndVersion = ((Type)(identifier & Traits::kIdentifierMask)) | (((Type)(version & Traits::kVersionMask)) << Traits::kIdentifierBitCount);
		}

		bool operator==(TEntity other) const
		{
			return IdentifierAndVersion == other.IdentifierAndVersion;
		}

		bool operator!=(TEntity other) const
		{
			return IdentifierAndVersion != other.IdentifierAndVersion;
		}

		constexpr IdentifierType GetIdentifier() const
		{
			return (IdentifierType)(IdentifierAndVersion) & Traits::kIdentifierMask;
		}

		constexpr VersionType GetVersion() const
		{
			if constexpr(Traits::kVersionMask == 0u)
				return VersionType{};

			return (Traits::VersionType)(IdentifierAndVersion >> Traits::kIdentifierBitCount) & Traits::kVersionMask;
		}

		constexpr TEntity GetAsNextVersion() const
		{
			return TEntity(GetIdentifier(), GetVersion() + 1);
		}
		
		union
		{
			struct
			{
				StorageType Identifier : Traits::kIdentifierBitCount;
				StorageType Version : Traits::kVersionBitCount;
			};

			StorageType IdentifierAndVersion;
		};
	};

#define B3D_ECS_64BIT_ENTITIES 0

#if B3D_ECS_64BIT_ENTITIES
	using Entity = TEntity<u64>;
#else
	using Entity = TEntity<u32>;
#endif

	// TODO - Doc
	struct NullEntity
	{
		template<typename Type>
		constexpr operator TEntity<Type>() const
		{
			return TEntity<Type>(TEntity<Type>::Traits::kIdentifierMask, TEntity<Type>::Traits::kVersionMask);
		}

		constexpr bool operator==(NullEntity) const
		{
			return true;
		}

		constexpr bool operator!=(NullEntity) const
		{
			return false;
		}

		template<typename Type>
		constexpr bool operator==(TEntity<Type> value) const
		{
			return value.GetIdentifier() == ((TEntity<Type>)*this).GetIdentifier();
		}

		template<typename Type>
		constexpr bool operator!=(TEntity<Type> value) const
		{
			return !(value == *this);
		}
	};

	template<typename Type>
	constexpr bool operator==(TEntity<Type> lhs, NullEntity rhs)
	{
		return rhs.operator==(lhs);
	}

	template<typename Type>
	constexpr bool operator!=(TEntity<Type> lhs, NullEntity rhs)
	{
		return !(rhs == lhs);
	}

	// TODO - Doc
	struct InvalidEntity
	{
		template<typename Type>
		constexpr operator TEntity<Type>() const
		{
			return TEntity<Type>(TEntity<Type>::Traits::kIdentifierMask, TEntity<Type>::Traits::kVersionMask);
		}

		constexpr bool operator==(InvalidEntity) const
		{
			return true;
		}

		constexpr bool operator!=(InvalidEntity) const
		{
			return false;
		}

		template<typename Type>
		constexpr bool operator==(TEntity<Type> value) const
		{
			if constexpr(TEntityTypeTraits<Type>::kVersionMask == 0u)
				return false;

			return value.GetVersion() == ((TEntity<Type>)*this).GetVersion();
		}

		template<typename Type>
		constexpr bool operator!=(Type value) const
		{
			return !(value == *this);
		}
	};

	template<typename Type>
	constexpr bool operator==(Type lhs, InvalidEntity rhs)
	{
		return rhs.operator==(lhs);
	}

	template<typename Type>
	constexpr bool operator!=(Type lhs, InvalidEntity rhs)
	{
		return !(rhs == lhs);
	}

	inline constexpr NullEntity kNullEntity;
	inline constexpr InvalidEntity kInvalidEntity;

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

	// TODO - Doc
	enum class SparseSetDeletePolicy
	{
		InPlace,
		SwapAndErase,
		SwapOnly
	};

	template<SparseSetDeletePolicy DeletePolicy = SparseSetDeletePolicy::SwapAndErase, u64 SparsePageSize = 4096>
	class TSparseSet
	{
		using SparseContainerType = TArray<Entity*>;
		using PackedContainerType = TArray<Entity>;

		static_assert(SparsePageSize && (SparsePageSize & SparsePageSize - 1) == 0, "SparsePageSize must be a power of two value.");

		static constexpr u64 kMaximumEntryCount = Entity::Traits::kIdentifierMask;
	public:
		using Iterator = TContiguousContainerIterator<PackedContainerType>;
		using ConstIterator = Iterator;
		using ReverseIterator = std::reverse_iterator<Iterator>;
		using ConstReverseIterator = std::reverse_iterator<ConstIterator>;

		TSparseSet() = default;
		virtual ~TSparseSet()
		{
			FreeSparsePages();
		}

		Entity operator[](u64 index)
		{
			return mPackedEntities[index];
		}

		Iterator Add(Entity entity)
		{
			return AddInternal(entity, false);
		}

		Iterator Add(Iterator first, Iterator last)
		{
			Iterator iterator = End();
			for(; first != last; ++first)
				iterator = AddInternal(*first, true);

			return iterator;
		}

		void Erase(Entity entity)
		{
			EraseInternal(entity);
		}

		void Erase(Iterator first, Iterator last)
		{
			for(; first != last; ++first)
				EraseInternal(*first);
		}

		bool EraseValid(Entity entity)
		{
			if(Contains(entity))
			{
				Erase(entity);
				return true;
			}

			return false;
		}

		u64 EraseValid(Iterator first, Iterator last)
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

		virtual void Clear()
		{
			FreeSparsePages();
			mPackedEntities.Clear();
			mFreeListHead = DeletePolicy != SparseSetDeletePolicy::SwapOnly ? kMaximumEntryCount : 0;
		}

		virtual void ClearInvalid()
		{
			auto fnMoveOrSwap = [](u64, u64) { };
			ClearInvalidInternal<TSparseSet, &TSparseSet::MoveOrSwapPayload>();
		}

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

				if(mSparseIndices[sparsePage] == nullptr)
					continue;

				if(sparsePage >= (u64)newSparseIndices.Size())
					newSparseIndices.Resize(sparsePage + 1, nullptr);

				newSparseIndices[sparsePage] = std::exchange(mSparseIndices[sparsePage], nullptr);
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

		virtual void Reserve(u64 capacity)
		{
			if(capacity == 0)
				return;

			const u64 requiredPageCapacity = GetSparsePage(capacity - 1) + 1;
			if(requiredPageCapacity > (u64)mSparseIndices.Size())
				mSparseIndices.Resize(requiredPageCapacity, nullptr);

			mPackedEntities.Reserve(capacity);
		}

		virtual u64 Capacity() const
		{
			return mPackedEntities.Capacity();
		}

		Entity::VersionType GetVersion(Entity entity) const
		{
			Entity* const sparseEntry = GetSparseEntryPointer(entity);
			if(sparseEntry == nullptr)
				return ((Entity)kInvalidEntity).GetVersion();
			
			return sparseEntry->GetVersion();
		}

		void UpdateVersion(Entity entity)
		{
			Entity& sparseEntry = GetSparseEntryReference(entity);
			sparseEntry = Entity(sparseEntry.GetIdentifier(), entity.GetVersion());

			const u64 packedEntryIndex = sparseEntry.GetIdentifier();
			mPackedEntities[packedEntryIndex] = entity;
		}

		Iterator Find(Entity entity) const
		{
			return Contains(entity) ? GetIterator(entity) : End();
		}

		bool Contains(Entity entity) const
		{
			Entity* const sparseEntry = GetSparseEntryPointer(entity);
			if(sparseEntry == nullptr)
				return false;

			if(sparseEntry->GetVersion() != entity.GetVersion())
				return false;

			return true;
		}

		u64 GetPackedIndex(Entity entity) const
		{
			return GetSparseEntryReference(entity).GetIdentifier();
		}

		// Note: For in-place deletion, this will also return deleted entries
		u64 Size() const { return mPackedEntities.Size(); }
		bool IsEmpty() const { return mPackedEntities.Empty(); }
		SparseSetDeletePolicy GetDeletePolicy() const { return DeletePolicy; }

		Iterator Begin() const { return Iterator(mPackedEntities, 0); }
		ConstIterator Cbegin() const { return Begin(); }

		Iterator End() const { return Iterator(mPackedEntities, Size()); }
		ConstIterator Cend() const { return End(); }

		ReverseIterator Rbegin() const { return std::make_reverse_iterator(End()); }
		ConstReverseIterator Crbegin() const { return Rbegin(); }

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

		reverse_iterator rbegin() const { return Rend(); }
		const_reverse_iterator crbegin() const { return Crbegin(); }

		reverse_iterator rend() const { return Rend(); }
		const_reverse_iterator crend() const { return Crend(); }

	protected:
		virtual Iterator AddInternal(Entity entity, bool forceAddAtEnd)
		{
			Entity& sparseSetEntry = GetOrCreateSparseEntryReference(entity);
			u64 packedEntryIndex = mPackedEntities.Size();

			if constexpr(DeletePolicy == SparseSetDeletePolicy::InPlace)
			{
				B3D_ENSURE(sparseSetEntry == kNullEntity);

				if(mFreeListHead != kMaximumEntryCount && !forceAddAtEnd)
				{
					packedEntryIndex = mFreeListHead;
					sparseSetEntry = Entity(GetPackedIndexAsEntryIdentifier(mFreeListHead), entity.GetVersion());
					mFreeListHead = (u64)(std::exchange(mPackedEntities[packedEntryIndex], entity).GetIdentifier());
				}
				else
				{
					mPackedEntities.Add(entity);
					sparseSetEntry = Entity(GetPackedIndexAsEntryIdentifier(packedEntryIndex), entity.GetVersion());
				}
			}
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

		virtual void EraseInternal(Entity entity)
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

		typedef void(*MoveOrSwapPredicate)(u64, u64);
		template<typename T, void(T::*MoveOrSwap)(u64, u64)>
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

					(((T*)this)->*MoveOrSwap)(validPackedEntryIndex, freePackedEntryIndex);
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

		Entity& GetOrCreateSparseEntryReference(Entity entity)
		{
			const u64 entityIdentifier = entity.GetIdentifier();
			const u64 sparsePage = GetSparsePage(entityIdentifier);

			if(sparsePage >= mSparseIndices.Size())
				mSparseIndices.Resize(sparsePage + 1, nullptr);

			if(mSparseIndices[sparsePage] == nullptr)
			{
				mSparseIndices[sparsePage] = B3DAllocateMultiple<Entity>(SparsePageSize);
				std::uninitialized_fill(mSparseIndices[sparsePage], mSparseIndices[sparsePage] + SparsePageSize, kNullEntity);
			}

			return *(mSparseIndices[sparsePage] + GetSparseIndexWithinPage(entityIdentifier));
		}

		Entity& GetSparseEntryReference(Entity value) const
		{
			const u64 entityIdentifier = value.GetIdentifier();
			return mSparseIndices[GetSparsePage(entityIdentifier)][GetSparseIndexWithinPage(entityIdentifier)];
		}

		Entity* GetSparseEntryPointer(Entity value) const
		{
			const u64 entityIdentifier = value.GetIdentifier();
			const u64 sparsePage = GetSparsePage(entityIdentifier);

			if(sparsePage < mSparseIndices.Size() && mSparseIndices[sparsePage] != nullptr)
				return &mSparseIndices[sparsePage][GetSparseIndexWithinPage(entityIdentifier)];

			return nullptr;
		}

		Iterator GetIterator(Entity entity) const
		{
			return Iterator(mPackedEntities, (i64)GetPackedIndex(entity));
		}

	private:
		void FreeSparsePages()
		{
			for(auto&& page : mSparseIndices)
			{
				if(page != nullptr)
				{
					std::destroy(page, page + SparsePageSize);
					B3DFree(page);
					page = nullptr;
				}
			}
			mSparseIndices.Clear();
		}

		void MoveOrSwapPayload(u64, u64)
		{
			// Do nothing
		}

		static constexpr Entity::IdentifierType GetPackedIndexAsEntryIdentifier(u64 packedIndex)
		{
			return (Entity::IdentifierType)packedIndex;
		}

		static constexpr u64 GetSparseIndexWithinPage(u64 entityIdentifier)
		{
			return entityIdentifier & (SparsePageSize - 1);
		}

		static constexpr u64 GetSparsePage(u64 entityIdentifier)
		{
			return entityIdentifier / SparsePageSize;
		}

		SparseContainerType mSparseIndices; // Entity identifiers are actually indexes into the packed array
		PackedContainerType mPackedEntities; // TODO - Might consider paging this. It won't be continous anymore, making it harder to iterate, but adding entries might prevent expensive resizes if there's a lot of entries

		/**
		 * For in-place delete policy points to first free entry, or kMaximumEntryCount if no free entries.
		 * For swap-only delete policy points to the first free entry, where all other elements are sequentially following the first free element. This value corresponds to valid entry count.
		 * For swap-and-erase delete policy this value is not used.
		 */
		u64 mFreeListHead = DeletePolicy != SparseSetDeletePolicy::SwapOnly ? kMaximumEntryCount : 0;
	};

 	template<typename ContainerType, u32 PageSize>
	struct TPagedContainerIterator final
	{
		using value_type = typename ContainerType::value_type;
		using pointer = typename ContainerType::pointer;
		using reference = typename ContainerType::reference;
		using difference_type = typename ContainerType::difference_type;
		using iterator_category = std::random_access_iterator_tag;

		TPagedContainerIterator() = default;
		TPagedContainerIterator(ContainerType& container, u64 index)
			: mContainer(&container), mIndex(index) { }

		TPagedContainerIterator& operator++()
		{
			++mIndex;
			return *this;
		}

		TPagedContainerIterator& operator--()
		{
			B3D_ENSURE(mIndex > 0);

			--mIndex;
			return *this;
		}

		TPagedContainerIterator& operator+=(u64 value)
		{
			mIndex += value;
			return *this;
		}

		TPagedContainerIterator& operator-=(u64 value)
		{
			B3D_ENSURE(mIndex >= value);

			mIndex -= value;
			return *this;
		}

		TPagedContainerIterator operator+(u64 value) const
		{
			TPagedContainerIterator copy = *this;
			return (copy += value);
		}

		TPagedContainerIterator operator-(u64 value) const
		{
			TPagedContainerIterator copy = *this;
			return (copy -= value);
		}

		reference operator[](u64 value) const
		{
			const u64 index = mIndex + value;
			return (*mContainer)[index / PageSize][index & (PageSize - 1)];
		}
		pointer operator->() const { return std::addressof(operator[](0)); }
		reference operator*() const { return operator[](0); }

		u64 Index() const { return mIndex; }

	private:
		ContainerType* mContainer = nullptr;
		u64 mIndex = 0;
	};

	template<typename ContainerType, u32 PageSize>
	i64 operator-(const TPagedContainerIterator<ContainerType, PageSize>& lhs, const TPagedContainerIterator<ContainerType, PageSize>& rhs)
	{
		return rhs.Index() - lhs.Index();
	}

	template<typename ContainerType, u32 PageSize>
	bool operator==(const TPagedContainerIterator<ContainerType, PageSize>& lhs, const TPagedContainerIterator<ContainerType, PageSize>& rhs)
	{
		return lhs.Index() == rhs.Index();
	}

	template<typename ContainerType, u32 PageSize>
	bool operator!=(const TPagedContainerIterator<ContainerType, PageSize>& lhs, const TPagedContainerIterator<ContainerType, PageSize>& rhs)
	{
		return !(lhs == rhs);
	}

	template<typename ContainerType, u32 PageSize>
	bool operator<(const TPagedContainerIterator<ContainerType, PageSize>& lhs, const TPagedContainerIterator<ContainerType, PageSize>& rhs)
	{
		return lhs.index() > rhs.index();
	}

	template<typename ContainerType, u32 PageSize>
	bool operator>(const TPagedContainerIterator<ContainerType, PageSize>& lhs, const TPagedContainerIterator<ContainerType, PageSize>& rhs)
	{
		return rhs < lhs;
	}

	template<typename ContainerType, u32 PageSize>
	bool operator<=(const TPagedContainerIterator<ContainerType, PageSize>& lhs, const TPagedContainerIterator<ContainerType, PageSize>& rhs)
	{
		return !(lhs > rhs);
	}

	template<typename ContainerType, u32 PageSize>
	bool operator>=(const TPagedContainerIterator<ContainerType, PageSize>& lhs, const TPagedContainerIterator<ContainerType, PageSize>& rhs)
	{
		return !(lhs < rhs);
	}

	template<typename ComponentType, bool InPlaceDelete = false, u64 PackedPageSize = 1024, u64 SparsePageSize = 4096>
	class TComponentSparseSet : public TSparseSet<InPlaceDelete ? SparseSetDeletePolicy::InPlace : SparseSetDeletePolicy::SwapAndErase, SparsePageSize>
	{
		using ComponentContainerType = TArray<ComponentType*>;

	public:
		using Super = TSparseSet<InPlaceDelete ? SparseSetDeletePolicy::InPlace : SparseSetDeletePolicy::SwapAndErase, SparsePageSize>;
		using Iterator = TPagedContainerIterator<ComponentContainerType, PackedPageSize>;
		using ConstIterator = TPagedContainerIterator<const ComponentContainerType, PackedPageSize>;
		using ReverseIterator = std::reverse_iterator<Iterator>;
		using ConstReverseIterator = std::reverse_iterator<ConstIterator>;

		~TComponentSparseSet() override
		{
			ShrinkComponentArray(0);
		}

		template<typename... Arguments>
		ComponentType& Add(Entity entity, Arguments&&... arguments)
		{
			auto iterator = AddInternal(entity, false, std::forward<Arguments>(arguments)...);
			return GetComponentReference(iterator.Index());
		}

		ComponentType& Get(Entity entity)
		{
			const u64 packedEntryIndex = Super::GetPackedIndex(entity);
			return GetComponentReference(packedEntryIndex);
		}

		const ComponentType& Get(Entity entity) const
		{
			const u64 packedEntryIndex = Super::GetPackedIndex(entity);
			return GetComponentReference(packedEntryIndex);
		}

		void Clear() override
		{
			if constexpr(InPlaceDelete)
			{
				for(auto it = Super::Begin(); it != Super::End(); ++it)
				{
					if(*it != kInvalidEntity)
					{
						ComponentType& component = GetComponentReference(GetPackedIndex(*it));
						component.~ComponentType();
					}
				}
			}
			else
			{
				for(auto it = Super::Begin(); it != Super::End(); ++it)
				{
					ComponentType& component = GetComponentReference(GetPackedIndex(*it));
					component.~ComponentType();
				}
			}

			Super::Clear();
		}

		void ClearInvalid() override
		{
			Super::template ClearInvalidInternal<TComponentSparseSet, &TComponentSparseSet::MoveOrSwapPayload>();
		}

		void Reserve(u64 capacity) override
		{
			if(capacity > 0)
			{
				Super::Reserve(capacity);
				GetOrCreateComponentPageFor(capacity - 1u);
			}
		}

		u64 Capacity() const override
		{
			return mComponents.Size() * PackedPageSize;
		}

		void Shrink() override
		{
			Super::Shrink();
			ShrinkComponentArray(Super::Size());
		}

		// TODO - Sort functions should use same pattern as ClearInvalid

		ConstIterator Cbegin() const { return ConstIterator(mComponents, 0); }
		ConstIterator Begin() const { return Cbegin(); }
		Iterator Begin() { return Iterator(mComponents, 0); }

		ConstIterator Cend() const { return ConstIterator(mComponents, Super::Size()); }
		ConstIterator End() const { return Cend(); }
		Iterator End() { return Iterator(mComponents, Super::Size()); }

		ConstReverseIterator Crbegin() const { return std::make_reverse_iterator(Cend()); }
		ConstReverseIterator Rbegin() const { return Crbegin(); }
		ReverseIterator Rbegin() { return std::make_reverse_iterator(End()); }

		ConstReverseIterator Crend() const { return std::make_reverse_iterator(Cbegin()); }
		ConstReverseIterator Rend() const { return Crend(); }
		ReverseIterator Rend() { return std::make_reverse_iterator(Begin()); }

		// For std compatibility
		using iterator = Iterator;
		using const_iterator = ConstIterator;
		using reverse_iterator = ReverseIterator;
		using const_reverse_iterator = ConstReverseIterator;

		const_iterator cbegin() const { return Cbegin(); }
		const_iterator begin() const { return Begin(); }
		iterator begin() { return Begin(); }

		const_iterator cend() const { return Cend(); }
		const_iterator end() const { return End(); }
		iterator end() { return End(); }

		const_reverse_iterator crbegin() const { return Crbegin(); }
		const_reverse_iterator rbegin() const { return Rbegin(); }
		reverse_iterator rbegin() { return Rend(); }

		const_reverse_iterator crend() const { return Crend(); }
		const_reverse_iterator rend() const { return Rend(); }
		reverse_iterator rend() { return Rend(); }

	private:
		using UnderlyingIterator = typename Super::Iterator;

		template<typename... Arguments>
		UnderlyingIterator AddInternal(Entity entity, bool forceAddAtEnd, Arguments&&... arguments)
		{
			auto iterator = Super::AddInternal(entity, forceAddAtEnd);
			ComponentType* component = GetOrCreateComponentPointer(iterator.Index());
			new(component) ComponentType(std::forward<Arguments>(arguments)...);

			return iterator;
		}

		void EraseInternal(Entity entity) override
		{
			const u64 packedIndex = Super::GetPackedIndex(entity);
			ComponentType& component = GetComponentReference(packedIndex);

			if constexpr(InPlaceDelete)
			{
				Super::EraseInternal(entity);
				component.~ComponentType();
			}
			else // SwapAndErase
			{
				ComponentType& otherComponent = GetComponentReference(Super::Size() - 1);
				component = std::move(otherComponent);
				otherComponent.~ComponentType();

				Super::EraseInternal(entity);
			}
		}

		void MoveOrSwapPayload(u64 fromPackedIndex, u64 toPackedIndex)
		{
			if constexpr(InPlaceDelete)
			{
				if(Super::operator[](toPackedIndex) == kInvalidEntity)
				{
					ComponentType& componentToMove = GetComponentReference(fromPackedIndex);

					ComponentType* destinationComponent = GetOrCreateComponentPointer(toPackedIndex);
					new(destinationComponent) ComponentType(std::move(componentToMove));

					componentToMove.~ComponentType();
				}
				else
					std::swap(GetComponentReference(fromPackedIndex), GetComponentReference(toPackedIndex));
			}
			else
				std::swap(GetComponentReference(fromPackedIndex), GetComponentReference(toPackedIndex));
		}

		void ShrinkComponentArray(u64 newComponentCount)
		{
			const u64 oldComponentCount = Super::Size();
			for(u64 packedComponentIndex = newComponentCount; packedComponentIndex < oldComponentCount; ++packedComponentIndex)
			{
				if constexpr(InPlaceDelete)
				{
					if(Super::mPackedEntities[packedComponentIndex] != kInvalidEntity)
						GetComponentReference(packedComponentIndex).~ComponentType();
				}
				else
					GetComponentReference(packedComponentIndex).~ComponentType();
			}

			const auto newPageCount = (newComponentCount + PackedPageSize - 1u) / PackedPageSize;
			const u64 oldPageCount = mComponents.Size();
			for(u64 page = newPageCount; page < oldPageCount; ++page)
				B3DFree(mComponents[page]);

			mComponents.Resize(newPageCount);
			mComponents.Shrink();
		}

		ComponentType* GetOrCreateComponentPageFor(u64 packedComponentIndex)
		{
			const u64 page = GetComponentPage(packedComponentIndex);

			if(page >= mComponents.Size())
				mComponents.Resize(page + 1, nullptr);

			if(mComponents[page] == nullptr)
			{
				mComponents[page] = B3DAllocateMultiple<ComponentType>(PackedPageSize);
				std::uninitialized_fill(mComponents[page], mComponents[page] + PackedPageSize, nullptr);
			}

			return mComponents[page];
		}

		ComponentType* GetOrCreateComponentPointer(u64 packedComponentIndex)
		{
			return GetOrCreateComponentPageFor(packedComponentIndex) + GetComponentIndexWithinPage(packedComponentIndex);
		}

		ComponentType& GetComponentReference(u64 packedComponentIndex)
		{
			return mComponents[GetComponentPage(packedComponentIndex)][GetComponentIndexWithinPage(packedComponentIndex)];
		}

		const ComponentType& GetComponentReference(u64 packedComponentIndex) const
		{
			return mComponents[GetComponentPage(packedComponentIndex)][GetComponentIndexWithinPage(packedComponentIndex)];
		}

		static constexpr u64 GetComponentIndexWithinPage(u64 packedComponentIndex)
		{
			return packedComponentIndex & (PackedPageSize - 1);
		}

		static constexpr u64 GetComponentPage(u64 packedComponentIndex)
		{
			return packedComponentIndex / PackedPageSize;
		}

		ComponentContainerType mComponents;
	};

	template<typename TagType, bool InPlaceDelete = false, u64 SparsePageSize = 4096>
	class TTagSparseSet : public TSparseSet<InPlaceDelete ? SparseSetDeletePolicy::InPlace : SparseSetDeletePolicy::SwapAndErase, SparsePageSize>
	{
	public:
		using Super = TSparseSet<InPlaceDelete ? SparseSetDeletePolicy::InPlace : SparseSetDeletePolicy::SwapAndErase, SparsePageSize>;

		~TTagSparseSet() override = default;

		void Add(Entity entity)
		{
			Super::AddInternal(entity, false);
		}
	};

	template<u64 SparsePageSize = 4096>
	class TEntitySparseSet : public TSparseSet<SparseSetDeletePolicy::SwapOnly, SparsePageSize>
	{
	public:
		using Super = TSparseSet<SparseSetDeletePolicy::SwapOnly, SparsePageSize>;

		~TEntitySparseSet() override = default;

		Entity Generate()
		{
			if(Super::Size() == Super::mFreeListHead)
				return CreateEntity();

			Entity entity = Super::mPackedEntities[Super::mFreeListHead];
			return *AddInternal(entity);
		}

		Entity Generate(Entity hint)
		{
			if(hint != kInvalidEntity && hint != kNullEntity)
			{
				Entity entity(hint.GetIdentifier(), Super::GetVersion(hint));
				if(entity == kInvalidEntity || Super::GetPackedIndex(entity) >= Super::mFreeListHead)
					return *Super::AddInternal(entity, false);
			}

			return Generate();
		}

		void Clear() override
		{
			Super::Clear();
			mNextEntityId = 0;
		}

	private:
		using UnderlyingIterator = typename Super::Iterator;

		UnderlyingIterator AddInternal(Entity hint, bool forceAddAtEnd) override
		{
			return Super::Find(Generate(hint));
		}

		Entity CreateEntity()
		{
			auto fnGetEntityChecked = [this]()
			{
				Entity entity(mNextEntityId, 0);
				if(B3D_ENSURE(entity != kNullEntity))
					mNextEntityId++;

				return entity;
			};

			Entity entity = fnGetEntityChecked();
			while(Super::GetVersion(entity) != ((Entity)kInvalidEntity).GetVersion() && entity != kNullEntity)
				entity = fnGetEntityChecked();

			return entity;
		}

		u64 mNextEntityId = 0;
	};

	/** @} */
} // namespace bs
